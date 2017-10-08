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
#include "lastfm.h"
#include "debug.h"
#include "utilities.h"
#include "networkaccess.h"

#include "global_actions.h"

#include "core/player/engine.h"
#include "settings.h"

#include <QtCore>
#include <QtNetwork>
#include <QDomDocument>
#include <QDateTime>
#include <QCryptographicHash>


/*******************************************************************************
    namespace LastFm
*******************************************************************************/ 
namespace LastFm {

  namespace GLOBAL {
        QString api_key;
        QString session_key;
        QString secret_key;
        QString auth_token;
        QString username;
  }
  
  void sign(QMap<QString, QString>& params) 
  {
    QString s;
    QMapIterator<QString, QString> i(params);
    while (i.hasNext()) {
        i.next();
        s += i.key() + i.value();
    }
    s += LastFm::GLOBAL::secret_key;
    params["api_sig"] = QString::fromLatin1(QCryptographicHash::hash(s.toUtf8(), QCryptographicHash::Md5).toHex());
  }  
  
  
  /* md5 of string */
  inline QString md5( const QByteArray& src )
  {
    QByteArray const digest = QCryptographicHash::hash( src, QCryptographicHash::Md5 );
    return QString::fromLatin1( digest.toHex() ).rightJustified( 32, '0' ).toLower();
  }
  
  QByteArray paramToBytearray( QMap<QString, QString>& params )
  {
     QByteArray data;
     QMapIterator<QString, QString> i(params);
     while (i.hasNext()) {
        i.next();
        data += QUrl::toPercentEncoding(i.key())
                + '='
                + QUrl::toPercentEncoding(i.value())
                + '&';
     }
     return data;
  }
}

/*
********************************************************************************
*                                                                              *
*    Class LastFmService                                                       *
*                                                                              *
********************************************************************************
*/
static LastFmService* INSTANCE = 0;

LastFmService* LastFmService::instance() 
{
    if (!INSTANCE) 
      INSTANCE = new LastFmService();
    
    return INSTANCE;
}


LastFmService::LastFmService()
{
    Debug::debug() << "    [LastFmService] create";
    init();  
}


LastFmService::~LastFmService()
{
    Debug::debug() << "    [LastFmService] delete";
    if(SETTINGS()->_useLastFmScrobbler && isAuthenticated())
      scrobble();
}

void LastFmService::saveSettings()
{
    //! save settings
    QSettings settings(UTIL::CONFIGFILE,QSettings::IniFormat,this);
    settings.beginGroup("Scrobbler");
    settings.setValue("username", LastFm::GLOBAL::username);
    settings.setValue("key", LastFm::GLOBAL::session_key);
    settings.endGroup();
    settings.sync();
}

    

void LastFmService::init()
{
    m_currentTrack    = 0;
    
    bool enable = SETTINGS()->_useLastFmScrobbler;
    Debug::debug() << "    [LastFmService] init enable " << enable;
    
    disconnect(Engine::instance(), 0,this, 0);

    /*  load settings */
    QSettings settings(UTIL::CONFIGFILE,QSettings::IniFormat,this);
    settings.beginGroup("Scrobbler");

    LastFm::GLOBAL::username      = settings.value("username", QString()).toString();
    LastFm::GLOBAL::session_key   = settings.value("key", QString()).toString();
    LastFm::GLOBAL::api_key       = "b3717637c18071e1619e92ee2c3eb0f8";
    LastFm::GLOBAL::secret_key    = "8192599d44d34b27c520d597d34f3714";
    settings.endGroup();
    
    if( enable && isAuthenticated() ) 
    {
       connect(Engine::instance(), SIGNAL(engineStateChanged()), this, SLOT(slot_state_changed()));
       connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(slot_track_changed()));
    }
    
    /* change status for send love action */
    ACTIONS()->value(PLAYQUEUE_TRACK_LOVE)->setEnabled(enable);
    ACTIONS()->value(PLAYING_TRACK_LOVE)->setEnabled(enable);
}

QString LastFmService::username() {return LastFm::GLOBAL::username;}

    
bool LastFmService::isAuthenticated() const
{
    return !LastFm::GLOBAL::session_key.isEmpty();
}


