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

#ifndef _PLAYQUEUE_PROXYMODEL_H_
#define _PLAYQUEUE_PROXYMODEL_H_

#include <QObject>
#include <QString>
#include <QModelIndex>
#include <QSortFilterProxyModel>

//! PlayqueueProxyModel is a filter model for playlist
class PlayqueueProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
    PlayqueueProxyModel(QObject *parent = 0);
  
    //! Decides wheter the source row will be displayed of hidden
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent = QModelIndex()) const;

private:
    QString    m_pattern;
};

#endif // _PLAYQUEUE_PROXYMODEL_H_
