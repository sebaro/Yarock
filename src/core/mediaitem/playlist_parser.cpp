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


#include "playlist_parser.h"
#include "debug.h"

#include <QtCore>
#include <QRegularExpression>

/* ---------------------------------------------------------------------------*/
/* Extension                                                                  */
/* ---------------------------------------------------------------------------*/
const QStringList m3u_extension  = QStringList() << "m3u" << "m3u8";
const QStringList pls_extension  = QStringList() << "pls";
const QStringList xspf_extension = QStringList() << "xspf";

/* ---------------------------------------------------------------------------*/
/* Internals functions                                                        */
/* ---------------------------------------------------------------------------*/
QList<MEDIA::TrackPtr>  readM3uPlaylist(QIODevice* device, const QDir& playlist_dir=QDir() );
QList<MEDIA::TrackPtr>  readPlsPlaylist(QIODevice* device, const QDir& playlist_dir=QDir() );
QList<MEDIA::TrackPtr>  readXspfPlaylist(QIODevice* device, const QDir& playlist_dir=QDir() );

void saveM3uPlaylist(QIODevice* device, const QDir& playlist_dir, QList<MEDIA::TrackPtr> list);
void savePlsPlaylist(QIODevice* device, const QDir& playlist_dir, QList<MEDIA::TrackPtr> list);
void saveXspfPlaylist(QIODevice* device, const QDir& playlist_dir, QList<MEDIA::TrackPtr> list);

/* ---------------------------------------------------------------------------*/
/* MEDIA::PlaylistFromFile                                                    */
/* ---------------------------------------------------------------------------*/
QList<MEDIA::TrackPtr> MEDIA::PlaylistFromFile(const QString& filename)
{
    QList<MEDIA::TrackPtr>   list;

    /* get file info */
    QFileInfo info(filename);
    const QString extension = info.suffix().toLower();
    const QDir playlist_dir = QDir(info.absoluteDir());

    Debug::debug() << "  [MEDIA] PlaylistFromFile :"  << filename;

    /* test opening file */
    QFile file(filename);

    if (!file.open(QFile::ReadOnly)) {
      Debug::warning() << "  [MEDIA] Failed to open read only file " << filename;
      return list;
    }

    /* choose parser */
    if( m3u_extension.contains(extension) )
      list = readM3uPlaylist( &file, playlist_dir);
    else if( pls_extension.contains(extension) )
      list = readPlsPlaylist( &file, playlist_dir);
    else if( xspf_extension.contains(extension ))
      list = readXspfPlaylist( &file, playlist_dir );

    Debug::debug() << "  [MEDIA] PlaylistFromFile : read " << list.size() << " entries";

    return list;
}


/* ---------------------------------------------------------------------------*/
/* MEDIA::PlaylistFromBytes                                                   */
/* ---------------------------------------------------------------------------*/
QList<MEDIA::TrackPtr> MEDIA::PlaylistFromBytes(QByteArray& bytes)
{
    Debug::debug() << "  [MEDIA]  PlaylistFromBytes";
    QList<MEDIA::TrackPtr>   list;

    QBuffer buffer(&bytes);
    if (!buffer.open(QFile::ReadOnly)) {
      Debug::debug() << "  [MEDIA] Failed to open data";
      return list;
    }

    // choose parser
    QByteArray peeked_data = buffer.peek(512);
    if( peeked_data.contains("#EXTM3U") || peeked_data.contains("#EXTINF") )
      list = readM3uPlaylist( &buffer ) ;
    else if( peeked_data.toLower().contains("[playlist]") )
      list = readPlsPlaylist( &buffer ) ;
    else if( peeked_data.contains("<playlist") && peeked_data.contains("<trackList") )
      list = readXspfPlaylist( &buffer ) ;
    else
    {
      // try m3u style
      Debug::warning() << "[MEDIA] PlaylistFromBytes unknown format, trying m3u playlist...";
      list = readM3uPlaylist( &buffer ) ;
    }
    Debug::debug() << "  [MEDIA]  PlaylistFromBytes END";

    return list;
}


