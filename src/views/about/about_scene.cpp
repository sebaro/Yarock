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
#include "about_scene.h"
#include "config.h"

#include "views/item_common.h"
#include "views/context/context_item.h"
#include "engine.h"
#include "iconmanager.h"
#include "debug.h"

// Qt
#include <QtCore>
#include <QLabel>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>
#include <QHBoxLayout>
#include <QApplication>


const QString S_MAIL      = "<a href=\"mailto:sebastien.amardeilh+yarock@gmail.com\">(sebastien.amardeilh+yarock@gmail.com)</a>";
const QString S_LAUNCHPAD = "<a href=\"https://bugs.launchpad.net/yarock\">https://bugs.launchpad.net/yarock</a>";
const QString S_TRANSLATE = "<a href=\"https://www.transifex.net/projects/p/yarock-translation/\">transifex</a>";
const QString S_DONATE    = "<a target=”_blank” href=\"https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=PBL2A87TRZPH4\">donate</a>";
const QString S_DONATE2   = "<a href=\"http://qt-apps.org/content/donate.php?content=129372\">other donate link</a>";
const QString S_WEBSITE   = "<a href=\"http://seb-apps.github.io/yarock/\">website</a>";

/*
********************************************************************************
*                                                                              *
*    Class AboutScene                                                          *
*                                                                              *
********************************************************************************
*/
AboutScene::AboutScene(QWidget* parent) : SceneBase(parent)
{
}


/*******************************************************************************
    initScene
*******************************************************************************/
void AboutScene::initScene()
{
    setInit(true);
}


/*******************************************************************************
    resizeScene
*******************************************************************************/
void AboutScene::resizeScene()
{
    update();
} 

