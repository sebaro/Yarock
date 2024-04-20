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

#ifndef _HISTO_MODEL_H_
#define _HISTO_MODEL_H_


#include <QList>
#include <QString>
#include <QObject>
#include <QMutex>

#include "core/mediaitem/mediaitem.h"

/*
********************************************************************************
*                                                                              *
*    Class HistoModel                                                          *
*                                                                              *
********************************************************************************
*/
class HistoModel  : public QObject
{
Q_OBJECT
    static HistoModel*   INSTANCE;

  public:
    HistoModel(QObject *parent);
    static HistoModel* instance() { return INSTANCE; }

    void updateModel();
    void clear();

    int itemCount() {return m_tracks.size();}
    MEDIA::TrackPtr trackAt(int row) const;

    /* filtering method */
    void setSearch(const QVariant v) {m_search = v;}
    bool isMediaMatch(MEDIA::MediaPtr);
     
  private:
    void addItem(MEDIA::TrackPtr media);

  private:
    QList<MEDIA::TrackPtr>   m_tracks;
    QMutex                   m_mutex;
    QVariant                 m_search;
};

#endif // _HISTO_MODEL_H_
