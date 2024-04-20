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

#include "core/mediasearch/search_query_widget.h"
#include "core/mediasearch/search_query.h"
#include "widgets/ratingwidget.h"
#include "debug.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

/*
********************************************************************************
*                                                                              *
*    Class Search_Query_Widget                                                 *
*                                                                              *
********************************************************************************
*/
Search_Query_Widget::Search_Query_Widget(QWidget *parent) : QWidget(parent)
{
    this->setMaximumHeight(30);
    this->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Fixed);

    QHBoxLayout* hl0 = new QHBoxLayout(this);
    hl0->setSpacing(0);
    hl0->setContentsMargins(0, 0, 0 ,0 );

    ui_search_field   =  new QComboBox(this);
    ui_search_field->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    ui_search_rules   =  new QComboBox(this);
    ui_search_rules->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    ui_stacked_widget =  new QStackedWidget(this);
    ui_stacked_widget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    ui_line_edit  = new QLineEdit();
    ui_line_edit->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    ui_spin_box   = new QSpinBox();
    ui_time_edit  = new QTimeEdit();
    ui_date_edit  = new QDateEdit();

    ui_time_edit->setDisplayFormat("mm:ss");
    ui_date_edit->setDisplayFormat("yyyy");

    ui_rating_widget  = new RatingWidget();
    ui_rating_widget->set_rating(0.0);
    ui_rating_widget->setMinimumHeight(25);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    sizePolicy.setHeightForWidth(ui_rating_widget->sizePolicy().hasHeightForWidth());
    ui_rating_widget->setSizePolicy(sizePolicy);


    ui_page_text = new QWidget();
    ui_page_text->setObjectName("ui_page_text");
    QVBoxLayout* vl0 = new QVBoxLayout(ui_page_text);
    vl0->setSpacing(0);
    vl0->setContentsMargins(0, 0, 0 ,0 );
    vl0->addWidget(ui_line_edit);

    ui_page_number = new QWidget();
    ui_page_number->setObjectName("ui_page_number");
    QVBoxLayout* vl1 = new QVBoxLayout(ui_page_number);
    vl1->setSpacing(0);
    vl1->setContentsMargins(0, 0, 0 ,0 );
    vl1->addWidget(ui_spin_box);

    ui_page_time = new QWidget();
    ui_page_time->setObjectName("ui_page_time");
    QVBoxLayout* vl2 = new QVBoxLayout(ui_page_time);
    vl2->setSpacing(0);
    vl2->setContentsMargins(0, 0, 0 ,0 );
    vl2->addWidget(ui_time_edit);

    ui_page_date = new QWidget();
    ui_page_date->setObjectName("ui_page_date");
    QVBoxLayout* vl3 = new QVBoxLayout(ui_page_date);
    vl3->setSpacing(0);
    vl3->setContentsMargins(0, 0, 0 ,0 );
    vl3->addWidget(ui_date_edit);

    ui_page_rating = new QWidget();
    ui_page_rating->setObjectName("ui_page_rating");
    QVBoxLayout* vl4 = new QVBoxLayout(ui_page_rating);
    vl4->setSpacing(0);
    vl4->setContentsMargins(0, 0, 0 ,0 );
    vl4->addWidget(ui_rating_widget);

    ui_stacked_widget->addWidget (ui_page_text);
    ui_stacked_widget->addWidget (ui_page_number);
    ui_stacked_widget->addWidget (ui_page_time);
    ui_stacked_widget->addWidget (ui_page_date);
    ui_stacked_widget->addWidget (ui_page_rating);

    ui_remove_button = new QPushButton(QIcon(":/images/remove_32x32.png"), QString());
    ui_remove_button->setStyleSheet(
      QString ("QPushButton { border: none;min-width: 20px;min-height: 20px;}")
    );

    hl0->addWidget(ui_search_field);
    hl0->addWidget(ui_search_rules);
    hl0->addWidget(ui_stacked_widget);
    hl0->addWidget(ui_remove_button);

    // connect
    QObject::connect(ui_search_field, SIGNAL(currentIndexChanged(int)), SLOT(slotFieldChanged(int)));
    QObject::connect(ui_search_rules, SIGNAL(currentIndexChanged(int)), SLOT(slotOperatorChanged(int)));
    QObject::connect(ui_remove_button, SIGNAL(clicked()), SIGNAL(signalRemoveClicked()));

    // Populate the combo boxes
    for (int i=0; i<CST_SEARCH_FIELD_COUNT; i++) {
      ui_search_field->addItem(SearchQuery::FieldName(SearchQuery::Search_Field(i)));
      ui_search_field->setItemData(i, i);
    }
}


