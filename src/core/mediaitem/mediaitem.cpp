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
// local
#include "mediaitem.h"
#include "core/database/database.h"
#include "utilities.h"
#include "debug.h"
#include "tag.h"

// Qt
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlField>

#include <QVariant>
#include <QFileInfo>
#include <QByteArray>
#include <QBuffer>
#include <QUrl>
#include <QFile>
#include <QImage>
#include <QTime>
#include <QPixmap>
#include <QCryptographicHash>

const QStringList mediaFilter    = QStringList() << "mp3" << "ogg" << "flac" << "wav" << "m4a" << "aac" << "ape";
const QStringList playlistFilter = QStringList() << "m3u" << "m3u8" << "pls" << "xspf";

/*
********************************************************************************
*                                                                              *
*    Class Link                                                                *
*                                                                              *
********************************************************************************
*/
MEDIA::Link::Link() : Media()
{
    this->setType(TYPE_LINK);
    this->state = 0; /* SERVICE::NO_DATA */
};

/*
********************************************************************************
*                                                                              *
*    Class Album                                                               *
*                                                                              *
********************************************************************************
*/
MEDIA::Album::Album() : Media()
{
    this->setType(TYPE_ALBUM);

    id             = -1;
    playcount      =  0;
    rating         = -1.0;

    isFavorite     = false;
    isPlaying      = false;
    isUserRating   = false;
};

QString MEDIA::Album::yearToString() const
{
    if (year == -1)
      return QString::null;

    return QString::number(year);
}


QPixmap MEDIA::Album::pixmap() const
{
    return QPixmap(coverpath());
}


QString MEDIA::Album::coverpath() const
{
    return QString(UTIL::CONFIGDIR + "/albums/" +
             MEDIA::coverName(MEDIA::ArtistPtr::staticCast( parent() )->name, this->name));
}

QList<int> MEDIA::Album::dbIds()
{
    QList<int> db_ids;
    if(isMultiset())
      db_ids << ids;
    else
      db_ids << id;

    return db_ids;
}

QStringList MEDIA::Album::genres()
{
    QStringList result;

    for (int i = 0; i < this->childCount(); i++) {
      MEDIA::TrackPtr track = MEDIA::TrackPtr::staticCast( this->child(i) );
      if( !track->genre.isEmpty() &&  !result.contains( track->genre ) )
        result << track->genre;
    }
    
    return result;
}

/*
********************************************************************************
*                                                                              *
*    Class Artist                                                              *
*                                                                              *
********************************************************************************
*/
MEDIA::Artist::Artist() : Media()
{
    this->setType(TYPE_ARTIST);

    id             = -1;
    playcount      = 0;
    rating         = -1.0;

    isFavorite     = false;
    isPlaying      = false;
    isUserRating   = false;
};


QPixmap MEDIA::Artist::pixmap() const
{
    return QPixmap(coverpath());
}


QString MEDIA::Artist::coverpath() const
{
    return QString(UTIL::CONFIGDIR + "/artists/" + MEDIA::artistHash( this->name ));
}

/*
********************************************************************************
*                                                                              *
*    Class Track                                                               *
*                                                                              *
********************************************************************************
*/
MEDIA::Track::Track() : Media()
{
    this->setType(TYPE_TRACK);

    id           = -1;
    num          = -1;
    year         = -1;
    rating       = -1.0;
    playcount    =  0;
    lastPlayed   = -1;

    isFavorite   = false;
    isPlaying    = false;
    isBroken     = false;
    isPlayed     = false;
    isStopAfter  = false;
}

QString MEDIA::Track::yearToString() const
{
    if (year == -1)
      return QString::null;

    return QString::number(year);
}

QString MEDIA::Track::durationToString() const
{
    return UTIL::durationToString(this->duration);
}

QString MEDIA::Track::path(const QString& filename)
{
    if(MEDIA::isLocal(filename))
      return QFileInfo(filename).canonicalFilePath();
    else
      return QUrl(filename).toString();
}


