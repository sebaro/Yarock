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
#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include "mediaitem.h"
#include "mediamimedata.h"

#include <QObject>
#include <QThreadPool>
#include <QList>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QMap>

// qrunnbale class
class PlaylistPopulator;    // thread to populate playlist
class PlaylistWriter;       // thread to save playlist to file
class PlaylistDbWriter;     // thread to save playlist to Database
class PlayqueueModel;
class StreamLoader;
/*
********************************************************************************
*                                                                              *
*    Class TaskManager                                                         *
*                                                                              *
********************************************************************************
*/
class TaskManager : public QObject
{
Q_OBJECT
  public:
    explicit TaskManager(PlayqueueModel* model);

    ~TaskManager();

    // Playlist Populator Thread
    void playlistAddFiles(const QStringList &files);
    void playlistAddFile(const QString &file);
    void playlistAddMediaItems(QList<MEDIA::TrackPtr> list, int playlist_row=-1);
    void playlistAddUrls(QList<QUrl> listUrl, int playlist_row=-1);

    // Playlist Writer Thread
    void playlistSaveToFile(const QString &filename);
    void playlistSaveToDb(const QString &name, int db_id=-1);
    void playlistSaveToDb(MEDIA::PlaylistPtr playlist);

    void savePlayqueueSession();
    void restorePlayqueueSession();
    
    void loadEditorPlaylist(MediaMimeData* mimedata, int row);
    
  private slots:
    void slot_load_async(MEDIA::TrackPtr,int);
    void slot_load_async_done(MEDIA::TrackPtr);

  private:
    PlayqueueModel         *m_model;
    QThreadPool            *m_threadPool;  // QRunnable manager
    PlaylistPopulator      *m_populator;   // QRunnable
    PlaylistWriter         *m_writer;      // QRunnable
    PlaylistDbWriter       *m_db_writer;   // QRunnable

    QMap<StreamLoader*, int/*row*/>  m_asyncloaders;
    
    //! messages Id for StatusWidget management
    QMap<QString, uint>    messageIds;
    
  signals:
    void loadPlaylist(MediaMimeData*,int);
    void playlistPopulated();
    void playlistSaved();
};

#endif // _TASK_MANAGER_H_
