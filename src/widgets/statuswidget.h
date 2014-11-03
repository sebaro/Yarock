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
#ifndef _STATUS_WIDGET_H_
#define _STATUS_WIDGET_H_


#include <QLabel>
#include <QString>
#include <QWidget>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QTimer>
//! Message Type to display

namespace STATUS {
typedef enum {
               TYPE_INFO = 0,
               TYPE_ERROR,
               TYPE_WARNING,
               TYPE_PLAYQUEUE,
               } T_MESSAGE;
};


/*
********************************************************************************
*                                                                              *
*    Class StatusWidget                                                        *
*                                                                              *
********************************************************************************
*/
//class StatusWidget : public QWidget
class StatusWidget : public QFrame
{
Q_OBJECT
  public:
    static StatusWidget         *INSTANCE;

    StatusWidget(QWidget *parent = 0);
    static StatusWidget* instance() { return INSTANCE; }

    //! Message methode
    void startShortMessage(const QString& m, STATUS::T_MESSAGE type, int ms );
    uint startProgressMessage(const QString& action);
    void stopProgressMessage(uint id);
    void updateProgressMessage(uint id, const QString & message);

  protected:
    void resizeEvent(QResizeEvent *event);
    void moveEvent(QMoveEvent *event);
    bool eventFilter(QObject *obj, QEvent *ev);
    
  private slots:
    void updateMessage();
    void stopShortMessage();
    void setPosition();

  private:
    QLabel                *message;
    QLabel                *pixmap;

    STATUS::T_MESSAGE     m_current_type;

    QWidget*              m_parent;
    QMap<uint,QString>    _longMessage;
    bool                  isShortMessage;
    QTimer               *m_short_timer;
};


#endif // _STATUS_WIDGET_H_
