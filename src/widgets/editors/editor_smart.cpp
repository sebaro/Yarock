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

#include "widgets/editors/editor_smart.h"
#include "core/mediasearch/search_query_widget.h"
#include "core/mediasearch/search_query.h"
#include "core/mediasearch/media_search_engine.h"

#include "widgets/dialogs/dialog_base.h"
#include "widgets/exlineedit.h"

#include "threadmanager.h"
#include "smartplaylist.h"
#include "settings.h"

#include "debug.h"

#include <QScrollArea>
#include <QGroupBox>
#include <QFormLayout>
#include <QtCore>


/*
********************************************************************************
*                                                                              *
*    Class EditorSmart                                                         *
*                                                                              *
********************************************************************************
*/

EditorSmart::EditorSmart(QWidget *parent) : QScrollArea(parent)
{
    starting = true;
    m_databaseId = -1;

    /* ----- init playlist data ----- */    
    m_playlist = MEDIA::PlaylistPtr( new MEDIA::Playlist() );
    m_playlist->id       =  -1;
    m_playlist->url      =  QString();
    m_playlist->name     =  QString("untitled playlist");
    m_playlist->p_type   =  T_SMART;
    

    /* ----- create ui ----- */    
    ui_buttonBox = new QDialogButtonBox();
    ui_buttonBox->setOrientation(Qt::Horizontal);
    ui_buttonBox->setStandardButtons(QDialogButtonBox::Apply|QDialogButtonBox::Close);

    QLabel* ui_header = new QLabel(tr("Edit smart playlist"));
    ui_header->setFont(QFont("Arial",12,QFont::Bold));
    ui_header->setStyleSheet(QString("QLabel { color : %1; }").arg(SETTINGS()->_baseColor.name()));
    
    QVBoxLayout *l2 = new QVBoxLayout();
    l2->setSpacing(2);
    l2->setContentsMargins(0, 6, 0, 6);
    l2->addWidget( ui_header );

    ui_edit_name = new ExLineEdit();
    ui_edit_name->clearFocus();
    ui_edit_name->setInactiveText(tr("playlist name"));

    QHBoxLayout *l3 = new QHBoxLayout();
    l3->setSpacing(2);
    l3->addWidget( new QLabel(tr("Name")) );
    l3->addWidget( ui_edit_name );
    
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setContentsMargins(4, 2, 4, 4);
    layout->addLayout( l2 );
    layout->addLayout( l3 );
    layout->addLayout( create_ui() );
    layout->addWidget( ui_buttonBox );

    QWidget *w  = new QWidget();
    w->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);    
    w->setLayout(layout);
    
//     QScrollArea *area = new QScrollArea();
    this->setWidget(w);
    this->setWidgetResizable(true);
    this->setFrameShape(QFrame::NoFrame);
    this->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Expanding);
    
    starting = false;

    QObject::connect(ui_buttonBox, SIGNAL(clicked ( QAbstractButton *)), this, SLOT(on_buttonBox_clicked(QAbstractButton *)));
}