void LastFmService::signIn(const QString& username, const QString& password)
{
    Debug::debug() << "    [LastFmService] Sign In request - username" << username;
    signOut();
    
    QUrl url("https://ws.audioscrobbler.com/2.0/");
    
    QMap<QString, QString> params;
    params["method"]    = "auth.getMobileSession";
    params["username"]  = username;
    params["authToken"] = LastFm::md5((username + LastFm::md5(password.toUtf8())).toUtf8());
    params["api_key"]   = LastFm::GLOBAL::api_key;
    LastFm::sign(params);

    LastFm::GLOBAL::auth_token =  LastFm::md5((username + LastFm::md5(password.toUtf8())).toUtf8());

    QByteArray array = LastFm::paramToBytearray( params );

    /* post request */
    NetworkReply *reply = HTTP()->post(url, array);
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(slot_sign_in_finished(QNetworkReply*)));
}


void LastFmService::slot_sign_in_finished(QNetworkReply* reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
        Debug::debug() << "    [LastFmService] Sign In request failed, http error" << status;
        emit signInFinished();
        return;
    }

    // Read the XML document
    QDomDocument document;
    document.setContent(reply->readAll());

    QDomElement lfm = document.documentElement();
    if (lfm.attribute("status", "") == "ok")
    {
        QDomElement name            = lfm.elementsByTagName("name").at(0).toElement();
        QDomElement key             = lfm.elementsByTagName("key").at(0).toElement();
        LastFm::GLOBAL::session_key = key.childNodes().at(0).nodeValue();
        LastFm::GLOBAL::username    = name.childNodes().at(0).nodeValue();

        Debug::debug() << "    [LastFmService] Sign in Successfull";
        Debug::debug() << "    [LastFmService] Sign in Successfull LastFm::GLOBAL::session_key " << LastFm::GLOBAL::session_key;
        Debug::debug() << "    [LastFmService] Sign in Successfull LastFm::GLOBAL::username " << LastFm::GLOBAL::username;
    }

    emit signInFinished();  
}

void LastFmService::signOut()
{
    Debug::debug() << "    [LastFmService] Sign out";
    LastFm::GLOBAL::username.clear();
    LastFm::GLOBAL::session_key.clear();  
}


void LastFmService::love(MEDIA::TrackPtr track, bool love)
{
    Debug::debug() << "    [LastFmService] love";

    QUrl url("https://ws.audioscrobbler.com/2.0/");

    QMap<QString, QString> params;
    if(love)
      params["method"]      = "track.love";
    else
      params["method"]      = "track.unlove";
    params["artist"]      = track->artist;
    params["track"]       = track->title;
    params["api_key"]     = LastFm::GLOBAL::api_key;
    params["sk"]          = LastFm::GLOBAL::session_key;
    
    LastFm::sign(params);

    QByteArray data = LastFm::paramToBytearray( params );
    
    /* post request */
    NetworkReply *reply = HTTP()->post(url, data);
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(scrobbled(QNetworkReply*)));  
  
}


void LastFmService::scrobble() 
{
    Debug::debug() << "    [LastFmService] scrobble ";
  
    if(!m_currentTrack) {
      Debug::debug() << "    [LastFmService] no current track ";
      return;
    }
    
    if (LastFm::GLOBAL::session_key.isEmpty()) {
        Debug::warning() <<  "    [LastFmService] Not authenticated to Last.fm";
        return;
    }
    
    m_playbackLength  += QDateTime::currentDateTime().toTime_t() - m_unpauseTime;
    //Debug::debug() << "    [LastFmService] scrobble track m_playbackLength" << m_playbackLength;
    //Debug::debug() << "    [LastFmService] scrobble track m_trackLength" << m_trackLength;

    /* The track must be played for at least half of it's duration or at least 4 minutes, whatever
       occurs first and must be longer then 30 seconds to be scrobbled */
    if (((m_playbackLength < m_currentTrack->duration / 2) && (m_playbackLength < 240)) || (m_currentTrack->duration < 30)) {
        Debug::debug() << "    [LastFmService] track " << m_currentTrack->title << "was not played long enough or is too short, not scrobbling";
        return;
    }  
    
    QUrl url("https://ws.audioscrobbler.com/2.0/");

    QMap<QString, QString> params;
    params["method"]      = "track.scrobble";
    params["timestamp"]   = QString::number(m_playbackStart);
    params["track"]       = m_currentTrack->title;
    params["artist"]      = m_currentTrack->artist;
    params["album"]       = m_currentTrack->album;
    params["trackNumber"] = QString::number(m_currentTrack->num);
    params["duration"]    = QString::number(m_currentTrack->duration);
    params["api_key"]     = LastFm::GLOBAL::api_key;
    params["sk"]          = LastFm::GLOBAL::session_key;

    LastFm::sign(params);

    QByteArray data = LastFm::paramToBytearray( params );
      
    /* post request */
    NetworkReply *reply = HTTP()->post(url, data);
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(slot_lastfm_response(QNetworkReply*)));
}


