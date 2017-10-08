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
#ifndef _EX_LINEEDIT_H_
#define _EX_LINEEDIT_H_

#include <QWidget>
#include <QLineEdit>
#include <QStyleOptionFrame>
#include <QPushButton>
#include <QToolButton>
#include <QMenu>


class ClearButton;

/*
********************************************************************************
*                                                                              *
*    Class ExLineEdit                                                          *
*                                                                              *
********************************************************************************
*/
class ExLineEdit : public QWidget
{
Q_OBJECT
  public:
    ExLineEdit(QWidget *parent=0);

    QString text() const;
    void setText(const QString&);
    QString inactiveText() const;
    void setInactiveText(const QString&);

    void clearFocus();

    QLineEdit* lineEdit() {return m_lineEdit;}
    // enter text field
    void doReturnPressed();

    // left icons
    void addLeftIcon(QPushButton* ib);

    // QWidget
    QSize sizeHint() const;

    void setCustomContextMenu(QMenu* m) {m_context_menu =m;}
    
  protected:
    void focusInEvent(QFocusEvent *event);
    void focusOutEvent(QFocusEvent *event);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *event);
    void updateGeometries();
    
    #if QT_VERSION >= 0x050000
    void initStyleOption(QStyleOptionFrame *option) const;
    #else
    void initStyleOption(QStyleOptionFrameV2 *option) const;
    #endif

    void contextMenuEvent ( QContextMenuEvent * event );

    void keyPressEvent(QKeyEvent *event);
    bool event(QEvent *event);

  public slots:
    void slotFocus();

  private slots:
    void slotOnTextChanged(const QString&);
    void slotClearField();

  protected:
    QToolButton   *m_clearButton;
    QList<QPushButton*> m_left_icons;
    
    QLineEdit     *m_lineEdit;
    QString        m_inactiveText;

    QMenu*         m_context_menu;
    
  signals:
    void textfield_entered();
    void textfield_changed();
};

#endif // _EX_LINEEDIT__H_
