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

#ifndef _MPRIS_MANAGER_H_
#define _MPRIS_MANAGER_H_

#include <QObject>
/*
********************************************************************************
*                                                                              *
*    MprisManager                                                              *
*                                                                              *
********************************************************************************
*/
class Mpris2;

class MprisManager : public QObject
{
Q_OBJECT
public:
    MprisManager(QObject *parent = 0);
    ~MprisManager();

    void reloadSettings();

private:
    Mpris2   *m_mpris2;

private slots:
    void activateMainWindow();

signals:
    void RaiseMainWindow();
};


#endif // _MPRIS_MANAGER_H_

