
#include "starter.h"
#include "config.h"
#include "mediaitem.h"
#include "widgets/equalizer/equalizer_preset.h"

#include <QApplication>
#include <QNetworkReply>
#include <QString>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusConnection>
#include <QDBusArgument>   // register QImage as qbusmetatype
#include <QImage>          // register QImage as qbusmetatype

#include <X11/Xlib.h>
#include <csignal>


QDBusArgument& operator<< (QDBusArgument& arg, const QImage& image);
const QDBusArgument& operator>> (const QDBusArgument& arg, QImage& image);

int main(int argc, char *argv[]) {
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
    application.setApplicationName(APP_NAME);
    application.setApplicationVersion(VERSION);
    application.setOrganizationName(ORG_NAME);
    application.setOrganizationDomain("yarock-player.org");
    application.setAttribute(Qt::AA_DontShowIconsInMenus, false);

    //! Qt Type registration
    qRegisterMetaType<QNetworkReply::NetworkError>("QNetworkReply::NetworkError");
    //qRegisterMetaTypeStreamOperators<Equalizer::EqPreset>("Equalizer::Params");

    qRegisterMetaType<MEDIA::MediaPtr>();
    qRegisterMetaType<MEDIA::ArtistPtr>();
    qRegisterMetaType<MEDIA::AlbumPtr>();
    qRegisterMetaType<MEDIA::TrackPtr>();
    qRegisterMetaType<MEDIA::PlaylistPtr>();

    //! DBUS
    QDBusConnection::sessionBus().registerService("com.seb-apps.yarock");
    qDBusRegisterMetaType<QImage>();
    qDBusRegisterMetaType<QVariantMap>();

    Starter starter(argc,argv);
    Q_UNUSED(starter)

    return application.exec();
}
