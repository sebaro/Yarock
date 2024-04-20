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


//! local
#include "equalizer_dialog.h"
#include "equalizer_preset.h"
#include "settings.h"
#include "dialog_base.h"
#include "debug.h"

//! qt
#include <QToolButton>
#include <QVariant>


/*
********************************************************************************
*                                                                              *
*    Class Equalizer_Dialog                                                    *
*                                                                              *
********************************************************************************
*/

Equalizer_Dialog::Equalizer_Dialog(QWidget *parent) : QDialog(parent)
{
    //! ----- setup Gui
    this->resize(435, 265);

    QVBoxLayout* verticalLayout = new QVBoxLayout(this);
    QHBoxLayout *horizontalLayout = new QHBoxLayout();

    _comboBoxPreset = new QComboBox(this);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(_comboBoxPreset->sizePolicy().hasHeightForWidth());
    _comboBoxPreset->setSizePolicy(sizePolicy);

    horizontalLayout->addWidget(_comboBoxPreset);

    QToolButton *_presetSaveButton = new QToolButton(this);
    _presetSaveButton->setIcon(QIcon::fromTheme("document-save"));
    _presetSaveButton->setToolTip(tr("Save preset"));
    horizontalLayout->addWidget(_presetSaveButton);

    QToolButton *_presetDelButton = new QToolButton(this);
    _presetDelButton->setIcon(QIcon::fromTheme("edit-delete"));
    _presetDelButton->setToolTip(tr("Delete preset"));
    horizontalLayout->addWidget(_presetDelButton);

    verticalLayout->addLayout(horizontalLayout);

    QFrame *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    verticalLayout->addWidget(line);

    _enableCheckBox = new QCheckBox(this);
    _enableCheckBox->setText(tr("Enable equalizer"));
    verticalLayout->addWidget(_enableCheckBox);


    _slider_container = new QWidget(this);
    _slider_container->setEnabled(false);
    QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(_slider_container->sizePolicy().hasHeightForWidth());
    _slider_container->setSizePolicy(sizePolicy1);

    _slider_container->setLayout(new QHBoxLayout() );
    verticalLayout->addWidget(_slider_container);
    QWidget::setTabOrder(_comboBoxPreset, _presetSaveButton);

    //!  ----- Add equalizer slider
    eqSliderPreamp = new EqualizerSlider("Pre-amp", _slider_container);
    connect(eqSliderPreamp, SIGNAL(valueChanged(int)), SLOT(equalizerChanged()));
    _slider_container->layout()->addWidget(eqSliderPreamp);

    QFrame* l = new QFrame(_slider_container);
    l->setFrameShape(QFrame::VLine);
    l->setFrameShadow(QFrame::Sunken);
    _slider_container->layout()->addWidget(l);

    for (int i=0 ; i < Equalizer::kBands ; ++i) {
      EqualizerSlider* eq = new EqualizerSlider(Equalizer::kGainText[i], _slider_container);
      eqSliderList[i] = eq;
      _slider_container->layout()->addWidget(eq);
      connect(eq, SIGNAL(valueChanged(int)), SLOT(equalizerChanged()));
    }

    //! ----- load Settings
    _settingChanged = false;
    restoreSettings();

    //! ----- signals connection
    connect(_comboBoxPreset, SIGNAL(currentIndexChanged(QString)), SLOT(applyPreset(QString)));
    connect(_enableCheckBox, SIGNAL(toggled(bool)), this, SLOT(enableChanged(bool)));

    connect(_presetSaveButton, SIGNAL(clicked()), this, SLOT(savePreset()));
    connect(_presetDelButton, SIGNAL(clicked()), this, SLOT(delPreset()));
}

Equalizer_Dialog::~Equalizer_Dialog()
{
    saveSettings();
}

void Equalizer_Dialog::closeEvent(QCloseEvent* e)
{
    saveSettings();

    QDialog::closeEvent(e);
}


