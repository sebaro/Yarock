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

#include "dialog_base.h"
#include "settings.h"
#include "debug.h"

#include <QDialogButtonBox>
#include <QApplication>
#include <QLabel>
#include <QStyle>
/*
********************************************************************************
*                                                                              *
*    Class DialogBase                                                          *
*                                                                              *
********************************************************************************
*/
DialogBase::DialogBase(QWidget *parent,const QString& title) : QDialog(parent)
{
    QPalette palette = QApplication::palette();
    palette.setColor(QPalette::Background, palette.color(QPalette::Base));
    this->setPalette(palette);
    
    this->setWindowFlags( Qt::Dialog | Qt::FramelessWindowHint );
    setAttribute(Qt::WA_ShowModal, true);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
 
    /* ui */
    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);    

    ui_title_label = new QLabel(title);
    ui_title_label->setFont(QFont("Arial",12,QFont::Bold));
    
    QPalette p = ui_title_label->palette();
    p.setColor(ui_title_label->foregroundRole(), SETTINGS()->_baseColor);
    ui_title_label->setPalette(p);  
}
  
void DialogBase::setTitle(const QString& title)
{
    ui_title_label->setText(title);  
}


void DialogBase::setContentLayout(QLayout* contentLayout, bool isSpacer)
{       
    QVBoxLayout * fl = new QVBoxLayout(this);
    fl->setContentsMargins(0, 0, 0, 0);
    
    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::Box);
    frame->setObjectName("mainframe");
    frame->setStyleSheet(QString("#mainframe {border: 1px ridge %1;}").arg(SETTINGS()->_baseColor.name()));

    fl->addWidget(frame);
  
    QVBoxLayout * layout = new QVBoxLayout(frame);
    layout->setContentsMargins(14, 14, 14, 14);    

    layout->addWidget(ui_title_label);
    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    layout->addLayout(contentLayout);
    if(isSpacer)
      layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    else
      layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum));
    layout->addWidget(ui_buttonBox);

    this->setLayout(fl);    
}


void DialogBase::setContentWidget(QWidget* contentWidget, bool isSpacer)
{
    QVBoxLayout * fl = new QVBoxLayout(this);
    fl->setContentsMargins(0, 0, 0, 0);
    
    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::Box);
    frame->setObjectName("mainframe");
    frame->setStyleSheet(QString("#mainframe {border: 1px ridge %1;}").arg(SETTINGS()->_baseColor.name()));

    fl->addWidget(frame);
  
    QVBoxLayout * layout = new QVBoxLayout(frame);
    layout->setContentsMargins(14, 14, 14, 14);    

    layout->addWidget(ui_title_label);
    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum));
    layout->addWidget(contentWidget);
    if(isSpacer)
      layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    else
      layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum));
    layout->addWidget(ui_buttonBox);

    this->setLayout(fl);    
}

/*
********************************************************************************
*                                                                              *
*    Class DialogMessage                                                       *
*                                                                              *
********************************************************************************
*/
DialogMessage::DialogMessage(QWidget *parent, const QString& title) : DialogBase(parent, title)
{
    buttonBox()->setStandardButtons(QDialogButtonBox::Ok);
    
    ui_message_label = new QLabel();
    setContentWidget(ui_message_label);
    
    connect(buttonBox(), SIGNAL(accepted()), this, SLOT(close()));
}


void DialogMessage::setMessage(const QString & text)
{
    ui_message_label->setText(text);
}

/*
********************************************************************************
*                                                                              *
*    Class DialogQuestion                                                      *
*                                                                              *
********************************************************************************
*/
DialogQuestion::DialogQuestion(QWidget *parent, const QString& title) : DialogBase(parent, title)
{
    buttonBox()->setStandardButtons(QDialogButtonBox::No|QDialogButtonBox::Yes);
    
    ui_question_label = new QLabel();
    setContentWidget(ui_question_label);
    
    connect(buttonBox(), SIGNAL(clicked ( QAbstractButton *)), this, SLOT(on_buttonBox_clicked(QAbstractButton *)));
}


void DialogQuestion::setQuestion(const QString & text)
{
    ui_question_label->setText(text);
}

void DialogQuestion::on_buttonBox_clicked(QAbstractButton * button)
{
    QDialogButtonBox::ButtonRole role = buttonBox()->buttonRole(button);

    switch (role)
    {
      case QDialogButtonBox::YesRole :
           accept();
           close();
        break;

      case QDialogButtonBox::NoRole :
         reject();
         close();
         break;

      default:break;
    }
}

/*
********************************************************************************
*                                                                              *
*    Class DialogInput                                                         *
*                                                                              *
********************************************************************************
*/
DialogInput::DialogInput(QWidget *parent, const QString& label, const QString& title ) : DialogBase(parent, title)
{
    ui_line_edit   = new QLineEdit();
    ui_line_edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    QGridLayout *ui_grid_layout = new QGridLayout();
    ui_grid_layout->setContentsMargins(0, 0, 0, 0);

    ui_grid_layout->addWidget(new QLabel(label), 0, 0, 1, 1);
    ui_grid_layout->addWidget(ui_line_edit, 0, 1, 1, 1);
    
    setContentLayout(ui_grid_layout);
  
    connect(buttonBox(), SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox(), SIGNAL(rejected()), this, SLOT(reject()));
}

QString DialogInput::editValue()
{
    return  ui_line_edit->text();
}

void DialogInput::setEditValue(const QString& value)
{
    ui_line_edit->setText(value);
}
  