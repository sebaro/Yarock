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
#ifndef _STREAM_LOADER_H_
#define _STREAM_LOADER_H_

#include "mediaitem.h"


#include <QObject>
#include <QByteArray>
#include <QTimer>
/*
********************************************************************************
*                                                                              *
*    Class StreamLoader                                                        *
*                                                                              *
********************************************************************************
*/
class StreamLoader : public QObject
{
Q_OBJECT
public: 
  StreamLoader(MEDIA::TrackPtr);
  void start_asynchronous_download(const QString &);
  
private slots:
  void slot_download_done(QByteArray);
  void slot_download_error();
  void slot_pending_task_done();

private:
  MEDIA::TrackPtr      m_parent;
  QList<StreamLoader*> m_pending_task;
  QTimer               *m_timeout_timer;

signals:
  void download_done(MEDIA::TrackPtr);
};

#endif // _STREAM_LOADER_H_