QLayout* EditorSmart::create_ui()
{
    //! Search mode part
    QLabel *l1 = new QLabel(tr("Search mode"));
    l1->setFont(QFont("Arial",10,QFont::Bold));

    ui_search_mode = new QComboBox();
    ui_search_mode->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    ui_search_mode->addItem(tr("Match search term (AND)"));
    ui_search_mode->setItemData(0, MediaSearch::Type_And);
    ui_search_mode->addItem(tr("Match search term (OR)"));
    ui_search_mode->setItemData(1, MediaSearch::Type_Or);
    ui_search_mode->addItem(tr("Match all tracks (ALL)"));
    ui_search_mode->setItemData(2, MediaSearch::Type_All);
    QObject::connect(ui_search_mode,SIGNAL(currentIndexChanged(int)), SLOT(slot_search_mode_change()));

    
    QFrame *f1 = new QFrame();
    f1->setFrameShape(QFrame::HLine);
    f1->setFrameShadow(QFrame::Sunken);

    //! Search Query part
    QLabel *l2 = new QLabel(tr("Search terms"));
    l2->setFont(QFont("Arial",10,QFont::Bold));

    Search_Query_Widget * ui_search_query_widget = new Search_Query_Widget();

    listSearchWidget.append(ui_search_query_widget);

    ui_search_query_layout = new QVBoxLayout();
    ui_search_query_layout->addWidget(ui_search_query_widget);

    //! Add search Button
    ui_add_search_button = new QPushButton(tr("Add search query"));
    QObject::connect(ui_add_search_button, SIGNAL(clicked()), this, SLOT(on_button_AddSearch_clicked()));


    QFrame *f2 = new QFrame();
    f2->setFrameShape(QFrame::HLine);
    f2->setFrameShadow(QFrame::Sunken);

    //! Sort part
    QLabel *l3 = new QLabel(tr("Sort result mode"));
    l3->setFont(QFont("Arial",10,QFont::Bold));

    QGroupBox *groupBox_1 = new QGroupBox();
    groupBox_1->setFlat ( true );

    ui_check_sort_no     = new QRadioButton( tr("Keep original order") );
    ui_check_sort_random = new QRadioButton( tr("Random") );
    ui_check_sort_by     = new QRadioButton( tr("Sort by") );
    ui_check_sort_no->setChecked(true);
    ui_sort_field   =  new QComboBox();
    ui_sort_order   =  new QComboBox();
    ui_sort_field->setMinimumWidth(50);
    ui_sort_order->setMinimumWidth(50);

    // Populate the combo boxes
    for (int i=0; i<CST_SEARCH_FIELD_COUNT; i++) {
      ui_sort_field->addItem(SearchQuery::FieldName(SearchQuery::Search_Field(i)));
      ui_sort_field->setItemData(i, i);
    }

    connect(ui_sort_field, SIGNAL(currentIndexChanged(int)), SLOT(slot_updateSortOrder()));
    slot_updateSortOrder();

    QHBoxLayout* hboxlayout = new QHBoxLayout();
    hboxlayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hboxlayout->addWidget(ui_sort_field);
    hboxlayout->addWidget(ui_sort_order);


    QFormLayout * layout_1 = new QFormLayout();
    layout_1->addRow(ui_check_sort_no);
    layout_1->addRow(ui_check_sort_random);
    layout_1->addRow(ui_check_sort_by , hboxlayout);

    groupBox_1->setLayout(layout_1);


    QFrame *f3 = new QFrame();
    f3->setFrameShape(QFrame::HLine);
    f3->setFrameShadow(QFrame::Sunken);

    //! Limit part
    QLabel *l4 = new QLabel(tr("Limit"));
    l4->setFont(QFont("Arial",10,QFont::Bold));


    QGroupBox *groupBox_2 = new QGroupBox();
    groupBox_2->setFlat ( true );

    ui_check_limit_no     = new QRadioButton( tr("No limit") );
    ui_check_limit_at     = new QRadioButton( tr("Limit at") );
    ui_check_limit_no->setChecked(true);

    ui_spintime_limit     = new QSpinBox();
    ui_spintime_limit->setMinimumWidth(120);
    ui_spintime_limit->setMaximum(1000);
    ui_spintime_limit->setMinimum(15);

    QFormLayout * layout_2 = new QFormLayout();
    layout_2->addRow(ui_check_limit_no);
    layout_2->addRow(ui_check_limit_at, ui_spintime_limit);
    groupBox_2->setLayout(layout_2);

    //! Final Layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(l1);
    layout->addWidget(ui_search_mode);
    layout->addWidget(f1);

    layout->addWidget(l2);
    layout->addLayout(ui_search_query_layout);
    layout->addWidget(ui_add_search_button);
    layout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
    layout->addWidget(f2);

    layout->addWidget(l3);
    layout->addWidget(groupBox_1);
    layout->addWidget(f3);

    layout->addWidget(l4);
    layout->addWidget(groupBox_2);

    return layout;
}

/*******************************************************************************
    EditorSmart::setPlaylist
*******************************************************************************/
void EditorSmart::setPlaylist(MEDIA::PlaylistPtr playlist)
{
    Debug::debug() << "EditorSmart::setPlaylist";
  
    m_playlist = playlist;
    m_databaseId = playlist->id;
    
    /* update ui  */
    ui_edit_name->setText( playlist->name );
    
    
    MediaSearch media_search = qvariant_cast<MediaSearch>( playlist->rules );
      
    /* search mode */
    ui_search_mode->setCurrentIndex(ui_search_mode->findData(media_search.search_type_));

    /* search query */
    qDeleteAll(listSearchWidget);
    listSearchWidget.clear();

    foreach(SearchQuery query, media_search.query_list_) {
      Search_Query_Widget* widget =  new Search_Query_Widget();
      widget->set_query(query);
      add_search_query_widget(widget);
    }

    /* sort */
    ui_check_sort_no->setChecked( (int)media_search.sort_type_ == 0 );
    ui_check_sort_random->setChecked( (int)media_search.sort_type_ == 1 );
    ui_check_sort_by->setChecked( (int)media_search.sort_type_ > 1 );
    ui_sort_order->setCurrentIndex( (int)media_search.sort_type_ == 3 ? 1 : 0  );
    ui_sort_field->setCurrentIndex(ui_sort_field->findData(media_search.sort_field_));

    /* limit */
    ui_check_limit_no->setChecked(media_search.limit_ == -1);
    ui_check_limit_at->setChecked(media_search.limit_ != -1);
    ui_spintime_limit->setValue(media_search.limit_);
}


