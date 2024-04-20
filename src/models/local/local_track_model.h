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

#ifndef _LOCAL_TRACK_MODEL_H
#define _LOCAL_TRACK_MODEL_H

#include <QVariant>
#include <QString>
#include <QStringList>
#include <QObject>

#include "core/mediaitem/mediaitem.h"

/*
********************************************************************************
*                                                                              *
*    Class LocalTrackModel                                                     *
*                                                                              *
********************************************************************************
*/
class LocalTrackModel : public QObject
{
static LocalTrackModel* INSTANCE;

Q_OBJECT
  public:
     LocalTrackModel(QObject *parent = 0);
     ~LocalTrackModel();
     static LocalTrackModel* instance() { return INSTANCE; }

     //! custom method
     MEDIA::MediaPtr rootItem() {return m_rootItem;}
     MEDIA::LinkPtr  rootLink() {return m_rootGenreItem;}
     MEDIA::LinkPtr  activeLink() {return m_active_link;}
     void setActiveLink(MEDIA::LinkPtr link) {m_active_link = link;}
     
     void clear();
     bool isEmpty() const;

     //! get Track method
     QList<MEDIA::TrackPtr> getItemChildrenTracks(const MEDIA::MediaPtr parent);
     QList<MEDIA::TrackPtr> getAlbumChildrenTracksGenre(const MEDIA::AlbumPtr album,const QString& genre);

     //! get Tags method
//      QStringList getItemTags(const MEDIA::MediaPtr media);

     //! get Rating method
     float getItemAutoRating(const MEDIA::MediaPtr media);

     //! filtering method
     void setSearch(const QVariant v) {m_search = v;}
     bool isMediaMatch(MEDIA::MediaPtr);

     //! list of MediaItem
     QHash<int, MEDIA::TrackPtr> trackItemHash;
     QList<MEDIA::AlbumPtr>      albumItemList;
     
     

  signals:
    void dataChanged();

  public slots:
    void slot_activate_link();

  private:
     MEDIA::LinkPtr   m_rootGenreItem;
     MEDIA::LinkPtr   m_active_link;
     MEDIA::MediaPtr  m_rootItem;
     QVariant         m_search;
};

#endif // _LOCAL_TRACK_MODEL_H
