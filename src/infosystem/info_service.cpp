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
#include "info_service.h"


/*
********************************************************************************
*                                                                              *
*    Class InfoService                                                         *
*                                                                              *
********************************************************************************
*/ 
INFO::InfoService::InfoService() : QObject()
{
}


INFO::InfoService::~InfoService()
{
}


void
INFO::InfoService::setName( const QString& name )
{
    m_name = name;
}


const QString
INFO::InfoService::name() const
{
    return m_name;
}

