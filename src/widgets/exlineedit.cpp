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

#include "widgets/exlineedit.h"

#include <QApplication>
#include <QToolButton>
#include <QStylePainter>
#include <QPainter>
#include <QEvent>
#include <QKeyEvent>

#include "settings.h"
#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    Class ClearButton                                                         *
*                                                                              *
********************************************************************************
*/
class ClearButton : public QToolButton
{
public:
    ClearButton(QWidget *parent = 0);
};


ClearButton::ClearButton(QWidget *parent) : QToolButton(parent)
{
    setCursor(Qt::ArrowCursor);
    setIcon(QIcon(":/images/blue-cross-32x32.png"));
    setStyleSheet("QToolButton { border: none; padding: 0px; }");
    setToolTip(QObject::tr("Clear"));
    setFocusPolicy(Qt::NoFocus);
    setIconSize(QSize(12,12));
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    setVisible(false);
}

/*
********************************************************************************
*                                                                              *
*    Class ExLineEdit                                                          *
*                                                                              *
********************************************************************************
*/
ExLineEdit::ExLineEdit(QWidget *parent) : QWidget(parent)
{
    m_context_menu = 0;
    m_lineEdit  = new QLineEdit(this);
    m_lineEdit->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );

    // widget settings
    this->setFocusPolicy(m_lineEdit->focusPolicy());
    this->setAttribute(Qt::WA_InputMethodEnabled);
    this->setSizePolicy(m_lineEdit->sizePolicy());
    this->setBackgroundRole(m_lineEdit->backgroundRole());
    this->setMouseTracking(true);
    

    setPalette(m_lineEdit->palette());

    // line edit
    m_lineEdit->setFrame(false);
    m_lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_lineEdit->setFocusProxy(this);
    m_lineEdit->setStyleSheet("background:transparent");

    // clearButton
    m_clearButton = new ClearButton(this);

    //! connection
    QObject::connect(m_clearButton, SIGNAL(clicked()), this, SLOT(slotClearField()));
    QObject::connect(m_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotOnTextChanged(const QString&)));
    QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SIGNAL(textfield_entered()));
}

/*******************************************************************************
 text method
*******************************************************************************/
QString ExLineEdit::text() const
{
    return m_lineEdit->text();
}

void ExLineEdit::setText(const QString& text)
{
    m_lineEdit->setText(text);
}

QString ExLineEdit::inactiveText() const
{
    return m_inactiveText;
}

void ExLineEdit::setInactiveText(const QString& text)
{
    m_inactiveText = text;
}

/*******************************************************************************
 left icons
*******************************************************************************/
void ExLineEdit::addLeftIcon(QPushButton* ib)
{
    ib->setParent(this);
    m_left_icons << ib; 
}

/*******************************************************************************
 focus method
*******************************************************************************/
void ExLineEdit::clearFocus()
{
    m_lineEdit->clearFocus();
    QWidget::clearFocus();
}

void ExLineEdit::slotFocus()
{
    m_lineEdit->setFocus(Qt::OtherFocusReason);
}

/*******************************************************************************
 ExLineEdit::doReturnPressed
 -> external call to trigger "return pressed"
*******************************************************************************/
void ExLineEdit::doReturnPressed()
{
   QMetaObject::invokeMethod(m_lineEdit, "returnPressed");
}

/*******************************************************************************
 slots
*******************************************************************************/
void ExLineEdit::slotClearField()
{
    m_lineEdit->clear();
    this->clearFocus();
    
    emit textfield_entered();
}

void ExLineEdit::slotOnTextChanged(const QString& text)
{
    m_clearButton->setVisible(!text.isEmpty());
    emit textfield_changed();
}

/*******************************************************************************
 UI
*******************************************************************************/
QSize ExLineEdit::sizeHint() const
{
    QFontMetrics fm( QApplication::font() );
    int H = fm.height();
    
    return QSize(
      m_lineEdit->sizeHint().width() + m_clearButton->sizeHint().width(),
         qMax(H + 8, 24)
      );
}

void ExLineEdit::resizeEvent(QResizeEvent *event)
{
    updateGeometries();
    QWidget::resizeEvent(event);
}

void ExLineEdit::updateGeometries()
{
    #if QT_VERSION >= 0x050000
    QStyleOptionFrame panel;
    #else
    QStyleOptionFrameV2 panel;
    #endif
    
    initStyleOption(&panel);
    QRect rect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

    int padding = 3;

    int width = rect.width();

    int clearButton_W = this->height();

    int icons_W = 0;
    int icon_H = this->height() -2;
    int icon_W = icon_H - 4;
    for (int i = 0; i < m_left_icons.count(); i++)
    {
        QPushButton *bt = m_left_icons.at(i);
        bt->setGeometry(rect.x() + 2 + icons_W,1, icon_W, icon_H);
        icons_W += icon_W ;
    }
    
    m_lineEdit->setGeometry(rect.x() + 2 + icons_W, padding,
                            width - clearButton_W - icons_W, this->height() - padding*2);

    m_clearButton->setGeometry(this->width() - clearButton_W, 0,
                               clearButton_W, this->height());
}

void ExLineEdit::paintEvent(QPaintEvent *event)
{
Q_UNUSED(event)
    //! draw the frame
    QPainter p(this);
    
    #if QT_VERSION >= 0x050000
    QStyleOptionFrame panel;
    #else
    QStyleOptionFrameV2 panel;
    #endif
    
    initStyleOption(&panel);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

    // draw line edit inactive text
    if (m_lineEdit->text().isEmpty() && !hasFocus())
    {
        QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

        QFontMetrics fm = fontMetrics();
        int horizontalMargin = 2;
        int leftMargin = !m_left_icons.isEmpty() ? m_left_icons.count()*(this->height()) : 2;

        QRect lineRect(horizontalMargin + r.x() + leftMargin,
                       r.y() + (r.height() - fm.height() + 1) / 2,
                       r.width() - 2 * horizontalMargin, fm.height());

        p.setPen(palette().brush(QPalette::Disabled, QPalette::WindowText).color());
        p.drawText(lineRect, Qt::AlignLeft|Qt::AlignVCenter, m_inactiveText);
    }
}

void ExLineEdit::initStyleOption(QStyleOptionFrameV2 *option) const
{
    option->initFrom(this);
    option->rect = contentsRect();
    option->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, option, this);
    option->midLineWidth = 0;
    option->state |= QStyle::State_Sunken;

    option->features = QStyleOptionFrameV2::None;
}

void ExLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && !m_lineEdit->text().isEmpty())
        m_lineEdit->clear();

    m_lineEdit->event(event);
    QWidget::keyPressEvent(event);
}

bool ExLineEdit::event(QEvent *event)
{
    if (event->type() == QEvent::ShortcutOverride ||
        event->type() == QEvent::InputMethod)
        m_lineEdit->event(event);
    return QWidget::event(event);
}

/*******************************************************************************
 FOCUS
*******************************************************************************/
void ExLineEdit::focusInEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);
    QWidget::focusInEvent(event);
}

void ExLineEdit::focusOutEvent(QFocusEvent *event)
{
    m_lineEdit->event(event);
    QWidget::focusOutEvent(event);
}


void ExLineEdit::contextMenuEvent ( QContextMenuEvent * event )
{
    if (m_context_menu) 
    {
      m_context_menu->popup(event->globalPos());
      event->accept();
    }
}