void EditorSmart::slot_updateSortOrder()
{
    const SearchQuery::Search_Field field = SearchQuery::Search_Field(ui_sort_field->currentIndex());
    const SearchQuery::Field_Type type    = SearchQuery::TypeOf(field);

    const QString asc  = SearchQuery::FieldSortOrderText(type, true);
    const QString desc = SearchQuery::FieldSortOrderText(type, false);

    const int old_current_index = !starting ? ui_sort_order->currentIndex() : 0;

    ui_sort_order->clear();
    ui_sort_order->addItem(asc);
    ui_sort_order->addItem(desc);
    ui_sort_order->setCurrentIndex(old_current_index);
}

void EditorSmart::slot_search_mode_change()
{
    MediaSearch::SearchType search_type = MediaSearch::SearchType(ui_search_mode->itemData(ui_search_mode->currentIndex()).toInt());

    const bool enable = (search_type == MediaSearch::Type_All) ? false : true;

    foreach(Search_Query_Widget *w, listSearchWidget) {
      w->setEnabled(enable);
    }
    ui_add_search_button->setEnabled(enable);
}


void EditorSmart::on_button_AddSearch_clicked()
{
    add_search_query_widget( new Search_Query_Widget() );
}


void EditorSmart::add_search_query_widget(Search_Query_Widget* widget)
{
    ui_search_query_layout->addWidget(widget);
    listSearchWidget.append(widget);

    QObject::connect(widget, SIGNAL(signalRemoveClicked()), this, SLOT(slot_remove_search_query_widget()));
}


void EditorSmart::slot_remove_search_query_widget()
{
    Search_Query_Widget * widget = qobject_cast<Search_Query_Widget*>(sender());
    if (!widget)
      return;

    ui_search_query_layout->removeWidget(widget);

    const int index = listSearchWidget.indexOf(widget);

    delete listSearchWidget.takeAt(index);

    update();
}


bool EditorSmart::is_search_valid()
{
    if( MediaSearch::Type_All == MediaSearch::SearchType(ui_search_mode->itemData(ui_search_mode->currentIndex()).toInt()) )
      return true;

    foreach(Search_Query_Widget *w, listSearchWidget)
    {
      if(!w->query().is_valid())
        return false;
    }

    return true;
}


void EditorSmart::on_buttonBox_clicked(QAbstractButton * button)
{
    Debug::debug() << "EditorSmart on_buttonBox_clicked";
  
    QDialogButtonBox::ButtonRole role = ui_buttonBox->buttonRole(button);

    switch (role)
    {
      case QDialogButtonBox::ApplyRole :
         if( is_search_valid() ) 
         {
           m_playlist->name = ui_edit_name->text();
           if(m_playlist->name.isEmpty())
             m_playlist->name = "no name";

           m_playlist->rules = QVariant::fromValue<MediaSearch>( get_search() );
   
           SmartPlaylist::updatePlaylist(m_playlist);
           ThreadManager::instance()->populateLocalPlaylistModel();
           emit close();
         }
         else
         {
           DialogMessage message(this , tr("Media Search Engine Dialog"));
           message.setMessage(tr("<p>Search query is not valid</p>"));
           message.resize(445, 120);
           message.exec();
         }
        break;

      case QDialogButtonBox::RejectRole :
         emit close();
         break;

      default:break;
    }
}

MediaSearch::SortType EditorSmart::sortType()
{
    if(ui_check_sort_no->isChecked())
    {
      return MediaSearch::Sort_No;
    }
    else if (ui_check_sort_random->isChecked())
    {
      return MediaSearch::Sort_Random;
    }
    else if(ui_check_sort_by->isChecked())
    {
     if( ui_sort_order->currentIndex() == 0)
       return MediaSearch::Sort_FieldAsc;
     else
       return MediaSearch::Sort_FieldDesc;
    }

    return MediaSearch::Sort_No;
}


int EditorSmart::limit()
{
    if(ui_check_limit_no->isChecked())
    {
      return -1;
    }
    else
    {
      return ui_spintime_limit->value();
    }
}


MediaSearch EditorSmart::get_search()
{
    MediaSearch  search;

    search.search_type_      = MediaSearch::SearchType(ui_search_mode->itemData(ui_search_mode->currentIndex()).toInt());
    search.sort_type_        = this->sortType();
    search.sort_field_       = SearchQuery::Search_Field(ui_sort_field->itemData(ui_sort_field->currentIndex()).toInt());
    search.limit_            = this->limit();

    search.query_list_.clear();

#ifdef TEST_FLAG
    int i = 1;
#endif
    foreach(Search_Query_Widget *w, listSearchWidget)
    {
      search.query_list_ << w->query();
#ifdef TEST_FLAG
      Debug::debug() << "EditorSmart::get_search query : " << i++;
      Debug::debug() << "EditorSmart::get_search query field: " << SearchQuery::FieldName(w->query().field_);
      Debug::debug() << "EditorSmart::get_search query operator: "
         << SearchQuery::OperatorText(SearchQuery::TypeOf(w->query().field_), w->query().operator_);

      Debug::debug() << "EditorSmart::get_search query value: " <<  w->query().value_;
#endif
    }

    return search;
}
