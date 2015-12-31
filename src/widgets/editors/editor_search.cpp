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

#include "widgets/editors/editor_search.h"
#include "core/mediasearch/search_query_widget.h"
#include "core/mediasearch/search_query.h"
#include "core/mediasearch/media_search_engine.h"
#include "core/mediasearch/media_search.h"

#include "debug.h"

#include <QtCore>

/*
********************************************************************************
*                                                                              *
*    Class EditorSearch                                                        *
*                                                                              *
********************************************************************************
*/

EditorSearch::EditorSearch(QWidget *parent) : QWidget(parent)
{
    //Debug::debug() << "      [EditorSearch] create";
    this->setWindowFlags(Qt::Popup);
 
    QLabel *l1 = new QLabel(tr("Search mode"));
    l1->setFont(QFont("Arial",10,QFont::Bold));
    
    ui_search_mode = new QComboBox();
    ui_search_mode->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    ui_search_mode->addItem(tr("Match search term (AND)"));
    ui_search_mode->setItemData(0, MediaSearch::Type_And);
    ui_search_mode->addItem(tr("Match search term (OR)"));
    ui_search_mode->setItemData(1, MediaSearch::Type_Or);

    QLabel *l2 = new QLabel(tr("Search terms"));
    l2->setFont(QFont("Arial",10,QFont::Bold));  
  
    
    ui_search_query_layout = new QVBoxLayout();
    ui_search_query_layout->setSpacing(0);
    ui_search_query_layout->setContentsMargins(0,0,0,0);
      
    Search_Query_Widget * ui_search_query_widget = new Search_Query_Widget();
    ui_search_query_layout->addWidget(ui_search_query_widget);    
    listSearchWidget.append(ui_search_query_widget);

    /* --- add search Button ---*/
    ui_add_search_button = new QPushButton(tr("Add search query"));
  
  
    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply | QDialogButtonBox::Reset);    
    
    
    /* --- scrollarea ---*/
    QWidget *w  = new QWidget();
    w->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
     
    QVBoxLayout* vl0 = new QVBoxLayout(w);
    vl0->addWidget(l1);
    vl0->addWidget(ui_search_mode);
    vl0->addWidget(l2);
    vl0->addLayout(ui_search_query_layout);
    vl0->addWidget(ui_add_search_button);
    vl0->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    vl0->addWidget(ui_buttonBox);
  
    ui_scrollarea = new QScrollArea();
    ui_scrollarea->setWidget(w);
    ui_scrollarea->setWidgetResizable(true);
    ui_scrollarea->setFrameShape(QFrame::NoFrame);
    ui_scrollarea->viewport()->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    
    /* --- layout ---*/
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(4, 4, 0, 4);
    layout->addWidget( ui_scrollarea );
    this->setLayout(layout);

    /* --- connection --- */
    QObject::connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(on_buttonBox_clicked(QAbstractButton *)));
    QObject::connect(ui_search_mode,SIGNAL(currentIndexChanged(int)), SLOT(slot_search_mode_change()));
    QObject::connect(ui_add_search_button, SIGNAL(clicked()), this, SLOT(on_button_AddSearch_clicked()));  

  
    /* --- initialization --- */
    m_isActive = false;
}




void EditorSearch::slot_search_mode_change()
{
    //Debug::debug() << "      [EditorSearch] slot_search_mode_change";

    MediaSearch::SearchType search_type = MediaSearch::SearchType(ui_search_mode->itemData(ui_search_mode->currentIndex()).toInt());

    const bool enable = (search_type == MediaSearch::Type_All) ? false : true;

    foreach(Search_Query_Widget *w, listSearchWidget) {
      w->setEnabled(enable);
    }
    ui_add_search_button->setEnabled(enable);
}


void EditorSearch::on_button_AddSearch_clicked()
{
    //Debug::debug() << "      [EditorSearch] on_button_AddSearch_clicked";

    add_search_query_widget( new Search_Query_Widget() );
}


