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

#include "context_widget.h"

#include "lyricseditor.h"
#include "core/player/engine.h"
#include "utilities.h"
#include "debug.h"
#include "covercache.h"

#include <QGraphicsView>
#include <QCryptographicHash>
#include <QGraphicsProxyWidget>


/*
********************************************************************************
*                                                                              *
*    Class ArtistInfoWidget                                                    *
*                                                                              *
********************************************************************************
*/
ArtistInfoWidget::ArtistInfoWidget(QWidget* parentView) :
  QGraphicsWidget(0)
{
    m_parent       = parentView;

    m_title   = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());

    m_subtitle = new TextGraphicItem();
    m_subtitle->setParentItem(this);
    
    QFont font = QApplication::font();
    font.setPointSize( font.pointSize() + 1 );
    font.setBold( true );
    m_subtitle->setFont(font);
    
    
    m_bio     =  new TextGraphicItem();
    m_bio->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_bio->setTextWidth(m_parent->width() - 240);

    m_button  = new WebLinkItem();
    m_button->setParentItem(this);

    m_image = new QLabel;
    m_image->setAttribute( Qt::WA_NoSystemBackground, true );
    m_image->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
    m_image->setMaximumWidth( 200 );

    QGraphicsProxyWidget *image_proxy_widget = new QGraphicsProxyWidget( this );
    image_proxy_widget->setWidget( m_image );
    image_proxy_widget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    
    QGraphicsLinearLayout* m_layout_v = new QGraphicsLinearLayout( Qt::Vertical );
    m_layout_v->setContentsMargins(20,0,0,0);
    m_layout_v->addItem( m_subtitle );
    m_layout_v->addItem( m_bio );

    QGraphicsLinearLayout* m_layout_h = new QGraphicsLinearLayout( Qt::Horizontal );
    m_layout_h->setContentsMargins(0,0,0,0);
    m_layout_h->setSpacing(10);
    m_layout_h->addItem( m_layout_v );
    m_layout_h->addItem( image_proxy_widget );
    m_layout_h->setAlignment( image_proxy_widget, Qt::AlignTop );
    
    QGraphicsLinearLayout* m_layout   = new QGraphicsLinearLayout( Qt::Vertical , this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->addItem( m_title );
    m_layout->addItem( m_layout_h );
    
    this->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
}

void ArtistInfoWidget::update()
{
    prepareGeometryChange();

    this->layout()->invalidate();

    updateGeometry();

    QGraphicsWidget::update();

    emit updated();
}

QSizeF ArtistInfoWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);
    return QSizeF( m_parent->width(),
                   qMax(/*title*/ 30 + m_image->sizeHint().height() + 30 /*margin*/, 
                        /*title*/ 30 + m_bio->height() + m_button->height()+ 30 /*margin*/)
                 );
}

void ArtistInfoWidget::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)

    m_bio->setTextWidth(m_parent->width() - m_image->width() -60);
    m_bio->updateItem();

    this->update();
}

void ArtistInfoWidget::set_artist_name(const QString &name)
{
    m_title->m_name  = name;
}

void ArtistInfoWidget::clear()
{
    m_image->clear();
    m_bio->clear();
    
    update();
}


void ArtistInfoWidget::setData(INFO::InfoRequestData request, QVariant data)
{
    if(request.type == INFO::InfoArtistBiography ) 
    {
      QVariantMap biohash =  qvariant_cast<QVariantMap>(data);

      if(!biohash.isEmpty())
      {     
        QString html = QString("<html>");
        QString text = biohash["text"].toString();
  
        int split  = text.indexOf('\n', 1024);
        if(split == -1)
          split = text.indexOf(". ", 1024);

        html += QString("<p align=\"justify\">");
        html += text.left(split);

        if (split != -1)
          html += "...";
      
        html += "</p></html>";
    
        m_bio->show();
        m_bio->setHtml(html);
        m_bio->updateItem();
            
        m_subtitle->setPlainText(tr("Biography"));

        m_button->setLink(biohash["url"].toString()); 
        m_button->setText(biohash["site"].toString()); 
        m_button->setPos(m_subtitle->x() + m_subtitle->width() + 30,m_subtitle->y() + 4);

        this->update();
      }
   }
   else if(request.type == INFO::InfoArtistImages ) 
   {
      const QByteArray bytes = data.toByteArray();

      QPixmap cover;
      cover.loadFromData( bytes );

      if(!cover.isNull()) 
      {
        QPixmap pix = UTIL::squareCenterPixmap( cover ).scaled(QSize(200,200), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        m_image->setPixmap( UTIL::createRoundedImage( pix ) );
        m_image->adjustSize();
        this->update();     
      }
   }
}


/*
********************************************************************************
*                                                                              *
*    Class ArtistSimilarWidget                                                 *
*                                                                              *
********************************************************************************
*/
ArtistSimilarWidget::ArtistSimilarWidget(QWidget* parentView) :
  QGraphicsWidget(0)
{
    m_parent       = parentView;

    m_title   = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->setPos(0,0);
    m_title->setParentItem(this);
}

void ArtistSimilarWidget::clear()
{
    qDeleteAll(m_artists);
    m_artists.clear();

    m_title->hide();
    m_title->m_name.clear();
    
    update();
}

void ArtistSimilarWidget::update()
{
    prepareGeometryChange();

    updateGeometry();

    doLayout(true);

    QGraphicsWidget::update();

    emit updated();
}

QSizeF ArtistSimilarWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);
    return doLayout(false);
}

