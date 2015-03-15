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


#ifndef _DBUS_NOTIFICATION_H_
#define _DBUS_NOTIFICATION_H_

#include <QDateTime>
#include <QImage>
#include <QObject>
#include <QDBusArgument>
#include <QDBusPendingCallWatcher>


class QDBusInterface;

QDBusArgument& operator<< (QDBusArgument& arg, const QImage& image);
const QDBusArgument& operator>> (const QDBusArgument& arg, QImage& image);

/*
********************************************************************************
*                                                                              *
*    Class DbusNotification                                                    *
*                                                                              *
********************************************************************************
*/
class DbusNotification : public QObject
{
Q_OBJECT
  public:
    DbusNotification(QObject* parent = 0);

    void reloadSettings();

  private:
    void init();
    void sendMessage(const QString& summary,
                     const QString& message = QString(),
                     const QString& icon = QString(),
                     const QImage& image = QImage());
  private slots :
    void handleStateChanged();
    void handleVolumeChanged();
    void sendNowPlayingNotif();
    void sendPaused();
    void sendStopped();
    void sendPlaylistFinished();
    void sendVolumeChanged();
    void callFinished(QDBusPendingCallWatcher* watcher);

  private:
    QDBusInterface      *m_interface;
    uint                 m_notification_id;
    QDateTime            m_last_notif_time;
};

#endif // _DBUS_NOTIFICATION_H_
