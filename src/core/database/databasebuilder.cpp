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

// local
#include "core/database/databasebuilder.h"
#include "core/database/database_cmd.h"
#include "core/database/database.h"

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/playlist_parser.h"

#include "utilities.h"
#include "debug.h"


// Qt
#include <QtCore>
#include <QImage>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
/*
********************************************************************************
*                                                                              *
*    Class DataBaseBuilder                                                     *
*                                                                              *
********************************************************************************
*/
DataBaseBuilder::DataBaseBuilder()
{
    m_exit            = false;
    m_do_rebuild      = false;
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::filesFromFilesystem                                       */
/* ---------------------------------------------------------------------------*/
QStringList DataBaseBuilder::filesFromFilesystem(const QString& directory)
{
    Debug::debug() << "  [DataBaseBuilder] filesFromFilesystem";
  
    QStringList files;
    const QStringList filters = QStringList()
    /* Audio */    << "*.mp3"  << "*.ogg" << "*.wav" << "*.flac" << "*.m4a" << "*.aac"
    /* Playlist */ << "*.m3u" << "*.m3u8" << "*.pls" << "*.xspf";

    QDir dir(directory);
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QDirIterator it(dir, QDirIterator::NoIteratorFlags);
    while(it.hasNext())
    {
      it.next();
      files << it.fileInfo().absoluteFilePath().toUtf8();
    }

    return files;
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::filesFromDatabase                                         */
/* ---------------------------------------------------------------------------*/
QHash<QString,uint> DataBaseBuilder::filesFromDatabase(const QString& directory)
{  
    Debug::debug() << "  [DataBaseBuilder] filesFromDatabase";
  
    QHash<QString,uint> files;
  
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `directories` WHERE `path`=?;");
    q.addBindValue( directory );
    q.exec();

    if ( q.next() ) {
      int dir_id = q.value(0).toString().toInt();

      q.prepare("SELECT `id`,`filename`,`mtime` FROM `tracks` WHERE `dir_id`=?;");
      q.addBindValue( dir_id );
      Debug::debug() << q.exec();
      while (q.next())
        files.insert(q.value(1).toString(),q.value(2).toUInt());
      
      q.prepare("SELECT `id`,`filename`,`mtime` FROM `playlists` WHERE `dir_id`=?;");
      q.addBindValue( dir_id );
      Debug::debug() << q.exec();
      while (q.next())
        files.insert(q.value(1).toString(),q.value(2).toUInt());      
    }
    
    //Debug::debug() << "  [DataBaseBuilder] filesFromDatabase files:" << files;
    return files;
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::updateFolder                                              */
/*      -> User entry point : add folder to parse                             */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::updateFolder(QStringList folder, bool doRebuild/*=false*/)
{
    m_do_rebuild = doRebuild;
    
    m_db_dirs.clear();
    m_fs_dirs.clear();

    m_input_folders.clear();
    m_input_folders.append(folder);
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::run                                                       */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::run()
{

    Debug::debug() << "  [DataBaseBuilder] run()";
    InfoSystem::instance()->activateCache( false );
    
    connect( InfoSystem::instance(),
                SIGNAL( info( INFO::InfoRequestData, QVariant ) ),
                SLOT( slot_systeminfo_received( INFO::InfoRequestData, QVariant ) ), Qt::QueuedConnection );    

    this->doScan();

    /* Start thread event loop           */
    /* This makes signals and slots work */
    exec();

    Debug::debug() << "  [DataBaseBuilder] run() exited";
    InfoSystem::instance()->activateCache( true );
    
    disconnect( InfoSystem::instance(),0, this, 0);
}


/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::doScan                                                    */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::doScan()
{
    int idxCount   = 0;
    int fileCount  = 0;
    m_requests_ids.clear();

    if ( m_input_folders.isEmpty() || !Database::instance()->open() ) {
      emit buildingFinished();
      exit();
      return;
    }

    m_sqlDb = Database::instance()->db();

    Debug::debug() << "  [DataBaseBuilder] starting Database update";

    /*-----------------------------------------------------------*/
    /* Get directories from database                             */
    /* ----------------------------------------------------------*/    
    QSqlQuery dirQuery("SELECT path, mtime FROM directories;",*m_sqlDb);
    while (dirQuery.next())
      m_db_dirs.insert(dirQuery.value(0).toString(),dirQuery.value(1).toUInt());

    
    /*-----------------------------------------------------------*/
    /* Get directories from filesystem                           */
    /* ----------------------------------------------------------*/    
    foreach(const QString& root_dir, m_input_folders)
    {
      Debug::debug() << "  [DataBaseBuilder] ROOT DIR :" << root_dir;

      QDir dir(root_dir);

      m_fs_dirs.insert( root_dir );
      
      /* ----- protect if dir is not mounted or not available - */
      if(  !dir.exists() || !dir.isReadable() )
      {
        m_fs_dirs.remove( root_dir );

        foreach ( const QString& db_dir, m_db_dirs.keys() )
        {
            if (db_dir.startsWith( root_dir ))
                m_db_dirs.remove(db_dir);
        }
        Debug::warning() << "  [DataBaseBuilder] skipping not readable dir:"<< root_dir;
        continue;
      }      
      
      /* ----- read childs folders ---------------------------- */
      dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

      QDirIterator it(dir,QDirIterator::Subdirectories);

      while(it.hasNext())
      {
        it.next();
        
        if ( it.fileInfo().isSymLink() )
        {
          QString new_path = it.fileInfo().symLinkTarget();

          new_path = QFileInfo(new_path).canonicalFilePath().toUtf8();
          
          if( QFileInfo(new_path).isDir() )
            m_fs_dirs.insert( new_path );
        }
        else if ( it.fileInfo().isDir() )
        {
          QString dir_path = it.fileInfo().canonicalFilePath().toUtf8();
          //Debug::debug() << "  [DataBaseBuilder] canonicalFilePath: " << dir_path;
          m_fs_dirs.insert( dir_path );
        }
      }
    }

    fileCount = m_fs_dirs.count();
    
    /*-----------------------------------------------------------*/
    /* Update database                                           */
    /* ----------------------------------------------------------*/
    //On SQLite --> it's MUCH faster to have everything in one transaction
    //with only one disk write than to commit every each insert individually
    QSqlQuery("BEGIN TRANSACTION;",*m_sqlDb);    
    
    foreach(const QString& dir_path, m_fs_dirs)
    {
      if(m_exit)
        break;

      /* If the directory is NOT in database then insert */
      if ( !m_db_dirs.contains(dir_path) )
      {
        addDirectory(dir_path);
      }
      /* If the file is in database but has another mtime then update it */
      else if ( m_db_dirs[dir_path] != QFileInfo(dir_path).lastModified().toTime_t() )
      {
          updateDirectory(dir_path);
      }

      m_db_dirs.remove(dir_path);

       /* signal progress */
       if(fileCount > 0) {
         int percent = 100 - ((fileCount - ++idxCount) * 100 / fileCount);
         emit buildingProgress(percent);
       }
    } /* end foreach file in filesystem */
    
    
    
    /* Get directories that are in DB but not on filesystem */
    QHashIterator<QString, uint> i(m_db_dirs);
    while (i.hasNext()) {
        i.next();
        removeDirectory(i.key());
    }

    m_db_dirs.clear();

    /* Check for interprets/albums/genres... that are not used anymore */
    DatabaseCmd::clean();

    /* Store last update time */
    QSqlQuery q("UPDATE `db_attribute` SET `value`=:date WHERE `name`=lastUpdate;",*m_sqlDb);
    q.bindValue(":date", QDateTime::currentDateTime().toTime_t());
    q.exec();

    /* Now write all data to the disk */
    QSqlQuery("COMMIT TRANSACTION;",*m_sqlDb);

    Debug::debug() << "  [DataBaseBuilder] end Database update";

    emit buildingFinished();    
    exit();
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::addDirectory                                              */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::addDirectory(const QString& path)
{
    Debug::debug() << "  [DataBaseBuilder] addDirectory :" << path;
    foreach (const QString& file, filesFromFilesystem(path) )
    {
        if (MEDIA::isAudioFile(file) )
           insertTrack(file);
        else
           insertPlaylist(file);
    }
    
    insertDirectory(path);
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::updateDirectory                                           */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::updateDirectory(const QString& path)
{
    Debug::debug() << "  [DataBaseBuilder] updateDirectory :" << path;
    
    QStringList files_fs = filesFromFilesystem(path);
    QHash<QString,uint> files_db = filesFromDatabase(path);  

    foreach(const QString& filepath, files_fs)
    {
      //! If the file is NOT in database then insert
      if (!files_db.contains(filepath) )
      {
        if (MEDIA::isAudioFile(filepath) )
          insertTrack(filepath);
        else
          insertPlaylist(filepath);
      }
      //! If the file is in database but has another mtime then update it
      else if (files_db[filepath] != QFileInfo(filepath).lastModified().toTime_t())
      {
        if (MEDIA::isAudioFile(filepath) )
          insertTrack(filepath);
        else
          insertPlaylist(filepath);
      }

       files_db.remove(filepath);
    } // end foreach file in filesystem
    
    
    //! Get files that are in DB but not on filesystem
    QHashIterator<QString, uint> i(files_db);
    while (i.hasNext()) {
        i.next();
        if( MEDIA::isAudioFile(i.key()) )
          removeTrack(i.key());
        else
          removePlaylist(i.key());
    }
    
    QSqlQuery q("", *m_sqlDb);
    q.prepare("UPDATE `directories` SET `mtime`=? WHERE `path`=?;");
    q.addBindValue(QFileInfo(path).lastModified().toTime_t());
    q.addBindValue(path);
    q.exec();
}
        
/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::removeDirectory                                           */
/* ---------------------------------------------------------------------------*/                     
void DataBaseBuilder::removeDirectory(const QString& path)
{
    Debug::debug() << "  [DataBaseBuilder] removeDirectory :" << path;

    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `directories` WHERE `path`=?;");
    q.addBindValue( path );
    q.exec();

    if ( q.next() ) {
      int dir_id = q.value(0).toString().toInt();
      
      q.prepare("DELETE FROM `tracks` WHERE `dir_id`=?;");
      q.addBindValue(dir_id);
      q.exec();

      q.prepare("DELETE FROM `playlists` WHERE `dir_id`=?;");
      q.addBindValue(dir_id);
      q.exec();
      
      q.prepare("DELETE FROM `directories` WHERE `id`=?;");
      q.addBindValue(dir_id);
      q.exec();
    }
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::insertDirectory                                           */
/* ---------------------------------------------------------------------------*/  
int DataBaseBuilder::insertDirectory(const QString & path)
{
    Debug::debug() << "  [DataBaseBuilder] insertDirectory " << path;

    uint mtime   = QFileInfo(path).lastModified().toTime_t();
    
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `directories` WHERE `path`=?;");
    q.addBindValue( path );
    Debug::debug() << q.exec();

    if ( !q.next() ) {
      q.prepare("INSERT INTO `directories`(`path`,`mtime`) VALUES (?,?);");
      q.addBindValue(path);
      q.addBindValue(mtime);
      Debug::debug() << q.exec();

      if(q.numRowsAffected() < 1) return -1;
      q.prepare("SELECT `id` FROM `directories` WHERE `path`=?;");
      q.addBindValue( path);
      q.exec();
      q.next();
    }
    return q.value(0).toInt();
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::insertTrack                                               */
/*     -> MEDIA::FromLocalFile(fname) to get track metadata                   */
/*     -> track->coverHash() to get hash of covername                         */
/* ---------------------------------------------------------------------------*/  
void DataBaseBuilder::insertTrack(const QString& filename)
{
    QFileInfo fileInfo(filename);

    QString fname = fileInfo.filePath().toUtf8();

    uint mtime = QFileInfo(filename).lastModified().toTime_t();

    /* ---------- Read tag from URL file (with taglib) ---------- */
    int disc_number = 0;
    MEDIA::TrackPtr track = MEDIA::FromLocalFile(fname, &disc_number);

    /* ---------- DIRECTORIES part in database ---------- */
    int id_dir = insertDirectory( QFileInfo(filename).canonicalPath() );
    
    /* ---------- GENRE part in database ---------- */
    int id_genre = DatabaseCmd::insertGenre( track->genre );

    /* ---------- YEAR part in database ---------- */
    int id_year = DatabaseCmd::insertYear( track->year );

    /* ---------- ARTIST part in database ---------- */
    bool isNewArtists = !(DatabaseCmd::isArtistExists(track->artist));
    
    int id_artist = DatabaseCmd::insertArtist( track->artist );

    if( isNewArtists )
    {
        fetchArtistImage( track );
    }
    
    /* ---------- ALBUM part in database ---------- */
    bool isNewAlbum = !(DatabaseCmd::isAlbumExists(track->album, id_artist));

    int id_album = DatabaseCmd::insertAlbum(track->album,id_artist,track->year,disc_number);

    if( isNewAlbum )
    {
        /* remove previous coverart as we do a complete rebuild */
        if (m_do_rebuild)
        {            
            QFile file( UTIL::CONFIGDIR + "/albums/" + track->coverHash() );
            if(file.exists()) 
                file.remove();
        }
        
        /* cover update */
        bool ok = saveAlbumCoverFromFile( track );
        
        if( !ok && Database::instance()->param()._option_check_cover )
          ok = saveAlbumCoverFromDirectory( track );
        
        if( !ok )
          fetchAlbumImage( track );
    }
    
    /* ---------- TRACK part in database ---------- */
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `tracks` WHERE `filename`=?;");
    q.addBindValue( fname );
    q.exec();

    if ( !q.next() ) 
    {
      Debug::debug() << "  [DataBaseBuilder] insert track :" << track->title;

      q.prepare("INSERT INTO `tracks`(`filename`,`trackname`,`number`,`length`,"    \
                    "`artist_id`,`album_id`,`year_id`,`genre_id`,`dir_id`,`mtime`," \
                    "`playcount`,`rating`,`bitrate`,`samplerate`,`bpm`,"            \
                    "`albumgain`,`albumpeakgain`,`trackgain`,`trackpeakgain`) "     \
                    " VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");

      q.addBindValue(fname);
      q.addBindValue(track->title);
      q.addBindValue(track->num);
      q.addBindValue(track->duration);

      q.addBindValue(id_artist);
      q.addBindValue(id_album);
      q.addBindValue(id_year);
      q.addBindValue(id_genre);
      q.addBindValue(id_dir);

      q.addBindValue(mtime);
      q.addBindValue(track->playcount);
      q.addBindValue(track->rating);
      q.addBindValue(track->extra["bitrate"]);
      q.addBindValue(track->extra["samplerate"]);
      q.addBindValue(track->extra["bpm"]);
      q.addBindValue(track->albumGain);
      q.addBindValue(track->albumPeak);
      q.addBindValue(track->trackGain);
      q.addBindValue(track->trackPeak);
      q.exec();      
      //Debug::debug() << q.lastError();
    }
    else
    {
      Debug::debug() << "  [DataBaseBuilder] update track :" << track->title;
      
      int id = q.value(0).toInt();
      q.prepare("UPDATE `tracks` SET `trackname`=?,`number`=?,`length`=?,"                      \
                    "`artist_id`=?,`album_id`=?,`year_id`=?,`genre_id`=?,`dir_id`=?,`mtime`=?," \
                    "`playcount`=?,`rating`=?,`bitrate`=?,`samplerate`=?,`bpm`=?,"              \
                    "`albumgain`=?,`albumpeakgain`=?,`trackgain`=?,`trackpeakgain`=?"           \
                    " WHERE `id`=?;");

      q.addBindValue(track->title);
      q.addBindValue(track->num);
      q.addBindValue(track->duration);

      q.addBindValue(id_artist);
      q.addBindValue(id_album);
      q.addBindValue(id_year);
      q.addBindValue(id_genre);
      q.addBindValue(id_dir);

      q.addBindValue(mtime);
      q.addBindValue(track->playcount);
      q.addBindValue(track->rating);
      q.addBindValue(track->extra["bitrate"]);
      q.addBindValue(track->extra["samplerate"]);
      q.addBindValue(track->extra["bpm"]);      
      q.addBindValue(track->albumGain);
      q.addBindValue(track->albumPeak);
      q.addBindValue(track->trackGain);
      q.addBindValue(track->trackPeak);
      
      q.addBindValue( id );
      q.exec();
      //Debug::debug() << q.lastError();
    }

    // delete media from memory
    if(track) {
      track.reset();
      delete track.data();
    }
}


/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::removeTrack                                               */
/* ---------------------------------------------------------------------------*/  
void DataBaseBuilder::removeTrack(const QString& filename)
{
    Debug::debug() << "  [DataBaseBuilder] deleting track :" << filename;
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();

    QSqlQuery query(*m_sqlDb);
    query.prepare("DELETE FROM `tracks` WHERE `filename`=?;");
    query.addBindValue(fname);
    query.exec();
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::insertPlaylist                                            */
/* ---------------------------------------------------------------------------*/  
void DataBaseBuilder::insertPlaylist(const QString& filename)
{
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();
    QString pname = fileInfo.baseName();
    uint mtime    = fileInfo.lastModified().toTime_t();

    Debug::debug() << "  [DataBaseBuilder] insert playlist :" << filename;

    int favorite = 0;

    
    
    //! DIRECTORIES part in database
    int id_dir = insertDirectory( QFileInfo(filename).canonicalPath() );
    
    
    //! PLAYLIST part in database
    QSqlQuery q("", *m_sqlDb);
    q.prepare("SELECT `id` FROM `playlists` WHERE `filename`=?;");
    q.addBindValue( fname );
    q.exec();

    if ( !q.next() ) 
    {
      q.prepare("INSERT INTO `playlists`(`filename`,`name`,`type`,`favorite`,`dir_id`,`mtime`)" \
                    "VALUES(?,?,?,?,?,?);");

      q.addBindValue(fname);
      q.addBindValue(pname);
      q.addBindValue((int) T_FILE);
      q.addBindValue(favorite);
      q.addBindValue(id_dir);
      q.addBindValue(mtime);
      q.exec();
    }
    else
    {
       q.prepare("UPDATE `playlists` SET `name`=?,`type`=?,`favorite`=?," \
                    "`dir_id`=?,`mtime`=? WHERE `id`=?;");
       
       q.addBindValue(pname);
       q.addBindValue((int) T_FILE);
       q.addBindValue(favorite);
       q.addBindValue(id_dir);
       q.addBindValue(mtime);
       q.addBindValue( q.value(0).toInt() );
       q.exec();       
    }


    //! PLAYLIST ITEM part in database
    QList<MEDIA::TrackPtr> list =  MEDIA::PlaylistFromFile(filename);
    foreach (MEDIA::TrackPtr mi, list)
    {
      QString url           = mi->url;
      QString name          = QFileInfo(url).baseName();

      //! Playlist Item part in database
      Debug::debug() << "  [DataBaseBuilder] insert playlistitem url: " << url;

      q.prepare("INSERT INTO `playlist_items`(`url`,`name`,`playlist_id`)" \
                    "VALUES(?," \
                    "       ?," \
                    "       (SELECT `id` FROM `playlists` WHERE `filename`=?));");
      q.addBindValue(url);
      q.addBindValue(name);
      q.addBindValue(fname);
      q.exec();
    } // end foreach url into playlist
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::removePlaylist                                            */
/* ---------------------------------------------------------------------------*/  
void DataBaseBuilder::removePlaylist(const QString& filename)
{
    Debug::debug() << "  [DataBaseBuilder] deleting playlist :" << filename;
    QFileInfo fileInfo(filename);
    QString fname = fileInfo.filePath().toUtf8();

    QSqlQuery query(*m_sqlDb);
    query.prepare("DELETE FROM `playlists` WHERE `filename`=?;");
    query.addBindValue(fname);
    query.exec();
}


/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::saveAlbumCoverFromFile                                    */
/* ---------------------------------------------------------------------------*/
bool DataBaseBuilder::saveAlbumCoverFromFile(MEDIA::TrackPtr track)
{
    //Debug::debug() << "  [DataBaseBuilder] saveAlbumCoverFromFile";

    const QString path = UTIL::CONFIGDIR + "/albums/" + track->coverHash();

    //! check if cover art already exist
    if( QFile(path).exists() )
        return true;

    //! get cover image from file
    QImage image = MEDIA::LoadImageFromFile(track->url);
    if( !image.isNull() ) 
    {
      image.save(path, "png", -1);
      return true;
    }
    return false;
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::saveAlbumCoverFromDirectory                               */
/* ---------------------------------------------------------------------------*/
bool DataBaseBuilder::saveAlbumCoverFromDirectory(MEDIA::TrackPtr track)
{
    //Debug::debug() << "  [DataBaseBuilder] saveAlbumCoverFromDirectory";
    const QString path = UTIL::CONFIGDIR + "/albums/" + track->coverHash();
    
    //! check if cover art already exist
    if( QFile(path).exists() )
        return true;

    //! search album art into file source directory
    const QStringList imageFilters = QStringList() << "*.jpg" << "*.png";
    QDir sourceDir(QFileInfo(track->url).absolutePath());

    sourceDir.setNameFilters(imageFilters);

    QStringList entryList = sourceDir.entryList(imageFilters, QDir::Files, QDir::Size);

    while(!entryList.isEmpty()) {
      //! I take the first one (the biggest one)
      //!WARNING simplification WARNING
      QString file = QFileInfo(track->url).absolutePath() + "/" + entryList.takeFirst();
      QImage image = QImage(file);
      //! check if not null image (occur when file is KO)
      if(!image.isNull()) {
        image = image.scaled(QSize(120,120), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        //! check if save is OK
        if(image.save(path, "png", -1))
          return true;
      }
    }
    
    return false;
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::fetchAlbumImage                                           */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::fetchAlbumImage(MEDIA::TrackPtr track)
{
    //Debug::debug() << "  [DataBaseBuilder] fetchAlbumImage";    
    const QString path = QString(UTIL::CONFIGDIR + "/albums/" + track->coverHash());

    if(QFile(path).exists()) 
      return;
    
    INFO::InfoStringHash hash;
    hash["artist"]     = track->artist;
    hash["album"]      = track->album;
        
    INFO::InfoRequestData request = INFO::InfoRequestData(INFO::InfoAlbumCoverArt, hash);
      
    m_requests_ids << request.requestId;         
    InfoSystem::instance()->getInfo( request );
}



/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::fetchArtistImage                                          */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::fetchArtistImage(MEDIA::TrackPtr track)
{
    QString path = QString(UTIL::CONFIGDIR + "/artists/" + MEDIA::artistHash( track->artist ));

    if(QFile(path).exists()) 
      return;
    
    INFO::InfoStringHash hash;
    hash["artist"]     = track->artist;

    INFO::InfoRequestData request = INFO::InfoRequestData(INFO::InfoArtistImages, hash);
      
    m_requests_ids << request.requestId;         
    InfoSystem::instance()->getInfo( request );
}

/* ---------------------------------------------------------------------------*/
/* DataBaseBuilder::slot_systeminfo_received                                  */
/* ---------------------------------------------------------------------------*/
void DataBaseBuilder::slot_systeminfo_received(INFO::InfoRequestData request, QVariant output)
{
    if(!m_requests_ids.contains(request.requestId))
      return;

    m_requests_ids.removeOne(request.requestId);

    /* get request info */
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();
    
    if ( request.type == INFO::InfoArtistImages )
    {
        QImage image = UTIL::artistImageFromByteArray( output.toByteArray() );
    
        QString path = QString(UTIL::CONFIGDIR + "/artists/" + MEDIA::artistHash( hash["artist"] ));
    
        image.save(path, "png", -1);
    }
    else if ( request.type == INFO::InfoAlbumCoverArt )
    {     
        QString path = QString(UTIL::CONFIGDIR + "/albums/" + MEDIA::coverHash(hash["artist"], hash["album"]));
        
        if(QFile(path).exists()) 
          return;
    
        QImage image = QImage::fromData( output.toByteArray() );

        if( !image.isNull() )
        {
          image = image.scaled(QSize(120, 120), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
          image.save(path, "png", -1);
        }
    }
}