/* ---------------------------------------------------------------------------*/
/* M3U playlist read                                                          */
/* ---------------------------------------------------------------------------*/
QList<MEDIA::TrackPtr>  readM3uPlaylist(QIODevice* device, const QDir& playlist_dir )
{
    QList<MEDIA::TrackPtr>   list;

    QString data = QString::fromUtf8(device->readAll());

    if(data.isEmpty()) return list;
    data.replace('\r', '\n');
    data.replace("\n\n", "\n");

    QByteArray bytes = data.toUtf8();
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::ReadOnly);

    //! metadata
    int       lenght;
    QString   title;
    QString   artist;

    //! main reading loop
    QString line;

    while(!buffer.atEnd()) {
      line = QString::fromUtf8(buffer.readLine()).trimmed();

      if(line.startsWith("#EXT"))
      {
        QString info = line.section(':', 1);
        QString l    = info.section(',', 0, 0);

        /* TODO
        bool ok = false;
        int length = l.toInt(&ok);
        if (!ok) {lenght = -1; continue;}
        */

        QString track_info = info.section(',', 1);
        QStringList list   = track_info.split('-');

        if (list.size() <= 1) {
          title   = track_info;
          continue;
        }

        artist = list[0].trimmed();
        title = list[1].trimmed();
      }
      else if (line.startsWith('#'))
      {
        continue;
      }
      else if( !line.isEmpty() )
      {
        //Debug::debug() << "  [MEDIA] readM3uPlaylist -> line " << line << "\n";
        MEDIA::TrackPtr track = MEDIA::TrackPtr(new MEDIA::Track());

        //! Find the Track location
        if (line.contains(QRegularExpression("^[a-z]+://"))) {
          QUrl url(line);
          if (url.isValid()) {
              track->setType(TYPE_STREAM);
              track->id          = -1;
              track->url         = url.toString();
              track->extra["station"]  = title.isEmpty() ? QString() : title;
              track->isFavorite  = false;
              track->isPlaying   = false;
              track->isBroken    = false;
              track->isPlayed    = false;
              track->isStopAfter = false;

              list.append(track);
          }
        }
        else {

          QString file_path = line;

          file_path = QDir::fromNativeSeparators(file_path);

          // Make the path absolute
          if (!QDir::isAbsolutePath(file_path))
            file_path = playlist_dir.absoluteFilePath(file_path);

          // Use the canonical path
          if (QFile::exists(file_path))
            file_path = QFileInfo(file_path).canonicalFilePath();

          track->setType(TYPE_TRACK);
          track->id          =  -1;
          track->url         =  file_path;
          track->title       =  title.isEmpty() ? QString() : title;
          track->duration    =  (lenght!=-1) ? lenght : 0;
          track->artist      =  artist.isEmpty() ? QString() : artist;
          track->isPlaying   =  false;
          track->isBroken    =  !QFile::exists(file_path);
          track->isPlayed    =  false;
          track->isStopAfter =  false;

          track->albumGain  =  0.0;
          track->albumPeak  =  0.0;
          track->trackGain  =  0.0;
          track->trackPeak  =  0.0;

          list.append(track);
        }


        lenght   = -1;
        title    = "";
        artist   = "";
      }
    } // end while

  return list;
}

