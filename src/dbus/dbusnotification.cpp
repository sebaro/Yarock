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

#include "dbus/dbusnotification.h"
#include "core/mediaitem/mediaitem.h"
#include "core/player/engine.h"
#include "covers/covercache.h"

#include "config.h"
#include "mainwindow.h"

#include "settings.h"
#include "debug.h"

// Qt
#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusReply>


#define DBUS_TIMEOUT_MS 7500
/*******************************************************************************
    QDBusArgument operator
*******************************************************************************/
QDBusArgument& operator<< (QDBusArgument& arg, const QImage& image)
{
  if (image.isNull()) {
    // Sometimes this gets called with a null QImage for no obvious reason.
    arg.beginStructure();
    arg << 0 << 0 << 0 << false << 0 << 0 << QByteArray();
    arg.endStructure();
    return arg;
  }
  QImage scaled = image.scaledToHeight(100, Qt::SmoothTransformation);
  QImage i = scaled.convertToFormat(QImage::Format_ARGB32).rgbSwapped();
  arg.beginStructure();
  arg << i.width();
  arg << i.height();
  arg << i.bytesPerLine();
  arg << i.hasAlphaChannel();
  int channels = i.isGrayscale() ? 1 : (i.hasAlphaChannel() ? 4 : 3);
  arg << i.depth() / channels;
  arg << channels;
  arg << QByteArray(reinterpret_cast<const char*>(i.bits()), i.byteCount());
  arg.endStructure();
  return arg;
}

const QDBusArgument& operator>> (const QDBusArgument& arg, QImage& /*image*/) {
  // This is needed to link but shouldn't be called.
  Q_ASSERT(0);
  return arg;
}

/*
********************************************************************************
*                                                                              *
*    Class DbusNotification                                                    *
*                                                                              *
********************************************************************************
*/
DbusNotification::DbusNotification(QObject* parent) : QObject(parent)
{
    init();
    reloadSettings();
}

void DbusNotification::init()
{
    m_notification_id = 0;
    m_interface       = 0;
}

void DbusNotification::reloadSettings()
{
    Debug::debug() << "  [DbusNotification] reloadSettings";
    bool isEnable   = SETTINGS()->_useDbusNotification;

    /*---------------------------------*/
    /* Dbus disable                    */
    /* --------------------------------*/
    if(!isEnable)
    {
      if(m_interface) {
        disconnect(Engine::instance(), 0,this, 0);
        disconnect(MainWindow::instance(), 0,this, 0);
        delete m_interface;
      }

      init();
      return;
    }

    /*---------------------------------*/
    /* Dbus enable                     */
    /* --------------------------------*/
    if(!m_interface)
      m_interface = new QDBusInterface("org.freedesktop.Notifications", "/org/freedesktop/Notifications");

    connect(Engine::instance(), SIGNAL(engineStateChanged()), this, SLOT(handleStateChanged()));
    connect(Engine::instance(), SIGNAL(mediaChanged()), this, SLOT(sendNowPlayingNotif()));
    
    /* FIXE ME */
    connect(MainWindow::instance(), SIGNAL(playlistFinished()), this, SLOT(sendPlaylistFinished()));
}


void DbusNotification::handleVolumeChanged()
{
    Debug::debug() << "  [DbusNotification] handleVolumeChanged";

}

void DbusNotification::handleStateChanged()
{
    //Debug::debug() << "  [DbusNotification] handleStateChanged";
    
/* note 07-12-2012 :
 * les messages pour la piste de lecture ne sont pas envoyés (mandriva + phonon-gstreamer)
 * il faut utiliser le signal  TrackChanged à la place du stateChange
 */
    ENGINE::E_ENGINE_STATE engine_state = Engine::instance()->state();

    switch (engine_state) {
      case ENGINE::PLAYING :
        //! with phonon-gstreamer PLAYING state is not send for each trach
        //! if playing is continuous (so use trach trackChanged signal)
        /*sendNowPlayingNotif(); */
        break;
      case ENGINE::PAUSED  :
        //! phonon-vlc send pause state between track
        /*sendPaused();*/
        break;
      case ENGINE::STOPPED : sendStopped(); break;
      case ENGINE::ERROR   : sendStopped(); break;
    }
}

