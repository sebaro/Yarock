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
#ifndef _DATABASE_BUILDER_H_
#define _DATABASE_BUILDER_H_

#include "infosystem/info_system.h"
#include "mediaitem/mediaitem.h"

#include <QThread>
#include <QObject>
#include <QStringList>
#include <QSet>
#include <QSqlDatabase>
#include <QString>
#include <QHash>

/*
********************************************************************************
*                                                                              *
*    Class DataBaseBuilder                                                     *
*                                                                              *
********************************************************************************
*/
// Thread thread that :
//   - parse collection directory
//   - read track file metada (using Taglib)
//   - write sql database with track information
class DataBaseBuilder :  public QThread
{
  Q_OBJECT
  public:
    DataBaseBuilder();
    void setExit(bool b) {m_exit = b;}
    void updateFolder(QStringList folder, bool doRebuild=false);

  protected:
    void run();

  private slots:
    void slot_systeminfo_received( INFO::InfoRequestData, QVariant );

  private:
    void doScan();
    QStringList filesFromFilesystem(const QString& directory);
    QHash<QString,uint> filesFromDatabase(const QString& directory);

    void addDirectory(const QString& path);
    void updateDirectory(const QString& path);
    void removeDirectory(const QString& path);

    void insertTrack(const QString& filename);
    void removeTrack(const QString& filename);

    void insertPlaylist(const QString& filename);
    void removePlaylist(const QString& filename);

    /* cover & image stuff */
    bool saveAlbumCoverFromFile(MEDIA::TrackPtr track);
    bool saveAlbumCoverFromDirectory(MEDIA::TrackPtr track);
    void fetchAlbumImage(MEDIA::TrackPtr track);
    void fetchArtistImage(MEDIA::TrackPtr track);

    int insertDirectory(const QString & path);
    int insertGenre(const QString & genre);
    int insertYear(int year);
    int insertArtist(const QString & artist);
    int insertAlbum(const QString & album, int artist_id,int year,int disc);


  private:
    QList<quint64>       m_requests_ids;

    // filename, mtime
    QHash<QString,uint>  m_db_dirs;
    QSet<QString>        m_fs_dirs;
    QStringList          m_input_folders;

    bool                 m_exit;
    bool                 m_do_rebuild;

    QSqlDatabase        *m_sqlDb;

  signals:
    void buildingFinished();
    void buildingProgress(int);
};

#endif // _DATABASE_BUILDER_H_