QString MEDIA::Track::coverName() const
{
    return MEDIA::coverName(artist, album);
}

QString MEDIA::Track::lastplayed_ago() const
{
    const QDateTime now  = QDateTime::currentDateTime();
    const QDateTime then = QDateTime::fromTime_t(this->lastPlayed);

    const int days_ago   = then.date().daysTo(now.date());
    const QString s_then = then.toString(QLocale::system().dateFormat(QLocale::ShortFormat));

    if (days_ago == 0)
      return QObject::tr("Today") + " " + s_then;
    if (days_ago == 1)
      return QObject::tr("Yesterday") + " " + s_then;
    if (days_ago <= 7)
      return QObject::tr("%1 days ago").arg(days_ago) + " " + s_then;

    return s_then;
}


/*
********************************************************************************
*                                                                              *
*    Class Playlist                                                            *
*                                                                              *
********************************************************************************
*/
MEDIA::Playlist::Playlist() : Media()
{
    this->setType(TYPE_PLAYLIST);

    id           = -1;
    date         = -1;

    isPlaying    = false;
    isBroken     = false;
}


QString MEDIA::Playlist::dateToString() const
{
    if (date == -1)
      return QString::null;

    QDateTime datetime = QDateTime::fromTime_t(date);

    return datetime.toString ( "dd.MM.yyyy" );
}
/*
********************************************************************************
*                                                                              *
*    Class MEDIA::Media                                                        *
*                                                                              *
********************************************************************************
*/
MEDIA::Media::Media() : QSharedData()
{
    setType(TYPE_EMPTY);
}

MEDIA::Media::~Media()
{
    MEDIA::qResetAll( childItems );
    childItems.clear();
}

MEDIA::MediaPtr MEDIA::Media::child(int index) const
{
    if (index < 0 || index >= childItems.count())
        return MEDIA::MediaPtr(0);

    return childItems.value(index);
}

int MEDIA::Media::childCount() const
{
    return childItems.count();
}

int MEDIA::Media::childNumber() const
{
    return 0;
}

MEDIA::MediaPtr MEDIA::Media::addChildren(T_TYPE type)
{
    MEDIA::MediaPtr childItem = MEDIA::MediaPtr(0);
    switch(type) {
      case TYPE_TRACK   : childItem = MEDIA::TrackPtr( new MEDIA::Track() );break;
      case TYPE_ARTIST  : childItem = MEDIA::ArtistPtr( new MEDIA::Artist() );break;
      case TYPE_ALBUM   : childItem = MEDIA::AlbumPtr( new MEDIA::Album() );break;
      case TYPE_PLAYLIST: childItem = MEDIA::PlaylistPtr( new MEDIA::Playlist() );break;
      case TYPE_LINK    : childItem = MEDIA::LinkPtr( new MEDIA::Link() );break;
      default:  return childItem;
    }

    childItems.append(childItem);
    return childItem;
}

void MEDIA::Media::insertChildren(MEDIA::MediaPtr child)
{
    childItems.append(child);
}

bool MEDIA::Media::removeChildren(int idx)
{
    if (idx < 0 || idx >= childItems.size())
        return false;

    MEDIA::MediaPtr child = childItems.takeAt(idx);
    child.reset();
    delete child.data();
    
    return true;
}

void MEDIA::Media::deleteChildren()
{
    foreach(MEDIA::MediaPtr child, childItems) {
      child.reset();
      delete child.data();
    }
    childItems.clear();
}



/*
********************************************************************************
*                                                                              *
*    namespace MEDIA                                                           *
*                                                                              *
********************************************************************************
*/
void MEDIA::qResetAll(const QList<MEDIA::MediaPtr> mediaList)
{
    foreach(MEDIA::MediaPtr p, mediaList)
      p.reset();
}


