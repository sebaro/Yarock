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
#ifndef _INFO_SYSTEM_WORKER_H_
#define _INFO_SYSTEM_WORKER_H_

#include "info_system.h"
#include "info_service.h"

#include <QObject>
#include <QVariant>
#include <QList>
#include <QCache>


/*
********************************************************************************
*                                                                              *
*    Class InfoSystemWorker                                                    *
*                                                                              *
********************************************************************************
*/ 
class InfoSystemWorker : public QObject
{
Q_OBJECT

public:
    InfoSystemWorker();
    ~InfoSystemWorker();

signals:
    void info( INFO::InfoRequestData requestData, QVariant output );

public slots:
    void init();
    void getInfo(INFO::InfoRequestData requestData );
    void slot_checkCache(INFO::InfoRequestData requestData);
    void slot_updateCache(INFO::InfoRequestData requestData, QVariant output);

private:
    QList< INFO::InfoService* > m_services;
    QCache< QString, QVariant > m_dataCache;
};

#endif // _INFO_SYSTEM_WORKER_H_
