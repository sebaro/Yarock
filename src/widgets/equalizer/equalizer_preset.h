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
#ifndef _EQUALIZER_PRESET__H_
#define _EQUALIZER_PRESET__H_

//! qt
#include <QMetaType>
#include <QDataStream>


/*
********************************************************************************
*                                                                              *
*    namespace Equalizer                                                       *
*                                                                              *
********************************************************************************
*/
namespace Equalizer {

  const int kBands = 10;
  
  extern const char* kGainText[kBands];// = {"60", "170", "310", "600", "1k", "3k", "6k", "12k", "14k", "16k"};

  struct EqPreset {
      EqPreset()
      {
        preamp = 0;
        for (int i=0 ; i<kBands ; ++i)
          gain[i] = 0;
      }

      EqPreset(int g0, int g1, int g2, int g3, int g4, int g5,int g6, int g7, int g8, int g9, int p)
      {
        preamp = p;
        gain[0] = g0;  gain[1] = g1; gain[2] = g2;  gain[3] = g3; gain[4] = g4;
        gain[5] = g5;  gain[6] = g6; gain[7] = g7;  gain[8] = g8; gain[9] = g9;
      }

      bool operator ==(const EqPreset& other) const;
      bool operator !=(const EqPreset& other) const;

      int        preamp;
      int        gain[kBands];
    };

} // end namespace Equalizer

Q_DECLARE_METATYPE(Equalizer::EqPreset);

QDataStream &operator<<(QDataStream& s, const Equalizer::EqPreset& p);
QDataStream &operator>>(QDataStream& s, Equalizer::EqPreset& p);


#endif //_EQUALIZER_PRESET__H_

