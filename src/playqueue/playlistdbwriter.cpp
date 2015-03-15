/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2015 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#include "playlistdbwriter.h"
#include "playqueue_model.h"

#include "core/database/database.h"
#include "core/mediaitem/mediaitem.h"
#include "settings.h"
#include "debug.h"

//Qt
#include <QStringList>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>

#include <QDateTime>
#include <QFileInfo>
#include <QCryptographicHash>

/*
********************************************************************************
*                                                                              *
*    Class PlaylistDbWriter                                                    *
*                                                                              *
********************************************************************************
*/
PlaylistDbWriter::PlaylistDbWriter()
{
    _isRunning     = false;
    _playlist_name = QString();
    _database_id   = -1;
}


/* ---------------------------------------------------------------------------*/
/* PlaylistDbWriter::saveToDatabase                                           */
/* ---------------------------------------------------------------------------*/
void PlaylistDbWriter::saveToDatabase(const QString& playlist_name, int bd_id /* = -1*/)
{
    Debug::debug() << "PlaylistDbWriter::saveToDatabase name: " << playlist_name;
    Debug::debug() << "PlaylistDbWriter::saveToDatabase id: " << bd_id;
  
    _isSessionSaving = false;
    _playlist_name   = playlist_name;
    _database_id     = bd_id;
}

/* ---------------------------------------------------------------------------*/
/* PlaylistDbWriter::saveSessionToDatabase                                    */
/* ---------------------------------------------------------------------------*/
void PlaylistDbWriter::saveSessionToDatabase()
{
    _isSessionSaving = true;
}

/* ---------------------------------------------------------------------------*/
/* PlaylistDbWriter::run                                                      */
/* ---------------------------------------------------------------------------*/
void PlaylistDbWriter::run()
{
    _isRunning     = true;
    
    if(_isSessionSaving)
      saveSession();
    else
      savePlaylist();
    
    _isRunning       = false;

     emit playlistSaved();
}

