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

#ifndef _SERVICE_BASE_H_
#define _SERVICE_BASE_H_

#include "core/mediaitem/mediaitem.h"
#include <QObject>

namespace SERVICE
{

enum Type {
    DIRBLE,
    RADIONOMY,
    TUNEIN,
    LOCAL
  };

enum State {
    NO_DATA      = 0,
    DATA_OK      = 1,
    DOWNLOADING  = 2,
    ERROR        = 3
  };
  
} // end namespace


/*
********************************************************************************
*                                                                              *
*    Class Service                                                             *
*                                                                              *
********************************************************************************
*/
class Service  : public QObject
{
Q_OBJECT
public:
    Service();
    Service(QString name, SERVICE::Type type);
    
    /* properties */
    QString name() {return m_name;}
    
    SERVICE::Type type() {return m_type;}

    /* links access */
    MEDIA::LinkPtr rootLink() {return m_root_link;}
    MEDIA::LinkPtr activeLink() {return m_active_link;}
    MEDIA::LinkPtr searchLink() {return m_search_link;}
    MEDIA::LinkPtr moreLink() {return m_more_link;}
    
    void setActiveLink(MEDIA::LinkPtr link) { m_active_link = link;}
    
    /* state */
    SERVICE::State state() { return m_state;}
    void set_state(SERVICE::State state) { m_state = state;}

    /* search term */
    const QString searchTerm() {return m_search_term;}
    void setSearchTerm(const QString& term) {m_search_term = term;}
    
    /* virtual method */
    virtual QList<MEDIA::TrackPtr> streams() = 0;
    virtual QList<MEDIA::LinkPtr> links() = 0;
    virtual void load() = 0;
    virtual void reload() = 0;
    virtual bool hasMoreLink() {return false;}

public slots:
    virtual void slot_activate_link(MEDIA::LinkPtr link=MEDIA::LinkPtr(0)) = 0;
   
signals:
    void stateChanged();
    void dataChanged();

private:
    SERVICE::Type      m_type;
    SERVICE::State     m_state;
    QString            m_name;
    QString            m_homepage;

protected:    
    MEDIA::LinkPtr     m_root_link;
    MEDIA::LinkPtr     m_active_link;
    MEDIA::LinkPtr     m_search_link;
    MEDIA::LinkPtr     m_more_link;
    MEDIA::LinkPtr     m_genre_link;

    QString            m_search_term;
};

#endif // _SERVICE_BASE_H_
