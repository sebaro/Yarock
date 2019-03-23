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

#include "iconloader.h"

#include <QFile>


QIcon IconLoader::Load(const QString &name) 
{
  QIcon ret;

  if (name.isEmpty())
    return ret;

#if QT_VERSION >= 0x040600
  // Try to load it from the theme initially
  ret = QIcon::fromTheme(name);
  if (!ret.isNull())
    return ret;
#endif

  // Otherwise use our fallback theme
  const QString path(":/images/%1_48x48.png");

  QString filename(path.arg(name));

  if (QFile::exists(filename))
    ret.addFile(filename, QSize(48, 48));

  return ret;
}
