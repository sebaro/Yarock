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

#include "shortcutdialog.h"
#include "debug.h"


/*
********************************************************************************
*                                                                              *
*    Class ShortcutDialog                                                      *
*                                                                              *
********************************************************************************
*/
ShortcutDialog::ShortcutDialog(const QString& key,const QString& shortcut, QWidget *parent) : DialogBase(parent)
{
    m_key          = key;
    m_old_shortcut = shortcut;

    //! ui
    this->setTitle(tr("Change shortcut"));
    this->resize(400, 170);

    QLabel * label= new QLabel(this);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(false);
    label->setText(tr("Press a keys that you want to set as a shortcut"));

    m_currentShortcut = new QLabel(this);
    m_currentShortcut->setAlignment(Qt::AlignCenter);
    m_currentShortcut->setText(m_old_shortcut.toString());

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(label);
    layout->addWidget(m_currentShortcut);

    setContentLayout(layout);
    
    //! connection
    connect(buttonBox(), SIGNAL(rejected()),this, SLOT(close()));
    connect(buttonBox(), SIGNAL(accepted()),this, SLOT(accept()));

    buttonBox()->setFocusPolicy( Qt::NoFocus );
    
    //! List legal modifiers
    m_modifier_keys << Qt::Key_Shift << Qt::Key_Control << Qt::Key_Meta << Qt::Key_Alt << Qt::Key_AltGr;
}

void ShortcutDialog::showEvent(QShowEvent* e) 
{
  QDialog::grabKeyboard();
  QDialog::showEvent(e);
}

void ShortcutDialog::hideEvent(QHideEvent* e)
{
  QDialog::releaseKeyboard();
  QDialog::hideEvent(e);
}


bool ShortcutDialog::event(QEvent* e) 
{
    QKeySequence tempkeyseq = QKeySequence();
    if (e->type() == QEvent::ShortcutOverride) 
    {
      QKeyEvent* ke = static_cast<QKeyEvent*>(e);
      
      if(ke->key() == Qt::Key_Return) 
      {
        accept();
        return true;
      }
      else if(ke->key() == Qt::Key_Space) 
      {
        tempkeyseq = QKeySequence();
        accept();
        return true;
      }

      if (m_modifier_keys.contains(ke->key()))
        tempkeyseq = QKeySequence(ke->modifiers());
      else
        tempkeyseq = QKeySequence(ke->modifiers() | ke->key());

      
      m_new_shortcut = QKeySequence( tempkeyseq );
      m_currentShortcut->setText(tempkeyseq.toString(QKeySequence::PortableText));

//       Debug::debug() << "ShortcutDialog::keyReleaseEvent ke->key():" << ke->key();
//       Debug::debug() << "ShortcutDialog::keyReleaseEvent :" << m_new_shortcut.toString(QKeySequence::PortableText);
//      Debug::debug() << "ShortcutDialog::keyReleaseEvent :" << m_new_shortcut.toString(QKeySequence::NativeText);
      
      if (!m_modifier_keys.contains(ke->key()))
        accept();
 
      return true;
    }
    return QDialog::event(e);
}



void ShortcutDialog::accept()
{
    //emit shortcutChanged(_index, _shortcut);
    QDialog::accept();
    this->close();
}


QString ShortcutDialog::newShortcut()
{
    return m_new_shortcut.toString(QKeySequence::PortableText);
}