/* ---------------------------------------------------------------------------*/
/* PLS playlist read                                                          */
/* ---------------------------------------------------------------------------*/
QList<MEDIA::TrackPtr>  readPlsPlaylist(QIODevice* device, const QDir& playlist_dir )
{
    QMap<int, MEDIA::TrackPtr> tracks;

    QList<MEDIA::TrackPtr> list;

    QRegularExpression n_re("\\d+$");

    while ( !device->atEnd() )
    {
      QString line = QString::fromUtf8(device->readLine()).trimmed();

      int equals    = line.indexOf('=');
      QString key   = line.left(equals).toLower();
      QString value = line.mid(equals + 1);

      QRegularExpressionMatch match = n_re.match(key);
      int n = match.captured(0).toInt();
      if( !tracks.contains(n) && n > 0 )
      {
        tracks[n] = MEDIA::TrackPtr(new MEDIA::Track());

        tracks.value(n)->setType(TYPE_STREAM);
        tracks.value(n)->id          = -1;
        tracks.value(n)->isFavorite  = false;
        tracks.value(n)->isPlaying   = false;
        tracks.value(n)->isBroken    = false;
        tracks.value(n)->isPlayed    = false;
        tracks.value(n)->isStopAfter = false;
      }

      //Debug::debug() << "  [MEDIA] readPlsPlaylist -> key:" << key << " value:" << value;

      if (key.startsWith("file"))
      {
        //! Find the Track location
        if (value.contains(QRegularExpression("^[a-z]+://")))
        {
          QUrl url(value);
          if (url.isValid()) {
              //Debug::debug() << "  [MEDIA] readPlsPlaylist -> url.isValid()" << url;
              tracks.value(n)->setType(TYPE_STREAM);
              tracks.value(n)->url         = value;
          }
        }
        else
        {
          QString file_path = value;

          file_path = QDir::fromNativeSeparators(file_path);

          // Make the path absolute
          if (!QDir::isAbsolutePath(file_path))
              file_path = playlist_dir.absoluteFilePath(file_path);

          // Use the canonical path
          if (QFile::exists(file_path))
              file_path = QFileInfo(file_path).canonicalFilePath();

          tracks.value(n)->setType(TYPE_TRACK);
          tracks.value(n)->url         =  file_path;
          tracks.value(n)->isBroken    =  QFile::exists(file_path) ? false : true;
        }
      } // key is filename
      else if (key.startsWith("title"))
      {
        tracks.value(n)->title = value;
      }
      else if (key.startsWith("length"))
      {
        if( value.toInt() > 0 )
            tracks.value(n)->duration = value.toInt();
      }
    } // fin while

    return tracks.values();
}


