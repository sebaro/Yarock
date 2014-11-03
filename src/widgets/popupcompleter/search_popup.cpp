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

#include "search_popup.h"
#include "popup_model.h"
#include "popup_view.h"
#include "debug.h"

#include "mainwindow.h"

#include <QVBoxLayout>
#include <QKeyEvent>

/*
********************************************************************************
*                                                                              *
*    Class SearchPopup                                                         *
*                                                                              *
********************************************************************************
*/
SearchPopup::SearchPopup(ExLineEdit *parent): QFrame(parent),
    m_line_edit(parent)
{
    //Debug::debug() << "SearchPopup::constructor";
    m_parent =       parent;

    this->setFrameStyle(QFrame::Panel);
    this->setWindowFlags(Qt::ToolTip);
    this->setFocusPolicy(Qt::NoFocus);
    this->setContentsMargins(0, 0, 0, 0);

    // model & view
    m_popup_model = new PopupModel();
    m_popup_view = new PopupView(this);
    m_popup_view->setModel(m_popup_model);

    // layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_popup_view);
    setLayout(layout);


    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(500);

    // connection
    connect(m_line_edit, SIGNAL(textfield_changed()), m_timer, SLOT(start()));
    connect(m_line_edit, SIGNAL(textfield_entered()), this, SLOT(slot_on_new_search()));

    connect(m_popup_view, SIGNAL(itemActivated(const QString&)),this, SLOT(slot_popup_item_clicked(const QString&)));
    connect(m_timer, SIGNAL(timeout()),this, SLOT(slot_exec_popup()));

    m_enableUpdate = true;
    MainWindow::instance()->installEventFilter(this);
}

SearchPopup::~SearchPopup()
{
}


/*******************************************************************************
 show_popup
*******************************************************************************/
void SearchPopup::show_popup()
{
    Debug::debug() << "SearchPopup::show_popup";

    if(m_popup_model->rowCount() == 0)
      return;

    m_popup_view->setCurrentIndex( m_popup_model->index(0,0, QModelIndex()) );

    this->adjustSize();
    this->setUpdatesEnabled(true);

    int height = (m_popup_view->sizeHintForRow(0) + 2) * m_popup_model->rowCount() + 4;
    this->resize(m_line_edit->sizeHint().width()-8, height);

    this->move(m_line_edit->mapToGlobal(QPoint(4, m_line_edit->height())));
    this->show();

    //! keep line edit focus
    m_line_edit->slotFocus();
}


/*******************************************************************************
 slot_on_new_search
*******************************************************************************/
void SearchPopup::slot_on_new_search()
{
    //Debug::debug() << "SearchPopup::slot_on_new_search";
    m_timer->stop();
    this->hide();
}

/*******************************************************************************
 slot_exec_popup
*******************************************************************************/
void SearchPopup::slot_exec_popup()
{
    Debug::debug() << "SearchPopup::slot_exec_popup m_enableUpdate" << m_enableUpdate;

    if (!m_enableUpdate) return;
    if (!m_line_edit->hasFocus()) return;

    const QString query = m_line_edit->text();
    m_old_search = query;

    //Debug::debug() << "SearchPopup slot_exec_popup search pattern" << query;
    if (query.isEmpty()) {
        this->hide();
        return;
    }

    // populate popup model
    this->setUpdatesEnabled(false);
    m_popup_model->clear();
    m_popup_model->populateModel(query);
    show_popup();
}

/*******************************************************************************
    slot_popup_item_clicked
*******************************************************************************/
void SearchPopup::slot_popup_item_clicked(const QString& text)
{
    Debug::debug() << "SearchPopup::slot_popup_item_clicked";
    m_timer->stop();
    this->hide();
    m_line_edit->setFocus();
    m_line_edit->setText(text);
    m_line_edit->doReturnPressed();
}


/*******************************************************************************
    eventFilter
*******************************************************************************/
bool SearchPopup::eventFilter(QObject *obj, QEvent *ev)
{
    //Debug::debug() << "SearchPopup eventFilter  obj" << obj;
    int type = ev->type();
    QWidget *wid = qobject_cast<QWidget*>(obj);

    if (obj == this)
    {
        return false;
    }

    // hide conditions of the SearchPopup
    if (wid
            && ((wid == m_parent
                 && (type == QEvent::Move || type == QEvent::Resize)) || ((wid->windowFlags() & Qt::Window)
                         && (type == QEvent::Move || type == QEvent::Hide || type == QEvent::WindowDeactivate)
                         && wid == m_parent->window()) || (type == QEvent::MouseButtonPress && !isAncestorOf(wid)))
       )
    {
        this->hide();
        return false;
    }

    //actions on the SearchPopup
    if (wid && wid->isAncestorOf(m_parent) && isVisible())
    {
        if (type == QEvent::KeyPress)
        {
            QKeyEvent *kev = static_cast<QKeyEvent *>(ev);
            bool consumed = false;
            //Debug::debug() << "SearchPopup Qt::KeyPress : kev->key()" << kev->key();

            switch (kev->key())
            {
              case Qt::Key_Enter:
              case Qt::Key_Return:
                Debug::debug() << "SearchPopup Qt::Key_Return";
                m_popup_view->slot_item_clicked ( m_popup_view->currentIndex() );
                consumed = true;
                break;
              case Qt::Key_Escape:
                Debug::debug() << "SearchPopup Qt::Key_Escape";
                m_line_edit->setFocus();

                if(m_old_search != m_line_edit->text())
                  m_line_edit->setText(m_old_search);
                this->hide();
                consumed = true;
                break;
              case Qt::Key_Up:
              case Qt::Key_Down:
              case Qt::Key_Home:
              case Qt::Key_End:
              case Qt::Key_PageUp:
              case Qt::Key_PageDown:
                {
                m_enableUpdate = false;
                m_popup_view->event(ev);
                const QString text = m_popup_view->currentIndex().data( Qt::DisplayRole ).toString();
                m_line_edit->setText(text);
                }
                break;

              default:
                m_line_edit->setFocus();
                this->hide();
                break;
            } // end switch
           return consumed;
        }
    }

    return QFrame::eventFilter(obj, ev);
}



void SearchPopup::hideEvent ( QHideEvent * /* event */)
{
    m_enableUpdate = true;
}
