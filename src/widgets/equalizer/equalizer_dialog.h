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
// thanks for clementine equalizer for inspiration

#ifndef _EQUALIZER_DIALOG_H_
#define _EQUALIZER_DIALOG_H_


//! local
#include "equalizer_preset.h"
#include "equalizer_slider.h"

//! qt
#include <QComboBox>
#include <QCheckBox>
#include <QWidget>
#include <QDialog>
#include <QMap>
#include <QList>
#include <QString>

/*
********************************************************************************
*                                                                              *
*    Class Equalizer_Dialog                                                    *
*                                                                              *
********************************************************************************
*/
class Equalizer_Dialog : public QDialog
{
Q_OBJECT

public:
  Equalizer_Dialog(QWidget *parent = 0);
  ~Equalizer_Dialog();

bool isEnabled() const;

protected:
  void closeEvent(QCloseEvent *);

private: //! Gui stuff
  QComboBox *_comboBoxPreset;
  QCheckBox *_enableCheckBox;
  QWidget   *_slider_container;

private:
  EqualizerSlider* eqSliderList[Equalizer::kBands];
  EqualizerSlider* eqSliderPreamp;

  QMap<QString, Equalizer::EqPreset> _presetList;

  bool _settingChanged;

  void addDefaultPreset();
  void addPreset(const QString& name, const Equalizer::EqPreset& eqPreset);
  void saveSettings();
  void restoreSettings();

private slots:
  void applyPreset(const QString& name);
  void equalizerChanged();
  void enableChanged(bool enable);
  void savePreset();
  void delPreset();

signals:
  void enabledChanged(bool enabled);
  void newEqualizerValue(const int preamp, const QList<int>& gainsList);
};

#endif // _EQUALIZER_DIALOG_H_
