
#include "service_lyrics.h"
#include "ultimatelyricsprovider.h"
#include "settings.h"
#include "networkaccess.h"
#include "utilities.h"
#include "debug.h"

#include <QtCore>
#include <QFutureWatcher>
#include <QRegularExpression>
#include <QtConcurrent>


/*
********************************************************************************
*                                                                              *
*    Class ServiceLyrics                                                       *
*                                                                              *
********************************************************************************
*/
ServiceLyrics::ServiceLyrics() : InfoService()
{
    Debug::debug() << "    [ServiceLyrics] start";

    setName("lyrics");

    m_supportedInfoTypes << INFO::InfoTrackLyrics;
}


ServiceLyrics::~ServiceLyrics() {}

void ServiceLyrics::init()
{
    //Debug::debug() << "    [ServiceLyrics] init";
    m_ultimateLyricsReader = new UltimateLyricsReader(this);

    QFuture<ProviderList> future = QtConcurrent::run(
             &UltimateLyricsReader::Parse, m_ultimateLyricsReader,
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
                  << "metrolyrics.com"
                  << "azlyrics.com"
                  << "songlyrics.com"
                  << "genius.com"
                  << "lyricsmania.com"
                  << "elyrics.net"
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
    full_list     << "genius.com"
                  << "lyrics.wikia.com"
                  << "lyricsreg.com"
                  << "lyricsmania.com"
                  << "metrolyrics.com"
                  << "azlyrics.com"
                  << "songlyrics.com"
                  << "elyrics.net"
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
    //Debug::debug() << "    [ServiceLyrics] fetchInfo";

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
        file_content.replace(QRegularExpression("\n"), "<br>");

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


