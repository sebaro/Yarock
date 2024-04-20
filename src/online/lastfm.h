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
#ifndef _LASTFM_H_
#define _LASTFM_H_

#include "mediaitem.h"
#include "core/player/engine.h"

#include <QObject>
#include <QNetworkReply>
#include <QTimer>

/*
********************************************************************************
*                                                                              *
*    Class LastFmService                                                       *
*                                                                              *
********************************************************************************
*/

class LastFmService : public QObject 
{
Q_OBJECT
public:
    ~LastFmService();
    static LastFmService* instance();
    void init();
    void saveSettings();

    /* authentication */
    bool isAuthenticated() const;
    void signIn(const QString& username, const QString& password);
    void signOut();
    QString username();
    
    /* scrobbler service */
    void love(MEDIA::TrackPtr track, bool love=true);  
    void ban(MEDIA::TrackPtr track);  
  
signals:
    void signInFinished();
    void trackScrobbled();
    void trackLoved();
    void trackBan();
    
private:
    LastFmService();

    /* scobbling and now playing are done accordine to engine state */
    void scrobble();
    void nowPlaying();
    
private slots:
    void slot_sign_in_finished(QNetworkReply*);
    void slot_lastfm_response(QNetworkReply*);
    void slot_track_changed();
    void slot_state_changed();
    

    
private:
    /* Current playing track */  
    MEDIA::TrackPtr   m_currentTrack;
    
    /* Helper time counter */  
    uint              m_playbackStart;
    int               m_playbackLength;
    uint              m_unpauseTime;
    
    ENGINE::E_ENGINE_STATE   m_engineOldState;
};

#endif // _LASTFM_H_
