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

#include "sort_widget.h"
#include "media_search.h"
#include "global_actions.h"

#include <QLayout>
#include <QLabel>

/*
********************************************************************************
*                                                                              *
*    Class SortWidget                                                          *
*                                                                              *
********************************************************************************
*/
SortWidget::SortWidget( QMenu* parent ) : QWidget(parent)
{
    m_parent = parent;
    
    QLabel *label_1 = new QLabel(tr("Sort field"));
    label_1->setFont(QFont("Arial",10,QFont::Bold));

    QLabel *label_2 = new QLabel(tr("Sort order"));
    label_2->setFont(QFont("Arial",10,QFont::Bold));

    /*   sort field */
    ui_sort_field   =  new QComboBox();
    ui_sort_field->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    
    // Populate the combo boxes
    for (int i=0; i<CST_SEARCH_FIELD_COUNT; i++) {
      ui_sort_field->addItem(SearchQuery::FieldName(SearchQuery::Search_Field(i)));
      ui_sort_field->setItemData(i, i);
    }
    
    /*   sort order   */
    ui_sort_order   =  new QComboBox();
    ui_sort_order->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui_sort_order->setMinimumWidth(200);
    
    /* QDialogButtonBox */
    ui_buttonBox = new QDialogButtonBox(this);
    ui_buttonBox->setObjectName(QString::fromUtf8("ui_buttonBox"));
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Ok);

    /*   final layout   */
    QVBoxLayout* vl0 = new QVBoxLayout(this);
    vl0->addWidget(label_1);
    vl0->addWidget(ui_sort_field);
    vl0->addWidget(label_2);
    vl0->addWidget(ui_sort_order);    
    vl0->addWidget(ui_buttonBox);    


    /* connection */
    connect(ui_sort_field, SIGNAL(currentIndexChanged(int)), SLOT(slot_updateSortOrder()));
    connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(on_buttonBox_clicked(QAbstractButton *)));
    
    /* initialisation */
    slot_updateSortOrder(true);
} 


void SortWidget::slot_updateSortOrder(bool first_start /*=false*/)
{
    const SearchQuery::Search_Field field = SearchQuery::Search_Field(ui_sort_field->currentIndex());
    const SearchQuery::Field_Type type    = SearchQuery::TypeOf(field);
    
    const QString asc  = SearchQuery::FieldSortOrderText(type, true);
    const QString desc = SearchQuery::FieldSortOrderText(type, false);

    const int old_current_index = !first_start ? ui_sort_order->currentIndex() : 0;

    ui_sort_order->clear();
    ui_sort_order->addItem(asc);
    ui_sort_order->addItem(desc);
    ui_sort_order->setCurrentIndex(old_current_index);    
}


void SortWidget::on_buttonBox_clicked(QAbstractButton * button)
{
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);
    if(role == QDialogButtonBox::AcceptRole)
    {
      m_parent->close();
      
      /* build media search_query */
      MediaSearch::SortType sort_type = (ui_sort_order->currentIndex() == 0) ? MediaSearch::Sort_FieldAsc : MediaSearch::Sort_FieldDesc;

      SearchQuery::Search_Field sort_field = SearchQuery::Search_Field(ui_sort_field->itemData(ui_sort_field->currentIndex()).toInt());
      
      MediaSearch media_search = MediaSearch(MediaSearch::Type_All, SearchQueryList(), sort_type, sort_field, -1);
      
      QVariant var = QVariant::fromValue<MediaSearch>( media_search );
      
      emit triggered(var);
    }
}
