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

#ifndef _CONTEXT_WIDGETS_H_
#define _CONTEXT_WIDGETS_H_

#include "views/context/context_item.h"
#include "views/item_button.h"
#include "views/item_common.h"

#include "mediaitem.h"
#include "info_system.h"

#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QPixmap>
#include <QString>
#include <QByteArray>

/*
********************************************************************************
*                                                                              *
*    Class ArtistInfoWidget                                                    *
*                                                                              *
********************************************************************************
*/
class ArtistInfoWidget : public QGraphicsWidget
{
Q_OBJECT
  public:
    ArtistInfoWidget(QWidget* parentView = 0);

    void set_artist_name(const QString &);
    void setData(INFO::InfoRequestData request, QVariant data);

    void clear();

    void update();

  private:
    void set_long_bio();
    void set_short_bio();

  private :
    QWidget                 *m_parent;

    CategorieLayoutItem     *m_title;
    QLabel                  *m_image;
    WebLinkItem             *m_button;

    TextGraphicItem         *m_bio;
    TextGraphicItem         *m_subtitle;

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    
  signals:
    void updated();
};

/*
********************************************************************************
*                                                                              *
*    Class ArtistSimilarWidget                                                 *
*                                                                              *
********************************************************************************
*/
class ArtistSimilarWidget : public QGraphicsWidget
{
Q_OBJECT
  public:
    ArtistSimilarWidget(QWidget* parentView = 0);

    void set_artist_name(const QString &);
    void setData(INFO::InfoRequestData request, QVariant data);
    
    void clear();
    void update();

  private :
    QSizeF doLayout(bool) const;

  private :
    QWidget                 *m_parent;
    CategorieLayoutItem     *m_title;
    
    /* map <provider release id, ArtistThumbGraphicItem*> */
    QMap<QString /*id*/, ArtistThumbGraphicItem*>   m_artists;
        
  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  signals:
    void updated();
};


/*
********************************************************************************
*                                                                              *
*    Class DiscoInfoWidget                                                     *
*                                                                              *
********************************************************************************
*/
class DiscoInfoWidget : public QGraphicsWidget
{
Q_OBJECT
  public:
    DiscoInfoWidget(QWidget* parentView = 0);

    void clear();
    void update();
    void setData(INFO::InfoRequestData request , QVariant data);

  private :
    QSizeF doLayout(bool) const;

  private :
    QWidget                 *m_parent;
    CategorieLayoutItem     *m_title;
    
    /* map <artist-album hash, AlbumThumbGraphicItem*> */
    QMap<QString, AlbumThumbGraphicItem*>   m_albums;

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  signals:
    void updated();
};

/*
********************************************************************************
*                                                                              *
*    Class LyricsInfoWidget                                                    *
*                                                                              *
********************************************************************************
*/
class LyricsInfoWidget : public QGraphicsWidget
{
Q_OBJECT

  public:
    LyricsInfoWidget(QWidget* parentView = 0);

    void clear();
    void update();

  public slots:
    void setData(INFO::InfoRequestData request , QVariant data);
    
  private :
    QWidget                 *m_parent;

    CategorieLayoutItem     *m_title;
    TextGraphicItem         *m_lyrics;
    TextGraphicItem         *m_subtitle;
    QLabel                  *m_header;

    ButtonItem              *m_button_add;
    ButtonItem              *m_button_remove;
    WebLinkItem             *m_button_link;

    bool                     m_lyrics_found;

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  private slots:
    void slot_add_lyrics();
    void slot_remove_lyrics();

  signals:
    void updated();
};


#endif // _CONTEXT_WIDGETS_H_
