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

//! local
#include "menubar.h"
#include "menumodel.h"

#include "settings.h"
#include "global_actions.h"

#include "debug.h"

//! Qt
#include <QVBoxLayout>
#include <QApplication>
#include <QPainter>
#include <QMenu>

#include <QtGui>
#if QT_VERSION >= 0x050000
  #include <QToolButton>
#endif
/*
********************************************************************************
*                                                                              *
*    Class MenuBar                                                             *
*                                                                              *
********************************************************************************
*/
MenuBar::MenuBar(QWidget * parent) : QWidget(parent)
{
    this->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
    this->setFocusPolicy(Qt::NoFocus);

    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);
    
    
    /* set fixed width to 50 (sizepolicy is Fixed) */
    this->setMinimumWidth(50);
    this->setMaximumWidth(50);
    
    m_model = MenuModel::instance();

    
    /* create ui */
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(10);
    layout->setContentsMargins(0, 0, 0, 0);

    
    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    
    for (int i=0; i < m_model->rowCount(QModelIndex()); i++)
    {
      const QModelIndex &idx = m_model->index(i, 0, QModelIndex());

      if( !idx.isValid() ) continue;

      QString text   = idx.data(Qt::DisplayRole).toString();
      QIcon icon     = qvariant_cast<QIcon>(idx.data(Qt::DecorationRole));

      MenuBarButton *button = new MenuBarButton(icon,text,this);

      this->layout()->addWidget(button);
      
      //! build menu
      QWidget* w = new QWidget();
#if QT_VERSION >= 0x050000
      w->setWindowFlags(Qt::ToolTip);
#else
      w->setWindowFlags(Qt::Popup);
#endif

      QVBoxLayout* vl0 = new QVBoxLayout(w);
      vl0->setSpacing(4);
      vl0->setContentsMargins(12, 12, 12, 12);

      for (int j=0; j < m_model->rowCount(idx); j++)
      {
          const QModelIndex childIdx = m_model->index(j, 0, idx);
          QAction *a = childIdx.data(MenuActionRole).value<QAction*>();
    
          QToolButton* tb = new QToolButton(w);
          tb->setDefaultAction(a);
          tb->setAutoRaise(true);
          tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
          vl0->addWidget(tb);
      }
      
      button->setMenuWidget( w );
      w->hide();
    }

    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));    
}


/*
********************************************************************************
*                                                                              *
*    Class MenuBarButton                                                       *
*                                                                              *
********************************************************************************
*/
MenuBarButton::MenuBarButton( const QIcon &icon, const QString &text, QWidget *parent )
    : QPushButton( icon, "", parent )
{
    this->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );  
    this->setFocusPolicy(Qt::TabFocus);
    this->setIconSize( QSize(26,26) );
    this->setFlat ( true );
    this->setCheckable(true);

    this->setStyleSheet(
      QString ("QPushButton { border: none;min-width: 40px;min-height: 32px;}" \
              "QPushButton:checked { background-color: %1 ;border: none;min-width: 40px;min-height: 32px;}" ) 
          .arg( QApplication::palette().color( QPalette::Window ).name() )
    );
    m_name = text;
    m_ismenumouseover = false;
    m_ismouseover = false;   
}


void MenuBarButton::setMenuWidget(QWidget* w) 
{
    m_menu_widget=w;
    m_menu_widget->installEventFilter(this);
}


bool MenuBarButton::eventFilter(QObject* obj, QEvent* event)
{
  QWidget *m = qobject_cast<QWidget*>(obj);
    
  if (m && (m == m_menu_widget) )
  {
      switch (event->type()) 
      {
        case QEvent::Leave:
        {
          m_ismenumouseover = false;

          QRect widgetRect = this->geometry();
          widgetRect.moveTopLeft(this->parentWidget()->mapToGlobal(widgetRect.topLeft()));
          widgetRect.adjust(0,0, 10, 0);
    
          if (!widgetRect.contains(QCursor::pos())) {
            if(!m_ismouseover)
              activate(false);
          }
          
          return true;
        }
        break;

      case QEvent::Enter:
        m_ismenumouseover = true;
        return true;
        break;
  
      default:break;
     }
  }

  return QObject::eventFilter (obj, event);
}


QSize MenuBarButton::sizeHint() const
{
    QSize size = QPushButton::sizeHint();

    int width = 8;
    if( !icon().isNull() ) 
    {
        width += iconSize().width();
    }

    if( !text().isEmpty() )  
    {
        QFontMetrics fm( this->font() );
        width += fm.width( text() );
        width += 10; // paddding
    }
    size.setWidth( width );
    return size;
}


void MenuBarButton::enterEvent(QEvent *e)
{
    m_ismouseover = true;
    activate(true);
    QPushButton::enterEvent(e);
}

void MenuBarButton::leaveEvent(QEvent *e)
{
    m_ismouseover = false;

    QRect widgetRect = this->geometry();
    widgetRect.moveTopLeft(this->parentWidget()->mapToGlobal(widgetRect.topLeft()));
    widgetRect.adjust(0,0, 10, 0);
    
    if (!widgetRect.contains(QCursor::pos())) {
      if(!m_ismenumouseover)
        activate(false);
    }
    
    QPushButton::leaveEvent(e);    
}

void MenuBarButton::activate(bool active)
{
    setChecked(active);

    if(!active) 
    {
      m_menu_widget->hide();
    }
    else if(!m_menu_widget->isVisible())
    {
      QPoint location = this->mapToGlobal( QPoint(this->width(),0) );
     
      m_menu_widget->move(location);
      m_menu_widget->show();
      m_menu_widget->raise();
       
    }
}

void MenuBarButton::mousePressEvent ( QMouseEvent * e )
{
    e->ignore();
}
