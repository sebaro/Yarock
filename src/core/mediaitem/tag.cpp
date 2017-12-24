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
#include "tag.h"
#include "mediaitem.h"
#include "debug.h"

// Qt
#include <QFile>
#include <QFileInfo>

// taglib
#include <taglib/fileref.h>
#include <taglib/tstring.h>

#include <taglib/mpegfile.h>
#include <taglib/asffile.h>
#include <taglib/vorbisfile.h>
#include <taglib/oggfile.h>
#include <taglib/oggflacfile.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>

#include <taglib/tag.h>
#include <taglib/id3v1tag.h>
#include <taglib/id3v2tag.h>
#include <taglib/xiphcomment.h>
#include <taglib/apetag.h>
#include <taglib/asftag.h>

#include <taglib/textidentificationframe.h>
#include <taglib/popularimeterframe.h>

// Taglib added support for FLAC pictures in 1.7.0
#if (TAGLIB_MAJOR_VERSION > 1) || (TAGLIB_MAJOR_VERSION == 1 && TAGLIB_MINOR_VERSION >= 7)
# define TAGLIB_HAS_FLAC_PICTURELIST
#endif

// system
#include <sys/stat.h> // for time stamps update if file write
#include <cmath> // for log10

#define NumberToASFAttribute(x) TagLib::ASF::Attribute(QStringToTaglibString(QString::number(x)))

const char* kMP4_FMPS_Rating_ID    = "----:com.apple.iTunes:FMPS_Rating";
const char* kMP4_FMPS_Playcount_ID = "----:com.apple.iTunes:FMPS_Playcount";
const char* kMP4_FMPS_Score_ID     = "----:com.apple.iTunes:FMPS_Rating_Amarok_Score";

