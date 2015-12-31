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

#include "equalizer_preset.h"

  
const char* Equalizer::kGainText[Equalizer::kBands] = {"60", "170", "310", "600", "1k", "3k", "6k", "12k", "14k", "16k"};
  
/*******************************************************************************
   Operator Equalizer::EqPreset
*******************************************************************************/
bool Equalizer::EqPreset::operator ==(const Equalizer::EqPreset& other) const
{
  if (preamp != other.preamp) return false;
  for (int i=0 ; i<Equalizer::kBands ; ++i) {
    if (gain[i] != other.gain[i]) return false;
  }
  return true;
}

bool Equalizer::EqPreset::operator !=(const Equalizer::EqPreset& other) const
{
  return ! (*this == other);
}


QDataStream &operator<<(QDataStream& s, const Equalizer::EqPreset& p)
{
  s << p.preamp;
  for (int i=0 ; i<Equalizer::kBands ; ++i)
    s << p.gain[i];
  return s;
}

QDataStream &operator>>(QDataStream& s, Equalizer::EqPreset& p)
{
  s >> p.preamp;
  for (int i=0 ; i<Equalizer::kBands ; ++i)
    s >> p.gain[i];
  return s;
}