void DbusNotification::sendNowPlayingNotif()
{
    QString summary;
    QString message;
    QString icon = "notification-audio-play";

    //! -- Playing State
    MEDIA::TrackPtr track = Engine::instance()->playingTrack();
    QPixmap        pix    = CoverCache::instance()->cover(track);

    if(track->type() == TYPE_TRACK) {
      summary =  track->title;
      message = QString("%1 - %2").arg(track->artist,track->album);
    }
    else if(track->type() == TYPE_STREAM) {
      summary =  track->name;
      message = "";
    }

    //! send message
    sendMessage(summary,message,icon,pix.toImage());
}


void DbusNotification::sendPaused()
{
    //! get now playing image
    QPixmap        pix    = CoverCache::instance()->cover(Engine::instance()->playingTrack());
    if(pix.isNull())
      pix = QPixmap(":/icon/yarock_64x64.png");

    QImage  image = pix.toImage();
    QString icon = "notification-audio-play";

    //! send message
    sendMessage(QCoreApplication::applicationName(), tr("Paused"),icon,image);
}

void DbusNotification::sendStopped()
{

    QPixmap pix = QPixmap(":/icon/yarock_64x64.png");
    QImage  image = pix.toImage();
    QString icon = "notification-audio-play";

    sendMessage(QCoreApplication::applicationName(), tr("Stopped"),icon,image);
}

//! TODO
void DbusNotification::sendPlaylistFinished()
{
    QPixmap pix = QPixmap(":/icon/yarock_64x64.png");
    QImage  image = pix.toImage();
    QString icon = "notification-audio-play";

    //! We get a PlaylistFinished followed by a Stopped from the player
    sendMessage(QCoreApplication::applicationName(), tr("Playlist finished"),icon,image);
}

void DbusNotification::sendVolumeChanged()
{
    //! get now playing image
    QPixmap pix    = CoverCache::instance()->cover(Engine::instance()->playingTrack());
    if(pix.isNull())
      pix = QPixmap(":/icon/yarock_64x64.png");

    QImage  image = pix.toImage();
    QString icon = "notification-audio-play";
    const int value = Engine::instance()->volume();
    sendMessage(QCoreApplication::applicationName(), tr("Volume %1%").arg(value),icon,image);
}

//! ----- SendMessage ----------------------------------------------------------
void DbusNotification::sendMessage(
               const QString& summary,
               const QString& message,
               const QString& icon,
               const QImage& image)
{
    //Debug::debug() << "  [DbusNotification] sendMessage" << message;

    QVariantMap hints;
    if (!image.isNull())
      hints["image_data"] = QVariant(image);

    int id = 0;

    //! Reuse the existing popup if it's still open.
    if (m_last_notif_time.secsTo(QDateTime::currentDateTime()) * 1000 < DBUS_TIMEOUT_MS)
      id = m_notification_id;

    QString member = "Notify";
    QVariantList params;
    params << APP_NAME;
    params << static_cast<unsigned int>(id);
    params << icon;
    params << summary << message;
    params << QStringList();
    params << hints;
    params << DBUS_TIMEOUT_MS;

    QDBusPendingCall pcall = m_interface->asyncCallWithArgumentList(member, params);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pcall, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                  this, SLOT(callFinished(QDBusPendingCallWatcher*)));
}

//! ----- CallFinished ----------------------------------------------------------
void DbusNotification::callFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<uint> reply = *watcher;
    if (reply.isError()) {
      qWarning() << "DbusNotification -> Error sending notification" << reply.error().name();
      return;
    }

    uint id = reply.value();
    if (id != 0) {
      m_notification_id        = id;
      m_last_notif_time = QDateTime::currentDateTime();
    }
}