void ArtistSimilarWidget::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    this->update();
}


void ArtistSimilarWidget::setData(INFO::InfoRequestData request, QVariant data)
{
    if(request.type == INFO::InfoArtistSimilars ) 
    {
        foreach(QVariant artist, data.toList())
        {
            QVariantMap artistMap =  qvariant_cast<QVariantMap>(artist);

            ArtistThumbGraphicItem* item = new ArtistThumbGraphicItem();

            item->m_name = artistMap["name"].toString();
            item->setParentItem(this);
            m_artists[artistMap["name"].toString()] = item;
        }
        
        if(!data.toList().isEmpty()) {
          m_title->m_name = tr("Similar artists");
          m_title->show();
        }
    }
    else if(request.type == INFO::InfoArtistImages ) 
    {
        INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();

        //Debug::debug() << Q_FUNC_INFO << "ArtistSimilarWidget : ";

        if(m_artists.contains( hash["artist"] ))
        {
            //Debug::debug() << Q_FUNC_INFO << "artist found in map in m_artists";
            const QByteArray bytes = data.toByteArray();

            QPixmap cover;
            cover.loadFromData( bytes );

            if(!cover.isNull()) {
              m_artists[hash["artist"]]->m_pix = 
              QPixmap( UTIL::squareCenterPixmap( cover ) ).scaled(QSize(120,120), Qt::KeepAspectRatio, Qt::SmoothTransformation);
              m_artists[hash["artist"]]->update(); 
            }
        }
    } 
    
    this->update();       
}


QSizeF ArtistSimilarWidget::doLayout(bool redraw) const
{
     if(m_title->m_name.isEmpty())
       return QSize(0,0);
     
     m_title->show();

     qreal left, top, right, bottom;
     getContentsMargins(&left, &top, &right, &bottom);

     const int HSpacing = 20;
     const int VSpacing = 20;
     left   += 40;
     right  += 40;
     top    += 40;
     bottom += 20;

     const qreal maxw = m_parent->width() - left - right;

     qreal x = 0;
     qreal y = 0;
     qreal maxRowHeight = 0;
     QSizeF pref;

     QList<ArtistThumbGraphicItem*> artists = m_artists.values();

     for (int i = 0; i < artists.count(); ++i) 
     {
         ArtistThumbGraphicItem* artist = artists.at(i);
         pref = artist->effectiveSizeHint(Qt::PreferredSize);
         maxRowHeight = qMax(maxRowHeight, pref.height());

         qreal next_x;
         next_x = x + pref.width();
         if (next_x > maxw) {
             if (x == 0) {
                 pref.setWidth(maxw);
             } else {
                 x = 0;
                 next_x = pref.width();
             }
             y += maxRowHeight + VSpacing;
             maxRowHeight = 0;
         }
         if(redraw) {
           artist->setPos(QPointF(left + x, top + y));
         }

         x = next_x + HSpacing;
     }
     maxRowHeight = qMax(maxRowHeight, pref.height());

     return QSizeF(maxw, top + y + maxRowHeight + bottom);
}



/*
********************************************************************************
*                                                                              *
*    Class DiscoInfoWidget                                                     *
*                                                                              *
********************************************************************************
*/
DiscoInfoWidget::DiscoInfoWidget(QWidget* parentView) :
  QGraphicsWidget(0)
{
    m_parent       = parentView;

    m_title   = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->setPos(0,0);
    m_title->setParentItem(this);
}

