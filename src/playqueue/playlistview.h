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

#ifndef _PLAYLIST_VIEW_H_
#define _PLAYLIST_VIEW_H_

#include "playqueue_model.h"
#include "core/mediaitem/mediaitem.h"


#include <QItemDelegate>
#include <QListView>
#include <QString>


/*
********************************************************************************
*                                                                              *
*    Class PlaylistDelegate                                                    *
*                                                                              *
********************************************************************************
*/
class PlayqueueModel;
class PlaylistView;

// delegate to have custom painting for track into playlist widget :
//   - paint specific icon for playing item
//   - paint specific icon for broken url item
//   - paint duration information
//   - paint simple remote url (with network platform icon)
class PlaylistDelegate : public QItemDelegate
{
Q_OBJECT
  public:
    PlaylistDelegate(QObject *parent, PlayqueueModel* model);

    void setView(PlaylistView* view) {m_view = view;}
      
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const;

    void set_mouse_over(QModelIndex idx, QPoint p)
    {
      m_mouse_over_index = idx;
      m_mouse_over_pos   = p;
    }
    
    QModelIndex mouse_over_index() {return m_mouse_over_index;}
    QPoint mouse_over_pos() {return m_mouse_over_pos;}
    
    void set_mouse_out() { m_mouse_over_index = QModelIndex(); }
    bool is_mouse_over() const { return m_mouse_over_index.isValid(); }
    
  private :
    QIcon     getIcon(const MEDIA::TrackPtr track) const;
    void      drawStop(QPainter * painter, QRect rect) const;

private:    
    PlayqueueModel      *m_model;
    PlaylistView        *m_view;

    QIcon     icon_media_playing;
    QIcon     icon_media_broken;
    QIcon     icon_media_track;
    QIcon     icon_media_stream;

    QFont     font_normal;
    QFont     font_bold;
    
    QModelIndex          m_mouse_over_index;
    QPoint               m_mouse_over_pos;
};


/*
********************************************************************************
*                                                                              *
*    Class PlaylistView                                                        *
*                                                                              *
********************************************************************************
*/
class PlaylistView : public QListView
{
Q_OBJECT
  public:
    PlaylistView(QWidget *parent, PlayqueueModel* model);

    const MEDIA::TrackPtr firstSelectedTrack();

    void keyPressEvent(QKeyEvent* event);

    //! selection
    bool isTrackSelected();

    void removeSelected();
    
  public slots:
    void selectionChanged (const QItemSelection & selected, const QItemSelection & deselected);
    void jumpToCurrentlyPlayingTrack();
    void slot_lastfm_love();

  protected:
    void paintEvent(QPaintEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* event);
    void mousePressEvent(QMouseEvent* event);
 
  private :
    PlayqueueModel      *m_model;
    PlaylistDelegate    *m_delegate;
    int                  m_drop_indicator_row;
    bool                 m_drag_over;
            
  private slots:
    void slot_itemActivated(const QModelIndex &index);
    void selectItems(QList<MEDIA::TrackPtr>);
    void slot_model_cleared();

  signals:
    void selectionChanged();
    void signal_playlist_itemDoubleClicked();
};


#endif // _PLAYLIST_VIEW_H_
