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

#include "service_lyrics.h"
#include "ultimatelyricsprovider.h"

#include "settings.h"
#include "networkaccess.h"
#include "utilities.h"
#include "constants.h"

#include "debug.h"

#include <QtCore>
#include <QFutureWatcher>

#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif

/*
********************************************************************************
*                                                                              *
*    Class ServiceLyrics                                                       *
*                                                                              *
********************************************************************************
*/
ServiceLyrics::ServiceLyrics() : InfoService()
{
    Debug::debug() << Q_FUNC_INFO;

    setName("lyrics");
    
    m_supportedInfoTypes << INFO::InfoTrackLyrics;
}


ServiceLyrics::~ServiceLyrics() {}

void ServiceLyrics::init()
{
    //Debug::debug() << "    [ServiceLyrics] init";
    m_ultimateLyricsReader = new UltimateLyricsReader(this);

    QFuture<ProviderList> future = QtConcurrent::run(
             m_ultimateLyricsReader, &UltimateLyricsReader::Parse,
             QString(":/data/lyrics/ultimate_providers.xml")
     );

    QFutureWatcher<ProviderList>* watcher = new QFutureWatcher<ProviderList>(this);
    watcher->setFuture(future);
    connect(watcher, SIGNAL(finished()), SLOT(slot_ultimate_lyrics_parsed()));
}
    
/*******************************************************************************
  ServiceLyrics::slot_ultimate_lyrics_parsed
*******************************************************************************/
void ServiceLyrics::slot_ultimate_lyrics_parsed()
{
    //Debug::debug() << "    [ServiceLyrics] slot_ultimate_lyrics_parsed";
    ProviderList list;

    QFutureWatcher<ProviderList>* watcher = static_cast<QFutureWatcher<ProviderList>*>(sender());

    foreach (UltimateLyricsProvider* provider, watcher->result()) {
      //Debug::debug() << "    [ServiceLyrics] slot_ultimate_lyrics_parsed # Provider Name: " << provider->name();
      list << provider;
    }

    /* sort user lyrics providers */
    QStringList user_names = SETTINGS()->_lyrics_providers;

    foreach (const QString& name, user_names) 
    {
      for(int i = 0;i<list.count();i++)
      {
       if (list.at(i)->name() == name) {

         UltimateLyricsProvider* provider = list.takeAt(i);
         m_providers << provider;
         Debug::debug() << "    [ServiceLyrics] slot_ultimate_lyrics_parsed # Provider Name =" << name;
 
         connect(provider, SIGNAL(InfoReady(INFO::InfoRequestData, const QString&)), this, SLOT(slot_lyrics_fetched(INFO::InfoRequestData,const QString&)), Qt::QueuedConnection);
         connect(provider, SIGNAL(Finished(INFO::InfoRequestData)), this, SLOT(slot_fetch_lyrics(INFO::InfoRequestData)), Qt::QueuedConnection);
  
         break;
       }
      }
    }
      
    while (!list.isEmpty())
      delete list.takeFirst();

    watcher->deleteLater();
    delete m_ultimateLyricsReader;
}

/*******************************************************************************
  ServiceLyrics::defaultProvidersList
*******************************************************************************/
QStringList ServiceLyrics::defaultProvidersList()
{
    QStringList default_order;
    default_order << "lyrics.wikia.com"
                  << "lyricsreg.com"
                  << "lyricsmania.com"
                  << "metrolyrics.com"
                  << "azlyrics.com"
                  << "songlyrics.com"
                  << "elyrics.net"
                  << "lyrics.com"
                  << "lyricsbay.com"
                  << "lyricsmode.com"
                  << "directlyrics.com"
                  << "loudsongs.com";
                  /*
                  << "teksty.org"
                  << "tekstowo.pl (Polish translations)"
                  << "vagalume.uol.com.br"
                  << "vagalume.uol.com.br (Portuguese translations)"
                  << "darklyrics.com";
                  */
                  
    return default_order;
}

/*******************************************************************************
  ServiceLyrics::fullProvidersList
*******************************************************************************/
QStringList ServiceLyrics::fullProvidersList()
{
    QStringList full_list;
    full_list     << "lyrics.wikia.com"
                  << "lyricsreg.com"
                  << "lyricsmania.com"
                  << "metrolyrics.com"
                  << "azlyrics.com"
                  << "songlyrics.com"
                  << "elyrics.net"
                  << "lyrics.com"
                  << "lyricsbay.com"
                  << "lyricsmode.com"
                  << "directlyrics.com"
                  << "loudsongs.com"
                  << "darklyrics.com"
                  << "teksty.org"
                  << "tekstowo.pl (Polish translations)"
                  << "vagalume.uol.com.br"
                  << "vagalume.uol.com.br (Portuguese translations)";
                  
    return full_list;
}
    
/*******************************************************************************
  ServiceLyrics::getInfo
*******************************************************************************/
void ServiceLyrics::getInfo( INFO::InfoRequestData requestData )
{
    emit checkCache( requestData );
}

/*******************************************************************************
  ServiceLyrics::fetchInfo
*******************************************************************************/
void ServiceLyrics::fetchInfo( INFO::InfoRequestData requestData )
{
    //Debug::debug() << Q_FUNC_INFO;

    if ( requestData.type == INFO::InfoTrackLyrics) 
    {
      m_current_providers.clear();
      m_current_providers.append(m_providers);
    
      if( !check_local_lyrics(requestData) )
        slot_fetch_lyrics(requestData);
    }
    else 
    {
        emit finished( requestData );
    }
}


/*******************************************************************************
  ServiceLyrics::check_local_lyrics
*******************************************************************************/
bool ServiceLyrics::check_local_lyrics(INFO::InfoRequestData requestData)
{
    INFO::InfoStringHash input = requestData.data.value< INFO::InfoStringHash >();

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(input["artist"].toLower().toUtf8().constData());
    hash.addData(input["title"].toLower().toUtf8().constData());

    const QString path = QString(UTIL::CONFIGDIR + "/lyrics/" + hash.result().toHex() + ".txt");

    if( QFile::exists(path) )
    {
      QFile file(path);

      if(file.open(QIODevice::ReadOnly))
      {
        QString file_content = QTextStream( &file ).readAll();
        file_content.replace(QRegExp("\n"), "<br>");

        QVariantHash output;
        output[ "provider" ] = QString("local");
        output[ "lyrics" ]   = file_content;
        output[ "url" ]      = path;

        emit info(requestData, output);

        return true;
      }
    }

    return false;
}

/*******************************************************************************
  ServiceLyrics::slot_fetch_lyrics
*******************************************************************************/
void ServiceLyrics::slot_fetch_lyrics(INFO::InfoRequestData requestData)
{
    if(m_current_providers.isEmpty()) return;

    UltimateLyricsProvider *provider = m_current_providers.takeFirst();

    provider->FetchInfo(requestData);
}


/*******************************************************************************
  ServiceLyrics::slot_lyrics_fethed
*******************************************************************************/
void ServiceLyrics::slot_lyrics_fetched(INFO::InfoRequestData requestData, const QString& lyric)
{
    // get provider sender
    UltimateLyricsProvider* provider = qobject_cast<UltimateLyricsProvider*>(sender());

    if(provider) 
    {
        QVariantHash output;
        output[ "provider" ] = provider->name();
        output[ "lyrics" ]   = lyric;
        output[ "url" ]      = provider->lyricsUrl();

        emit info(requestData, output);
    }
}


