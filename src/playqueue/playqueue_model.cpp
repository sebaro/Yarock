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
#include "playqueue_model.h"
#include "playqueue_proxymodel.h"
#include "task_manager.h"

#include "media_search_engine.h"  // for sorting

#include "utilities.h"
#include "debug.h"

#include <QBuffer>
/*******************************************************************************
 SortByRowAsc function object
   -> avoid to use boost to bind member function in qSort
*******************************************************************************/
struct SortByRowAsc
{
    SortByRowAsc(PlayqueueModel* model) : m_model(model) { }

    bool operator()(MEDIA::TrackPtr trackleft, MEDIA::TrackPtr trackright)
    {
          return m_model->rowForTrack(trackleft) < m_model->rowForTrack(trackright);
    }
    
private:
    PlayqueueModel *m_model;
};


/*
******************************************************************************************
*                                                                                        *
*    Class PlayqueueModel                                                                *
*                                                                                        *
******************************************************************************************
*/
PlayqueueModel::PlayqueueModel(QObject* parent) : 
   QAbstractListModel(parent), PlayqueueBase()
{
    m_stop_after_track   =  MEDIA::TrackPtr(0);

    m_proxy_model = new PlayqueueProxyModel(this);
    m_proxy_model->setSourceModel(this);
    PlayqueueBase::setProxy(m_proxy_model);
    
    m_task_manager = new TaskManager(this);
    
    connect( this, SIGNAL( insertTrack(const MEDIA::TrackPtr, int) ), this, SLOT( slot_insert_mediaitem(const MEDIA::TrackPtr, int)) );
}


TaskManager* PlayqueueModel::manager() const
{
    return m_task_manager;
}

/*******************************************************************************
    PlayqueueModel::clear
*******************************************************************************/
void PlayqueueModel::clear()
{
    //Debug::debug() << "      [PlayqueueModel]  clear";
    beginRemoveRows(QModelIndex(), 0, PlayqueueBase::size()-1);
    PlayqueueBase::clear();    
    endRemoveRows();
    
    emit dataChanged(QModelIndex(), QModelIndex());
    emit updated();
    emit modelCleared();    
}

/*******************************************************************************
    QAbstractListModel implementation
*******************************************************************************/
int PlayqueueModel::rowCount(const QModelIndex & /* parent */) const
{
    return PlayqueueBase::size();
}

QVariant PlayqueueModel::data(const QModelIndex &index, int role) const
{
Q_UNUSED(index)
Q_UNUSED(role)
    return QVariant();
}


bool PlayqueueModel::removeRows(int position, int count, const QModelIndex & /*parent*/)
{
    //Debug::debug() << "      [PlayqueueModel]  removeRow position rows" << position;
    //Debug::debug() << "      [PlayqueueModel]  removeRow position count" << count;
    if( position + count - 1 >= PlayqueueBase::size()) return false;

    beginRemoveRows(QModelIndex(), position, position + count - 1);
    for (int itemcount = (count-1); itemcount >= 0; itemcount--) {
      MEDIA::TrackPtr track = PlayqueueBase::removeTrackAt(position + itemcount);
      track.reset();
    }
    endRemoveRows();

    return true;
}

/*******************************************************************************
    PlayqueueModel::updatePlayingItem
*******************************************************************************/
void PlayqueueModel::updatePlayingItem(MEDIA::TrackPtr tk)
{
    //Debug::debug() << "      [PlayqueueModel]  updatePlayingItem";
    
    int playing_row = PlayqueueBase::rowForTrack(PlayqueueBase::playingTrack());

    if(playing_row !=  -1) 
    {
        QModelIndex newIndex = index(playing_row, 0, QModelIndex());
        emit dataChanged(newIndex, newIndex);      
    }    
    
    PlayqueueBase::setPlayingTrack(tk);
    
    if(rowForTrack(tk) != -1) 
    {
      QModelIndex newIndex = index(rowForTrack(tk), 0, QModelIndex());
      emit dataChanged(newIndex, newIndex);
    }    
}

/*******************************************************************************
    PlayqueueModel::skipBackward
*******************************************************************************/
int PlayqueueModel::skipBackward(bool repeat /*=false*/)
{
   /* tant que l'on ne trouve pas de ligne située avant presente dans le proxy */
   int i = PlayqueueBase::rowForTrack(PlayqueueBase::requestedTrack()); /* source row requested */ 
   --i;
   while(i >= 0 && !PlayqueueBase::filterContainsRow(i))
     --i;
   
   if(repeat && i == -1) {
     int j = rowCount() -1;
     while(j > i && !PlayqueueBase::filterContainsRow(j))
       --j;
       
     i = j;
   }
   return i; 
}

