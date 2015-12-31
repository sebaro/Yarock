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

//! this file is extract from Clementine Player 0.7.1
//! thanks for all good works for Clementine developers ;)



#include "ultimatelyricsprovider.h"
#include "ultimatelyricsreader.h"
#include "debug.h"

#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamReader>

/*
********************************************************************************
*                                                                              *
*    Class UltimateLyricsReader                                                *
*                                                                              *
********************************************************************************
*/
UltimateLyricsReader::UltimateLyricsReader(QObject* parent)
  : QObject(parent)
{
}

QList<UltimateLyricsProvider*> UltimateLyricsReader::Parse(const QString& filename) const {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly)) {
    Debug::warning() << "Error opening" << filename;
    return QList<UltimateLyricsProvider*>();
  }

  return ParseDevice(&file);
}

QList<UltimateLyricsProvider*> UltimateLyricsReader::ParseDevice(QIODevice* device) const {
  QList<UltimateLyricsProvider*> ret;

  QXmlStreamReader reader(device);
  while (!reader.atEnd()) {
    reader.readNext();

    if (reader.name() == "provider") {
      UltimateLyricsProvider* provider = ParseProvider(&reader);
      if (provider) {
        provider->moveToThread(qApp->thread());
        ret << provider;
      }
    }
  }

  return ret;
}

UltimateLyricsProvider* UltimateLyricsReader::ParseProvider(QXmlStreamReader* reader) const
{
  //Debug::debug() << "      [UltimateLyricsReader] ParseProvider";
  
  QXmlStreamAttributes attributes = reader->attributes();

  UltimateLyricsProvider* scraper = new UltimateLyricsProvider;
  scraper->set_name(attributes.value("name").toString());
  scraper->set_title(attributes.value("title").toString());
  scraper->set_charset(attributes.value("charset").toString());
  scraper->set_url(attributes.value("url").toString());

  while (!reader->atEnd()) {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::EndElement)
      break;

    if (reader->tokenType() == QXmlStreamReader::StartElement) 
    {
      if (reader->name() == "extract")
        scraper->add_extract_rule(ParseRule(reader));
      else if (reader->name() == "exclude")
        scraper->add_exclude_rule(ParseRule(reader));
      else if (reader->name() == "invalidIndicator")
        scraper->add_invalid_indicator(ParseInvalidIndicator(reader));
      else if (reader->name() == "urlFormat") {
        scraper->add_url_format(reader->attributes().value("replace").toString(),
                                reader->attributes().value("with").toString());
        reader->skipCurrentElement();
      }
      else
        reader->skipCurrentElement();
    }
  }
  return scraper;
}

UltimateLyricsProvider::Rule UltimateLyricsReader::ParseRule(QXmlStreamReader* reader) const {
  UltimateLyricsProvider::Rule ret;
  //Debug::debug() << "      [UltimateLyricsReader] ParseRule";

  while (!reader->atEnd()) 
  {
    reader->readNext();

    if (reader->tokenType() == QXmlStreamReader::EndElement)
      break;

    if (reader->tokenType() == QXmlStreamReader::StartElement) {
      if (reader->name() == "item") {
        //Debug::debug() << "      [UltimateLyricsReader] ParseRule find item";

        QXmlStreamAttributes attr = reader->attributes();
        if (attr.hasAttribute("tag"))
          ret << UltimateLyricsProvider::RuleItem(attr.value("tag").toString(), QString());
        else if (attr.hasAttribute("begin")) 
        {
          ret << UltimateLyricsProvider::RuleItem(attr.value("begin").toString(),
                                                  attr.value("end").toString());
        }
      }
      reader->skipCurrentElement();
    }
  }
  return ret;
}

QString UltimateLyricsReader::ParseInvalidIndicator(QXmlStreamReader* reader) const
{
  QString ret = reader->attributes().value("value").toString();
  reader->skipCurrentElement();
  return ret;
}

