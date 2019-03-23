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


#ifndef _ICON_MANAGER_H_
#define _ICON_MANAGER_H_


#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QRect>
#include <QVariantMap>



//---------------------------------------------------------------------------------------

class IconManagerIconPainter;

/// The main class for managing icons
/// This class requires a 2-phase construction. You must first create the class and then initialize it via an init* method
class IconManager : public QObject
{
Q_OBJECT
    static IconManager* INSTANCE;
public:

    explicit IconManager(QObject *parent = 0);
    virtual ~IconManager();
    static IconManager* instance() { return INSTANCE; }
 
    void init( const QString& fontname );
    bool initFontAwesome();

    void addNamedCodepoint( const QString& name, int codePoint );
    QHash<QString,uint> namedCodePoints() { return m_namedCodepoints; }

    void setDefaultOption( const QString& name, const QVariant& value  );
    QVariant defaultOption( const QString& name );

    QIcon icon( const QString& name, const QVariantMap& options = QVariantMap() );
    QIcon icon(const QString& name, QString colorstyle, QColor color = QColor());

    QFont font( int size );

    /// Returns the font-name that is used as icon-map
    QString fontName() { return fontName_ ; }

private:
    QString fontName_;
    QHash<QString,uint> m_namedCodepoints;                   

    QHash<QString, IconManagerIconPainter*> painterMap_;
    QVariantMap defaultOptions_;
    IconManagerIconPainter* m_fontIconPainter;
    
private:
    QIcon icon(IconManagerIconPainter* painter, const QVariantMap& optionMap = QVariantMap() );    
};


//---------------------------------------------------------------------------------------


/// The IconManagerIconPainter is a specialized painter for painting icons
/// your can implement an iconpainter to create custom font-icon code
class IconManagerIconPainter
{
public:
    virtual ~IconManagerIconPainter() {}
    virtual void paint( IconManager* awesome, QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state, const QVariantMap& options ) = 0;
};


#endif // _ICON_MANAGER_H_ 