/*******************************************************************************
    PlayqueueModel::skipForward
*******************************************************************************/
int PlayqueueModel::skipForward(bool repeat /*=false*/)
{
   /* tant que l'on ne trouve pas de ligne située aprés presente dans le proxy */
   int i = PlayqueueBase::rowForTrack(PlayqueueBase::requestedTrack()); /* source row requested */ 
   int source_row  = i;
   ++i;
   while(i < rowCount() && !PlayqueueBase::filterContainsRow(i))
     ++i;

   if(i >= rowCount()) i = -1;
   
   if(repeat && i == -1) {
     int j = 0;
     while(j < source_row && !PlayqueueBase::filterContainsRow(j))
       ++j;

     i = j;
   }
   return i;    
}

/*******************************************************************************
    PlayqueueModel::trackAt
*******************************************************************************/
MEDIA::TrackPtr PlayqueueModel::trackAt(int row)
{
    if (PlayqueueBase::rowExists(row))
      return PlayqueueBase::tracks().at(row);

    return MEDIA::TrackPtr(0);
}


/*******************************************************************************
    STOP AFTER TRACK
*******************************************************************************/
MEDIA::TrackPtr PlayqueueModel::stopAfterTrack() const
{
    return m_stop_after_track;
}

void PlayqueueModel::setStopAfterTrack(MEDIA::TrackPtr track)
{
    /* unset old track if exist */
    if(!m_stop_after_track.isNull() && (m_stop_after_track != track)) {
      m_stop_after_track->isStopAfter = !m_stop_after_track->isStopAfter;

      /* emit change */
      int row = PlayqueueBase::rowForTrack(m_stop_after_track);
      QModelIndex newIndex = index(row, 0, QModelIndex());
      emit dataChanged(newIndex, newIndex);
    }

    /* get new track */
    m_stop_after_track = track;
    m_stop_after_track->isStopAfter = !m_stop_after_track->isStopAfter;


    /* emit change */
    int row = PlayqueueBase::rowForTrack(m_stop_after_track);
    QModelIndex newIndex = index(row, 0, QModelIndex());
    emit dataChanged(newIndex, newIndex);
}

/*******************************************************************************
    addMediaItem
    addMediaItems
    insertMediaItem
    removeDuplicate
*******************************************************************************/
void PlayqueueModel::addMediaItem(const MEDIA::TrackPtr media)
{
    //Debug::debug() << "      [PlayqueueModel]  addMediaItem";
    beginInsertRows(QModelIndex(), PlayqueueBase::size(), PlayqueueBase::size());
    PlayqueueBase::addTrack(media);
    endInsertRows();
}

void PlayqueueModel::addMediaItems(QList<MEDIA::TrackPtr> list)
{
    //Debug::debug() << "      [PlayqueueModel]  addMediaItems";
    beginInsertRows(QModelIndex(), PlayqueueBase::size(), PlayqueueBase::size());
    PlayqueueBase::addTracks(list);
    endInsertRows();
}

void PlayqueueModel::request_insert_track(const MEDIA::TrackPtr mediaitem, int pos)
{
    emit insertTrack(mediaitem, pos);
}


void PlayqueueModel::slot_insert_mediaitem(const MEDIA::TrackPtr media, int pos)
{
    const int start = (pos == -1) ? rowCount() : pos;

    beginInsertRows(QModelIndex(), start, start);
    
    PlayqueueBase::insertTrack(media, start);
    
    endInsertRows();
}

void PlayqueueModel::removeDuplicate()
{
    //Debug::debug() << "      [PlayqueueModel]  removeDuplicate";
    QSet<QUrl> filenames;

    foreach(MEDIA::TrackPtr media, PlayqueueBase::tracks()) {
        QString path = media->url;
        if(filenames.contains( path ))
        {
          int row = PlayqueueBase::rowForTrack(media);
          removeRows(row,1,QModelIndex());
        }
        else
        {
          filenames.insert( path );
        }
    }
}

/*******************************************************************************
    PlayqueueModel::queueDuration
*******************************************************************************/
QString PlayqueueModel::queueDuration()
{
    //Debug::debug() << "      [PlayqueueModel]  queueDuration";

    int seconds = 0;
    foreach (MEDIA::TrackPtr track, PlayqueueBase::tracks()) {
      if(track->type() != TYPE_TRACK)
        continue;

      if(track->duration > 0)
        seconds += track->duration;
    }
    return UTIL::durationToString(seconds);
}


/*******************************************************************************
    DRAG AND DROP
    -> supportedDropActions
    -> flags
    -> mimeTypes
    -> mimeData
    -> dropMimeData
*******************************************************************************/
Qt::DropActions PlayqueueModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}