/* ---------------------------------------------------------------------------*/
/* XSPF playlist read                                                         */
/* ---------------------------------------------------------------------------*/
QList<MEDIA::TrackPtr>  readXspfPlaylist(QIODevice* device, const QDir& playlist_dir )
{
    QList<MEDIA::TrackPtr>  list;

    QXmlStreamReader xml(device);

    MEDIA::TrackPtr mi = MEDIA::TrackPtr(0);



    while(!xml.atEnd() && !xml.hasError())
    {
       xml.readNext();
       if (xml.isStartElement() && xml.name().toString() == "trackList")
         break;
    }


    while (!xml.atEnd() && !xml.hasError())
    {
       xml.readNext();

      if (xml.isStartElement() && xml.name().toString() == "track")
      {
        //Debug::debug() << "  [MEDIA] readXspfPlaylist -> NEW Track ";
        mi = MEDIA::TrackPtr(new MEDIA::Track());
      }
      else if (xml.isStartElement() && xml.name().toString() == "location")
      {
            QString file_path = QString(xml.readElementText());

            //Debug::debug() << "  [MEDIA] readXspfPlaylist -> Find the Track location" << file_path;
            if (!MEDIA::isLocal(file_path)) {
              QUrl url(file_path);
              if (url.isValid()) {

                //Debug::debug() << "  [MEDIA] readXspfPlaylist -> it's an url";
                if(mi) {
                  mi->setType(TYPE_STREAM);
                  mi->id          = -1;
                  mi->url         = file_path;
                  mi->isFavorite  = false;
                  mi->isPlaying   = false;
                  mi->isBroken    = false;
                  mi->isPlayed    = false;
                  mi->isStopAfter = false;
                }
              }
            }
            else {
              //Debug::debug() << "  [MEDIA] readXspfPlaylist -> it's a local file";

              file_path = QDir::fromNativeSeparators(file_path);
              //Debug::debug() << "  [MEDIA] readXspfPlaylist -> file_path" << file_path;

              // Make the path absolute
              if (!QDir::isAbsolutePath(file_path))
                file_path = playlist_dir.absoluteFilePath(file_path);
              //Debug::debug() << "  [MEDIA] readXspfPlaylist -> file_path" << file_path;

              // Use the canonical path
              if (QFile::exists(file_path))
                file_path = QFileInfo(file_path).canonicalFilePath();
              //Debug::debug() << "  [MEDIA] readXspfPlaylist -> file_path" << file_path;

              if(mi) {
                mi->setType(TYPE_TRACK);
                mi->id          =  -1;
                mi->url         =  file_path;
                mi->isPlaying   =  false;
                mi->isBroken    =  !QFile::exists(file_path);
                mi->isPlayed    =  false;
                mi->isStopAfter =  false;
             }
           }
      } // end location
      else if (xml.isStartElement() && xml.name().toString() == "title")
      {
         if(mi->type() == TYPE_TRACK)
            mi->title = QString(xml.readElementText());
         else
            mi->extra["station"] = QString(xml.readElementText());
      }
      else if (xml.isStartElement() && xml.name().toString() == "creator")
      {
          mi->artist = QString(xml.readElementText());
      }
      else if (xml.isStartElement() && xml.name().toString() == "album")
      {
          mi->album = QString(xml.readElementText());
      }
      else if (xml.isStartElement() && xml.name().toString() == "category")
      {
          if(mi->type() == TYPE_STREAM)
            mi->genre = QString(xml.readElementText());
      }
      else if (xml.isEndElement() && xml.name().toString() == "track")
      {
        //Debug::debug() << "  [MEDIA] readXspfPlaylist -> list.append(mi)" << mi;
        if(mi)
          list.append(mi);
        mi = MEDIA::TrackPtr(0);
      }
    }  // End while xml end

    //Debug::debug() << "  [MEDIA] readXspfPlaylist -> END OK";

    return list;
}


/* ---------------------------------------------------------------------------*/
/* MEDIA::PlaylistToFile                                                      */
/* ---------------------------------------------------------------------------*/
void MEDIA::PlaylistToFile(const QString& filename, QList<MEDIA::TrackPtr> list)
{
    Debug::debug() << "  [MEDIA] PlaylistToFile track count :" << list.size();

    /* test opening file */
    QFile file(filename);
    if (!file.open(QFile::WriteOnly)) {
      Debug::warning() << "Failed to open file for writing" << filename;
      return;
    }

    /* get file info */
    QFileInfo info(filename);
    const QString extension = info.suffix().toLower();
    const QDir playlist_dir = QDir(info.absoluteDir());

    /* choose playlist parser */
    if( m3u_extension.contains(extension) )
      saveM3uPlaylist(&file, playlist_dir, list);
    else if( pls_extension.contains(extension) )
      savePlsPlaylist(&file, playlist_dir, list);
    else if( xspf_extension.contains(extension) )
      saveXspfPlaylist(&file, playlist_dir, list);

    file.close();
}

