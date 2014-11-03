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
#ifndef _INFO_SERVICE__H_
#define _INFO_SERVICE__H_

#include "info_system.h"

#include <QObject>
#include <QVariant>
#include <QSet>

namespace INFO
{
/*
********************************************************************************
*                                                                              *
*    Class InfoService                                                        *
*                                                                              *
********************************************************************************
*/ 
class InfoService : public QObject
{
Q_OBJECT

public:
    InfoService();

    virtual ~InfoService();

    void setName( const QString& name );
    virtual const QString name() const;

    QSet< INFO::InfoType > supportedInfoTypes() const { return m_supportedInfoTypes; }

signals:
    void info( INFO::InfoRequestData requestData, QVariant output );
    void finished( INFO::InfoRequestData requestData );
    void checkCache( INFO::InfoRequestData requestData);

protected slots:
    virtual void init() = 0;
    virtual void getInfo( INFO::InfoRequestData requestData ) = 0;
    virtual void fetchInfo( INFO::InfoRequestData requestData ) = 0;

protected:
    InfoType        m_type;
    QString         m_name;
    QSet< INFO::InfoType > m_supportedInfoTypes;
};

} // namespace INFO

#endif // _INFO_SERVICE__H_