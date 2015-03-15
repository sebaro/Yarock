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
#ifndef _INFOSYSTEM_H_
#define _INFOSYSTEM_H_

#include <QHash>
#include <QVariant>
#include <QThread>
#include <QPointer>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    namespace InfoSystem                                                      *
*                                                                              *
********************************************************************************
*/ 
namespace INFO
{

  typedef QHash< QString, QString > InfoStringHash;
        
  enum InfoType
  {
      InfoNoInfo                = 0,

      InfoArtistBiography       = 1,
      InfoArtistImages          = 2,
      InfoArtistBlog            = 3,
      InfoArtistFamiliarity     = 4,
      InfoArtistHotttness       = 5,
      InfoArtistSongs           = 6,
      InfoArtistSimilars        = 7,
      InfoArtistNews            = 8,
      InfoArtistProfile         = 9,
      InfoArtistReviews         = 10,
      InfoArtistTerms           = 11,
      InfoArtistLinks           = 12,
      InfoArtistReleases        = 13,

      InfoAlbumCoverArt         = 14,

      InfoAlbumInfo             = 16,
      InfoAlbumSongs            = 17,
      InfoAlbumGenre            = 18,
            
      InfoTrackLyrics           = 19,
      InfoTrackSimilars         = 20,

      InfoChart                 = 21,

      InfoMiscTopTerms          = 22
  };
        

  struct InfoRequestData
  {
    quint64        requestId;
    INFO::InfoType type;    
    QVariant       data;
    
    InfoRequestData();
    InfoRequestData( const INFO::InfoType t, INFO::InfoStringHash hash );
  };

  QString requestMd5 ( const INFO::InfoRequestData &);
  
  QString albumKey(const QString&, const QString&);
  
  quint64 infosystemRequestId();

} // end namespace

/*
********************************************************************************
*                                                                              *
*    Class InfoSystem                                                          *
*                                                                              *
********************************************************************************
*/ 
class InfoSystemWorkerThread;

class InfoSystem : public QObject
{
Q_OBJECT

public:
    static InfoSystem* instance();

    InfoSystem( QObject* parent );
    ~InfoSystem();

    bool getInfo( const INFO::InfoRequestData& requestData );

    void activateCache( bool active ) { m_active_cache = active;}
    bool cacheActivated() { return m_active_cache;}
    
signals:
    void info( INFO::InfoRequestData requestData, QVariant output );
    void finished( INFO::InfoRequestData requestData );

private slots:
    void slot_init();

private:
    bool m_inited;
    InfoSystemWorkerThread  *m_workerThread;

    static InfoSystem* INSTANCE;
    
    bool        m_active_cache;
};

/*
********************************************************************************
*                                                                              *
*    Class InfoSystemWorkerThread                                              *
*                                                                              *
********************************************************************************
*/ 
class InfoSystemWorker;

class InfoSystemWorkerThread : public QThread
{
Q_OBJECT

public:
    InfoSystemWorkerThread( QObject* parent );
    virtual ~InfoSystemWorkerThread();

    void run();
    InfoSystemWorker* worker() const;

private:

    QPointer< InfoSystemWorker > m_worker;
};
 
Q_DECLARE_METATYPE( INFO::InfoRequestData )
Q_DECLARE_METATYPE( INFO::InfoStringHash )


#endif // _INFOSYSTEM_H_