Qt::ItemFlags PlayqueueModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (index.isValid())
    return flags | Qt::ItemIsDragEnabled;

  return Qt::ItemIsDropEnabled;
}

QStringList PlayqueueModel::mimeTypes() const
{
    // Encoding Exported Data with mimeTypes
    QStringList types;
    types << "text/uri-list" << MEDIA_MIME;
    return types;
}

QMimeData* PlayqueueModel::mimeData( const QModelIndexList &indexes ) const
{
    // drag from playlist to external software (file browser)
    if (indexes.isEmpty())
      return NULL;
  
    MediaMimeData* mime = new MediaMimeData(SOURCE_PLAYQUEUE);

    QList<QUrl> urls;
    QList<MEDIA::TrackPtr> tracks;
    foreach( const QModelIndex &index, indexes ) {
        const int row = index.row();
        if (row >= 0 && row < PlayqueueBase::size()) {
          if(PlayqueueBase::tracks().at(row)->type() == TYPE_TRACK)
            urls << QUrl::fromLocalFile( PlayqueueBase::tracks().at(row)->url );
          else
            urls << QUrl(PlayqueueBase::tracks().at(row)->url);
        }
        tracks << PlayqueueBase::tracks().at(row);
    }    

    mime->setUrls(urls);    
    mime->addTracks(tracks);  
    
    return mime;    
}

//!------------PlayqueueModel::dropMimeData -------------------------------------
// --> Inserting Dropped Data into a Model
bool PlayqueueModel::dropMimeData(const QMimeData *data,
                                 Qt::DropAction action, int row, int column,
                                 const QModelIndex &parent)
{
  Q_UNUSED(parent)
  Q_UNUSED(column)
    //Debug::debug() << "      [PlayqueueModel]  dropMimeData row " << row;

    if (action == Qt::IgnoreAction) return true;

    /* ---- internal drop ---- */
    if (data->hasFormat(MEDIA_MIME)) 
    {
      const MediaMimeData* mediaMimeData = dynamic_cast<const MediaMimeData*>( data );
      if(!mediaMimeData)
        return false;

      /* internal move ---- */
      if(mediaMimeData->source() == SOURCE_PLAYQUEUE)
      {
        move(mediaMimeData, row);
      }
      /* drop from collection browser ---- */
      else
      {
        m_task_manager->playlistAddMediaItems(mediaMimeData->getTracks(), row);
      }
      return true;
    }
    /* ---- external drop ---- */
    else if (data->hasUrls()) 
    {
      //Debug::debug() << "      [PlayqueueModel]  dropMimeData HAS URLS" << data->urls();
      m_task_manager->playlistAddUrls(data->urls(), row);
      return true;
    }

    return false;
}

//!------------PlayqueueModel::move ---------------------------------------------
void PlayqueueModel::move(const MediaMimeData *data, int pos)
{
    Debug::debug() << "      [PlayqueueModel]  move";
    emit layoutAboutToBeChanged();

    QList<MEDIA::TrackPtr> moved_items = data->getTracks();

    SortByRowAsc sorter(this);
    qSort(moved_items.begin(), moved_items.end(),  sorter);

    if (pos < 0)
      pos = PlayqueueBase::size();

    int start  = pos;
    foreach (MEDIA::TrackPtr track, moved_items) {
      int source_row = rowForTrack(track);
      PlayqueueBase::removeTrackAt(source_row);
      if (pos > source_row)
        start --;
      
      PlayqueueBase::insertTrack(track,start);
      start++;
    }

    emit layoutChanged();
    emit needSelectionAfterMove(moved_items);
}



/*******************************************************************************
    PlayqueueModel::slot_sort
*******************************************************************************/
void PlayqueueModel::slot_sort(QVariant query)
{
    Debug::debug() << "      [PlayqueueModel]  slot_sort";
    emit layoutAboutToBeChanged();
    
    MediaSearch search = qvariant_cast<MediaSearch>( query );

    SearchEngine* search_engine = new SearchEngine();
    search_engine->init_search_engine(search);
    search_engine->doSearch(true);

    QList<MEDIA::TrackPtr> tracks = search_engine->result();

    /* update track list */
    PlayqueueBase::clear(false);
    PlayqueueBase::addTracks(tracks);
    
    emit layoutChanged();
}


/*
********************************************************************************
*                                                                              *
*    Class Playqueue                                                           *
*                                                                              *
********************************************************************************
*/

Playqueue* Playqueue::INSTANCE = 0;

Playqueue::Playqueue()
{
      INSTANCE = this;
}