void LastFmService::nowPlaying() 
{
    Debug::debug() << "    [LastFmService] now playing";
    if(!m_currentTrack) {
      Debug::debug() << "    [LastFmService] now playing : no current track ";
      return;
    }
    
    if(m_currentTrack->type() != TYPE_TRACK)
      return;
      
    QUrl url("https://ws.audioscrobbler.com/2.0/");

    QMap<QString, QString> params;
    params["method"] = "track.updateNowPlaying";
    params["track"]  = m_currentTrack->title;
    params["artist"] = m_currentTrack->artist;
    params["album"]  = m_currentTrack->album;
    params["trackNumber"] = QString::number(m_currentTrack->num) ;
    params["duration"] = QString::number(m_currentTrack->duration);

    params["api_key"] = LastFm::GLOBAL::api_key;
    params["sk"]      = LastFm::GLOBAL::session_key;

    LastFm::sign(params);
  
    QByteArray data = LastFm::paramToBytearray( params );
    
    /* post request */
    NetworkReply *reply = HTTP()->post(url, data);
    connect(reply, SIGNAL(finished(QNetworkReply*)), SLOT(slot_lastfm_response(QNetworkReply*)));
}   


void LastFmService::slot_lastfm_response(QNetworkReply* reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 200)
    {
        Debug::debug() << "    [LastFmService] http error" << status;
        return;
    }

    // Read the XML document
    QDomDocument document;
    document.setContent(reply->readAll());

    QDomElement lfm = document.documentElement();
    if (lfm.attribute("status", "") == "ok") 
    {
        Debug::debug() << "    [LastFmService] successfull";
    }
    else 
    {
        Debug::debug() << "    [LastFmService] failed";
    }  
}     


void LastFmService::slot_track_changed() 
{
    Debug::debug() << "    [LastFmService] slot_track_changed " << m_currentTrack;

    if (m_currentTrack)
      scrobble();
      
    m_currentTrack  = Engine::instance()->playingTrack();
    
    /* init time counter */
    m_playbackStart  = QDateTime::currentDateTime().toTime_t();
    m_playbackLength = 0;
    m_unpauseTime    = m_playbackStart;
 
    nowPlaying();
}


void LastFmService::slot_state_changed() 
{
    Debug::debug() << "    [LastFmService] slot_state_changed " << m_currentTrack;

    if ( m_engineOldState == ENGINE::PAUSED && Engine::instance()->state() == ENGINE::PLAYING)
    {
      if (m_currentTrack)
        m_unpauseTime = QDateTime::currentDateTime().toTime_t();
    }
    else if ( m_engineOldState == ENGINE::PLAYING && Engine::instance()->state() == ENGINE::PAUSED)
    {
      if (m_currentTrack)
        m_playbackLength += QDateTime::currentDateTime().toTime_t() - m_unpauseTime;
    }
    else if ( Engine::instance()->state() == ENGINE::STOPPED )
    {
      if (m_currentTrack)
        scrobble(); 

      /* unset current track */
      m_currentTrack = MEDIA::TrackPtr(0);      
    }
    
    m_engineOldState = Engine::instance()->state();  
}

