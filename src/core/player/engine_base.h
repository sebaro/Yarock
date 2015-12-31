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

#ifndef _ENGINE_BASE_H_
#define _ENGINE_BASE_H_

#include <QtCore/QObject>
#include <QtCore/QString>


#include "mediaitem.h"


namespace ENGINE {
  enum E_ENGINE_STATE {PLAYING, PAUSED, STOPPED, ERROR};
}


namespace ENGINE {
  enum E_ENGINE_TYPE 
  {
      NO_ENGINE = 0, 
      PHONON,
      VLC,
      MPV,
      QTMULTIMEDIA
  };
}

/*
********************************************************************************
*                                                                              *
*    Class EngineBase                                                          *
*                                                                              *
********************************************************************************
*/
class EngineBase : public QObject
{
Q_OBJECT
Q_DISABLE_COPY( EngineBase )
  
public:
    EngineBase();
    EngineBase(const QString& name);

    QString name() {return m_name;}
    QString version() {return m_version;}
    
    ENGINE::E_ENGINE_TYPE type() {return m_type;}
    bool isEngineOK() {return m_isEngineOK;}    

    /* play/pause/stop */
    virtual void play()  {};
    virtual void pause() {};
    virtual void stop();
    
    /* media management*/
    virtual void setMediaItem(MEDIA::TrackPtr ) {};
    virtual void setNextMediaItem(MEDIA::TrackPtr ) {};
    MEDIA::TrackPtr playingTrack() const {return m_currentMediaItem;}
    MEDIA::TrackPtr nextTrack() const {return m_nextMediaItem;}
    
    /* audio*/ 
    virtual int volume() const {return 0;};
    virtual void setVolume(const int &) {};
    virtual bool isMuted() const {return false;};
    virtual void setMuted( bool ) {};

    /* state & behavior */
    ENGINE::E_ENGINE_STATE state() const {return m_current_state;};
    static QString stateToString(ENGINE::E_ENGINE_STATE state);
    
    /* time */
    qint64 currentTime() const {return m_lastTick;}
    qint64 currentTotalTime() const {return m_totalTime;}
    virtual void seek( qint64 ) {};

    /* effect */
    virtual bool isEqualizerAvailable() { return false;};
    virtual void addEqualizer() {};
    virtual void removeEqualizer() {};
    virtual void applyEqualizer(QList<int>) {};
    
public slots:
    virtual void volumeMute( ) {};
    virtual void volumeInc( ) {};
    virtual void volumeDec( ) {};
    
public:
    bool                    m_isEngineOK;
    ENGINE::E_ENGINE_STATE  m_current_state;
    ENGINE::E_ENGINE_STATE  m_old_state;
    
    /* playing media properties */    
    qint64                  m_totalTime;
    bool                    m_aboutToFinishEmitted;
    qint32                  m_tickInterval;
    qint64                  m_lastTick;
    
    MEDIA::TrackPtr         m_currentMediaItem;
    MEDIA::TrackPtr         m_nextMediaItem;
    
signals:
    void mediaChanged();
    void mediaMetaDataChanged();
    void mediaAboutToFinish();
    void mediaFinished();
    void mediaTick(qint64 time);
    void mediaTotalTimeChanged(qint64 newTotalTime);
    void mediaSeekableChanged(bool);
    
    void engineQueueFinished();
    void engineStateChanged();
    void engineRequestStop();
    
    void volumeChanged();
    void muteStateChanged();
    
    
protected :
    QString                 m_name;
    QString                 m_version;
    ENGINE::E_ENGINE_TYPE   m_type;
};

Q_DECLARE_INTERFACE(EngineBase, "yarock.EngineBase/1.0")

#endif // _ENGINE_BASE_H_