/* ---------------------------------------------------------------------------*/
/* PlaylistDbWriter::savePlaylist                                             */
/* ---------------------------------------------------------------------------*/
void PlaylistDbWriter::savePlaylist()
{
    Debug::debug() << "PlaylistDbWriter::savePlaylist";
    
    //! playlist data for Database
    QDateTime date   = QDateTime::currentDateTime();
    uint mtime       = date.toTime_t();
    QString pname    = "playlist-" + date.toString("dd-MM-yyyy-hh:mm");
    QString fname    = QString(QCryptographicHash::hash(pname.toUtf8().constData(), QCryptographicHash::Sha1).toHex());

    pname = _playlist_name;
    if(pname.isEmpty())
      pname = tr("no name");

    Debug::debug() << "    [PlaylistDbWriter] insert playlist: " << pname;

    Debug::debug() << "    [PlaylistDbWriter] _database_id " << _database_id;


    if (!Database::instance()->open())
      return;
    
    QSqlQuery q("", *Database::instance()->db());
    q.prepare("SELECT `id` FROM `playlists` WHERE `type`=? AND `id`=?;");
    q.addBindValue( (int)T_DATABASE );
    q.addBindValue( _database_id );
    q.exec();

    if ( q.next() ) 
    {
      q.prepare("DELETE FROM `playlist_items` WHERE `playlist_id`=?;");
      q.addBindValue( _database_id );
      q.exec();
      
      q.prepare("UPDATE `playlists` SET `filename`=?,`name`=?,`type`=?,`favorite`=?,`dir_id`=?,`mtime`=? WHERE `id`=?");      
      q.addBindValue( fname );
      q.addBindValue( pname );
      q.addBindValue( (int) T_DATABASE );
      q.addBindValue(  0 );
      q.addBindValue( -1 );
      q.addBindValue( mtime );
      q.addBindValue( _database_id );
      Debug::debug() << "query exec " << q.exec();  
    }
    else
    {
      q.prepare("INSERT INTO `playlists` (`filename`,`name`,`type`,`favorite`,`dir_id`,`mtime`)" \
                 " VALUES(?,?,?,?,?,?);");

      q.addBindValue( fname );
      q.addBindValue( pname );
      q.addBindValue( (int) T_DATABASE );
      q.addBindValue(  0 );
      q.addBindValue( -1 );
      q.addBindValue( mtime );
      Debug::debug() << "query exec " << q.exec();
      
      if(q.numRowsAffected() < 1)
        Debug::warning() << "    [PlaylistDbWriter] playlist database insertion failure";

      q.prepare("SELECT `id` FROM `playlists` WHERE `filename`=?;");
      q.addBindValue( fname );
      Debug::debug() << "query exec " << q.exec();    
      q.next();
    }
    
     _database_id = q.value(0).toInt();

    Debug::debug() << "    [PlaylistDbWriter] _database_id " << _database_id;

    /*-----------------------------------------------------------*/
    /* PLAYLIST ITEMS part in database                           */
    /* ----------------------------------------------------------*/
    for (int i = 0; i < m_model->rowCount(QModelIndex()); i++) 
    {
        const QString item_url   = m_model->trackAt(i)->url;
        const QString item_name  = MEDIA::isLocal(m_model->trackAt(i)->url) ? m_model->trackAt(i)->title : m_model->trackAt(i)->name;
       
        /* 
           Debug::debug() << "    [PlaylistDbWriter] insert playlist item url: " << item_url;
           Debug::debug() << "    [PlaylistDbWriter] insert playlist item name: " << m_model->trackAt(i)->name;
           Debug::debug() << "    [PlaylistDbWriter] insert playlist item title: " << m_model->trackAt(i)->title;
        */

        q.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`) VALUES(?,?,?);");
        q.addBindValue( item_url );
        q.addBindValue( item_name );
        q.addBindValue( _database_id );
        q.exec();
    }
}

/* ---------------------------------------------------------------------------*/
/* PlaylistDbWriter::saveSession                                              */
/* ---------------------------------------------------------------------------*/
void PlaylistDbWriter::saveSession()
{
    Debug::debug() << "PlaylistDbWriter::saveSession";

    //! playlist data for Database
    uint mtime       = QDateTime::currentDateTime().toTime_t();
    QString pname    = "playqueue_session";
    QString fname    = QString(QCryptographicHash::hash(pname.toUtf8().constData(), QCryptographicHash::Sha1).toHex());

    if (!Database::instance()->open())
      return;

    /*-----------------------------------------------------------*/
    /* clean up database                                         */
    /* ----------------------------------------------------------*/      
    QSqlQuery query(*Database::instance()->db());

    query.prepare("SELECT `id` FROM `playlists` WHERE `type`=? LIMIT 1;");
    query.addBindValue((int)T_PLAYQUEUE);
    query.exec();
    
    if ( query.next() ) 
    {
      _database_id = query.value(0).toString().toInt();
    
      Debug::debug() << "PlaylistDbWriter::saveSession db id " << _database_id;
    
      query.prepare("DELETE FROM `playlist_items` WHERE `playlist_id`=?;");
      query.addBindValue( _database_id );
      Debug::debug() << "query exec " << query.exec();    
      
      query.prepare("UPDATE `playlists` SET `mtime`=? WHERE `id`=?");      
      query.addBindValue( mtime );
      query.addBindValue( _database_id );
      Debug::debug() << "query exec " << query.exec();        
    }
    else
    {
      query.prepare("INSERT INTO `playlists`(`filename`,`name`,`type`,`favorite`,`dir_id`,`mtime`)" \
                    " VALUES(?,?,?,?,?,?);");

      query.addBindValue( fname );
      query.addBindValue( pname );
      query.addBindValue( (int) T_PLAYQUEUE );
      query.addBindValue(  0 );
      query.addBindValue( -1 );
      query.addBindValue( mtime );
      Debug::debug() << "query exec " << query.exec();        
    }

    /*-----------------------------------------------------------*/
    /* PLAYLIST part in database                                 */
    /* ----------------------------------------------------------*/    
    query.prepare("SELECT `id` FROM `playlists` WHERE `type`=? LIMIT 1;");
    query.addBindValue((int)T_PLAYQUEUE);
    query.exec();
    
    if ( query.next() )  
    {
      _database_id = query.value(0).toString().toInt();
    }
    else                        
    {
      Debug::warning() << " no playlist in database to operate";
      return;
    }
    
    /*-----------------------------------------------------------*/
    /* PLAYLIST ITEMS part in database                           */
    /* ----------------------------------------------------------*/
    for (int i = 0; i < m_model->rowCount(QModelIndex()); i++) 
    {
        const QString item_url   = m_model->trackAt(i)->url;
        const QString item_name  = MEDIA::isLocal(m_model->trackAt(i)->url) ? m_model->trackAt(i)->title : m_model->trackAt(i)->name;
       
        QSqlQuery itemQuery(*Database::instance()->db());
         
//         Debug::debug() << "    [PlaylistDbWriter] insert playlist item url: " << item_url;
//         Debug::debug() << "    [PlaylistDbWriter] insert playlist item name: " << m_model->trackAt(i)->name;
//         Debug::debug() << "    [PlaylistDbWriter] insert playlist item title: " << m_model->trackAt(i)->title;
        

        query.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`) VALUES(?,?,?);");
        query.addBindValue( item_url );
        query.addBindValue( item_name );
        query.addBindValue( _database_id );
        query.exec();
    }
}

