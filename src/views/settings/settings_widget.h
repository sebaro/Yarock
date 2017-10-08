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
#ifndef _SETTINGS_WIDGETS_H_
#define _SETTINGS_WIDGETS_H_

// local
#include "widgets/dialogs/shortcutdialog.h"
#include "addfolder_widget.h"
#include "database.h"

// Qt
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>
#include <QListWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QSpinBox>

class CategorieLayoutItem;
class ButtonItem;

/*
********************************************************************************
*                                                                              *
*    Class PageGeneral                                                         *
*                                                                              *
********************************************************************************
*/
class PageGeneral : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageGeneral(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isSystrayChanged();
    bool isDbusChanged();
    bool isMprisChanged();

    void setContentVisible(bool b);
    
    void update();
    void doLayout();

  private:
    void createGui();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  private slots:
    void slot_color_button_clicked();  
    void slot_on_titlebutton_clicked();
    
  private:
    QCheckBox              *ui_check_systray;
    QCheckBox              *ui_check_hideAtStartup;
    QCheckBox              *ui_check_enableMpris;
    QCheckBox              *ui_check_enableDbus;
    QPushButton            *ui_color_button;

    CategorieLayoutItem    *m_title;
    QGraphicsProxyWidget   *proxy_widget;
    QWidget                *m_parent;
    
    ButtonItem             *m_button;
    bool                   isOpen;
    
  signals:
    void layout_changed();
};


/*
********************************************************************************
*                                                                              *
*    Class PagePlayer                                                          *
*                                                                              *
********************************************************************************
*/
class PagePlayer : public QGraphicsWidget
{
Q_OBJECT
  public:
    PagePlayer(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isEngineChanged() {return _isEngineChanged;}
    
    void setContentVisible(bool b);
    
    void update();
    void doLayout();

  private:
    void createGui();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    
  private slots:
    void slot_engineClicked();
    void slot_enable_replaygain();
    void slot_on_titlebutton_clicked();
  
  private:
    bool                   _isEngineChanged; /* restart needed */
      
    QCheckBox              *ui_stopOnPlayqueueClear;
    QCheckBox              *ui_restartPlayingAtStartup;
    QCheckBox              *ui_restorePlayqueue;

    QGroupBox              *ui_groupBox;
    QRadioButton           *ui_mode_track;
    QRadioButton           *ui_mode_album;

    QPushButton            *ui_engineButton;
    QActionGroup           *ui_engineGroup;
    
    QCheckBox              *ui_enable_replaygain;
    QComboBox              *ui_comboRGMode;
    
    CategorieLayoutItem    *m_title;
    QGraphicsProxyWidget   *proxy_widget;
    QWidget                *m_parent;
    
    ButtonItem             *m_button;
    bool                   isOpen;

  signals:
    void layout_changed();    
};



/*
********************************************************************************
*                                                                              *
*    Class PageLibrary                                                         *
*                                                                              *
********************************************************************************
*/
class PageLibrary : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageLibrary(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isLibraryChanged() {return _isLibraryChanged;}
    bool isViewChanged();
    bool isCoverSizeChanged();

    void setContentVisible(bool b);
    
    void update();
    void doLayout();

  private:
    void createGui();
    void addDatabaseParam(const QString& name, const Database::Param& param);
    
  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  private slots:
    void loadDatabaseParam(QString);
    void newDatabaseParam();
    void delDatabaseParam();
    void renameDatabaseParam();
    void slot_oncheckbox_clicked();
    void slot_on_titlebutton_clicked();

    void slot_on_add_folder_clicked(const QString text=QString());
    void slot_on_remove_folder_clicked();
    void slot_on_path_changed();

    
  private:
    bool         _isLibraryChanged; //! we need to rebuild database
    
    QList<AddFolderWidget*>  ui_folderWidgets;
    QVBoxLayout              *ui_folders_layout;
    
    QCheckBox              *ui_auto_update;
    QCheckBox              *ui_search_cover;
    QCheckBox              *ui_download_cover;
    QCheckBox              *ui_group_albums;
    QCheckBox              *ui_use_artist_image;
    QCheckBox              *ui_rating_to_file;
    QComboBox              *ui_choose_db;

    QSpinBox               *ui_cover_size_spinbox;
    
    QPushButton            *ui_add_path_button;

    QToolButton            *ui_db_new_button;
    QToolButton            *ui_db_del_button;
    QToolButton            *ui_db_rename_button;

    QMap<QString, Database::Param> m_db_params;

    CategorieLayoutItem    *m_title;
    QGraphicsProxyWidget   *proxy_widget;
    QWidget                *m_parent;
    QWidget                *m_main_widget;
    
    ButtonItem             *m_button;
    bool                   isOpen;

  signals:
    void layout_changed();
};

/*
********************************************************************************
*                                                                              *
*    Class ShortcutGraphicItem                                                 *
*                                                                              *
********************************************************************************
*/
class ShortcutGraphicItem : public QGraphicsObject, public QGraphicsLayoutItem
{
Q_OBJECT
Q_INTERFACES(QGraphicsLayoutItem)