SearchQuery Search_Query_Widget::query()
{
    const int field = ui_search_field->itemData(ui_search_field->currentIndex()).toInt();
    const int op    = ui_search_rules->itemData(ui_search_rules->currentIndex()).toInt();

    SearchQuery result;
    result.field_     = SearchQuery::Search_Field(field);
    result.operator_  = SearchQuery::Search_Operator(op);

    // The value depends on the data type
    const QWidget* value_page = ui_stacked_widget->currentWidget();

    if (value_page == ui_page_text)
    {
      result.value_ = ui_line_edit->text();
    }
    else if (value_page == ui_page_number)
    {
      result.value_ = ui_spin_box->value();
    }
    else if (value_page == ui_page_date)
    {
      result.value_ = ui_date_edit->date();
      //Debug::debug() << "        [SearchQuery] QDate :" << result.value_;
    }
    else if (value_page == ui_page_time)
    {
      result.value_ = ui_time_edit->time();
      //Debug::debug() << "        [SearchQuery] QTime :" << result.value_;
    }
    else if (value_page == ui_page_rating)
    {
      result.value_ = ui_rating_widget->rating();
    }

    return result;
}


void Search_Query_Widget::set_query(SearchQuery& query)
{
    ui_search_field->setCurrentIndex(ui_search_field->findData(query.field_));
    ui_search_rules->setCurrentIndex(ui_search_rules->findData(query.operator_));

    // The value depends on the data type
    switch (SearchQuery::TypeOf(query.field_))
    {
      case SearchQuery::type_Text:
        ui_line_edit->setText( query.value_.toString() );
        break;

      case SearchQuery::type_Number:
        ui_spin_box->setValue( query.value_.toInt() );
        break;

      case SearchQuery::type_Date:
        //ui_date_edit->setDateTime( QDateTime::fromSecsSinceEpoch(query.value_.toInt()) );
        ui_date_edit->setDateTime( query.value_.toDateTime() );
        break;

      case SearchQuery::type_Time:
        ui_time_edit->setTime( QTime(0,0).addSecs(query.value_.toInt()) );
        break;

      case SearchQuery::type_Rating:
        ui_rating_widget->set_rating( query.value_.toFloat() );
        break;
    }
}

void Search_Query_Widget::slotFieldChanged(int i)
{
    //Debug::debug() << "        [SearchQuery] slotFieldChanged" << i;

    SearchQuery::Search_Field field = SearchQuery::Search_Field(
      ui_search_field->itemData(i).toInt()
    );

    SearchQuery::Field_Type type = SearchQuery::TypeOf(field);

    // Populate the operator combo box
    ui_search_rules->clear();
    foreach (SearchQuery::Search_Operator op, SearchQuery::OperatorsForType(type))
    {
      const int i = ui_search_rules->count();
      ui_search_rules->addItem(SearchQuery::OperatorText(type, op));
      ui_search_rules->setItemData(i, op);
    }

    // Show the correct value editor
    QWidget* page = NULL;
    switch (type)
    {
      case SearchQuery::type_Time:   page = ui_page_time;   break;
      case SearchQuery::type_Number: page = ui_page_number; break;
      case SearchQuery::type_Text:   page = ui_page_text;   break;
      case SearchQuery::type_Date:   page = ui_page_date;
        if(field == SearchQuery::field_track_lastPlayed)
          ui_date_edit->setDisplayFormat("dd.MM.yyyy");
        else
          ui_date_edit->setDisplayFormat("yyyy");
        break;
      case SearchQuery::type_Rating: page = ui_page_rating; break;
    }
    ui_stacked_widget->setCurrentWidget(page);

    //emit Changed();
}




void Search_Query_Widget::slotOperatorChanged(int)
{
    //Debug::debug() << "        [SearchQuery] slotOperatorChanged";
}

