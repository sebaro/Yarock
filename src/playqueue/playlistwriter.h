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

#ifndef _PLAYLISTWRITER_H_
#define _PLAYLISTWRITER_H_

#include <QRunnable>
#include <QObject>
#include <QList>
#include <QString>

#include "core/mediaitem/mediaitem.h"

class PlayqueueModel;
/*
********************************************************************************
*                                                                              *
*    Class PlaylistWriter                                                      *
*                                                                              *
********************************************************************************
*/
class PlaylistWriter : public QObject, public QRunnable
{
Q_OBJECT
  public:
    explicit PlaylistWriter();

    bool isRunning() {return m_isRunning;}

    void setModel(PlayqueueModel* m) {m_model = m;}
    
    void run();

    void saveToFile(const QString& filename);

  private:
    PlayqueueModel   *m_model;
    QString           m_fileToSave;
    bool              m_isRunning;

    void updateDatabase(QList<MEDIA::TrackPtr> list);

  signals:
    void playlistSaved();
};

#endif // _PLAYLISTWRITER_H_
