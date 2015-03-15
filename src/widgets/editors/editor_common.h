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
#ifndef _EDITOR_COMMON_H_
#define _EDITOR_COMMON_H_

#include "mediaitem.h"

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QVariant>

/*
********************************************************************************
*                                                                              *
*    Class EdLineEdit                                                          *
*                                                                              *
********************************************************************************
*/
class EdLineEdit : public QLineEdit
{
Q_OBJECT
public:
  EdLineEdit(QWidget *parent=0);
    
private slots:
  void slot_on_text_changed();
};

/*
********************************************************************************
*                                                                              *
*    Class EdTrackEdit                                                         *
*                                                                              *
********************************************************************************
*/
class EdTrackEdit : public QWidget
{
Q_OBJECT
public:
  EdTrackEdit(MEDIA::TrackPtr track, QWidget *parent=0);

  int trackId() {return m_track->id;}
  
  bool isModified();
  
  int      trackNum()   {return ui_num->text().toInt();}
  QString  trackTitle() {return ui_title->text();}
  QString  trackGenre() {return ui_genre->text();}
  
private:
  MEDIA::TrackPtr  m_track;
  EdLineEdit*      ui_num;
  EdLineEdit*      ui_title;
  EdLineEdit*      ui_genre;  
};

/*
********************************************************************************
*                                                                              *
*    Class EdPushButton                                                        *
*                                                                              *
********************************************************************************
*/
class EdPushButton : public QPushButton
{
Q_OBJECT
public:
  EdPushButton(QWidget *parent=0);
  
  EdPushButton( const QIcon & icon, const QString & text, QWidget *parent=0 );
};

/*
********************************************************************************
*                                                                              *
*    Class EdMultiFieldEdit                                                    *
*                                                                              *
********************************************************************************
*/
class EdMultiFieldEdit : public QWidget
{
Q_OBJECT
public:
  enum Type {
      LINEEDIT,
      DATEEDIT,
      SPINBOX,
      RATING
    };
    
  EdMultiFieldEdit(Type type, QWidget* edit_widget, QWidget *parent=0);

  bool iEnabled() {return ui_check_enable->isChecked();}
  
  void setValue(const QVariant &v);
  void setValues(const QVariantList &list);
  
  bool isValueChanged() {return (
    ( !ui_check_enable->isVisible() && m_isValueChanged) ||
    (  ui_check_enable->isVisible() && ui_check_enable->isChecked() && m_isValueChanged)
   );
  }
  
  QWidget* editWidget() {return ui_edit_widget;}
  QWidget* checkWidget() {return ui_check_enable;}

private:
  void setEnabled(bool b);
  void connect_edit_widget();

private slots:
  void slot_on_checkbox_change();
  void slot_on_value_changed();

private:
  QCheckBox     *ui_check_enable;
  QWidget       *ui_edit_widget;
  Type          m_type;
  bool          m_isValueChanged;

private:
  static QChar multiValue() { return QChar(0x2260); }
  
};

#endif // _EDITOR_COMMON_H_
