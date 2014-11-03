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
#ifndef _STREAMMODEL_H_
#define _STREAMMODEL_H_

#include "core/mediaitem/mediaitem.h"

#include <QList>
#include <QObject>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    Class StreamModel                                                         *
*                                                                              *
********************************************************************************
*/
class StreamModel  : public QObject
{
Q_OBJECT
    static StreamModel* INSTANCE;

  public:
    StreamModel(QObject *parent);

    //! return global instance
    static StreamModel* instance() { return INSTANCE; }

    //! update streams
    void clear();
    void setStreams(const QList<MEDIA::TrackPtr> &streams);

    int itemCount() {return m_streams.size();}

    //! get Stream Item method
    MEDIA::TrackPtr streamAt(int row) const;

    //! filtering method
    void setFilter(const QString & f) {m_filter_pattern = f;}
    bool isStreamFiltered(const int row);

    /* duplicate */
    void setDuplicate(bool b) { m_show_duplicate = b;}

    /* playing status update */
    void     updateStatusOfPlayingItem(MEDIA::TrackPtr);
    
  private:
    QList<MEDIA::TrackPtr>   m_streams;
    MEDIA::TrackPtr          m_playing_stream;
    QString                  m_filter_pattern;
    bool                     m_show_duplicate;
};

#endif // _STREAMMODEL_H_
