/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#ifndef _PLAYLIST_POPULATOR_H_
#define _PLAYLIST_POPULATOR_H_

#include "mediaitem.h"


#include <QRunnable>
#include <QObject>
#include <QStringList>
#include <QString>
#include <QList>
#include <QUrl>
#include <QMap>

class StreamLoader;
class PlayqueueModel;

/*
********************************************************************************
*                                                                              *
*    Class PlaylistPopulator                                                   *
*                                                                              *
********************************************************************************
*/
class PlaylistPopulator : public QObject, public QRunnable
{
Q_OBJECT
  public:
    explicit PlaylistPopulator();
    void setModel(PlayqueueModel* m) {m_model = m;};
    void run();
    bool isRunning() {return m_isRunning;}

    void addFiles(const QStringList &files);
    void addFile(const QString &file);
    void addUrls(QList<QUrl> listUrl, int playlist_row);
    void addMediaItems(QList<MEDIA::TrackPtr> list, int playlist_row);

    void restoreSession();
    
  private:
    PlayqueueModel          *m_model;      
    QStringList              m_files;
    QList<MEDIA::TrackPtr>   m_tracks;
    bool                     m_isRunning;
    int                      m_playlist_row;
  
  signals:
    void playlistPopulated();
    void async_load(MEDIA::TrackPtr,int);
};

#endif // _PLAYLIST_POPULATOR_H_