float MEDIA::rating(const MediaPtr mi)
{
    if(mi->type() == TYPE_TRACK) {
      return MEDIA::TrackPtr::staticCast( mi )->rating;
    }
    else if(mi->type() == TYPE_ALBUM) {
      return MEDIA::AlbumPtr::staticCast( mi )->rating;
    }
    else if(mi->type() == TYPE_ARTIST) {
      return MEDIA::ArtistPtr::staticCast( mi )->rating;
    }
    else
      return 0.0;

}

/* ---------------------------------------------------------------------------*/
/* MEDIA::FromLocalFile                                                       */
/*     -> with file path                                                      */
/* ---------------------------------------------------------------------------*/
MEDIA::TrackPtr MEDIA::FromLocalFile(const QString url, int* p_disc)
{
    Debug::debug() << "[MEDIA] from local file : " << url;
  
    /* URL check */
    if(url.isEmpty()) {
      Debug::warning() <<  " Media Item from local file empty url : ";
      return MEDIA::MediaPtr(0);  
    }
  
    MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());

    media->id         = -1;
    media->url        = QFileInfo(url).absoluteFilePath().toUtf8();
    media->name       = QFileInfo(url).baseName();
    
    Tag::readFile(media, url, p_disc);
    
    return media;
}

/* ---------------------------------------------------------------------------*/
/* MEDIA::FromDataBase                                                        */
/*      -> with track url                                                     */
/* ---------------------------------------------------------------------------*/
MEDIA::TrackPtr MEDIA::FromDataBase(const QString url)
{
    //Debug::debug() << "[MEDIA] from db : " << url;

    /* search track info into database */
    QSqlQuery q("", *Database::instance()->db());
     q.prepare("SELECT id,filename,trackname," \
        "number,length,artist_name,genre_name,album_name,year,last_played," \
        "albumgain,albumpeakgain,trackgain,trackpeakgain,playcount,rating " \
        "FROM view_tracks WHERE filename=? LIMIT 1;");
    
    q.addBindValue( QFileInfo(url).canonicalFilePath() );
    q.exec();
    
    if (q.first()) 
    {
      MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());

      //Debug::debug() << "[MEDIA] Build MediaItem FROM DATABASE ok";
      media->id         =  q.value(0).toInt();
      media->url        =  url;
      media->name       =  q.value(1).toString();
      media->title      =  q.value(2).toString();
      media->num        =  q.value(3).toUInt();
      media->duration   =  q.value(4).toInt();
      media->artist     =  q.value(5).toString();
      media->genre      =  q.value(6).toString();
      media->album      =  q.value(7).toString();
      media->year       =  q.value(8).toUInt();
      media->lastPlayed =  q.value(9).toInt();
      media->albumGain  =  q.value(10).value<qreal>();
      media->albumPeak  =  q.value(11).value<qreal>();
      media->trackGain  =  q.value(12).value<qreal>();
      media->trackPeak  =  q.value(13).value<qreal>();
      media->playcount  =  q.value(14).toInt();
      media->rating     =  q.value(15).toFloat();

      //! default state value
      media->isPlaying    =  false;
      media->isBroken     =  false;
      media->isPlayed     =  false;
      media->isStopAfter  =  false;

      return media;
  }

  return MEDIA::TrackPtr(0);
}