/* ---------------------------------------------------------------------------*/
/* M3U playlist save                                                          */
/* ---------------------------------------------------------------------------*/
void saveM3uPlaylist(QIODevice* device, const QDir& playlist_dir, QList<MEDIA::TrackPtr> list)
{
    device->write("#EXTM3U\n");

    foreach (MEDIA::TrackPtr media, list)
    {
      if (!media) continue;
      if (media->type() != TYPE_TRACK && media->type() != TYPE_STREAM) continue;

      QString info;
      QString media_path;
      if (media->type() == TYPE_TRACK ) {
        info = QString("#EXTINF:%1,%2 - %3\n").arg(QString::number(media->duration))
                                              .arg(media->artist)
                                              .arg(media->title);
        device->write(info.toUtf8());

        //! Get the path to MediaItem relative to the Playlist File directory
        media_path = playlist_dir.relativeFilePath(media->url);
      }
      else {
        info = QString("#EXTINF:\n");
        info = QString("#EXTINF:%1,%2 - %3\n").arg(0)
                                              .arg("")
                                              .arg(media->extra["station"].toString());

        device->write(info.toUtf8());
        media_path = media->url;
      }

    device->write(media_path.toUtf8());
    device->write("\n");
    }
}


/* ---------------------------------------------------------------------------*/
/* PLS playlist save                                                          */
/* ---------------------------------------------------------------------------*/
void savePlsPlaylist(QIODevice* device, const QDir& playlist_dir, QList<MEDIA::TrackPtr> list)
{
    QTextStream stream(device);
    stream << "[playlist]" << Qt::endl;;
    stream << "Version=2" << Qt::endl;;
    stream << "NumberOfEntries=" << list.size() << Qt::endl;;

    int n = 1;
    foreach (MEDIA::TrackPtr media, list)
    {
      if (!media) continue;
      if (media->type() != TYPE_TRACK && media->type() != TYPE_STREAM) continue;

      /* TYPE_TRACK */
      if (media->type() == TYPE_TRACK )
      {
        QString media_path = media->url;
        media_path = playlist_dir.relativeFilePath(media_path).toUtf8();

        stream << "File" << n << "=" << media_path << Qt::endl;;
        stream << "Title" << n << "=" << media->title << Qt::endl;;
        stream << "Length" << n << "=" << media->duration << Qt::endl;;
        ++n;
      }
      else /* TYPE_STREAM */
      {
        stream << "File" << n << "=" << QString(media->url).toUtf8() << Qt::endl;;
        stream << "Title" << n << "=" << QString(media->extra["station"].toString()).toUtf8() << Qt::endl;;
        stream << "Length" << n << "=" << Qt::endl;;
        ++n;
      }
    } // fin Foreach
}


/* ---------------------------------------------------------------------------*/
/* XSPF playlist save                                                         */
/* ---------------------------------------------------------------------------*/
void saveXspfPlaylist(QIODevice* device, const QDir& playlist_dir, QList<MEDIA::TrackPtr> list)
{
    QXmlStreamWriter xml(device);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeStartElement("playlist");
    xml.writeAttribute("version", "1");
    xml.writeDefaultNamespace("http://xspf.org/ns/0/");

    xml.writeStartElement("trackList");
    foreach (MEDIA::TrackPtr media, list)
    {
      if (!media) continue;
      if (media->type() != TYPE_TRACK && media->type() != TYPE_STREAM) continue;

      xml.writeStartElement("track");

      QString media_path;
      // write metadata
      if (media->type() == TYPE_TRACK )
      {
        media_path = playlist_dir.relativeFilePath(media->url).toUtf8();

        xml.writeTextElement("location", media_path);
        xml.writeTextElement("title", media->title);
        xml.writeTextElement("creator", media->artist);
        xml.writeTextElement("album", media->album);
        xml.writeTextElement("duration", QString::number(media->duration* 1000));
      }
      else /* TYPE_STREAM */
      {
        media_path = media->url;

        xml.writeTextElement("location", media_path);
        xml.writeTextElement("title", media->extra["station"].toString());

        if( !media->genre.isEmpty() )
        {
          xml.writeStartElement("extension");
          xml.writeAttribute("application", "yarock");
          xml.writeTextElement("category",  media->genre);
          xml.writeEndElement(); //extension
        }
      }

      xml.writeEndElement(); //track
    }
    xml.writeEndElement(); //trackList
    xml.writeEndElement(); //playlist
    xml.writeEndDocument();
}
