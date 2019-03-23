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

#include "iconmanager.h"
#include "debug.h"

#include <QDebug>
#include <QPalette>
#include <QApplication>
#include <QFontDatabase>


IconManager* IconManager::INSTANCE = 0;


/// The font-awesome icon painter
class IconManagerCharIconPainter: public IconManagerIconPainter
{
public:
    virtual void paint( IconManager* awesome, QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state, const QVariantMap& options  )
    {
        Q_UNUSED(mode);
        Q_UNUSED(state);
        Q_UNUSED(options);

        painter->save();


        // set the correct color
        QColor color = options.value("color").value<QColor>();
        QString text = options.value("text").toString();

        if( mode == QIcon::Disabled ) {
            color = options.value("color-disabled").value<QColor>();
            QVariant alt = options.value("text-disabled");
            if( alt.isValid() ) {
                text = alt.toString();
            }
        } else if( mode == QIcon::Active ) {
            color = options.value("color-active").value<QColor>();
            QVariant alt = options.value("text-active");
            if( alt.isValid() ) {
                text = alt.toString();
            }
        } else if( mode == QIcon::Selected ) {
            color = options.value("color-selected").value<QColor>();
            QVariant alt = options.value("text-selected");
            if( alt.isValid() ) {
                text = alt.toString();
            }
        }
        painter->setPen(color);

        // add some 'padding' around the icon
        int drawSize = qRound(rect.height()*options.value("scale-factor").toFloat());

        painter->setFont( awesome->font(drawSize) );
        painter->drawText( rect, text, QTextOption( Qt::AlignCenter|Qt::AlignVCenter ) );
        painter->restore();
    }

};


//---------------------------------------------------------------------------------------


/// The painter icon engine.
class IconManagerIconPainterIconEngine : public QIconEngine
{

public:

    IconManagerIconPainterIconEngine( IconManager* awesome, IconManagerIconPainter* painter, const QVariantMap& options  )
        : awesomeRef_(awesome)
        , iconPainterRef_(painter)
        , options_(options)
    {
    }

    virtual ~IconManagerIconPainterIconEngine() {}

    IconManagerIconPainterIconEngine* clone() const
    {
        return new IconManagerIconPainterIconEngine( awesomeRef_, iconPainterRef_, options_ );
    }

    virtual void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state)
    {
        Q_UNUSED( mode );
        Q_UNUSED( state );
        iconPainterRef_->paint( awesomeRef_, painter, rect, mode, state, options_ );
    }

    virtual QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
    {
        QPixmap pm(size);
        pm.fill( Qt::transparent ); // we need transparency
        {
            QPainter p(&pm);
            paint(&p, QRect(QPoint(0,0),size), mode, state);
        }
        return pm;
    }

private:

    IconManager* awesomeRef_;                  ///< a reference to the IconManager instance
    IconManagerIconPainter* iconPainterRef_;   ///< a reference to the icon painter
    QVariantMap options_;                    ///< the options for this icon painter
};


//---------------------------------------------------------------------------------------

/// The default icon colors
IconManager::IconManager( QObject* parent )
    : QObject( parent )
    , m_namedCodepoints()
{
    INSTANCE = this;
    
// set code point
    addNamedCodepoint("list", 0xe000);
    addNamedCodepoint("asterisk", 0xe001);
    addNamedCodepoint("calendar", 0xe002);
    addNamedCodepoint("chevron-down", 0xe003);
    addNamedCodepoint("chevron-left", 0xe004);
    addNamedCodepoint("chevron-right", 0xe005);
    addNamedCodepoint("chevron-up", 0xe006);
    addNamedCodepoint("check", 0xe007);
    addNamedCodepoint("clock", 0xe008);
    addNamedCodepoint("desktop", 0xe009);
    addNamedCodepoint("heart", 0xe00a);
    addNamedCodepoint("heart-o", 0xe00b);
    addNamedCodepoint("music", 0xe00c);
    addNamedCodepoint("media-pause", 0xe00d);
    addNamedCodepoint("pencil", 0xe00e);
    addNamedCodepoint("media-play", 0xe00f);
    addNamedCodepoint("plus", 0xe010);
    addNamedCodepoint("goto", 0xe011);
    addNamedCodepoint("media-prev", 0xe012);
    addNamedCodepoint("media-next", 0xe013);
    addNamedCodepoint("tag", 0xe014);
    addNamedCodepoint("tags", 0xe015);
    addNamedCodepoint("undo", 0xe016);
    addNamedCodepoint("artist", 0xe017);
    addNamedCodepoint("arrows-alt", 0xe018);
    addNamedCodepoint("setting", 0xe019);
    
    addNamedCodepoint("home", 0xe01a);
    addNamedCodepoint("history", 0xe01b);
    addNamedCodepoint("delete", 0xe01c);
    addNamedCodepoint("computer", 0xe01d);
    addNamedCodepoint("info-circle", 0xe01e);
    addNamedCodepoint("cd", 0xe01f);
    addNamedCodepoint("track", 0xe020);
    addNamedCodepoint("vynil", 0xe021);
    addNamedCodepoint("bulb", 0xe022);
    addNamedCodepoint("chart", 0xe023);
    addNamedCodepoint("download", 0xe024);
    addNamedCodepoint("random", 0xe025);
    addNamedCodepoint("repeat", 0xe026);
    addNamedCodepoint("globe", 0xe027);
    addNamedCodepoint("playlist1", 0xe028);
    addNamedCodepoint("equalizer", 0xe029);
    addNamedCodepoint("media-stop", 0xe02a);

    addNamedCodepoint("repeat1", 0xe02b);
    addNamedCodepoint("repeat2", 0xe02c);
    addNamedCodepoint("repeat3", 0xe02d);

    addNamedCodepoint("shuffle1", 0xe02e);
    addNamedCodepoint("shuffle2", 0xe02f);
    addNamedCodepoint("shuffle3", 0xe030);    

   
    
    // initialize the default options
    setDefaultOption( "color", QApplication::palette().color(QPalette::Normal,QPalette::WindowText));
 
    //setDefaultOption( "color", QColor(50,50,50) );
    setDefaultOption( "color-disabled",  QApplication::palette().color(QPalette::Disabled,QPalette::WindowText));
    setDefaultOption( "color-active", QApplication::palette().color(QPalette::Normal,QPalette::Highlight));
    setDefaultOption( "color-selected", QApplication::palette().color(QPalette::Active,QPalette::Highlight));
    setDefaultOption( "scale-factor", 0.9 );

    setDefaultOption( "text", QVariant() );
    setDefaultOption( "text-disabled", QVariant() );
    setDefaultOption( "text-active", QVariant() );
    setDefaultOption( "text-selected", QVariant() );

    m_fontIconPainter = new IconManagerCharIconPainter();

    this->init("yarock-font");
    
}


