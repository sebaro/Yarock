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
#ifndef _TASK_TAG_SEARCH_
#define _TASK_TAG_SEARCH_

#include "info_system.h"

#include <QStringList>
#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QTimer>

/*
********************************************************************************
*                                                                              *
*    Class TagSearch                                                           *
*                                                                              *
********************************************************************************
*/
class TagSearch : public QThread
{
Q_OBJECT
 public:
    explicit TagSearch(QObject *parent = 0);
    ~TagSearch();    
    
    enum TYPE {
       TASK_NONE  = 0,
       ALBUM_COVER_SINGLE,  
       ALBUM_COVER_FULL,
       ALBUM_GENRE_SINGLE,
       ALBUM_GENRE_FULL,
       ARTIST_IMAGE_SINGLE,
       ARTIST_IMAGE_FULL,
       ARTIST_ALBUM_FULL
    };
    
    bool isRunning() {return m_isRunning;};

    void setExit(bool b) {m_exit = b;}

    void run();
    void setSearch(TYPE type, INFO::InfoRequestData request=INFO::InfoRequestData());
    //void start(TYPE type, INFO::InfoRequestData request=INFO::InfoRequestData());

  private:
    void  process_search();
    void  finish_search();
    
    void  set_requests_cover_search();
    void  set_requests_genre_search();
    void  set_requests_artist_search();

    void  handle_cover_search_result(INFO::InfoStringHash, QVariant );
    void  handle_genre_search_result(INFO::InfoStringHash, QVariant );
    void  handle_artist_search_result(INFO::InfoStringHash, QVariant );
          
  private slots:
    void slot_request_timeout();
    void slot_system_info( INFO::InfoRequestData, QVariant );
    
  private:
    TYPE                   m_type;
    bool                   m_isRunning;
    QMap<quint64, INFO::InfoRequestData> m_requests;
    QTimer                 m_timeout;
    
    int                    m_max;
    bool                   m_exit;
    
  signals:
    void finished();
    void progress(int);
};


#endif // _TASK_TAG_SEARCH_
 