void DiscoInfoWidget::setData(INFO::InfoRequestData request , QVariant data)
{
    //Debug::debug() << "######" << Q_FUNC_INFO;
    INFO::InfoStringHash input = request.data.value< INFO::InfoStringHash >();
         
    if(request.type == INFO::InfoArtistReleases ) 
    {
        QVariantMap vmap =  qvariant_cast<QVariantMap>(data);

        foreach(QVariant release, vmap.value("releases").toList())
        {
            QVariantMap releasevmap =  qvariant_cast<QVariantMap>(release);
            
            const QString akey =  INFO::albumKey(input.value("artist"), releasevmap["album"].toString());
            
            if( m_albums.keys().contains(akey) )
              continue;

            AlbumThumbGraphicItem* album_item = new AlbumThumbGraphicItem();

            album_item->m_title = releasevmap["album"].toString();
            album_item->m_year  = releasevmap["year"].toString();
            album_item->setParentItem(this);
            m_albums[ akey ] = album_item;
        }
        
        if(!vmap.value("releases").toList().isEmpty()) {
          m_title->m_name = tr("Discography");
          m_title->show();
        }        
    }
    else if(request.type == INFO::InfoAlbumCoverArt ) 
    {
        const QString akey =  INFO::albumKey(input.value("artist"), input.value("album"));
            
        if(m_albums.contains( akey ))
        {
            //Debug::debug() << "#####" << Q_FUNC_INFO << "INFO::InfoAlbumCoverArt : album found " << input.value("album");
            const QByteArray bytes = data.toByteArray();

            if( !bytes.isEmpty() ) {
              QPixmap cover;
              cover.loadFromData( bytes );

              if(!cover.isNull()) {
                m_albums[akey]->m_pix = cover.scaled(QSize(120, 120), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                m_albums[akey]->update();
              }
            }
        }
    }
  
    this->update();
}
    
void DiscoInfoWidget::clear()
{
    //Debug::debug() << "DiscoInfoWidget::clear ";
    qDeleteAll(m_albums);
    m_albums.clear();
    m_title->hide();
    m_title->m_name.clear();
    
    update();    
}

void DiscoInfoWidget::update()
{
    prepareGeometryChange();

    updateGeometry();

    doLayout(true);

    QGraphicsWidget::update();

    emit updated();
}

QSizeF DiscoInfoWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which)
Q_UNUSED(constraint);
    return doLayout(false);
}

void DiscoInfoWidget::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)
    this->update();
}

QSizeF DiscoInfoWidget::doLayout(bool redraw) const
{
     if(m_title->m_name.isEmpty())
       return QSize(0,0);
     
     m_title->show();
     //Debug::debug() << "DiscoInfoWidget::doLayout ";
     qreal left, top, right, bottom;
     getContentsMargins(&left, &top, &right, &bottom);

     const int HSpacing = 20;
     const int VSpacing = 20;
     left   += 40;
     right  += 40;
     top    += 40;
     bottom += 20;
     
     const qreal maxw = m_parent->width() - left - right;

     qreal x = 0;
     qreal y = 0;
     qreal maxRowHeight = 0;
     QSizeF pref;

     
     QList<AlbumThumbGraphicItem*> albums = m_albums.values();

     for (int i = 0; i < albums.count(); ++i) {
         AlbumThumbGraphicItem* album = albums.at(i);
         pref = album->effectiveSizeHint(Qt::PreferredSize);
         maxRowHeight = qMax(maxRowHeight, pref.height());

         qreal next_x;
         next_x = x + pref.width();
         if (next_x > maxw) {
             if (x == 0) {
                 pref.setWidth(maxw);
             } else {
                 x = 0;
                 next_x = pref.width();
             }
             y += maxRowHeight + VSpacing;
             maxRowHeight = 0;
         }
         if(redraw) {
           album->setPos(QPointF(left + x, top + y));
         }

         x = next_x + HSpacing;
     }
     maxRowHeight = qMax(maxRowHeight, pref.height());

     return QSizeF(maxw, top + y + maxRowHeight + bottom );
}



/*
********************************************************************************
*                                                                              *
*    Class LyricsInfoWidget                                                    *
*                                                                              *
********************************************************************************
*/
LyricsInfoWidget::LyricsInfoWidget(QWidget* parentView) :
  QGraphicsWidget(0)
{
    //Debug::debug() << " LyricsInfoWidget constructor";

    m_parent       = parentView;
    m_lyrics_found   = false;

    m_title         = new CategorieLayoutItem(qobject_cast<QGraphicsView*> (m_parent)->viewport());
    m_title->m_name = tr("Song lyrics");
    
    m_lyrics        = new TextGraphicItem();
    m_lyrics->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );


    m_button_add    = new ButtonItem();
    m_button_add->setToolTip(tr("Add file"));
    m_button_add->setPixmap(QPixmap(":/images/add_32x32.png"));
    connect(m_button_add, SIGNAL(clicked()), this, SLOT(slot_add_lyrics()));
    m_button_add->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_button_remove = new ButtonItem();
    m_button_remove->setPixmap(QPixmap(":/images/remove_32x32.png"));
    m_button_remove->setToolTip(tr("Remove file"));
    m_button_remove->hide();
    connect(m_button_remove, SIGNAL(clicked()), this, SLOT(slot_remove_lyrics()));

    m_button_link  = new WebLinkItem();
    m_button_link->setText("Lyrics Link");
    m_button_link->hide();

    QGraphicsLinearLayout* m_layout_h0 = new QGraphicsLinearLayout( Qt::Horizontal);
    m_layout_h0->addItem( m_button_link );
    m_layout_h0->addItem( m_button_add );
    m_layout_h0->addItem( m_button_remove );

    m_layout_h0->setAlignment(m_button_link, Qt::AlignVCenter);
    m_layout_h0->setAlignment(m_button_add, Qt::AlignVCenter);
    m_layout_h0->setAlignment(m_button_remove, Qt::AlignVCenter);
    
    QGraphicsLinearLayout* m_layout_v2 = new QGraphicsLinearLayout( Qt::Vertical);
    m_layout_v2->setContentsMargins(20,0,0,0);
    m_layout_v2->addItem( m_layout_h0 );
    m_layout_v2->addItem( m_lyrics );

    QGraphicsLinearLayout* m_layout    = new QGraphicsLinearLayout( Qt::Vertical , this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->addItem( m_title );
    m_layout->addItem( m_layout_v2 );
}