/*
********************************************************************************
*                                                                              *
*    namespace Tag                                                             *
*                                                                              *
********************************************************************************
*/
namespace Tag
{

/*----------------------------------------------------------------------------*/
/* String convert                                                             */
/*----------------------------------------------------------------------------*/
static TagLib::String QStringToTaglibString (const QString &str)
{
    return TagLib::String(str.toUtf8().data(), TagLib::String::UTF8);
}

static QString TaglibStringToQString (const TagLib::String &str)
{
    return QString::fromUtf8(str.toCString(true)).trimmed();
}


/*----------------------------------------------------------------------------*/
/* ReplayGain                                                                 */
/*    peakToDecibels                                                          */
/*    maybeAddPeak                                                            */
/*    maybeAddGain                                                            */
/*----------------------------------------------------------------------------*/
// converts a peak value from the normal digital scale form to the more useful decibel form
// decibels are relative to the /adjusted/ waveform
static qreal peakToDecibels( qreal scaleVal )
{
    if ( scaleVal > 0 )
        return 20 * log10( scaleVal );
    else
        return 0;
}

// adds the converted version of the scale value if it is a valid, non-negative float
static float maybeAddPeak( const TagLib::String &scaleVal )
{
    // scale value is >= 0, and typically not much bigger than 1
    QString value = TStringToQString( scaleVal );
    bool ok = false;
    qreal peak = value.toFloat( &ok );
    if ( ok && peak >= 0 )
      return peakToDecibels( peak );

    return 0.0;
}

static float maybeAddGain( const TagLib::String &input )
{
    QString value = TStringToQString( input ).remove( " dB" );
    bool ok = false;
    qreal gain = value.toFloat( &ok );
    if (ok)
      return gain;
    
    return 0.0;
}

/*----------------------------------------------------------------------------*/
/* convertPOPMRating / convertToPOPMRating                                    */
/*  (from clementine)                                                         */
/*----------------------------------------------------------------------------*/
// static float convertPOPMRating(const int POPM_rating) 
// {
//   if (POPM_rating < 0x01) {
//     return 0.0;
//   } else if (POPM_rating < 0x40) {
//     return 0.20; // 1 star
//   } else if (POPM_rating < 0x80) {
//     return 0.40; // 2 stars
//   } else if (POPM_rating < 0xC0) {
//     return 0.60; // 3 stars
//   } else if (POPM_rating < 0xFC) { // some players store 5 stars as 0xFC
//     return 0.80; // 4 stars
//   }
//   return 1.0; // 5 stars
// }

static int convertToPOPMRating(const float rating) 
{
  if (rating < 0.20) {
    return 0x00;
  } else if (rating < 0.40) {
    return 0x01;
  } else if (rating < 0.60) {
    return 0x40;
  } else if (rating < 0.80) {
    return 0x80;
  } else if (rating < 1.0) {
    return 0xC0;
  }
  return 0xFF;
}


/*----------------------------------------------------------------------------*/
/* init                                                                       */
/*----------------------------------------------------------------------------*/
static void init(MEDIA::TrackPtr track)
{
    track->id         =  -1;
    /*track->url        =   do not clear mediaitem set it already */
    /*track->name       =   do not clear mediaitem set it already */
    track->title      =   QString();
    track->num        =   0;
    track->duration   =   0;
    track->artist     =   QString();
    track->genre      =   QString();
    track->album      =   QString();
    track->year       =   0;
    track->lastPlayed =  -1;
    track->albumGain  =   0.0;
    track->albumPeak  =   0.0;
    track->trackGain  =   0.0;
    track->trackPeak  =   0.0;
    track->playcount  =   0;
    track->rating     =  -1.0;

    /* default state value */
    track->isPlaying    =  false;
    track->isBroken     =  false;
    track->isPlayed     =  false;
    track->isStopAfter  =  false;    
}


/*----------------------------------------------------------------------------*/
/* readID3v2Tags                                                              */
/*----------------------------------------------------------------------------*/
static void readID3v2Tags(TagLib::ID3v2::Tag *tag, MEDIA::TrackPtr track, QString& s_disc )
{
    Debug::debug() << " [Tag] readID3v2Tags";  
    
    const TagLib::ID3v2::FrameListMap& map = tag->frameListMap();

    if ( !map["TPOS"].isEmpty() )
        s_disc = TaglibStringToQString(map["TPOS"].front()->toString());

    if ( !map["TPE2"].isEmpty() ) // non-standard: Apple, Microsoft
        track->artist = TaglibStringToQString( map["TPE2"].front()->toString() );

    if ( !map["TBPM"].isEmpty() )
        track->extra["bpm"] = TaglibStringToQString(map["TBPM"].front()->toString()).trimmed().toFloat();


    /* read FMPS frames */
    for (int i=0 ; i < map["TXXX"].size() ; ++i) 
    {
        const TagLib::ID3v2::UserTextIdentificationFrame* frame =
          dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(map["TXXX"][i]);

        if (frame && frame->description().startsWith("FMPS_")) 
        {
          TagLib::StringList fields = frame->fieldList();

          if( fields.size() >= 2 )
          {
              QString value = TaglibStringToQString( fields[1] );

              if( fields[0] == TagLib::String("FMPS_Rating") )
                track->rating = value.toFloat();
              else if( fields[0] == TagLib::String("FMPS_Playcount") )
                track->playcount = value.toInt();
          }
        }
    }
      
    /* read POPM tags */        
    if ( !map["POPM"].isEmpty() ) 
    {
        const TagLib::ID3v2::PopularimeterFrame* frame =
          dynamic_cast<const TagLib::ID3v2::PopularimeterFrame*>( map["POPM"].front() );
        
        if (frame) 
        {
          /* only read anonymous ratings */
          if( TaglibStringToQString( frame->email() ).isEmpty() )
          {
              if( track->rating <= 0 && frame->rating() > 0 )
                track->rating = qRound( frame->rating() / 256.0 );
              if( track->playcount <= 0 && frame->counter() > 0 )
                track->playcount = frame->counter();
          }
        }
    }        
    
    /* read ID3v2.3.0 tags replay gain tags */
    bool albumTagOK = false;
    for (int i=0 ; i < map["TXXX"].size() ; ++i) 
    {
        const TagLib::ID3v2::UserTextIdentificationFrame* frame =
          dynamic_cast<const TagLib::ID3v2::UserTextIdentificationFrame*>(map["TXXX"][i]);

        if ( frame && frame->fieldList().size() >= 2 )
        {
            QString desc = TStringToQString( frame->description() ).toLower();
            if ( desc == "replaygain_album_gain" || desc == "replaygain_album_peak")
                albumTagOK = true;
            if ( desc == "replaygain_album_gain" )
                track->albumGain = maybeAddGain( frame->fieldList()[1] );
            if ( desc == "replaygain_album_peak" )
                track->albumPeak = maybeAddPeak( frame->fieldList()[1] );
            if ( desc == "replaygain_track_gain" )
                track->trackGain = maybeAddGain( frame->fieldList()[1] );
            if ( desc == "replaygain_track_peak" )
                track->trackPeak = maybeAddPeak( frame->fieldList()[1] );
        }
    }
    
    if ( ! albumTagOK ) 
    {
      track->albumGain = track->trackGain;
      track->albumPeak = track->trackPeak;
    }
    
    /* TODO ID3v2.4.0 RVA2 frame */
}


/*----------------------------------------------------------------------------*/
/* readMP4Tags                                                                */
/*----------------------------------------------------------------------------*/
static void readMP4Tags(TagLib::MP4::Tag *tag, MEDIA::TrackPtr track, QString& s_disc )
{
    const TagLib::MP4::ItemListMap& items = tag->itemListMap();

    /* album artists tags */
    TagLib::MP4::ItemListMap::ConstIterator it = items.find("aART");
    if ( it != items.end() ) 
    {
        TagLib::StringList album_artists = it->second.toStringList();
        if ( !album_artists.isEmpty() )
          track->artist = TaglibStringToQString( album_artists.front() );
    }    
    
    /* disc number */
    if ( items.contains("disk") ) {
        s_disc = TaglibStringToQString(TagLib::String::number(items["disk"].toIntPair().first));
    }    
    
    /* rating */
    if ( items.contains(kMP4_FMPS_Rating_ID) ) {
        float rating = TaglibStringToQString(items[kMP4_FMPS_Rating_ID].toStringList().toString('\n')).toFloat();
        
        if( track->rating <= 0 && rating > 0 )
          track->rating = rating;
    }
    
    /* playcount */    
    if ( items.contains(kMP4_FMPS_Playcount_ID) ) {
        int playcount = TaglibStringToQString(items[kMP4_FMPS_Playcount_ID].toStringList().toString('\n')).toFloat();
        
        if( track->playcount <= 0 && playcount > 0 )
                track->playcount = playcount;
    }
}


/*----------------------------------------------------------------------------*/
/* readOggTags                                                                */
/*----------------------------------------------------------------------------*/
static void readOggTags(TagLib::Ogg::XiphComment *tag, MEDIA::TrackPtr track, QString& s_disc )
{
    Debug::debug() << " [Tag] readOggTags";  
  
    const TagLib::Ogg::FieldListMap& map = tag->fieldListMap();
  
    if (!map["ALBUMARTIST"].isEmpty()) 
    {
        track->artist = TaglibStringToQString( map["ALBUMARTIST"].front() );
    }
    else if (!map["ALBUM ARTIST"].isEmpty()) 
    {
        track->artist = TaglibStringToQString( map["ALBUM ARTIST"].front() );
    }

    if ( !map["BPM"].isEmpty() )
        track->extra["bpm"] = TaglibStringToQString( map["BPM"].front() ).trimmed().toFloat();
  
    if (!map["DISCNUMBER"].isEmpty() )
        s_disc = TaglibStringToQString( map["DISCNUMBER"].front() );
    
    if (!map["FMPS_RATING"].isEmpty() && track->rating <= 0)
        track->rating = TaglibStringToQString( map["FMPS_RATING"].front() ).toFloat();

    if (!map["FMPS_PLAYCOUNT"].isEmpty() && track->playcount <= 0)
        track->playcount = TaglibStringToQString( map["FMPS_PLAYCOUNT"].front() ).toFloat();
    
    /* get replay gain */
    if ( !map["REPLAYGAIN_TRACK_GAIN"].isEmpty() )
        track->trackGain = maybeAddGain( map["REPLAYGAIN_TRACK_GAIN"].front() );
    if ( !map["REPLAYGAIN_TRACK_PEAK"].isEmpty() )
        track->trackPeak = maybeAddPeak( map["REPLAYGAIN_TRACK_PEAK"].front() );

    if ( !map["REPLAYGAIN_ALBUM_GAIN"].isEmpty() )
        track->albumGain = maybeAddGain( map["REPLAYGAIN_ALBUM_GAIN"].front() );
    else
        track->albumGain = track->trackGain; 

    if ( !map["REPLAYGAIN_ALBUM_PEAK"].isEmpty() )
        track->albumPeak = maybeAddPeak( map["REPLAYGAIN_ALBUM_PEAK"].front() );
    else
        track->albumPeak = track->trackPeak; 
}

/*----------------------------------------------------------------------------*/
/* readAPETags                                                                */
/*----------------------------------------------------------------------------*/
static void readAPETags(TagLib::APE::Tag *tag, MEDIA::TrackPtr track, QString& s_disc )
{
Q_UNUSED(s_disc)  
    Debug::debug() << " [Tag] readAPETags";  

    const TagLib::APE::ItemListMap &map = tag->itemListMap();

    if ( map.contains("FMPS_RATING") )
    {
      track->rating = TaglibStringToQString( map["FMPS_RATING"].values()[0] ).toFloat();
    }

    if ( map.contains("FMPS_PLAYCOUNT") )
    {
      track->playcount = TaglibStringToQString( map["FMPS_PLAYCOUNT"].values()[0] ).toInt();
    }  
    
    /* get replay gain */
    if ( map.contains("REPLAYGAIN_TRACK_GAIN") )
        track->trackGain = maybeAddGain( map["REPLAYGAIN_TRACK_GAIN"].values()[0] );
    if ( map.contains("REPLAYGAIN_TRACK_PEAK") )
        track->trackPeak = maybeAddPeak( map["REPLAYGAIN_TRACK_PEAK"].values()[0] );

    if ( map.contains("REPLAYGAIN_ALBUM_GAIN") )
        track->albumGain  = maybeAddGain( map["REPLAYGAIN_ALBUM_GAIN"].values()[0] );
    else
        track->albumGain = track->trackGain;

    if ( map.contains("REPLAYGAIN_ALBUM_PEAK") )
        track->albumPeak = maybeAddPeak( map["REPLAYGAIN_ALBUM_PEAK"].values()[0] );
    else
        track->albumPeak = track->trackPeak;
}

/*----------------------------------------------------------------------------*/
/* readASFTags                                                                */
/*----------------------------------------------------------------------------*/
static void readASFTags(TagLib::ASF::Tag *tag, MEDIA::TrackPtr track, QString& s_disc )
{
Q_UNUSED(s_disc)  
    Debug::debug() << " [Tag] readASFTags";  

    const TagLib::ASF::AttributeListMap& map = tag->attributeListMap();
    
    if (map.contains("FMPS/Rating")) 
    {
      const TagLib::ASF::AttributeList& attributes = map["FMPS/Rating"];
      if (!attributes.isEmpty()) {
        float rating = TaglibStringToQString(attributes.front().toString()).toFloat();
        if (track->rating <= 0 && rating > 0) {
          track->rating = rating;
        }
      }
    }
    
    if (map.contains("FMPS/Playcount")) 
    {
      const TagLib::ASF::AttributeList& attributes = map["FMPS/Playcount"];
      if (!attributes.isEmpty()) {
        int playcount = TaglibStringToQString(attributes.front().toString()).toInt();
        if (track->playcount <= 0 && playcount > 0) {
          track->playcount = playcount;
        }
      }
    }

    if (map.contains("REPLAYGAIN_TRACK_GAIN") && !map["REPLAYGAIN_TRACK_GAIN"].isEmpty()) 
        track->trackGain = maybeAddGain( map["REPLAYGAIN_TRACK_GAIN"].front().toString() );
    if ( map.contains("REPLAYGAIN_TRACK_PEAK") && !map["REPLAYGAIN_TRACK_PEAK"].isEmpty() )
        track->trackPeak = maybeAddPeak( map["REPLAYGAIN_TRACK_PEAK"].front().toString() );
    
    if ( map.contains("REPLAYGAIN_ALBUM_GAIN") && !map["REPLAYGAIN_ALBUM_GAIN"].isEmpty() )
        track->albumGain = maybeAddGain( map["REPLAYGAIN_ALBUM_GAIN"].front().toString() );
    else
        track->albumGain = track->trackGain;

    if ( map.contains("REPLAYGAIN_ALBUM_PEAK") && !map["REPLAYGAIN_ALBUM_PEAK"].isEmpty() )
        track->albumPeak = maybeAddGain( map["REPLAYGAIN_ALBUM_PEAK"].front().toString() );
    else
        track->albumPeak = track->trackPeak;
}


/*----------------------------------------------------------------------------*/
/* Reader                                                                     */
/*----------------------------------------------------------------------------*/
void readFile(MEDIA::TrackPtr track, const QString& url, int* p_disc)
{
    Debug::debug() << " [Tag] readFile";
    
    QString s_disc;
    
    /* initial value */
    init(track);
    
    /* get taglib fileref */
    #ifdef COMPLEX_TAGLIB_FILENAME
        const wchar_t *encodedName = reinterpret_cast< const wchar_t * >( QFileInfo(url).canonicalFilePath().utf16() );
    #else
        QByteArray fileName = QFile::encodeName( QFileInfo(url).canonicalFilePath() );
        const char *encodedName = fileName.constData();
    #endif
    
    if (!encodedName) {
      Debug::error() << "media item -> encoded path error :" << url;
      track->isBroken   = true;
      return;
    }

    //! TagLib reference
    TagLib::FileRef fileref = TagLib::FileRef(encodedName, true);
    if (fileref.isNull()) {
      Debug::warning() << "media item -> taglib access failed :" << url;
      track->isBroken   = true;
      return;
    }
    

    /* read basic tag */
    TagLib::Tag* tag = fileref.tag();
    if ( tag ) {
        track->title      = TaglibStringToQString( tag->title() );
        track->artist     = TaglibStringToQString( tag->artist() );
        track->album      = TaglibStringToQString( tag->album() );
        track->genre      = TaglibStringToQString( tag->genre() );
        track->num        = tag->track();
        track->year       = tag->year();
    }    

    /* duration reading */
    TagLib::AudioProperties *audioProperties = fileref.audioProperties();
    if (audioProperties) {
        track->duration            = audioProperties->length(); // Returns the length of the file in seconds
        track->extra["bitrate"]    = audioProperties->bitrate();
        track->extra["samplerate"] = audioProperties->sampleRate();
    }


    /* read tags */
    /* -> handle all files which have VorbisComments (Ogg, OPUS, ...) */
    if (TagLib::Ogg::XiphComment* tag = dynamic_cast<TagLib::Ogg::XiphComment*>(fileref.file()->tag())) 
    {
        readOggTags( tag, track, s_disc );
    }
    
    if ( TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File *>( fileref.file() ) )
    {
        /* read IDV3v2 tag first, only try APE if no IDV3v2 */
        if ( file->ID3v2Tag() )
            readID3v2Tags( file->ID3v2Tag(), track, s_disc );
        else if ( file->APETag() )
            readAPETags( file->APETag(), track, s_disc );
    }
    else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>( fileref.file() )) 
    {
        if ( file->xiphComment() )
            readOggTags( file->xiphComment(), track, s_disc );
        else if ( file->ID3v2Tag() )
            readID3v2Tags( file->ID3v2Tag(), track, s_disc );
    }
    else if (TagLib::MP4::File* file = dynamic_cast<TagLib::MP4::File*>( fileref.file() ))
    {
        if ( file->tag() )
            readMP4Tags( file->tag(), track, s_disc );
    }
    else if (TagLib::ASF::File* file = dynamic_cast<TagLib::ASF::File*>( fileref.file() ))
    {
        if ( file->tag() )
            readASFTags( file->tag(), track, s_disc );
    }
    
