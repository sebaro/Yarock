

#include "scene_base.h"



/*
********************************************************************************
*                                                                              *
*    Class SceneBase                                                           *
*                                                                              *
********************************************************************************
*/
SceneBase::SceneBase(QWidget *parent) : QGraphicsScene(parent)
{
    m_parentWidget = parent;
    this->setParent(m_parentWidget);
    this->setItemIndexMethod(QGraphicsScene::NoIndex);
    this->setStickyFocus(true);
    
    is_initialised = false;
}


SceneBase::~SceneBase()
{
    clear(); 
}

