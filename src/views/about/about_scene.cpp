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
#include "about_scene.h"
#include "constants.h"

#include "views/item_common.h"
#include "debug.h"

// Qt
#include <QtCore>
#include <QLabel>
#include <QGraphicsProxyWidget>
#include <QHBoxLayout>
#include <QFrame>
#include <QTextBrowser>

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
 
    /* content page */ 
    QWidget* main_widget = new QWidget();
    main_widget->setAttribute(Qt::WA_NoBackground, true);
    main_widget->setAutoFillBackground(true);
    main_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    main_widget->setMinimumWidth(600);
    main_widget->setMinimumHeight(600);
    

    QLabel *title = new QLabel(main_widget);
    title->setText(APP_NAME);

    QFont title_font;
    title_font.setBold(true);
    title_font.setPointSize(title_font.pointSize() + 4);
    title->setFont(title_font);

    QLabel *version = new QLabel(main_widget);
    version->setText(VERSION);

    QLabel *copyright = new QLabel(main_widget);
    copyright->setText("(c) 2010-2014 yarock author");

    QLabel *icon = new QLabel(main_widget);
    icon->setPixmap( QPixmap(":/icon/yarock_64x64.png") );
    icon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QFrame *line = new QFrame(main_widget);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout * vl0 = new QVBoxLayout();
    vl0->setContentsMargins(10,0,0,0);
    vl0->addWidget(title);
    vl0->addWidget(version);
    vl0->addWidget(copyright);
    vl0->addWidget(line);

    QVBoxLayout * vl1 = new QVBoxLayout();
    vl1->addWidget(icon);
    vl1->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));



    QTextBrowser* content = new QTextBrowser();
    content->setOpenExternalLinks(true);
    content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    content->setStyleSheet("QTextBrowser { background:transparent }");
    content->setFrameShape(QFrame::NoFrame);
    content->setMinimumHeight(150);


    content->setHtml("<dl>"
                     "<dt><b>About</b></dt>"
                     "<dd>"
                     "Yarock is Qt4 Modern Music Player designed to provide an easy and pretty music collection browser based on cover art."
                     " It's a Free Software distributed under the <a href=\"http://www.gnu.org/licenses/gpl-3.0.en.html\">(GPLv3)</a> license."
                     " Project is hosted on <a href=\"https://launchpad.net/yarock\">launchpad</a> repository."
                     "</dd>"
                     "<br>"
                     "<dt><b>Author</b></dt>"
                     "<dd>"
                     "Yarock is developped by <b>Sebastien Amardeilh</b> <a href=\"mailto:sebastien.amardeilh+yarock@gmail.com\">(sebastien.amardeilh+yarock@gmail.com)</a>"
                     "</dd>"
                     "<br>"
                     "<dt><b>Bugs report</b></dt>"
                     "<dd>Please use <a href=\"https://bugs.launchpad.net/yarock\">https://bugs.launchpad.net/yarock</a> to report bugs."
                     "</dd>"
                     "<br>"
                     "<dt><b>Translations</b></dt>"
                     "<dd>Please use <a href=\"https://www.transifex.net/projects/p/yarock-translation/\">transifex</a> for translations."
                     "</dd>"
                     "<br>"
                     "<dt><b>Donation</b></dt>"
                     "<dd>If you can, please <a href=\"https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=PBL2A87TRZPH4\">donate</a> to support the development</dd>"
                     "</dl>");

     
    vl0->addWidget(content);
    
    /*      final layout   */
    QHBoxLayout * hl0 = new QHBoxLayout();
    hl0->addLayout(vl1);
    hl0->addLayout(vl0);
    
    main_widget->setLayout(hl0);
    
    /* proxy widget */
    QGraphicsProxyWidget* proxy_widget = new QGraphicsProxyWidget( );
    proxy_widget->setWidget( main_widget );
    proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );    
    
    proxy_widget->setPos(10, 30);
    
    
    /* proxy widget */
    this->addItem(proxy_widget);
    
    
    /* ajust SceneRect */
    setSceneRect ( itemsBoundingRect().adjusted(0, -10, 0, 40) );
}