    /* return disc number */
    if(p_disc != 0) 
    {
      *p_disc = 0;
      
      if ( !s_disc.isEmpty() ) 
      {
        int i = s_disc.indexOf('/');
        if ( i != -1 ) {
          *p_disc      = s_disc.left( i ).toInt();
           if( s_disc.right( i ).toInt() == 1)
            *p_disc = 0;
        }
        else
          *p_disc = s_disc.toInt();
      
        //Debug::debug() << " [Tag] Item from local file disc number: " << *p_disc;
      }
    }
    
    /* ----- post traitement ----- */
    if ( track->title.isEmpty() )
      track->title = QFileInfo(url).baseName();

    if( track->artist.isEmpty() )
      track->artist = QObject::tr("unknown artist");

    if( track->album.isEmpty() )
      track->album  = QObject::tr("unknown album");

    if( track->genre.isEmpty() )
      track->genre  = QObject::tr("unknown genre");

    if( track->extra["bitrate"].toInt() < 0 )
      track->extra["bitrate"] = 0;

    if( track->extra["samplerate"].toInt() < 0 )
      track->extra["samplerate"] = 0;
}


/*----------------------------------------------------------------------------*/
/* writeTrackRatingToFile                                                     */
/*----------------------------------------------------------------------------*/
bool writeTrackRatingToFile(const QString& url, float rating)
{
    if (url.isNull())
      return false;

    Debug::debug() << " [Tag] Saving song rating tags to" << url;

    #ifdef COMPLEX_TAGLIB_FILENAME
      const wchar_t *encodedName = reinterpret_cast< const wchar_t * >( QFileInfo(url).canonicalFilePath().utf16() );
    #else
      QByteArray fileName = QFile::encodeName( QFileInfo(url).canonicalFilePath() );
      const char *encodedName = fileName.constData();
    #endif
    
    if (!encodedName) {
      Debug::warning() << " [Tag] encoded path error :" << url;
      return false;
    }

    /* TagLib reference */
    TagLib::FileRef fileref = TagLib::FileRef( encodedName, true);
    if (fileref.isNull()) {
      Debug::warning() << " [Tag] taglib access failed :" << url;
      return false;
    }

    /* write tags */
    if (TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(fileref.file())) 
    {
      TagLib::ID3v2::Tag* tag = file->ID3v2Tag(true);

      /* update FMPS frame */
        const TagLib::String name  = QStringToTaglibString( "FMPS_Rating" );
        const TagLib::String value = QStringToTaglibString( QString::number( rating ) );

        TagLib::ID3v2::UserTextIdentificationFrame* frame = TagLib::ID3v2::UserTextIdentificationFrame::find(tag, name);

        if (frame)
          tag->removeFrame(frame);

        frame = new TagLib::ID3v2::UserTextIdentificationFrame(TagLib::String::UTF8);
        frame->setDescription( name );
        frame->setText( value );
        tag->addFrame(frame);
    
      /* update POPM frame */
        TagLib::ID3v2::PopularimeterFrame* popframe = NULL;

        const TagLib::ID3v2::FrameListMap& map = tag->frameListMap();
        if ( !map["POPM"].isEmpty() )
          popframe = dynamic_cast<TagLib::ID3v2::PopularimeterFrame*>(map["POPM"].front());

        if ( !popframe ) {
          popframe = new TagLib::ID3v2::PopularimeterFrame();
          tag->addFrame(popframe);
        }

        popframe->setRating( convertToPOPMRating( rating ) );
    }
    else if (TagLib::FLAC::File* file = dynamic_cast<TagLib::FLAC::File*>(fileref.file())) 
    {
      TagLib::Ogg::XiphComment* vorbis_comments = file->xiphComment(true);
      
      vorbis_comments->addField("FMPS_RATING", QStringToTaglibString(QString::number( rating )));
    }
    else if (TagLib::Ogg::XiphComment* tag = dynamic_cast<TagLib::Ogg::XiphComment*>(fileref.file()->tag())) 
    {
      tag->addField("FMPS_RATING", QStringToTaglibString(QString::number( rating )));
    }
    else if (TagLib::ASF::File* file = dynamic_cast<TagLib::ASF::File*>(fileref.file())) 
    {
      TagLib::ASF::Tag* tag = file->tag();
      tag->addAttribute("FMPS/Rating",    NumberToASFAttribute( rating ));
    }
    else if (TagLib::MP4::File* file = dynamic_cast<TagLib::MP4::File*>(fileref.file())) 
    {
      TagLib::MP4::Tag* tag = file->tag();
      tag->itemListMap()[kMP4_FMPS_Rating_ID] = TagLib::StringList(QStringToTaglibString(QString::number( rating )));
    }
    else 
    {
      return true;
    }

    bool ret = fileref.save();

    if (ret) {
       /* Linux: inotify doesn't seem to notice the change to the file unless we
          change the timestamps as well. (this is what touch does) */
       utimensat(0, QFile::encodeName(url).constData(), NULL, 0);
    }
    
    return ret;
}

