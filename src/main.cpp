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
//! Qt
#include <QApplication>
#include <QTextCodec>
#include <QNetworkReply>
#include <QString>
#include <QDebug>

#include <QDBusMetaType>
#include <QDBusConnection>
#include <QDBusArgument>   // register QImage as qbusmetatype
#include <QImage>          // register QImage as qbusmetatype


//! X11 and signal
#include <X11/Xlib.h>
#include <csignal>

//! local
#include "starter.h"
#include "config.h"

#include "mediaitem.h"
#include "widgets/equalizer/equalizer_preset.h"

/*
********************************************************************************
*                                                                              *
*    DBus                                                                      *
*                                                                              *
********************************************************************************
*/

QDBusArgument& operator<< (QDBusArgument& arg, const QImage& image);
const QDBusArgument& operator>> (const QDBusArgument& arg, QImage& image);

/*
********************************************************************************
*                                                                              *
*    main                                                                      *
*                                                                              *
********************************************************************************
*/
int main(int argc, char *argv[])
{
    //! core application settings
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(VERSION);
    QCoreApplication::setOrganizationName(ORG_NAME);
    QCoreApplication::setOrganizationDomain("yarock-player.org");
    
    //! taken from amarok
    // This call is needed to prevent a crash on exit with Phonon-VLC and LibPulse
    XInitThreads();
    // Rewrite default SIGINT and SIGTERM handlers
    // to make amarok save current playlists during forced
    // application termination (logout, Ctr+C in console etc.)
    signal( SIGINT, &QCoreApplication::exit );
    signal( SIGTERM, &QCoreApplication::exit );

    /* start Full application */
    QApplication application(argc, argv);
#if QT_VERSION < 0x050000
    application.setGraphicsSystem("raster");
#endif    
    application.setApplicationName(APP_NAME);
    application.setApplicationVersion(VERSION);
    application.setOrganizationName(ORG_NAME);
    application.setOrganizationDomain("yarock-player.org");
    application.setAttribute(Qt::AA_DontShowIconsInMenus, false);
    
    
    //! Qt Type registration
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    qRegisterMetaTypeStreamOperators<Equalizer::EqPreset>("Equalizer::Params");

    qRegisterMetaType<MEDIA::MediaPtr>();
    qRegisterMetaType<MEDIA::ArtistPtr>();
    qRegisterMetaType<MEDIA::AlbumPtr>();
    qRegisterMetaType<MEDIA::TrackPtr>();
    qRegisterMetaType<MEDIA::PlaylistPtr>();
   
    //! DBUS
    QDBusConnection::sessionBus().registerService("com.sebastien.yarock");
    qDBusRegisterMetaType<QImage>();
    qDBusRegisterMetaType<QVariantMap>();



    Starter starter(argc,argv);
    Q_UNUSED(starter)
    
    return application.exec();
}