IconManager::~IconManager()
{
    delete m_fontIconPainter;
    
    qDeleteAll(painterMap_);
}


/// initializes the IconManager icon factory with the given fontname
void IconManager::init(const QString& fontname)
{
    fontName_ = fontname;
}



bool IconManager::initFontAwesome( )
{
    //! load awesome font
    if (QFontDatabase::addApplicationFont(":/fonts/yarock-font.ttf") < 0)
    {
        Debug::debug() << "Error loading font file !";
        return false;
    }

    Debug::debug() << "Loading font OK !";
    
    return true;
}

void IconManager::addNamedCodepoint( const QString& name, int codePoint)
{
    m_namedCodepoints.insert( name, codePoint);
}


/// Sets a default option. These options are passed on to the icon painters
void IconManager::setDefaultOption(const QString& name, const QVariant& value)
{
    defaultOptions_.insert( name, value );
}




/// Returns the default option for the given name
QVariant IconManager::defaultOption(const QString& name)
{
    return defaultOptions_.value( name );
}


// internal helper method to merge to option amps
static QVariantMap mergeOptions( const QVariantMap& defaults, const QVariantMap& override )
{
    QVariantMap result= defaults;
    if( !override.isEmpty() ) {
        QMapIterator<QString,QVariant> itr(override);
        while( itr.hasNext() ) {
            itr.next();
            result.insert( itr.key(), itr.value() );
        }
    }
    return result;
}



QIcon IconManager::icon(const QString& name, QString colorstyle, QColor color)
{
    QVariantMap vmap;

    if( colorstyle == "normal")
    {
      vmap["color"] = QApplication::palette().color(QPalette::Normal,QPalette::WindowText);
    }
    else if ( colorstyle == "active")
    {
      vmap["color"] = QApplication::palette().color(QPalette::Normal,QPalette::Highlight);
    }
    else if ( colorstyle == "selected")
    {
      vmap["color"] = QApplication::palette().color(QPalette::Active,QPalette::Highlight);
    }   
    else if ( colorstyle == "disabled")
    {
        vmap["color"] = QApplication::palette().color(QPalette::Disabled,QPalette::WindowText);
    }      
    else if ( colorstyle == "custom")
    {
        vmap["color"] = color;
    } 
    
    return icon(name, vmap);
}




QIcon IconManager::icon(const QString& name, const QVariantMap& options)
{    
    if( m_namedCodepoints.contains(name) )
    {
        //Debug::debug() << " ########### loading font value: " + QString::number( m_namedCodepoints.value(name));
        
        // create a merged QVariantMap to have default options and icon-specific options
        QVariantMap optionMap = mergeOptions( defaultOptions_, options );
        optionMap.insert("text", QString( QChar(m_namedCodepoints.value(name))) );

        return icon( m_fontIconPainter, optionMap );
    }
    else        
    {
        Debug::warning() << "Loading font for: " + name + "=> NOT FOUND";
        
        return QIcon();
    }
}



QIcon IconManager::icon(IconManagerIconPainter* painter, const QVariantMap& optionMap )
{
    // Warning, when you use memoryleak detection. You should turn it of for the next call
    // QIcon's placed in gui items are often cached and not deleted when my memory-leak detection checks for leaks.
    // I'm not sure if it's a Qt bug or something I do wrong
    IconManagerIconPainterIconEngine* engine = new IconManagerIconPainterIconEngine( this, painter, optionMap  );
    return QIcon( engine );
}



QFont IconManager::font( int size )
{
    QFont font( fontName_);
    font.setPixelSize(size);
    return font;
} 