/*----------------------------------------------------------------------------*/
/* loadCoverByteArrayFromFile                                                 */
/*----------------------------------------------------------------------------*/
QByteArray loadCoverByteArrayFromFile(const QString& filename)
{
    if (filename.isEmpty())
      return QByteArray();
    
    TagLib::FileRef ref(QFile::encodeName(filename).constData());

    if (ref.isNull() || !ref.file())
      return QByteArray();
    
    /*-----------------------------------------------------------*/
    /* MP3                                                       */
    /* ----------------------------------------------------------*/    
    TagLib::MPEG::File* file = dynamic_cast<TagLib::MPEG::File*>(ref.file());
    if (file && file->ID3v2Tag())
    {      
      TagLib::ID3v2::FrameList apic_frames = file->ID3v2Tag()->frameListMap()["APIC"];
      if (apic_frames.isEmpty())
        return QByteArray();

      if (apic_frames.size() != 1) 
      {
        TagLib::ID3v2::FrameList::Iterator it = apic_frames.begin();
        for (; it != apic_frames.end(); ++it) 
        {
            // This must be dynamic_cast<>, TagLib will return UnknownFrame in APIC for encrypted frames.
            TagLib::ID3v2::AttachedPictureFrame *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it);

            // Both thumbnail and full size should use FrontCover, as FileIcon may be too small even for thumbnail.
            if (frame && frame->type() != TagLib::ID3v2::AttachedPictureFrame::FrontCover)
              continue;

            return QByteArray((const char*) frame->picture().data(), frame->picture().size());
        }
      }

      // If we get here we failed to pick a picture, or there was only one, so just use the first picture.
      TagLib::ID3v2::AttachedPictureFrame *frame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame *>(apic_frames.front());
      return QByteArray((const char*) frame->picture().data(), frame->picture().size());
    }

    /*-----------------------------------------------------------*/
    /* Ogg vorbis/speex                                          */
    /* ----------------------------------------------------------*/    
    TagLib::Ogg::XiphComment* xiph_comment = dynamic_cast<TagLib::Ogg::XiphComment*>(ref.file()->tag());

    if (xiph_comment)
    {
      TagLib::Ogg::FieldListMap map = xiph_comment->fieldListMap();

      // Ogg lacks a definitive standard for embedding cover art, but it seems
      // b64 encoding a field called COVERART is the general convention
      if (!map.contains("COVERART"))
        return QByteArray();

      return QByteArray::fromBase64(map["COVERART"].toString().toCString());
    }
  

    /*-----------------------------------------------------------*/
    /* FLAC                                                      */
    /* ----------------------------------------------------------*/