  public:
    ShortcutGraphicItem(const QString&, const QString&, QPixmap);

    QString m_name;
    QString m_key;
    QPixmap m_pixmap;
    bool    m_status;
    
  protected:
    // Inherited from QGraphicsLayoutItem
    void setGeometry(const QRectF &geom);
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    // Implement QGraphicsItem method
    QRectF boundingRect() const;
    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );

  private:
    #if QT_VERSION >= 0x050000
    QStyleOptionViewItem opt;
    #else
    QStyleOptionViewItemV4 opt;
    #endif

  signals:
    void clicked();
};

/*
********************************************************************************
*                                                                              *
*    Class PageShortcut                                                        *
*                                                                              *
********************************************************************************
*/
class PageShortcut : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageShortcut(QWidget* parentView = 0);
    ~PageShortcut();

    void saveSettings();

    bool isChanged() {return _isChanged;}

    void setContentVisible(bool b);
    
    void update();
    void doLayout();

  public slots:
    void restoreSettings();

  private:
    void createGui();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  private slots:
    void slot_on_shorcutItem_clicked();
    void slot_on_titlebutton_clicked();
    void enableChange();

  private:
    bool                    _isChanged;
    bool                    _isEnableOld;
    
    QCheckBox               *ui_enable_shortcut;
    
    QMap<QString, ShortcutGraphicItem*>  m_items;
    QWidget                 *m_parent;

    CategorieLayoutItem     *m_title;

    QGraphicsProxyWidget    *proxy_widget;
    
    ButtonItem              *m_button;
    bool                    isOpen;

  signals:
    void layout_changed();    
};


/*
********************************************************************************
*                                                                              *
*    Class PageScrobbler                                                       *
*                                                                              *
********************************************************************************
*/
class PageScrobbler : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageScrobbler(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isChanged() {return _isChanged;}

    void setContentVisible(bool b);
    
    void update();
    void doLayout();

  private slots:
    void signInClicked();
    void slotSignInDone();
    void enableChange();
    void slot_on_titlebutton_clicked();
    
  private:
    void updateSignInStatus();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  private:
    CategorieLayoutItem   *m_title;
    QGraphicsProxyWidget  *proxy_widget;
    QWidget               *m_parent;

    QCheckBox             *useLastFmScrobbler;
    QGroupBox             *accountGroupB;
    QLineEdit             *lineEdit_1;
    QLineEdit             *lineEdit_2;
    QPushButton           *signInButton;
    QLabel                *statusLabel;
    QLabel                *statusPixmap;

    bool                   _isChanged;
    bool                   _isEnableOld;
    
    ButtonItem             *m_button;
    bool                   isOpen;

  signals:
    void layout_changed();
 };
 
 
/*
********************************************************************************
*                                                                              *
*    Class PageSongInfo                                                        *
*                                                                              *
********************************************************************************
*/
class PageSongInfo : public QGraphicsWidget
{
Q_OBJECT
  public:
    PageSongInfo(QWidget* parentView = 0);

    void restoreSettings();
    void saveSettings();

    bool isChanged() {return _isChanged;}
 
    void setContentVisible(bool b);

    void update();
    void doLayout();

  private:
    void Move(int);  
    
  private slots:
    void slot_on_titlebutton_clicked();
    void slot_item_changed(QListWidgetItem*);
    void slot_current_item_changed(QListWidgetItem*);
    void slot_move_up();
    void slot_move_down();

  protected:
    void resizeEvent( QGraphicsSceneResizeEvent *event );
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

  private:
    CategorieLayoutItem   *m_title;
    QGraphicsProxyWidget  *proxy_widget;

    QWidget               *m_parent;
    QListWidget           *ui_listwidget;
    QPushButton           *ui_move_up;
    QPushButton           *ui_move_down;
    
    ButtonItem             *m_button;

    bool                   isOpen;
    bool                   _isChanged;
   
  signals:
    void layout_changed();
};
 
 
#endif // _SETTINGS_WIDGETS_H_