void EditorSearch::add_search_query_widget(Search_Query_Widget* widget)
{
    //Debug::debug() << "      [EditorSearch] add_search_query_widget";
  
    ui_search_query_layout->addWidget(widget);
    listSearchWidget.append(widget);

    this->resize(this->size() + QSize(0, widget->sizeHint().height()));
    ui_scrollarea->viewport()->updateGeometry();
    ui_scrollarea->viewport()->update();
    ui_scrollarea->update();
    
    QObject::connect(widget, SIGNAL(signalRemoveClicked()), this, SLOT(slot_remove_search_query_widget()));
}

void EditorSearch::remove_search_query_widget(Search_Query_Widget* widget)
{
    //Debug::debug() << "      [EditorSearch] remove_search_query_widget";
  
     const int H = widget->sizeHint().height();
     
     ui_search_query_layout->removeWidget(widget);
     const int index = listSearchWidget.indexOf(widget);

     this->resize(this->size() - QSize(0,H ));
     ui_scrollarea->viewport()->updateGeometry();
     ui_scrollarea->viewport()->update();
     ui_scrollarea->update();
      
     delete listSearchWidget.takeAt(index);       
}


void EditorSearch::slot_remove_search_query_widget()
{
    //Debug::debug() << "      [EditorSearch] slot_remove_search_query_widget";

    Search_Query_Widget * widget = qobject_cast<Search_Query_Widget*>(sender());
    if (!widget)
      return;

    remove_search_query_widget(widget);
}


bool EditorSearch::is_search_valid()
{
    //Debug::debug() << "      [EditorSearch] is_search_valid";

    if(listSearchWidget.isEmpty())
    {
      return false;
    }
    else
    {
      foreach(Search_Query_Widget *w, listSearchWidget)
        if(!w->query().is_valid())
          return false;
    }

    return true;
}


void EditorSearch::on_buttonBox_clicked(QAbstractButton * button)
{
    //Debug::debug() << "      [EditorSearch] on_buttonBox_clicked";
  
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);

    if ( role == QDialogButtonBox::ResetRole )
    {
      ui_search_mode->setCurrentIndex(0);
 
      foreach(Search_Query_Widget *w, listSearchWidget)
        remove_search_query_widget(w);
     
      m_isActive = false;
    }
    else
    {
      m_isActive = true; 
    }
    
    emit search_triggered();
    this->hide();
}

void EditorSearch::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Enter /*|| event->key() == Qt::Key_Return*/)
    {
      m_isActive = true; 
    
      this->hide();
      event->accept();
      emit search_triggered();
    }
}


void EditorSearch::set_search(QVariant v_search)
{
    //Debug::debug() << "      [EditorSearch] set_search";
  
    MediaSearch media_search = qvariant_cast<MediaSearch>(v_search);
     
    m_isActive = is_search_valid() ? true : false;
     
    /* search mode */
    ui_search_mode->setCurrentIndex(ui_search_mode->findData(media_search.search_type_));
 
    /* search query */
    foreach(Search_Query_Widget *w, listSearchWidget)
      remove_search_query_widget(w);
      
    listSearchWidget.clear();
 
    foreach(SearchQuery query, media_search.query_list_) {
      Search_Query_Widget* widget =  new Search_Query_Widget();
      widget->set_query(query);
      add_search_query_widget(widget);
    }
}


QVariant EditorSearch::get_search()
{
    //Debug::debug() << "      [EditorSearch] get_search";
  
    if(!is_search_valid()) {
      Debug::debug() << "      [EditorSearch] search is not valid";
      return QVariant();
    }
    
    MediaSearch  search;

    search.search_type_      = MediaSearch::SearchType(ui_search_mode->itemData(ui_search_mode->currentIndex()).toInt());
    search.query_list_.clear();

    foreach(Search_Query_Widget *w, listSearchWidget) {
      search.query_list_ << w->query();

//       Debug::debug() << "---------------------------------------------------";
//       Debug::debug() << "EditorSmart::get_search query field: " << SearchQuery::FieldName(w->query().field_);
//       Debug::debug() << "EditorSmart::get_search query operator: "
//          << SearchQuery::OperatorText(SearchQuery::TypeOf(w->query().field_), w->query().operator_);
// 
//       Debug::debug() << "EditorSmart::get_search query value: " <<  w->query().value_;
    }      
      
    
    QVariant stored;
    stored.setValue(search);
    
    return stored;
}