/*******************************************************************************
    populateScene
*******************************************************************************/
void AboutScene::populateScene()
{
    Debug::debug() << "   [AboutScene] populateScene";    
    clear();
 
    /* proxy widget content */
      QWidget* widget1 = new QWidget();
      widget1->setAttribute(Qt::WA_NoBackground, true);
      widget1->setAutoFillBackground(true);
      widget1->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    

      QLabel *title = new QLabel(widget1);
      title->setText(APP_NAME);

      QFont title_font;
      title_font.setBold(true);
      title_font.setPointSize(title_font.pointSize() + 4);
      title->setFont(title_font);

      QLabel *version = new QLabel(widget1);
      version->setText(VERSION);

      QLabel *copyright = new QLabel(widget1);
      copyright->setText("(c) 2010-2015 Sebastien Amardeilh");

      QLabel *icon = new QLabel(widget1);
      icon->setPixmap( QPixmap(":/icon/yarock_64x64.png") );
      icon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

      QVBoxLayout * vl0 = new QVBoxLayout();
      vl0->setContentsMargins(0,0,0,0);
      vl0->addWidget(title);
      vl0->addWidget(version);
      vl0->addWidget(copyright);

      QVBoxLayout * vl1 = new QVBoxLayout();
      vl1->addWidget(icon);

   
      QHBoxLayout * hl0 = new QHBoxLayout();
      hl0->addLayout(vl1);
      hl0->addLayout(vl0);
      widget1->setLayout(hl0);


    /* ------------------ ABOUT HEADER ------------------  */    
    int Ypos = 10;
    
    CategorieGraphicItem *cat = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    cat->m_name = tr("About");
    cat->setPos( 0 ,Ypos);
    addItem(cat);
    
    Ypos += 30;
    
    /* proxy widget */
    QGraphicsProxyWidget* proxy_widget = new QGraphicsProxyWidget( );
    proxy_widget->setWidget( widget1 );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );    
    
    proxy_widget->setPos(20, Ypos);
    addItem(proxy_widget);

    Ypos += widget1->height() +10;
    
    
    TextGraphicItem *text0 = new TextGraphicItem();
    text0->setOpenExternalLinks(true);
    text0->setHtml( QString(tr(
                "Yarock is a Qt4 modern music player designed to provide an easy and pretty music collection browser based on cover art.<br>"
                "It's a Free Software distributed under the <a href=\"http://www.gnu.org/licenses/gpl-3.0.en.html\">(GPLv3)</a> license.<br>"
                            )));
            
    text0->setPos( 20, Ypos);
    addItem(text0);
    
    Ypos += text0->height() + 10;
    
    /* ------------------ CONTRIBUTE ------------------  */    
    CategorieGraphicItem *cat2 = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    cat2->m_name = tr("Contribute");
    cat2->setPos( 0 ,Ypos);
    addItem(cat2);    
       
    Ypos += 35;
    
    
    /* WEBSITE */    
    TextGraphicItem *text1 = new TextGraphicItem();
    text1->setOpenExternalLinks(true);
    text1->setHtml( QString("Yarock %1").arg(S_WEBSITE) );
            
    text1->setPos( 40, Ypos);
    addItem(text1);
        
    Ypos += text1->height() + 10;
                   
    /* AUTHOR */    
    QGraphicsPixmapItem* pix2 = this->addPixmap( IconManager::instance()->icon( "artist").pixmap(QSize(18,18)));
    pix2->setPos( 20, Ypos+5);
                
    TextGraphicItem *text2 = new TextGraphicItem();
    text2->setOpenExternalLinks(true);
    text2->setHtml( QString(tr("Yarock is developped by <b>Sebastien Amardeilh</b> %1")).arg(S_MAIL) );
            
    text2->setPos( 40, Ypos);
    addItem(text2);
    
    
    Ypos += text2->height() + 10;
    
    /* BUGREPORT */    
    TextGraphicItem *text3 = new TextGraphicItem();
    text3->setOpenExternalLinks(true);
    text3->setHtml( QString(tr("Please use %1 to report bugs.")).arg(S_LAUNCHPAD) );
            
    text3->setPos( 40, Ypos);
    addItem(text3);
        
    Ypos += text3->height() + 10;    
    
    /* Translations */    
    QGraphicsPixmapItem* pix4 = this->addPixmap( QPixmap(":/images/rename-48x48.png").scaled(QSize(18, 18), Qt::KeepAspectRatio, Qt::SmoothTransformation) );
    pix4->setPos( 20, Ypos+5);
    
    TextGraphicItem *text4 = new TextGraphicItem();
    text4->setOpenExternalLinks(true);
    text4->setHtml( QString(tr("Please use %1 for translations.")).arg(S_TRANSLATE) );
            
    text4->setPos( 40, Ypos);
    addItem(text4);
        
    Ypos += text4->height() + 10;
  
    /* DONATE */
    QGraphicsPixmapItem* pix5 = this->addPixmap( QPixmap(":/images/favorites-18x18.png") );
    pix5->setPos( 20, Ypos+5);
                
    TextGraphicItem *text5 = new TextGraphicItem();
    text5->setOpenExternalLinks(true);
    text5->setHtml( QString(tr("If you can, please %1 (or %2) to support the development"))
                    .arg(S_DONATE,S_DONATE2) );
    text5->setPos( 40, Ypos);
    addItem(text5);

    Ypos += text5->height() + 30;    
    
    /* ------------------ DEBUG INFO ------------------  */    
    CategorieGraphicItem *cat3 = new CategorieGraphicItem(qobject_cast<QGraphicsView*> (parentView())->viewport());
    cat3->m_name = tr("Debug Information");
    cat3->setPos( 0 ,Ypos);
    addItem(cat3);    
       
    Ypos += 35;

    TextGraphicItem *text6 = new TextGraphicItem();
    text6->setHtml( QString(tr("<b>Compiled with Qt Version</b>: %1")).arg(QT_VERSION_STR) );
    text6->setPos( 40, Ypos);
    addItem(text6);
 
    Ypos += text6->height() + 10;
    
    TextGraphicItem *text7 = new TextGraphicItem();
    text7->setHtml( QString(tr("<b>Qt Runtime Version</b>: %1")).arg(qVersion()) );
    text7->setPos( 40, Ypos);
    addItem(text7);
    
    Ypos += text7->height() + 10;
    
    QString audio_engine_info = Engine::instance() ? 
        Engine::instance()->name() + "-" + Engine::instance()->version() : 
        QString("no engine");
    
    TextGraphicItem *text8 = new TextGraphicItem();
    text8->setHtml( QString(tr("<b>Audio engine</b>: %1")).arg(audio_engine_info) );
    text8->setPos( 40, Ypos);
    addItem(text8);


    /* ------------------ ajust SceneRect ------------------  */
    setSceneRect ( itemsBoundingRect().adjusted(0, -10, 0, 40) );
}

