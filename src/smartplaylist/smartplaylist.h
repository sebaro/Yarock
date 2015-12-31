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
#ifndef _SMART_PLAYLIST_H_
#define _SMART_PLAYLIST_H_

#include "core/mediaitem/mediaitem.h"
#include "core/mediasearch/media_search.h"

#include <QtSql/QSqlDatabase>
#include <QVariant>

/*
********************************************************************************
*                                                                              *
*    Class SmartPlaylist                                                       *
*                                                                              *
********************************************************************************
*/
class SmartPlaylist : public QObject
{
Q_OBJECT

public:
    SmartPlaylist(QObject* parent =0);

    static void createDatabase();

    static QList<MEDIA::TrackPtr> mediaItem(QVariant variant);

    static void updatePlaylist(MEDIA::PlaylistPtr playlist);
};

#endif // _SMART_PLAYLIST_H_
