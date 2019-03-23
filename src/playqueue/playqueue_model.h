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

#ifndef _PLAYQUEUE_MODEL_H_
#define _PLAYQUEUE_MODEL_H_

#include "core/mediaitem/mediaitem.h"
#include "core/mediaitem/mediamimedata.h"
#include "playqueue_base.h"

// #include <QMutex>
#include <QAbstractListModel>

/*
******************************************************************************************
*                                                                                        *
*    Class PlayqueueModel                                                                *
*                                                                                        *
******************************************************************************************
*/
class PlayqueueProxyModel;
class TaskManager;

class PlayqueueModel : public QAbstractListModel, public PlayqueueBase
{
Q_OBJECT
  public:
    PlayqueueModel(QObject* parent = 0);

    QObject* parentObject() {return m_parent;}

    //! inherited from QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool removeRows(int position, int count, const QModelIndex &parent);

    //! drag and drop
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    QMimeData* mimeData( const QModelIndexList &indexes ) const;
    bool dropMimeData(const QMimeData *data,
                      Qt::DropAction action, int row, int column,
                      const QModelIndex &parent);

    void move(const MediaMimeData *data, int pos);

    //! custom methods
    MEDIA::TrackPtr trackAt(int row);
    QString queueDuration();

    //! playing MediaItem method
    void updatePlayingItem(MEDIA::TrackPtr);
    
    //! add/insert mediaitem
    void addMediaItem(const MEDIA::TrackPtr mediaitem);
    void addMediaItems(QList<MEDIA::TrackPtr>);
    void request_insert_track(const MEDIA::TrackPtr mediaitem, int pos=-1);
    void removeDuplicate();

    //! Stop After media Action
    MEDIA::TrackPtr stopAfterTrack() const;
    void setStopAfterTrack(MEDIA::TrackPtr track);

    //! play next/prev
    int skipBackward(bool repeat = false);
    int skipForward(bool repeat = false);
    
    //! TaskManager
    TaskManager* manager() const;    

    
  private slots:
    void slot_insert_mediaitem(const MEDIA::TrackPtr media, int pos=-1);
 
  public slots:    
    void clear();
    void slot_sort(QVariant query);    

  signals:
    void needSelectionAfterMove(QList<MEDIA::TrackPtr>);
    void updated();
    void modelCleared();
    void insertTrack(const MEDIA::TrackPtr, int);
    void insertTracks(QList<MEDIA::TrackPtr>, int);

  private:
    PlayqueueProxyModel     *m_proxy_model;
    TaskManager             *m_task_manager;

    MEDIA::TrackPtr          m_stop_after_track;
    
    QObject                 *m_parent;
    Q_DISABLE_COPY( PlayqueueModel )
};


/*
******************************************************************************************
*                                                                                        *
*    Class Playqueue                                                                     *
*                                                                                        *
******************************************************************************************
*/
class Playqueue : public PlayqueueModel
{
public:
  Playqueue();
  static Playqueue* INSTANCE;
  static Playqueue* instance() { return INSTANCE; }
  
};

#endif // _PLAYLIST_MODEL_H_
