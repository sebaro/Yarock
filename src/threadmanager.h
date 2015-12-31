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
#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include "core/mediaitem/mediaitem.h"
#include "infosystem/info_system.h"
#include "infosystem/tasks/tagsearch.h"


#include <QObject>
#include <QList>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QMap>

enum E_MODEL_TYPE {
                   MODEL_COLLECTION = 0,
                   MODEL_PLAYLIST   = 1,
                   MODEL_ALL        = 2
                   };

Q_DECLARE_METATYPE( E_MODEL_TYPE )

/*
********************************************************************************
*                                                                              *
*    Class ThreadManager                                                       *
*                                                                              *
********************************************************************************
*/
class ThreadManager : public QObject
{
Q_OBJECT
  public:
    explicit ThreadManager();
    static ThreadManager    *INSTANCE;
    static ThreadManager* instance() { return INSTANCE; }

    ~ThreadManager();

    void stopThread();

    // Database Builder Thread
    void databaseBuild(QStringList listDir, bool doRebuild=false);
    bool isDbRunning();

    // Tag/Metadata/Image Search thread
    void startTagSearch(TagSearch::TYPE type, INFO::InfoRequestData request=INFO::InfoRequestData());
    
  public slots:
    // Model Populator Thread
    void populateLocalTrackModel();
    void populateLocalPlaylistModel();

  private:
    enum E_THREAD { DB_THREAD, 
                    POPULATOR_C_THREAD, 
                    POPULATOR_P_THREAD, 
                    TAG_SEARCH_THREAD};
    
    void cancelThread(E_THREAD thread);

  private slots:
    void dbBuildProgressChanged(int progress);
    void dbBuildFinish();

    void slot_on_localtrackmodel_populated();
    void slot_on_localtrackmodel_populating_changed(int progress);

    void slot_on_localplaylistmodel_populated();
    void slot_on_localplaylistmodel_populating_changed(int progress);

    void slot_tagsearch_finished();
    void slot_tagsearch_progress(int progress);

  private:
    class DataBaseBuilder         *m_databaseBuilder;         // QThread
    class LocalTrackPopulator     *m_localTrackPopulator;     // QThread
    class LocalPlaylistPopulator  *m_localPlaylistPopulator;  // QThread
    class TagSearch               *m_tagSearchTask; 

    //! messages Id for StatusWidget management
    QMap<QString, uint>    messageIds;

  signals:
    void dbBuildStart();
    void dbBuildFinished();
    void dbBuildProgress(int);
    void modelPopulationFinished(E_MODEL_TYPE type);
    void tagSearchFinished();
};

#endif // _THREAD_MANAGER_H_
