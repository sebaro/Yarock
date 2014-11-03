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

#include "lyricseditor.h"
#include "utilities.h"
#include "debug.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QCryptographicHash>
#include <QFile>


/*
********************************************************************************
*                                                                              *
*    Class LyricsEditor                                                        *
*                                                                              *
********************************************************************************
*/
LyricsEditor::LyricsEditor(QWidget *parent) :  QDialog(parent)
{
    //! setup GUI
    this->setWindowTitle(tr("Lyrics editor"));
    this->resize(442, 456);
    this->setSizeGripEnabled(true);

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->setSpacing(1);
    verticalLayout->setContentsMargins(3, 0, 3, 3);

    _songName = new QLabel(this);

    _lyricsText = new QPlainTextEdit(this);
    _lyricsText->setTabChangesFocus(true);
    _lyricsText->setLineWrapMode(QPlainTextEdit::NoWrap);

    _buttonBox = new QDialogButtonBox(this);
    _buttonBox->setStandardButtons(QDialogButtonBox::Discard|QDialogButtonBox::Reset|QDialogButtonBox::Save);

    verticalLayout->addWidget(_songName);
    verticalLayout->addWidget(_lyricsText);
    verticalLayout->addWidget(_buttonBox);

    //! signals
    connect(this->_buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(clicked(QAbstractButton*)));
}

void LyricsEditor::setInfo(const QString & artistName, const QString & songName)
{
    _artistName = artistName;
    _songName->setText(songName);
}


void LyricsEditor::setLyrics(const QString& lyrics)
{
    _lyricsText->setPlainText(lyrics);
}


//! ------------- clicked ------------------------------------------------------------
void LyricsEditor::clicked(QAbstractButton* button)
{
    switch (this->_buttonBox->buttonRole(button))
    {
      case QDialogButtonBox::DestructiveRole:
        close();
        break;
      case QDialogButtonBox::ResetRole:
        this->_lyricsText->clear();
        break;
      case QDialogButtonBox::AcceptRole:
        saveLyrics();
        close();
        break;
      default:break;
    }
}

void LyricsEditor::saveLyrics()
{
    //! Get Text
    QString lyricsData = "";

    QStringList lines = this->_lyricsText->document()->toPlainText().split('\n');
    if(lines.isEmpty()) return;

    for (int i = 0; i < lines.length(); i++) {
      lyricsData += "\n" + lines[i];
    }

    //! Build Lyrics Hash (Artist Name + Song Name)
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(_artistName.toLower().toUtf8().constData());
    hash.addData(_songName->text().toLower().toUtf8().constData());

    QString path = QString(UTIL::CONFIGDIR + "/lyrics/" + hash.result().toHex() + ".txt");

    Debug::debug() << " ---- LyricsEditor::saveLyrics" << path;

    //! Write File
    QFile lyricsFile(path);
    lyricsFile.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream out(&lyricsFile);
    out << lyricsData;

    //! Emit Lyrics Changed
    lyricsData.replace(QRegExp("\n"), "<br>");

    /* notify info */
    QVariantHash output;
    output[ "provider" ] = QString("local");
    output[ "lyrics" ]   = lyricsData;
    output[ "url" ]      = path;
        
    INFO::InfoRequestData request;
    request.type = INFO::InfoTrackLyrics;

    emit lyricsChanged(request, output);
}