/* ---------------------------------------------------------------------------*/
/* MEDIA::FromDataBase                                                        */
/*      -> with trackid                                                       */
/* ---------------------------------------------------------------------------*/
MEDIA::TrackPtr MEDIA::FromDataBase(int trackId)
{
  //Debug::debug() << "[MEDIA] from db -> track id " << trackId;

  /* search track info into database */
  QSqlQuery q("", *Database::instance()->db());
  q.prepare("SELECT id,filename,trackname," \
       "number,length,artist_name,genre_name,album_name,year,last_played," \
       "albumgain,albumpeakgain,trackgain,trackpeakgain,playcount,rating " \
       "FROM view_tracks WHERE id=? LIMIT 1;");
    
  q.addBindValue( QString::number(trackId) );
  q.exec();
  
  if (q.first()) 
  {
      MEDIA::TrackPtr media = MEDIA::TrackPtr(new MEDIA::Track());

      //Debug::debug() << "[MEDIA] Build MediaItem FROM DATABASE ok";
      media->id         =  q.value(0).toInt();
      media->url        =  q.value(1).toString();
      media->name       =  q.value(1).toString();
      media->title      =  q.value(2).toString();
      media->num        =  q.value(3).toUInt();
      media->duration   =  q.value(4).toInt();
      media->artist     =  q.value(5).toString();
      media->genre      =  q.value(6).toString();
      media->album      =  q.value(7).toString();
      media->year       =  q.value(8).toUInt();
      media->lastPlayed =  q.value(9).toInt();
      media->albumGain  =  q.value(10).value<qreal>();
      media->albumPeak  =  q.value(11).value<qreal>();
      media->trackGain  =  q.value(12).value<qreal>();
      media->trackPeak  =  q.value(13).value<qreal>();
      media->playcount  =  q.value(14).toInt();
      media->rating     =  q.value(15).toFloat();

      //! default state value
      media->isPlaying    =  false;
      media->isBroken     =  false;
      media->isPlayed     =  false;
      media->isStopAfter  =  false;

      return media;
  }

  return MEDIA::TrackPtr(0);
}

/* ---------------------------------------------------------------------------*/
/* MEDIA::LoadImageFromFile                                                   */
/* ---------------------------------------------------------------------------*/
QImage MEDIA::LoadImageFromFile(const QString& filename, QSize size)
{
    QImage image = QImage::fromData( Tag::loadCoverByteArrayFromFile(filename) );

    if(image.isNull())
      return QImage();
    
    if(size != image.size()) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  
    return image;
}

