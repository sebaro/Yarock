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

#ifndef _LOCAL_TRACK_POPULATOR_H_
#define _LOCAL_TRACK_POPULATOR_H_

#include "mediaitem.h"

#include <QThread>
#include <QObject>
#include <QMultiMap>
#include <QString>

class LocalTrackModel;

/*
********************************************************************************
*                                                                              *
*    Class LocalTrackPopulator                                                 *
*                                                                              *
********************************************************************************
*/
class LocalTrackPopulator :  public QThread
{
Q_OBJECT
public:
    explicit LocalTrackPopulator();
    void setExit(bool b) {m_exit = b;}

protected:
    void run();

private:
    QString getAlbumHash(const QString&, const QString&);
    void initGenreModel();
    
private:
    QList<MEDIA::TrackPtr>      tracks_by_genre;
    bool               m_isGrouping;
  
    LocalTrackModel    *m_model;
    bool                m_exit;

signals:
    void populatingFinished();
    void populatingProgress(int);
};

#endif // _LOCAL_TRACK_POPULATOR_H_
