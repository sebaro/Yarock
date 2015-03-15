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
#ifndef _SERVICE_LYRICS_H_
#define _SERVICE_LYRICS_H

#include <QString>

#include "info_service.h"
#include "info_system.h"
#include "ultimatelyricsreader.h"

class UltimateLyricsProvider;
typedef QList<UltimateLyricsProvider*> ProviderList;

/*
********************************************************************************
*                                                                              *
*    Class ServiceLyrics                                                       *
*                                                                              *
********************************************************************************
*/
class ServiceLyrics :  public INFO::InfoService
{
Q_OBJECT
public:
    ServiceLyrics();
    virtual ~ServiceLyrics();

    static QStringList defaultProvidersList();
    static QStringList fullProvidersList();
    
protected slots:
    virtual void init();
    void getInfo( INFO::InfoRequestData requestData );
    void fetchInfo( INFO::InfoRequestData requestData );

  private:
    bool check_local_lyrics(INFO::InfoRequestData requestData);

  private slots:
    void slot_ultimate_lyrics_parsed();
    void slot_fetch_lyrics(INFO::InfoRequestData requestData);
    void slot_lyrics_fetched(INFO::InfoRequestData requestData,const QString& lyric);

  private:
    UltimateLyricsReader   *m_ultimateLyricsReader;
    ProviderList            m_providers;
    ProviderList            m_current_providers;
};

#endif // _SERVICE_LYRICS_H
