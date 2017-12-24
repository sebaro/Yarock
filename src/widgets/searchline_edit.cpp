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

#include "widgets/searchline_edit.h"

#include <QApplication>
#include <QToolButton>
#include <QStylePainter>
#include <QPainter>
#include <QEvent>
#include <QKeyEvent>

#include "utilities.h"
#include "settings.h"
#include "debug.h"


/*
********************************************************************************
*                                                                              *
*    Class SearchLineEdit                                                      *
*                                                                              *
********************************************************************************
*/
SearchLineEdit::SearchLineEdit(QWidget *parent) : ExLineEdit(parent)
{
    isCustomSearch = false;
}

void SearchLineEdit::setCustomSearch(bool b) 
{
    isCustomSearch = b;
    
    if(isCustomSearch)
      m_lineEdit->clear();
    
    m_lineEdit->setReadOnly( isCustomSearch );
    
    if(isCustomSearch)
      m_clearButton->setVisible(true);
    else
      m_clearButton->setVisible(!m_lineEdit->text().isEmpty());
}
    

void SearchLineEdit::paintEvent(QPaintEvent *event)
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
    UTIL::getStyle()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &p, this);

    // draw line edit inactive text
    if (m_lineEdit->text().isEmpty() && !hasFocus() && !isCustomSearch)
    {
        QRect r = UTIL::getStyle()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

        QFontMetrics fm = fontMetrics();
        int horizontalMargin = 2;
        int leftMargin = !m_left_icons.isEmpty() ? m_left_icons.count()*(this->height()) : 2;

        QRect lineRect(horizontalMargin + r.x() + leftMargin,
                       r.y() + (r.height() - fm.height() + 1) / 2,
                       r.width() - 2 * horizontalMargin, fm.height());

        QPainter painter(this);
        painter.setPen(palette().brush(QPalette::Disabled, QPalette::WindowText).color());
        painter.drawText(lineRect, Qt::AlignLeft|Qt::AlignVCenter, m_inactiveText);
    }
    else if (isCustomSearch)
    {
        QRect r = UTIL::getStyle()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
      
        QFont font = QApplication::font();
        font.setBold(true);
    
        QFontMetrics fm(font);
        int horizontalMargin = 2;
        int leftMargin = !m_left_icons.isEmpty() ? m_left_icons.count()*(this->height()) : 2;

        QRect lineRect(horizontalMargin + r.x() + leftMargin,
                       r.y() + (r.height() - fm.height() + 1) / 2,
                       r.width() - 2 * horizontalMargin - m_clearButton->width(), 
                       fm.height());

        QPainter painter(this);
        painter.setFont(font);
        painter.setPen(QPen(SETTINGS()->_baseColor, 0.1, Qt::SolidLine, Qt::RoundCap));
        painter.drawText(lineRect, Qt::AlignLeft|Qt::AlignVCenter, tr("Custom search"));
    }
}
