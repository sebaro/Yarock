/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
*                                                                                       *
*  This program is free software; you can redistribute it and/or modify it under        *
*  the terms of the GNU General Public License as published by the Free Software        *
*  Foundation; either version 2 of the License, or (at your option) any later           *
*  version.                                                                             *
*                                                                                       *
*  This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
*  PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                       *
*  You should have received a copy of the GNU General Public License along with         *
*  this program.  If not, see <http://www.gnu.org/licenses/>.                           *
*****************************************************************************************/

#include "playlistwriter.h"
#include "playqueue_model.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/playlist_parser.h"
#include "core/database/database.h"

#include "debug.h"

//Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>

#include <QFileInfo>
#include <QStringList>
#include <QDateTime>

/*
********************************************************************************
*                                                                              *
*    Class PlaylistWriter                                                      *
*                                                                              *
********************************************************************************
*/
PlaylistWriter::PlaylistWriter()
{
    m_fileToSave    = QString();
    m_isRunning     = false;
}

//! ----------------------- PlaylistWriter::run --------------------------------
void PlaylistWriter::run()
{
    if (m_fileToSave.isEmpty()) return;
    m_isRunning     = true;
    
    if(MEDIA::isPlaylistFile(m_fileToSave)) 
    {
        //! save to file
        MEDIA::PlaylistToFile( m_fileToSave, m_model->tracks() );

        //! check if we need to update database
        QString path = QFileInfo(m_fileToSave).absolutePath();
        QStringList listDir = QStringList() << Database::instance()->param()._paths;

        bool dbChange = false;
        foreach(const QString& s, listDir) {
          if(s.contains(path)) {
            dbChange = true;
            break;
          }
        }

        //! playlist is in collection directory --> add or update into database
        if(dbChange) {
          updateDatabase(m_model->tracks());
          emit playlistSaved();
        }
    } // fin Media is playlist

    m_isRunning     = false;
}


//! ----------------------- PlaylistWriter::saveToFile -------------------------
void PlaylistWriter::saveToFile(const QString& filename)
{
    m_fileToSave = filename;
}


//! ----------------------- PlaylistWriter::updateDatabase ---------------------
void PlaylistWriter::updateDatabase(QList<MEDIA::TrackPtr> list)
{
    QFileInfo fileInfo(m_fileToSave);
    QString fname    = fileInfo.filePath().toUtf8();
    QString pname    = fileInfo.baseName();
    QString dir_path = fileInfo.canonicalPath();
    uint mtime       = fileInfo.lastModified().toTime_t();

    if (!Database::instance()->open())
      return;

    /*-----------------------------------------------------------*/
    /* clean up database                                         */
    /* ----------------------------------------------------------*/    
    QSqlQuery query(*Database::instance()->db());
    query.prepare("DELETE FROM `playlists` WHERE `filename`=?;");
    query.addBindValue(fname);
    query.exec();
  
    query.prepare("DELETE FROM `playlist_items` WHERE `playlist_id` NOT IN (SELECT `id` FROM `playlists`);");
    query.exec();

    /*-----------------------------------------------------------*/
    /* directory part in DATABASE                                */
    /* ----------------------------------------------------------*/     
    query.prepare("SELECT `id` FROM `directories` WHERE `path`=:val;");
    query.bindValue(":val", dir_path );
    query.exec();

    if ( !query.next() ) {
      uint mtime   = QFileInfo(dir_path).lastModified().toTime_t();
      
      query.prepare("INSERT INTO `directories`(`path`,`mtime`) VALUES (?,?);");
      query.addBindValue(dir_path);
      query.addBindValue(mtime);
      query.exec();

      query.prepare("SELECT `id` FROM `directories` WHERE `path`=:val;");
      query.bindValue(":val", dir_path);
      query.exec();
      query.next();
    }

    int dir_id = query.value(0).toString().toInt();

    /*-----------------------------------------------------------*/
    /* PLAYLIST part in database                                 */
    /* ----------------------------------------------------------*/ 
    Debug::debug() << "  [PlaylistWriter] insert playlist into db" << m_fileToSave;

    query.prepare("INSERT INTO `playlists`(`filename`,`name`,`type`,`favorite`,`dir_id`,`mtime`)" \
                  "VALUES(?,?,?,?,?,?);");

    query.addBindValue(fname);
    query.addBindValue(pname);
    query.addBindValue((int) T_FILE);
    query.addBindValue(  0 );
    query.addBindValue( dir_id );
    query.addBindValue(mtime);
    query.exec();

    /*-----------------------------------------------------------*/
    /* PLAYLIST ITEMS part in database                           */
    /* ----------------------------------------------------------*/
    foreach(MEDIA::TrackPtr track, list)
    {
      QString url       = track->url;
      QString name      = QFileInfo(url).baseName();

      //! Playlist Item part in database
      QSqlQuery itemQuery(*Database::instance()->db());
      //Debug::debug() << "  [PlaylistWriter] insert playlistitem url:" << url;

      itemQuery.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`)" \
                        "VALUES(?,?,(SELECT `id` FROM `playlists` WHERE `filename`=?));");
      itemQuery.addBindValue(url);
      itemQuery.addBindValue(name);
      itemQuery.addBindValue(fname);
      itemQuery.exec();
    }
}
