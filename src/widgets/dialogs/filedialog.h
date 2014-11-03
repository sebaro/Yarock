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
#ifndef _FILE_DIALOG_H_
#define _FILE_DIALOG_H_

#include "dialog_base.h"

#include <QFileSystemModel>
#include <QListView>
#include <QTreeView>
#include <QLineEdit>
#include <QObject>
#include <QToolButton>
#include <QString>
#include <QStringList>
#include <QModelIndex>
#include <QComboBox>
#include <QCheckBox>


class FileDialog : public DialogBase
{
Q_OBJECT
public:
    enum Mode
    {
        AddFile = 0,   /*!< Get existing file */
        AddDir,        /*!< Get existing directory */
        AddFiles,      /*!< Get existing files */
        AddDirs,       /*!< Get existing directories */
        SaveFile       /*!< Get existing or non-existing file */
    };
 
    FileDialog(QWidget* parent, Mode mode, const QString& title=QString());

    QString addDirectory();
    QStringList addDirectories();
    QStringList addFiles();
    QString addFile();     
    QString saveFile();  
        
    void setFilters (QStringList filters);
    
private slots:
    void slot_listview_itemDoubleClicked(const QModelIndex &index);
    void slot_listview_selectionChanged();
    void slot_onfilter_changed(int);
    void slot_on_filename_textChanged(const QString & text);
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void slot_on_prev_clicked();
    void slot_on_next_clicked();
    void slot_on_up_clicked();
    void slot_on_home_clicked();
    void slot_show_hidden_triggered();
    
private:
    void setupUi();
    void updateCurrentIndex(const QModelIndex &idx);
    void enableButton(bool enable);
  
private : 
    QListView          *ui_listView;
    QLineEdit          *ui_current_path;
    QToolButton        *ui_prev_button;
    QToolButton        *ui_next_button;
    QToolButton        *ui_up_button;
    QToolButton        *ui_home_button;
    QLineEdit          *ui_filename_lineedit;
    QComboBox          *ui_filter_cb;
    QToolButton        *ui_show_hidden;
    
    
    Mode                m_mode;
    QFileSystemModel   *m_model;
    QModelIndex         m_rootIdx;
    
    int                 m_current_index;
    QList<QModelIndex>  m_browserIdxs;
    
    QStringList         m_results;
};

#endif // _FILE_DIALOG_H_