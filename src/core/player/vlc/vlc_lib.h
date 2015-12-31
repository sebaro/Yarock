/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2016 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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
#ifdef ENABLE_VLC

#ifndef _VLC_LIB_H_
#define _VLC_LIB_H_

#include <QString>
#include <QStringList>

struct libvlc_instance_t;


class VlcLib
{
static VlcLib* INSTANCE;

public:
    VlcLib();
    ~VlcLib();

    //! returns libvlc instance object.
    static VlcLib* instance()  { return INSTANCE; }
    libvlc_instance_t* core();

    //! initialization 
    bool init();

    //! print most recent error message of libvlc
    static void print_error();
    static bool isError();

private:
    libvlc_instance_t     *m_vlcInstance;
};

#endif // _VLC_LIB_H_
#endif // ENABLE_VLC
