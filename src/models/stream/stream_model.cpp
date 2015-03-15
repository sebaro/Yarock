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

#include "stream_model.h"
#include "core/mediaitem/mediaitem.h"
#include "debug.h"


StreamModel* StreamModel::INSTANCE = 0;

/*
********************************************************************************
*                                                                              *
*    Class StreamModel                                                         *
*                                                                              *
********************************************************************************
*/
StreamModel::StreamModel(QObject *parent) : QObject(parent)
{
    Debug::debug() << "StreamModel -> creation";
    INSTANCE          = this;
    m_filter_pattern  = "";
    m_show_duplicate  = false;
    m_playing_stream  = MEDIA::TrackPtr(0);
}

void StreamModel::clear()
{
    foreach(MEDIA::TrackPtr media, m_streams)
      media.reset();

    m_streams.clear();
    m_playing_stream  = MEDIA::TrackPtr(0);
}


void StreamModel::updateStatusOfPlayingItem(MEDIA::TrackPtr tk)
{
    //Debug::debug() << "#### StreamModel -> updatePlayingItem";
    if(!tk) return;
        
    MEDIA::TrackPtr parent_tk = tk;
    if(tk->parent()) {
      if(tk->parent()->type() == TYPE_STREAM)
        parent_tk = MEDIA::TrackPtr::staticCast(tk->parent());
    }
        
    foreach(MEDIA::TrackPtr stream, m_streams) {
      if(stream == parent_tk) 
      {
        stream->isPlaying = true;
        break;
      }
    }  
}


//! ------------------ StreamModel::setStreams ---------------------------------
void StreamModel::setStreams(const QList<MEDIA::TrackPtr> &streams)
{
    //Debug::debug() << "StreamModel -> setStreams";
    this->clear();
    
    if (!streams.empty())
      m_streams.append(streams);    
    
    if(!m_show_duplicate) {
      //Debug::warning() << "StreamModel -> remove duplicate streams";
      QSet<QString> name_list;
      foreach(MEDIA::TrackPtr stream, m_streams) {
          QString name = stream->name;
          if(name_list.contains( name ))
            m_streams.removeOne ( stream );
          else
            name_list.insert( name );
      }
    }
}


//!------------StreamModel::streamAt -------------------------------------------
MEDIA::TrackPtr StreamModel::streamAt(int row) const
{
    if (row < 0 || row >= m_streams.size())
      return MEDIA::TrackPtr(0);

    return m_streams.at(row);
}

//! --------- Filtering method -------------------------------------------------
bool StreamModel::isStreamFiltered(const int row)
{
    MEDIA::TrackPtr stream = streamAt(row);
    
    //! check stream item
    if(m_filter_pattern.length() < 3)
    {
      if ( stream->name.startsWith(m_filter_pattern, Qt::CaseInsensitive)    ||
           stream->categorie.startsWith(m_filter_pattern, Qt::CaseInsensitive))
      return true;
    }
    else 
    {
      if ( stream->name.contains(m_filter_pattern, Qt::CaseInsensitive)    ||
           stream->categorie.contains(m_filter_pattern, Qt::CaseInsensitive))
      return true;
    }

    return false;
}
