/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2014 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#ifndef _PLAYLISTDBWRITER_H_
#define _PLAYLISTDBWRITER_H_

#include <QRunnable>
#include <QObject>
#include <QString>

class PlayqueueModel;
/*
********************************************************************************
*                                                                              *
*    Class PlaylistDbWriter                                                    *
*                                                                              *
********************************************************************************
*/
class PlaylistDbWriter : public QObject, public QRunnable
{
Q_OBJECT
  public:
    explicit PlaylistDbWriter();

    bool isRunning() {return _isRunning;}
    
    void setModel(PlayqueueModel* m) {m_model = m;}
    
    void run();

    void saveToDatabase(const QString& playlist_name, int bd_id = -1);
    void saveSessionToDatabase();
    
  private:
   void savePlaylist();
   void saveSession();
   
  private:
    PlayqueueModel    *m_model; 
    bool              _isRunning;
    bool              _isSessionSaving;
    QString           _playlist_name;
    int               _database_id;

  signals:
    void playlistSaved();
};

#endif // _PLAYLISTDBWRITER_H_
