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

#include "playqueue_proxymodel.h"
#include "playqueue_model.h"
#include "mediaitem.h"


PlayqueueProxyModel::PlayqueueProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setFilterRole(Qt::EditRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(false);
}

bool PlayqueueProxyModel::filterAcceptsRow(int sourceRow,const QModelIndex &sourceParent) const
{
Q_UNUSED(sourceParent)  
    // get track
    PlayqueueModel* source_model = static_cast<PlayqueueModel*>(sourceModel());
    MEDIA::TrackPtr track = source_model->trackAt(sourceRow);
    
    const QString pattern = filterRegExp().pattern();
    if(pattern.isEmpty())
      return true;
    
    if(track->type() == TYPE_TRACK) {
      if(pattern.length() < 3) 
      {
        return  (track->url.startsWith ( pattern, Qt::CaseInsensitive )   ||
                 track->title.startsWith ( pattern, Qt::CaseInsensitive ) ||
                 track->artist.startsWith ( pattern, Qt::CaseInsensitive ) ||
                 track->album.startsWith ( pattern, Qt::CaseInsensitive ) );
      }
      else
      {
        return  (track->url.contains ( pattern, Qt::CaseInsensitive )   ||
                 track->title.contains ( pattern, Qt::CaseInsensitive ) ||
                 track->artist.contains ( pattern, Qt::CaseInsensitive ) ||
                 track->album.contains ( pattern, Qt::CaseInsensitive ) );
      }
    }
    else { // STREAM TYPE
      if(pattern.length() < 3) 
      {
        return  (track->url.startsWith ( pattern, Qt::CaseInsensitive )   ||
                 track->name.startsWith ( pattern, Qt::CaseInsensitive )  ||
                 track->categorie.startsWith ( pattern, Qt::CaseInsensitive ));
      }
      else
      {
        return  (track->url.contains ( pattern, Qt::CaseInsensitive )   ||
                 track->name.contains ( pattern, Qt::CaseInsensitive )  ||
                 track->categorie.contains ( pattern, Qt::CaseInsensitive ));
      }
    }

    return true;
}
