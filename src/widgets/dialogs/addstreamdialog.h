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

#ifndef _ADDSTREAM_DIALOG_H_
#define _ADDSTREAM_DIALOG_H_

#include "dialog_base.h"
#include "core/mediaitem/mediaitem.h"
#include "widgets/editors/editor_common.h"


#include <QLineEdit>

/*
********************************************************************************
*                                                                              *
*    Class AddStreamDialog                                                     *
*                                                                              *
********************************************************************************
*/
class AddStreamDialog : public DialogBase
{
Q_OBJECT
  public:
    AddStreamDialog(MEDIA::TrackPtr stream, QWidget *parent = 0);
    
    MEDIA::TrackPtr stream() { return m_stream;}

  private slots:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

  private:
    MEDIA::TrackPtr       m_stream;
      
    // TODO add image support     QLabel                *ui_image;
    EdLineEdit            *ui_edit_name;
    EdLineEdit            *ui_edit_url;
    EdLineEdit            *ui_edit_website;
    EdLineEdit            *ui_edit_genre;
    EdLineEdit            *ui_edit_bitrate;
    EdLineEdit            *ui_edit_samplerate;
    EdLineEdit            *ui_edit_format;
};

#endif // _ADDSTREAM_DIALOG_H_