/* ---------------------------------------------------------------------------*/
/* MEDIA::LoadCoverFromFile                                                   */
/* ---------------------------------------------------------------------------*/
QPixmap MEDIA::LoadCoverFromFile(const QString& filename, QSize size)
{
    QImage image = QImage::fromData( Tag::loadCoverByteArrayFromFile(filename) );

    if(image.isNull())
      return QPixmap();
    
    if(size != image.size()) {
        image = image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
  
    return QPixmap::fromImage( image );
}

/* ---------------------------------------------------------------------------*/
/* MEDIA::isLocal                                                             */
/* ---------------------------------------------------------------------------*/
bool MEDIA::isLocal(const QString& url)
{
    if (url.contains(QRegExp("^[a-z]{3,}:"))) {
      if (QUrl(url).scheme() == "file")
        return true;
      else
        return false;
    }
    return true;
}

/* ---------------------------------------------------------------------------*/
/* MEDIA commun filters                                                       */
/* ---------------------------------------------------------------------------*/
bool MEDIA::isAudioFile(const QString& url)
{
    return mediaFilter.contains(QFileInfo(url).suffix().toLower());
}

bool MEDIA::isPlaylistFile(const QString& url)
{
    return playlistFilter.contains(QFileInfo(url).suffix().toLower());
}

bool MEDIA::isCueFile(const QString& url)
{
    const QStringList cue_extension = QStringList() << "cue";
    return cue_extension.contains(QFileInfo(url).suffix().toLower());
}

bool MEDIA::isMediaPlayable(const QString& url)
{
    return 
      /* not a playlist */            
      !playlistFilter.contains(QFileInfo(url).suffix().toLower()) &&
      /* not a shoutcast type .pls?<id> */
      !url.contains(".pls?") &&
      /* not a tune in type */
      !url.contains("ashx?id");  
}

/* ---------------------------------------------------------------------------*/
/* MEDIA compare/sorting utilities                                            */
/* ---------------------------------------------------------------------------*/
bool MEDIA::compareTrackNatural(const TrackPtr mi1, const TrackPtr mi2)
{
    QString s1, s2; 
    if( mi1->num != 0 && mi2->num != 0)
    {
      s1 = mi1->artist + mi1->album + QString::number(mi1->num).rightJustified(4, '0');
      s2 = mi2->artist + mi2->album + QString::number(mi2->num).rightJustified(4, '0');
    }
    else
    {
      s1 = mi1->artist + mi1->album + mi1->url;
      s2 = mi2->artist + mi2->album + mi2->url;
    }
    return (s1 < s2);
}


bool MEDIA::compareTrackItemGenre(const TrackPtr mi1, const TrackPtr mi2)
{
    const QString s1 = mi1->genre + mi1->album + QString::number(mi1->disc_number);
    const QString s2 = mi2->genre + mi2->album + QString::number(mi2->disc_number);
    
    return QString::compare(s1, s2, Qt::CaseInsensitive) < 0;
}

bool MEDIA::compareAlbumItemYear(const AlbumPtr mi1, const AlbumPtr mi2)
{
    return mi1->year > mi2->year;
}

bool MEDIA::compareStreamName(const TrackPtr mi1, const TrackPtr mi2)
{
    return mi1->name.toLower() < mi2->name.toLower();
}

bool MEDIA::compareStreamCategorie(const TrackPtr mi1, const TrackPtr mi2)
{
    return mi1->genre.toLower() <= mi2->genre.toLower();
}

bool MEDIA::compareAlbumItemPlaycount(const AlbumPtr mi1, const AlbumPtr mi2)
{
    return mi1->playcount > mi2->playcount;
}

bool MEDIA::compareArtistItemPlaycount(const ArtistPtr mi1, const ArtistPtr mi2)
{
    return mi1->playcount > mi2->playcount;
}

bool MEDIA::compareAlbumItemRating(const AlbumPtr mi1, const AlbumPtr mi2)
{
   return mi1->rating > mi2->rating;
}

bool MEDIA::compareArtistItemRating(const ArtistPtr mi1, const ArtistPtr mi2)
{
   return mi1->rating > mi2->rating;
}


/* ---------------------------------------------------------------------------*/
/* MEDIA::registerTrackPlaying                                                */
/* ---------------------------------------------------------------------------*/
void MEDIA::registerTrackPlaying(MEDIA::TrackPtr tk, bool isPlaying) 
{
    //Debug::debug() << "[MEDIA] registerTrackPlaying " << isPlaying;
    MEDIA::MediaPtr media = tk;
    
    do
    {
      switch (media->type()) {
        case TYPE_ARTIST   : MEDIA::ArtistPtr::staticCast(media)->isPlaying   = isPlaying; break;
        case TYPE_ALBUM    : MEDIA::AlbumPtr::staticCast(media)->isPlaying    = isPlaying; break;
        case TYPE_PLAYLIST : MEDIA::PlaylistPtr::staticCast(media)->isPlaying = isPlaying; break;
        case TYPE_STREAM   :
        case TYPE_TRACK    : MEDIA::TrackPtr::staticCast(media)->isPlaying    = isPlaying; 
                             
                             if(isPlaying) {
                               MEDIA::TrackPtr::staticCast(media)->isPlayed     = true; 
                               MEDIA::TrackPtr::staticCast(media)->isBroken     = false;
                             }
                             break;
        default: break;
      }
    
      media  = media->parent();
    } while(media);
}

/* ---------------------------------------------------------------------------*/
/* MEDIA::registerTrackBroken                                                 */
/* ---------------------------------------------------------------------------*/
void MEDIA::registerTrackBroken(MEDIA::TrackPtr tk, bool isBroken)
{
    MEDIA::MediaPtr media = tk;
    
    do
    {
      if (media->type() == TYPE_STREAM || media->type() == TYPE_TRACK ) 
      {
           MEDIA::TrackPtr::staticCast(media)->isBroken     = isBroken; 
           MEDIA::TrackPtr::staticCast(media)->isPlaying    = false; 
      }
    
      media  = media->parent();
    } while(media);
}
