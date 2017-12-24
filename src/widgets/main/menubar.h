/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
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

#ifndef _MENU_BAR_H_
#define _MENU_BAR_H_

#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QString>
#include <QEvent>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>

/*
********************************************************************************
*                                                                              *
*    Class MenuBar                                                             *
*                                                                              *
********************************************************************************
*/
class MenuModel;
class MenuBarButton;

class MenuBar : public QWidget
{
Q_OBJECT
  public:
    MenuBar(QWidget *parent = 0);

  private:
    MenuModel               *m_model;
    MenuBarButton           *m_settings_button;
    
  private slots:
    void slot_on_database_menu_changed();    
};


/*
********************************************************************************
*                                                                              *
*    Class MenuBarButton                                                       *
*                                                                              *
********************************************************************************
*/
class MenuBarButton : public QPushButton
{
Q_OBJECT
  public:
    MenuBarButton( const QIcon & icon, const QString & text, QWidget *parent );
    QSize sizeHint() const;

    void setMenuWidget(QWidget* w);
    QWidget* menuWidget() {return m_menu_widget;}

  protected:
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent ( QMouseEvent * e );
    bool eventFilter(QObject *obj, QEvent *ev);

  private:
    void activate(bool active);

  private:
    QWidget   *m_menu_widget;
    bool       m_ismenumouseover;
    bool       m_ismouseover;
    QString    m_name;
};


#endif // _MENU_BAR_H_