#ifdef TAGLIB_HAS_FLAC_PICTURELIST
    TagLib::FLAC::File* flac_file = dynamic_cast<TagLib::FLAC::File*>(ref.file());
    if (flac_file && flac_file->xiphComment()) 
    {
        const TagLib::List<TagLib::FLAC::Picture*> picturelist = flac_file->pictureList();
        for( TagLib::List<TagLib::FLAC::Picture*>::ConstIterator it = picturelist.begin(); it != picturelist.end(); it++ )
        {
            TagLib::FLAC::Picture* picture = *it;

            if( ( picture->type() == TagLib::FLAC::Picture::FrontCover || picture->type() == TagLib::FLAC::Picture::Other ))
            {
              return QByteArray(picture->data().data(), picture->data().size());
            }
        }
    }
#endif  

    /*-----------------------------------------------------------*/
    /* MP4/AAC                                                   */
    /* ----------------------------------------------------------*/
    TagLib::MP4::File* aac_file = dynamic_cast<TagLib::MP4::File*>(ref.file());
    if (aac_file) 
    {
      TagLib::MP4::Tag* tag = aac_file->tag();
      const TagLib::MP4::ItemListMap& items = tag->itemListMap();
      TagLib::MP4::ItemListMap::ConstIterator it = items.find("covr");
      if (it != items.end()) 
      {
        const TagLib::MP4::CoverArtList& art_list = it->second.toCoverArtList();

        if (!art_list.isEmpty()) 
        {
          // Just take the first one for now
          const TagLib::MP4::CoverArt& art = art_list.front();
          return QByteArray(art.data().data(), art.data().size());
        }
      }
    }

    return QByteArray();
}

    
} // end namespace