//! ----------------------- equalizerChanged -----------------------------------
void Equalizer_Dialog::equalizerChanged()
{
    //! get equalizer value
    QList<int> gainList;

    int preamp = eqSliderPreamp->value();

    for (int i=0 ; i < Equalizer::kBands ; ++i) {
      gainList << eqSliderList[i]->value();
    }

    //! signal change
    emit newEqualizerValue(preamp, gainList);
    Debug::debug() << "- Equalizer -> new equalizer value emit" << gainList;
}

//! ----------------------- enableChanged --------------------------------------
void Equalizer_Dialog::enableChanged(bool enable)
{
    //! signal change
    emit enabledChanged(enable);
    _slider_container->setEnabled(enable);
    //Debug::debug() << "- Equalizer -> enableChanged emit" << enable;
    _settingChanged = true;
}


//! ----------------------- Enable Management ----------------------------------
bool Equalizer_Dialog::isEnabled() const
{
    return _enableCheckBox->isChecked();
}


/*******************************************************************************
   Preset Management
*******************************************************************************/
//! ----------------------- addDefaultPreset -----------------------------------
void Equalizer_Dialog::addDefaultPreset()
{
    addPreset("Custom",             Equalizer::EqPreset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    addPreset("Classical",          Equalizer::EqPreset(0, 0, 0, 0, 0, 0, -40, -40, -40, -50, 0));
    addPreset("Club",               Equalizer::EqPreset(0, 0, 20, 30, 30, 30, 20, 0, 0, 0, 0));
    addPreset("Dance",              Equalizer::EqPreset(50, 35, 10, 0, 0, -30, -40, -40, 0, 0, 0));
    addPreset("Full Bass",          Equalizer::EqPreset(70, 70, 70, 40, 20, -45, -50, -55, -55, -55, 0));
    addPreset("Full Treble",        Equalizer::EqPreset(-50, -50, -50, -25, 15, 55, 80, 80, 80, 85, 0));
    addPreset("Full Bass + Treble", Equalizer::EqPreset(35, 30, 0, -40, -25, 10, 45, 55, 60, 60, 0));
    addPreset("Laptop/Headphones",  Equalizer::EqPreset(25, 50, 25, -20, 0, -30, -40, -40, 0, 0, 0));
    addPreset("Large Hall",         Equalizer::EqPreset(50, 50, 30, 30, 0, -25, -25, -25, 0, 0, 0));
    addPreset("Live",               Equalizer::EqPreset(-25, 0, 20, 25, 30, 30, 20, 15, 15, 10, 0));
    addPreset("Party",              Equalizer::EqPreset(35, 35, 0, 0, 0, 0, 0, 0, 35, 35, 0));
    addPreset("Pop",                Equalizer::EqPreset(-10, 25, 35, 40, 25, -5, -15, -15, -10, -10, 0));
    addPreset("Reggae",             Equalizer::EqPreset(0, 0, -5, -30, 0, -35, -35, 0, 0, 0, 0));
    addPreset("Rock",               Equalizer::EqPreset(40, 25, -30, -40, -20, 20, 45, 55, 55, 55, 0));
    addPreset("Soft",               Equalizer::EqPreset(25, 10, -5, -15, -5, 20, 45, 50, 55, 60, 0));
    addPreset("Ska",                Equalizer::EqPreset(-15, -25, -25, -5, 20, 30, 45, 50, 55, 50, 0));
    addPreset("Soft Rock",          Equalizer::EqPreset(20, 20, 10, -5, -25, -30, -20, -5, 15, 45, 0));
    addPreset("Techno",             Equalizer::EqPreset(40, 30, 0, -30, -25, 0, 40, 50, 50, 45, 0));
    addPreset("Zero",               Equalizer::EqPreset(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
}

//! ----------------------- addPreset ------------------------------------------
void Equalizer_Dialog::addPreset(const QString& name, const Equalizer::EqPreset& eqPreset)
{
    _presetList[name] = eqPreset;

    if (_comboBoxPreset->findText(name) == -1)
      _comboBoxPreset->addItem(name);
}

//! ----------------------- applyPreset ----------------------------------------
void Equalizer_Dialog::applyPreset(const QString& name)
{
    if (!_presetList.contains(name))  return;

    //Debug::debug() << "- Equalizer -> applyPreset Preset name" << name;
    for (int i=0 ; i < Equalizer::kBands ; ++i) {
      eqSliderList[i]->setValue(_presetList[name].gain[i]);
    }

    eqSliderPreamp->setValue(_presetList[name].preamp);
    equalizerChanged();
}

//! ----------------------- delPreset ------------------------------------------
void Equalizer_Dialog::delPreset()
{
    QString name = _comboBoxPreset->currentText();

    if (!_presetList.contains(name) || name.isEmpty()) return;

    DialogQuestion dlg(this,tr("Delete preset"));
    dlg.setQuestion(tr("Are you sure you want to delete the \"%1\" preset?").arg(name));
    dlg.resize(445, 120);

    if(dlg.exec() == QDialog::Accepted)  {
      _presetList.remove(name);
      _comboBoxPreset->removeItem(_comboBoxPreset->currentIndex());
    }
}

//! ----------------------- savePreset -----------------------------------------
void Equalizer_Dialog::savePreset()
{
    QString currentPresetName = _comboBoxPreset->currentText();

    DialogInput input(this, tr("Save preset"), tr("Name"));
    input.setEditValue(currentPresetName);
    input.setFixedSize(480,140);
    

    if(input.exec() == QDialog::Accepted) {
      QString name = input.editValue();
      
      if (name.isEmpty())  return;
      
      
      Equalizer::EqPreset currentPreset;
      for (int i=0 ; i < Equalizer::kBands ; ++i) {
        currentPreset.gain[i] = eqSliderList[i]->value();
      }
      currentPreset.preamp = eqSliderPreamp->value();

      addPreset(name, currentPreset);
      _comboBoxPreset->setCurrentIndex(_comboBoxPreset->findText(name));

      _settingChanged = true;
    }
}

/*******************************************************************************
   Settings Management
*******************************************************************************/
//! ----------------------- saveSettings ---------------------------------------
void Equalizer_Dialog::saveSettings()
{
    if(!_settingChanged) return;

    SETTINGS()->_enableEq      = _enableCheckBox->isChecked();
    SETTINGS()->_currentPreset = _comboBoxPreset->currentText();

    SETTINGS()->_presetEq.clear();
    foreach (const QString& name, _presetList.keys()) {
      SETTINGS()->_presetEq[name] = _presetList[name];
    }
}


//! ----------------------- restoreSettings ------------------------------------
void Equalizer_Dialog::restoreSettings()
{
    _presetList.clear();
    _comboBoxPreset->clear();

    QMap<QString, Equalizer::EqPreset> presetsEq = SETTINGS()->_presetEq;

    //! Load presets
    if ( presetsEq.isEmpty() ) {
      addDefaultPreset();
    }
    else {
      foreach (const QString& name, presetsEq.keys()) {
        addPreset( name , presetsEq[name] );
      }
    }

    //! Load selected preset
    const QString selected_preset = SETTINGS()->_currentPreset;
    const int selectedIdx = _comboBoxPreset->findText(selected_preset);

    if (selectedIdx != -1) {
      _comboBoxPreset->setCurrentIndex(selectedIdx);
      for (int i=0 ; i < Equalizer::kBands ; ++i) {
        eqSliderList[i]->setValue(_presetList[selected_preset].gain[i]);
      }
    }

    //! Load Enabled state
    _enableCheckBox->setChecked( SETTINGS()->_enableEq );
    _slider_container->setEnabled(_enableCheckBox->isChecked());

    //! signal loaded preset
    if(_enableCheckBox->isChecked())
      equalizerChanged();
}
