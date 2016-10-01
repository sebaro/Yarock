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
#include "editor_common.h"
#include "widgets/ratingwidget.h"

#include "debug.h"

#include <QApplication>
#include <QLayout>
#include <QHBoxLayout>
#include <QDateEdit>
#include <QSpinBox>
#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class EdLineEdit                                                          *
*                                                                              *
********************************************************************************
*/
EdLineEdit::EdLineEdit(QWidget *parent) : QLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(slot_on_text_changed()));
    setPalette( QApplication::palette() );
}



void EdLineEdit::slot_on_text_changed()
{
    if( this->isModified() )
    {
      QPalette p;
      p.setColor(QPalette::Text, QColor( 255, 0, 0 ) );
      setPalette(p);
    }
    else
    {
      setPalette( QApplication::palette() );
    }
}

/*
********************************************************************************
*                                                                              *
*    Class EdTrackEdit                                                         *
*                                                                              *
********************************************************************************
*/
EdTrackEdit::EdTrackEdit(MEDIA::TrackPtr track, QWidget *parent) : QWidget(parent)
{
    /* ---- ui ---- */
    ui_num      = new EdLineEdit();
    ui_title    = new EdLineEdit();
    ui_genre    = new EdLineEdit();

    ui_num->setMaximumWidth( 35 );
  
    ui_title->sizePolicy().setHorizontalStretch(2);
    ui_genre->sizePolicy().setHorizontalStretch(1);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget( ui_num );
    layout->addWidget( ui_title );
    layout->addWidget( ui_genre );
  
    /* ---- init ---- */
    m_track     = track;
  
    ui_num->setText( QString::number(m_track->num) );
    ui_title->setText( m_track->title );
    ui_genre->setText( m_track->genre );
}


bool EdTrackEdit::isModified()
{
    return ( 
        ( m_track->title != trackTitle() ) ||
        ( m_track->genre != trackGenre() ) ||
        ( m_track->num != trackNum() ) 
    );
}


/*
********************************************************************************
*                                                                              *
*    Class EdPushButton                                                        *
*                                                                              *
********************************************************************************
*/
EdPushButton::EdPushButton(QWidget *parent) : QPushButton(parent)
{
    setStyleSheet(QString("QPushButton{background-color: %1;border: none;  margin: 2px;padding: 2px;}")
        .arg(QApplication::palette().color( QPalette::Window ).name())
    );  
}


EdPushButton::EdPushButton( const QIcon & icon, const QString & text, QWidget *parent )
: QPushButton(icon,text,parent)
{

    setStyleSheet(QString("QPushButton{background-color: %1;border: none;  margin: 2px;padding: 2px;}")
        .arg(QApplication::palette().color( QPalette::Window ).name())
    );
}



/*
********************************************************************************
*                                                                              *
*    Class EdMultiFieldEdit                                                    *
*                                                                              *
********************************************************************************
*/
EdMultiFieldEdit::EdMultiFieldEdit(Type type, QWidget* edit_widget, QWidget *parent) : QWidget(parent)
{
    m_isValueChanged = false;
    
    m_type = type;
    
    ui_edit_widget  = edit_widget;
   
    ui_check_enable = new QCheckBox(this);
    
    connect( ui_check_enable, SIGNAL(stateChanged ( int ) ), this, SLOT(slot_on_checkbox_change()));
}

void EdMultiFieldEdit::setEnabled(bool b)
{
   ui_check_enable->setChecked( b );
   
   ui_edit_widget->setEnabled( b );
}

void EdMultiFieldEdit::slot_on_value_changed()
{
    m_isValueChanged = true;
}

void EdMultiFieldEdit::slot_on_checkbox_change()
{
   ui_edit_widget->setEnabled( ui_check_enable->isChecked() );
}

void EdMultiFieldEdit::setValue(const QVariant &v)
{
//     Debug::debug() << "EdMultiFieldEdit::setValue ";
    if ( m_type == DATEEDIT )
    {
      qobject_cast<QDateEdit*>(ui_edit_widget)->setDate( QDate(v.toInt(),1,1) );
    }
    else if ( m_type == LINEEDIT )
    {
      qobject_cast<QLineEdit*>(ui_edit_widget)->setText( v.toString() );  
    }    
    else if ( m_type == SPINBOX )
    {
      qobject_cast<QSpinBox*>(ui_edit_widget)->setValue( v.toInt() );
    }
    else if ( m_type == RATING )
    {
      qobject_cast<RatingWidget*>(ui_edit_widget)->set_rating( v.toFloat() );
    }    
    
    connect_edit_widget();
}


void EdMultiFieldEdit::setValues(const QVariantList &list)
{
    QList<QVariant> set;
    
    foreach (const QVariant variant, list)
    {
       if( !set.contains(variant) )
         set << variant;
    }
    
    if( set.size() == 1 )
    {
        setValue( set.first() );
    }
    else
    {
        if ( m_type == DATEEDIT )
        {
          /* ugly workaround : date is below the minimum -> the widgets shows the special value text */
          qobject_cast<QDateEdit*>(ui_edit_widget)->setSpecialValueText( multiValue() );
          qobject_cast<QDateEdit*>(ui_edit_widget)->setDate( QDate::fromString( "01/01/0001","dd/MM/yyyy") );
        }
        else if ( m_type == LINEEDIT )
        {
          qobject_cast<QLineEdit*>(ui_edit_widget)->setText( multiValue() );  
        }    
        else if ( m_type == SPINBOX )
        {
          qobject_cast<QSpinBox*>(ui_edit_widget)->setSpecialValueText( multiValue() );
        }
        else if ( m_type == RATING )
        {
          //qobject_cast<RatingWidget*>(ui_edit_widget)->set_rating( v.toFloat() );
        }         
        setEnabled(false);

        connect_edit_widget();
    }
}

void EdMultiFieldEdit::connect_edit_widget()
{
    /* ------------ connections ------------ */
    if ( m_type == DATEEDIT )
    {
      connect(qobject_cast<QDateEdit*>(ui_edit_widget), SIGNAL(dateChanged ( const QDate &)), this, SLOT(slot_on_value_changed()));
    }
    else if ( m_type == LINEEDIT )
    {
      connect(qobject_cast<QLineEdit*>(ui_edit_widget), SIGNAL(textChanged ( const QString&)), this, SLOT(slot_on_value_changed()));
    }    
    else if ( m_type == SPINBOX )
    {
      connect(qobject_cast<QSpinBox*>(ui_edit_widget), SIGNAL(valueChanged (int)), this, SLOT(slot_on_value_changed()));
    }
    else if ( m_type == RATING )
    {
      connect(qobject_cast<RatingWidget*>(ui_edit_widget), SIGNAL(RatingChanged(float)), this, SLOT(slot_on_value_changed()));
    } 
}

