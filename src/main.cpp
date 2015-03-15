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


//! Qt
#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QNetworkReply>
#include <QDir>
#include <QString>
#include <QTime>

#include <QDBusMetaType>
#include <QDBusConnection>
#include <QDBusArgument>              // register QImage as qbusmetatype
#include <QImage>                     // register QImage as qbusmetatype

#include <QLocale>
#include <QLibraryInfo>
#include <QFontDatabase>
#include <QDebug>


//! local
#include "commandlineoptions.h"
#include "mainwindow.h"
#include "mediaitem.h"
#include "widgets/equalizer/equalizer_preset.h"  // type EqPreset
#include "constants.h"
#include "utilities.h"
#include "debug.h"

//! third
#include "qtsingleapplication.h"
#include "qtsinglecoreapplication.h"

//! X11 and signal
#include <X11/Xlib.h>
#include <csignal>

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
*    LoadTranslation                                                           *
*                                                                              *
********************************************************************************
*/
void LoadTranslation(const QString& prefix,
                     const QString& path,
                     const QString& language)
{
  QTranslator* t = new QTranslator();
  if (t->load(prefix + "_" + language, path))
    QCoreApplication::installTranslator(t);
  else
    delete t;
}

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

    //! command line option
    CommandlineOptions options(argc, argv);

    //! start core application
    // (console Qt applications,works without an X server)
    // (before parsing option to have right system locale)
    { //! BEGIN BLOCK
       QtSingleCoreApplication application(argc, argv);

       //! parse command line option
       if (!options.Parse()) return 1;

       //! check application instance
       if (application.isRunning()) {
         if (options.isEmpty()) {
           qDebug() << "== main -> Yarock is already running - activating existing window";
         }
         if (application.sendMessage(options.Serialize(), 5000)) {
           return 0;
         }
       }
    } //!  END BLOCK

    //! taken from amarok
    // This call is needed to prevent a crash on exit with Phonon-VLC and LibPulse
    XInitThreads();
    // Rewrite default SIGINT and SIGTERM handlers
    // to make amarok save current playlists during forced
    // application termination (logout, Ctr+C in console etc.)
    signal( SIGINT, &QCoreApplication::exit );
    signal( SIGTERM, &QCoreApplication::exit );

    //! start Full application
    QtSingleApplication application(argc, argv);
    #if QT_VERSION < 0x050000
    application.setGraphicsSystem("raster");
    #endif    
    application.setApplicationName(APP_NAME);
    application.setApplicationVersion(VERSION);
    application.setOrganizationName(ORG_NAME);
    application.setOrganizationDomain("yarock-player.org");


    //! check instance again
    // WHY : because another instance might have started by now
    if (application.isRunning() && application.sendMessage(options.Serialize(), 5000))
    {
      return 0;
    }

    //! init debug activation
    qDebug() << "== main -> options.debug()" << options.debug();
    Debug::setDebugEnabled( options.debug() );
    if(!options.debug()) {
      qDebug() << "**********************************************************************************************";
      qDebug() << "** YAROCK WAS STARTED IN NORMAL MODE. IF YOU WANT TO SEE DEBUGGING INFORMATION, PLEASE USE: **";
      qDebug() << "** yarock --debug                                                                           **";
      qDebug() << "**********************************************************************************************";
    }

    //! get langage (user or locale)
    QString language = options.language();
    language = language.isEmpty() ? QLocale::system().name() : language;
    qDebug() << "== main -> locale : " << QLocale::system().name();

    //! setup translations
    LoadTranslation("qt", QLibraryInfo::location(QLibraryInfo::TranslationsPath), language);
    LoadTranslation("yarock", QLatin1String("/usr/share/yarock/translations"), language);
    LoadTranslation("yarock", application.applicationDirPath() + "../translation", language); // in case of local running

    #if QT_VERSION < 0x050000    
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    #endif
    //! QRand initialisation
    qsrand( QTime( 0, 0, 0 ).secsTo( QTime::currentTime() ) );

    //! Directories settings
    QDir().mkpath(UTIL::CONFIGDIR);
    QDir().mkpath(UTIL::CONFIGDIR + "/radio");
    QDir().mkpath(UTIL::CONFIGDIR + "/albums");
    QDir().mkpath(UTIL::CONFIGDIR + "/artists");
    QDir().mkpath(UTIL::CONFIGDIR + "/lyrics");

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

    //! mainwindows start
    //qDebug() << "== main -> start Mainwindow";
    MainWindow         mainwindow;
    mainwindow.show();

    //! connect command line option messages
    QObject::connect(&application, SIGNAL(messageReceived(QByteArray)),
                     &mainwindow, SLOT(slot_commandline_received(QByteArray)));

    
    mainwindow.set_command_line(options);

    return application.exec();
}