void LyricsInfoWidget::clear()
{
    m_lyrics_found = false;
    m_lyrics->clear();
    m_lyrics->setHtml( QString(tr("No lyrics found")) );

}

void LyricsInfoWidget::update()
{
    prepareGeometryChange();

    updateGeometry();

    this->layout()->invalidate();

    QGraphicsWidget::update();

    emit updated();
}

QSizeF LyricsInfoWidget::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
Q_UNUSED(which);
Q_UNUSED(constraint);

    return QSizeF(m_parent->width(),
       qMax(30 /* title */ + m_button_add->height()*2,
            30 /* title */ + m_lyrics->height()));
}

void LyricsInfoWidget::resizeEvent( QGraphicsSceneResizeEvent *event )
{
Q_UNUSED(event)

    m_lyrics->setTextWidth(m_parent->width() -30);

    this->update();
}

void LyricsInfoWidget::setData(INFO::InfoRequestData request , QVariant data)
{
    //Debug::debug() << Q_FUNC_INFO;
 
    if(request.type == INFO::InfoTrackLyrics ) 
    {
        if(m_lyrics_found) return; // lyrics already found

        QVariantHash vhash =  qvariant_cast<QVariantHash>(data);
        
        //! local provider
        if(vhash["provider"].toString() == "local")
        {
            m_button_remove->show();
            m_button_add->setToolTip(tr("Edit file"));
            m_button_add->setPixmap(QPixmap(":/images/edit-48x48.png"));
        }
        else
        {
            m_button_remove->hide();

            m_button_add->setToolTip(tr("Add file"));
            m_button_add->setPixmap(QPixmap(":/images/add_32x32.png"));

            m_button_link->setLink(vhash["url"].toString());
            m_button_link->show();
        }

        //! get plain text for lyrics editor
        QString htmlLyrics = "<body><html>";
        htmlLyrics += vhash["lyrics"].toString();
        htmlLyrics += "</body></html>";

        m_lyrics->setHtml( htmlLyrics );
        m_lyrics->updateItem();

        m_lyrics_found = true;

        this->update();
    }
}


/*******************************************************************************
  LyricsInfoWidget::slot_add_lyrics
*******************************************************************************/
void LyricsInfoWidget::slot_add_lyrics()
{
    //Debug::debug() << " ---- LyricsInfoWidget::slot_add_lyrics";
    //! Show settings dialog
    LyricsEditor *le = new LyricsEditor(m_parent);
    MEDIA::TrackPtr media = Engine::instance()->playingTrack();

    if(media->type() == TYPE_TRACK)
      le->setInfo( media->artist,media->title);

    if(m_lyrics_found)
      le->setLyrics( m_lyrics->toPlainText() );

    connect(le,SIGNAL(lyricsChanged(INFO::InfoRequestData,QVariant)),this, SLOT(setData(INFO::InfoRequestData,QVariant)));
    le->exec();
}

/*******************************************************************************
  LyricsInfoWidget::slot_remove_lyrics
*******************************************************************************/
void LyricsInfoWidget::slot_remove_lyrics()
{
    //Debug::debug() << " ---- LyricsInfoWidget::slot_remove_lyrics";

    QCryptographicHash hash(QCryptographicHash::Sha1);

    MEDIA::TrackPtr media = Engine::instance()->playingTrack();

    if(media->type() == TYPE_TRACK) {
      hash.addData(media->artist.toLower().toUtf8().constData());
      hash.addData(media->title.toLower().toUtf8().constData());

      QString path = QString(UTIL::CONFIGDIR + "/lyrics/" + hash.result().toHex() + ".txt");

      QFile file(path);
      if(file.exists()) {
        file.remove();
        //Debug::debug() << " ---- LyricsInfoWidget::slot_remove_lyrics  file removed !";
      }

      m_lyrics_found = false;
    }
}

