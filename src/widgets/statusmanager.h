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
#ifndef _STATUS_MANAGER_H_
#define _STATUS_MANAGER_H_


#include <QObject>
#include <QString>
#include <QWidget>
#include <QTimer>
#include <QMap>

//! Message Type to display
namespace STATUS {
typedef enum {
               INFO = 0,
               WARNING, 
               ERROR, 
               PLAYQUEUE,
               INFO_CLOSE,
               ERROR_CLOSE
               } TYPE;
};

class StatusWidget;
/*
********************************************************************************
*                                                                              *
*    Class StatusManager                                                       *
*                                                                              *
********************************************************************************
*/
class StatusManager : public QObject
{
Q_OBJECT
  public:
    static StatusManager         *INSTANCE;

    StatusManager(QWidget *parent = 0);
    static StatusManager* instance() { return INSTANCE; }

    //! Message methode
    uint startMessage(const QString& text, STATUS::TYPE type=STATUS::INFO, int ms=-1 );
    void stopMessage(uint id);
    void updateMessage(uint id, const QString& text);

    void onResize();
    
  private slots:
    void onTimerStop();
    void onCloseClicked();

  private:
    void doLayout();
    int newStatusWidget(const QString& text, STATUS::TYPE type);

  private:
    QWidget*                  m_parent;

    QMap<uint, StatusWidget*> m_widgets;
    QMap<uint, QTimer*>       m_timers;
};


#endif // _STATUS_MANAGER_H_
