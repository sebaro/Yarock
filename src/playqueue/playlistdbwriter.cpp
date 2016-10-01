/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
void PlaylistDbWriter::saveToDatabase(const QString& playlist_name)
{
    Debug::debug() << "  [PlaylistDbWriter] saveToDatabase name:" << playlist_name << "id:" << -1;
    m_playlist       = 0;
    _isSessionSaving = false;
    _playlist_name   = playlist_name;
    _database_id     = -1;
}

/* ---------------------------------------------------------------------------*/
/* PlaylistDbWriter::saveToDatabase                                           */
/* ---------------------------------------------------------------------------*/
void PlaylistDbWriter::saveToDatabase(MEDIA::PlaylistPtr playlist)
{
    Debug::debug() << "  [PlaylistDbWriter] saveToDatabase name:" << playlist->name;
    
    m_playlist       = playlist;
    _isSessionSaving = false;
    _playlist_name   = m_playlist->name;
    _database_id     = m_playlist->id;
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
    Debug::debug() << "  [PlaylistDbWriter] savePlaylist";
    
    //! playlist data for Database
    uint mtime       = QDateTime::currentDateTime().toTime_t();
    QString fname    = QString(QCryptographicHash::hash(QString::number(mtime).toUtf8().constData(), QCryptographicHash::Sha1).toHex());


    Debug::debug() << "  [PlaylistDbWriter] saving playlist:" << _playlist_name << "id:" << _database_id;

    if (!Database::instance()->open())
      return;
    
    QSqlQuery q("", *Database::instance()->db());
    q.prepare("SELECT `id` FROM `playlists` WHERE `type`=? AND `id`=?;");
    q.addBindValue( (int)T_DATABASE );
    q.addBindValue( _database_id );
    q.exec();
    
    if ( q.next() ) 
    {
      _database_id = q.value(0).toInt();

      Debug::debug() << "  [PlaylistDbWriter] existing playlist -> update contents name" << m_playlist->name;
      Debug::debug() << "  [PlaylistDbWriter] existing playlist -> update contents id" << m_playlist->id;
        
      q.prepare("DELETE FROM `playlist_items` WHERE `playlist_id`=?;");
      q.addBindValue( _database_id );
      Debug::debug() << "query exec " << q.exec();  
      
      q.prepare("UPDATE `playlists` SET `filename`=?,`name`=?,`type`=?,`favorite`=?,`dir_id`=?,`mtime`=? WHERE `id`=?");      
      q.addBindValue( fname );
      q.addBindValue( _playlist_name );
      q.addBindValue( (int) T_DATABASE );
      q.addBindValue(  0 );
      q.addBindValue( -1 );
      q.addBindValue( mtime );
      q.addBindValue( _database_id );
      Debug::debug() << "query exec " << q.exec();
    }
    else
    {
      Debug::debug() << "  [PlaylistDbWriter] new playlist -> create";
        
      q.prepare("INSERT INTO `playlists` (`filename`,`name`,`type`,`favorite`,`dir_id`,`mtime`)" \
                 " VALUES(?,?,?,?,?,?);");

      q.addBindValue( fname );
      q.addBindValue( _playlist_name );
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
      _database_id = q.value(0).toInt();
    }
    
    Debug::debug() << "  [PlaylistDbWriter] new database id:" << _database_id;    

    /*-----------------------------------------------------------*/
    /* PLAYLIST ITEMS part in database                           */
    /* ----------------------------------------------------------*/
    // take playlist item because we save a playlist item outside playqueue widget/editor
    if( m_playlist && m_model == Playqueue::instance() )
    {
        for (int i = 0; i < m_playlist->childCount(); i++) 
        {
            MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast(m_playlist->child(i));
                                
            const QString item_url   = track->url;
            const QString item_name  = MEDIA::isLocal(track->url) ? 
            track->title : track->extra["station"].toString();
        
            Debug::debug() << "  [PlaylistDbWriter] insert playlist item url: " << item_url;
            Debug::debug() << "  [PlaylistDbWriter] insert playlist item title: " << item_name;

            q.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`) VALUES(?,?,?);");
            q.addBindValue( item_url );
            q.addBindValue( item_name );
            q.addBindValue( m_playlist->id );
            q.exec();
        }
    }
    // take model from playqueue/playlist editor
    else
    {
        for (int i = 0; i < m_model->rowCount(QModelIndex()); i++) 
        {
            const QString item_url   = m_model->trackAt(i)->url;
            const QString item_name  = MEDIA::isLocal(m_model->trackAt(i)->url) ? 
            m_model->trackAt(i)->title : m_model->trackAt(i)->extra["station"].toString();
        
            Debug::debug() << "  [PlaylistDbWriter] insert playlist item url: " << item_url;
            Debug::debug() << "  [PlaylistDbWriter] insert playlist item title: " << m_model->trackAt(i)->title;
            Debug::debug() << "  [PlaylistDbWriter] insert playlist item station: " << m_model->trackAt(i)->extra["station"].toString();

            q.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`) VALUES(?,?,?);");
            q.addBindValue( item_url );
            q.addBindValue( item_name );
            q.addBindValue( _database_id );
            q.exec();
        }
    }
    
    q.finish();
    if( m_playlist )
    {
        m_playlist->id   = _database_id;
    }
}



/* ---------------------------------------------------------------------------*/
/* PlaylistDbWriter::saveSession                                              */
/* ---------------------------------------------------------------------------*/
void PlaylistDbWriter::saveSession()
{
    Debug::debug() << "  [PlaylistDbWriter] saveSession";

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
    
      Debug::debug() << "  [PlaylistDbWriter] saveSession db id " << _database_id;
    
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
      Debug::warning() << "  [PlaylistDbWriter] no playlist in database to operate";
      return;
    }
    
    /*-----------------------------------------------------------*/
    /* PLAYLIST ITEMS part in database                           */
    /* ----------------------------------------------------------*/
    for (int i = 0; i < m_model->rowCount(QModelIndex()); i++) 
    {
        const QString item_url   = m_model->trackAt(i)->url;
        const QString item_name  = MEDIA::isLocal(m_model->trackAt(i)->url) ?
          m_model->trackAt(i)->title : m_model->trackAt(i)->extra["station"].toString();

        QSqlQuery itemQuery(*Database::instance()->db());

        //Debug::debug() << "  [PlaylistDbWriter] insert playlist item url: " << item_url;
        //Debug::debug() << "  [PlaylistDbWriter] insert playlist item name: " << m_model->trackAt(i)->name;
        //Debug::debug() << "  [PlaylistDbWriter] insert playlist item title: " << m_model->trackAt(i)->title;

        query.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`) VALUES(?,?,?);");
        query.addBindValue( item_url );
        query.addBindValue( item_name );
        query.addBindValue( _database_id );
        query.exec();
    }
}

