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

#include "task_manager.h"

#include "playqueue_model.h"
#include "playlistpopulator.h"
#include "playlistwriter.h"
#include "playlistdbwriter.h"

#include "views/stream/stream_loader.h"

#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    Class TaskManager                                                         *
*                                                                              *
********************************************************************************
*/
TaskManager::TaskManager(PlayqueueModel* model) : QObject()
{
    m_model       = model;
    
    m_threadPool  = new QThreadPool(this);

    // Qrunnable
    m_populator   = new PlaylistPopulator();
    m_populator->setModel(m_model);
    m_populator->setAutoDelete(false);

    m_writer      = new PlaylistWriter();
    m_writer->setModel(m_model);
    m_writer->setAutoDelete(false);

    m_db_writer   = new PlaylistDbWriter();
    m_db_writer->setModel(m_model);
    m_db_writer->setAutoDelete(false);

    // connection
    QObject::connect(m_populator,SIGNAL(playlistPopulated()),this,SIGNAL(playlistPopulated()));
    QObject::connect(m_populator,SIGNAL(async_load(MEDIA::TrackPtr,int)),this,SLOT(slot_load_async(MEDIA::TrackPtr,int)));

    QObject::connect(m_writer,SIGNAL(playlistSaved()),this,SIGNAL(playlistSaved()));
    QObject::connect(m_db_writer,SIGNAL(playlistSaved()),this,SIGNAL(playlistSaved()));
}

TaskManager::~TaskManager()
{
    Debug::debug() << "[TaskManager] wait to finish ...";
    m_threadPool->waitForDone();
    delete m_populator;
    delete m_writer;
    delete m_db_writer;
    delete m_threadPool;
}

/*******************************************************************************
  slot_load_async
*******************************************************************************/
void TaskManager::slot_load_async(MEDIA::TrackPtr track,int row)
{
    Debug::debug() << "[TaskManager] slot_load_async";
  
    StreamLoader* loader = new StreamLoader(track);
    connect(loader, SIGNAL(download_done(MEDIA::TrackPtr)), this, SLOT(slot_load_async_done(MEDIA::TrackPtr)), Qt::UniqueConnection );

    m_asyncloaders[loader] = row;
    loader->start_asynchronous_download(track->url);
}
  
void TaskManager::slot_load_async_done(MEDIA::TrackPtr parent)
{
    StreamLoader* loader = qobject_cast<StreamLoader*>(sender());
  
    Debug::debug() << "[TaskManager] slot_load_async_done";  
    int row = -1;
    if(m_asyncloaders.keys().contains(loader))
      row = m_asyncloaders.value(loader);
      
    foreach(MEDIA::MediaPtr track, parent->children()) 
    {
        m_model->request_insert_track(track, row);
    }
    
    m_asyncloaders.remove(loader);
    delete loader;
}


/*******************************************************************************
 PlayQueue Population
*******************************************************************************/
void TaskManager::playlistAddFiles(const QStringList &files)
{
    if(m_populator->isRunning()) return;
    m_populator->addFiles(files);
    m_threadPool->start(m_populator, 1);
}

void TaskManager::playlistAddFile(const QString &file)
{
    if(m_populator->isRunning()) return;
    m_populator->addFile(file);
    m_threadPool->start (m_populator, 1);
}

void TaskManager::playlistAddUrls(QList<QUrl> listUrl, int playlist_row)
{
    if(m_populator->isRunning()) return;
    m_populator->addUrls(listUrl, playlist_row);
    m_threadPool->start (m_populator, 1);
}

void TaskManager::playlistAddMediaItems(QList<MEDIA::TrackPtr> list, int playlist_row)
{
    if(m_populator->isRunning()) return;
    m_populator->addMediaItems(list, playlist_row);
    m_threadPool->start (m_populator, 1);
}

void TaskManager::restorePlayqueueSession()
{
     if(m_populator->isRunning()) return;
     m_populator->restoreSession();
}


/*******************************************************************************
 Playqueue to playlist Writer
*******************************************************************************/
void TaskManager::playlistSaveToFile(const QString &filename)
{
    if(m_writer->isRunning()) return;
    m_writer->saveToFile(filename);
    m_threadPool->start (m_writer);
}

void TaskManager::playlistSaveToDb(const QString &name, int bd_id)
{
    if(m_db_writer->isRunning()) return;
    m_db_writer->saveToDatabase(name, bd_id);
    m_threadPool->start (m_db_writer);
}

void TaskManager::savePlayqueueSession()
{
    if(m_db_writer->isRunning()) return;
    m_db_writer->saveSessionToDatabase();
    m_threadPool->start (m_db_writer);
}
