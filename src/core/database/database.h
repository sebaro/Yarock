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

#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <QString>
#include <QStringList>

#include <QSqlDatabase>
#include <QMap>

/*
********************************************************************************
*                                                                              *
*    Class Database                                                            *
*                                                                              *
********************************************************************************
*/
class Database : public QObject
{
Q_OBJECT

static Database         *INSTANCE;

public:
    Database();
    ~Database();

    static Database* instance() { return INSTANCE; }

    struct Param
    {
      QString        _name;
      QStringList    _paths;
    
      bool           _option_auto_rebuild;
      bool           _option_check_cover;
      bool           _option_group_albums;
      bool           _option_artist_image;
      bool           _option_wr_rating_to_file;
      
      Param() {
         _name  = QString("collection"); 
         _paths = QStringList();
         _option_auto_rebuild = false;
         _option_check_cover  = true;
         _option_group_albums = false;
         _option_artist_image = true;
         _option_wr_rating_to_file = false;
      }
    };
  
    /* database public api */
    void close();                 /* close all connections */
    bool open(bool create=false); /* open connection for current thread */
    QSqlDatabase* db();

    bool exist();
    bool versionOK();
    void remove();
    void create();
    
    void settings_restore();
    void settings_save();

    void change_database(const QString& db_name);

    const Database::Param& param(const QString& name=QString());
    void param_add(const Database::Param& param);
    void param_clear() {m_params.clear();}
    QStringList param_names();
    
    bool isMultiDb() { return (m_params.keys().count() > 1);}
    
  private:
    QString idForName(const QString&);
    
    /* database connections managment */
    QMap<QString /* connection name */, QSqlDatabase * /* database */>  m_sqldbs;

    /* database parameters managment */
    QMap<QString /*database ID */, Database::Param>   m_params;
    QString m_current_id;
};

#endif // _DATABASE_H_
