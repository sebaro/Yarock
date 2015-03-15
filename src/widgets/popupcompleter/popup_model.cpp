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

#include "popup_model.h"
#include "popup_item.h"

// data model
#include "models/local/local_track_model.h"
#include "models/local/histo_model.h"
#include "models/stream/stream_model.h"

#include "settings.h"
#include "views.h"
#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    Class PopupModel                                                          *
*                                                                              *
********************************************************************************
*/
PopupModel::PopupModel(QObject *parent) : QStandardItemModel(parent)
{

}


void PopupModel::populateModel(const QString & query)
{

    switch(SETTINGS()->_viewMode)
    {
      case VIEW::ViewAlbum     :
      case VIEW::ViewArtist    :
      case VIEW::ViewTrack     :
      case VIEW::ViewGenre     :
      case VIEW::ViewYear      :
      case VIEW::ViewFavorite  :
      case VIEW::ViewDashBoard :
        getCollectionSuggestions(query);
        break;

      case VIEW::ViewHistory   :
        getHistorySuggestions(query);
        break;

      case VIEW::ViewDirble        :
      case VIEW::ViewShoutCast     :
      case VIEW::ViewTuneIn        :
      case VIEW::ViewFavoriteRadio :
        getStreamSuggestions(query);
        break;

      case VIEW::ViewPlaylist      :
      case VIEW::ViewSmartPlaylist :
        getCollectionSuggestions(query);
        break;

      default :
        break;
    }

    emit dataChanged(QModelIndex(), QModelIndex());
}

/*******************************************************************************
    getCollectionSuggestions
*******************************************************************************/
void PopupModel::getCollectionSuggestions(const QString & query)
{
    QStandardItem *rootItem = this->invisibleRootItem();

    QStringList artists;
    QStringList albums;
    QStringList titles;

    // get data from model
    LocalTrackModel* model = LocalTrackModel::instance();
    QList<MEDIA::TrackPtr> tracks = model->trackItemHash.values();

    foreach (MEDIA::TrackPtr track, tracks)
    {
       bool found = false;
       if(query.length() < 3)
        found = track->artist.startsWith ( query, Qt::CaseInsensitive );
       else
        found = track->artist.contains ( query, Qt::CaseInsensitive );

       if(found && !artists.contains(track->artist) && artists.count() < 5)
         artists << track->artist;

       found = false;
       if(query.length() < 3)
        found = track->album.startsWith ( query, Qt::CaseInsensitive );
       else
        found = track->album.contains ( query, Qt::CaseInsensitive );

       if(found && !albums.contains(track->album) && albums.count() < 5)
         albums << track->album;

       found = false;
       if(query.length() < 3)
        found = track->title.startsWith ( query, Qt::CaseInsensitive );
       else
        found = track->title.contains ( query, Qt::CaseInsensitive );

       if(found && !titles.contains(track->title) && titles.count() < 5)
         titles << track->title;

       if(artists.count() == 5 && albums.count() == 5 && titles.count() == 5)
         break;
    }

    // make model item
    foreach (const QString& artist, artists) {
      PopupItem* item = new PopupItem(artist, QIcon(":/images/view-artist.png"));
      rootItem->appendRow(item);
    }

    foreach (const QString& album, albums) {
      PopupItem* item = new PopupItem(album, QIcon(":/images/album.png"));
      rootItem->appendRow(item);
    }

    foreach (const QString& title, titles) {
      PopupItem* item = new PopupItem(title, QIcon(":/images/track-48x48.png"));
      rootItem->appendRow(item);
    }
}


/*******************************************************************************
    getHistorySuggestions
*******************************************************************************/
void PopupModel::getHistorySuggestions(const QString & query)
{
    QStandardItem *rootItem = this->invisibleRootItem();

    QStringList artists;
    QStringList albums;
    QStringList titles;


    HistoModel* model = HistoModel::instance();
    for ( int i = 0; i < model->itemCount(); i++ )
    {
       MEDIA::TrackPtr track = model->trackAt(i);

       bool found = false;
       if(query.length() < 3)
        found = track->artist.startsWith ( query, Qt::CaseInsensitive );
       else
        found = track->artist.contains ( query, Qt::CaseInsensitive );

       if(found && !artists.contains(track->artist) && artists.count() < 5)
         artists << track->artist;

       found = false;
       if(query.length() < 3)
        found = track->album.startsWith ( query, Qt::CaseInsensitive );
       else
        found = track->album.contains ( query, Qt::CaseInsensitive );

       if(found && !albums.contains(track->album) && albums.count() < 5)
         albums << track->album;

       found = false;
       if(query.length() < 3)
        found = track->title.startsWith ( query, Qt::CaseInsensitive );
       else
        found = track->title.contains ( query, Qt::CaseInsensitive );

       if(found && !titles.contains(track->title) && titles.count() < 5)
         titles << track->title;

       if(artists.count() == 5 && albums.count() == 5 && titles.count() == 5)
         break;
    }

    // make model item
    foreach (const QString& artist, artists) {
      PopupItem* item = new PopupItem(artist, QIcon(":/images/view-artist.png"));
      rootItem->appendRow(item);
    }

    foreach (const QString& album, albums) {
      PopupItem* item = new PopupItem(album, QIcon(":/images/album.png"));
      rootItem->appendRow(item);
    }

    foreach (const QString& title, titles) {
      PopupItem* item = new PopupItem(title, QIcon(":/images/track-48x48.png"));
      rootItem->appendRow(item);
    }
}


/*******************************************************************************
    getStreamSuggestions
*******************************************************************************/
void PopupModel::getStreamSuggestions(const QString & query)
{
    QStandardItem *rootItem = this->invisibleRootItem();

    QStringList names;
    QStringList categories;

    // get data from model
    StreamModel* model = StreamModel::instance();

    for ( int i = 0; i < model->itemCount(); i++ )
    {
       MEDIA::TrackPtr stream = model->streamAt(i);

       bool found = false;
       if(query.length() < 3)
        found = stream->categorie.startsWith ( query, Qt::CaseInsensitive );
       else
        found = stream->categorie.contains ( query, Qt::CaseInsensitive );

       if(found && !categories.contains(stream->categorie) && categories.count() < 5)
         categories << stream->categorie;

       found = false;
       if(query.length() < 3)
        found = stream->name.startsWith ( query, Qt::CaseInsensitive );
       else
        found = stream->name.contains ( query, Qt::CaseInsensitive );

       if(found && !names.contains(stream->name) && names.count() < 5)
         names << stream->name;

        if(categories.count() == 5 && names.count() == 5)
          break;
    }

    // make model item
    foreach (const QString& category, categories) {
      PopupItem* item = new PopupItem(category, QIcon(":/images/genre.png"));
      rootItem->appendRow(item);
    }

    foreach (const QString& name, names) {
      PopupItem* item = new PopupItem(name, QIcon(":/images/media-url-18x18.png"));
      rootItem->appendRow(item);
    }
}
