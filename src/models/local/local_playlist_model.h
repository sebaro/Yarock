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

#ifndef _LOCAL_PLAYLIST_MODEL_H_
#define _LOCAL_PLAYLIST_MODEL_H_

#include <QVariant>
#include <QString>
#include <QStringList>
#include <QObject>

#include "core/mediaitem/mediaitem.h"

/*
********************************************************************************
*                                                                              *
*    Class LocalPlaylistModel                                                  *
*                                                                              *
********************************************************************************
*/
class LocalPlaylistModel : public QObject
{
Q_OBJECT
     static LocalPlaylistModel*  INSTANCE;

  public:
     LocalPlaylistModel(QObject *parent = 0);
     ~LocalPlaylistModel();
     static LocalPlaylistModel* instance() { return INSTANCE; }

     //! custom method
     MEDIA::MediaPtr rootItem();
     void clear();

     //! get Url method
     QList<MEDIA::TrackPtr> getItemChildrenTracks(const MEDIA::MediaPtr parent);

     //! filtering method
     void setSearch(const QVariant v) {m_search = v;}
     bool isMediaMatch(MEDIA::MediaPtr);
     
  signals:
    void dataChanged();

  private:
     MEDIA::MediaPtr        m_rootItem;
     QVariant               m_search;
};

#endif // _LOCAL_PLAYLIST_MODEL_H_
