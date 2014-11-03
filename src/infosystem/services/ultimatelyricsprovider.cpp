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

//! this file is based on file from Clementine Player Project
//! thanks for all good works to Clementine developers ;)

#include "ultimatelyricsprovider.h"
#include "core/mediaitem/mediaitem.h"
#include "networkaccess.h"
#include "debug.h"

/*
********************************************************************************
*                                                                              *
*    Class UltimateLyricsProvider                                              *
*                                                                              *
********************************************************************************
*/
UltimateLyricsProvider::UltimateLyricsProvider()
{
}

void UltimateLyricsProvider::FetchInfo(INFO::InfoRequestData request)
{
    //Debug::debug() << name() << " fetch info ";

    const QTextCodec* codec = QTextCodec::codecForName(charset_.toLatin1().constData());
    if (!codec) {
      qWarning() << "Invalid codec" << charset_;
      emit Finished(request);
      return;
    }

    //! Fill in fields in the URL
    //Debug::debug() << " ---- UltimateLyricsProvider::base url " << url_;

    lyrics_url_ = url_;
    ReplaceFields(request, &lyrics_url_);

    //Debug::debug() << " ---- UltimateLyricsProvider::complete url " << lyrics_url_;
    QUrl url(lyrics_url_);

    QObject *reply = HTTP()->get( url );
    m_requests[reply] = request;

    connect(reply, SIGNAL(data(QByteArray)), this, SLOT(LyricsFetched(QByteArray)));
    connect(reply, SIGNAL(error(QNetworkReply*)), this, SLOT(signalFinish()));
}


void UltimateLyricsProvider::signalFinish()
{
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))   return;
    
    INFO::InfoRequestData request =  m_requests.take(reply);
 
    emit Finished(request);
}


void UltimateLyricsProvider::LyricsFetched(QByteArray bytes)
{
    //Debug::debug() << "[UltimateLyricsProvider] " << name_ << " LyricsFetched";
 
    QObject* reply = qobject_cast<QObject*>(sender());
    if (!reply || !m_requests.contains(reply))   return;

    INFO::InfoRequestData request =  m_requests.take(reply);

    const QTextCodec* codec = QTextCodec::codecForName(charset_.toLatin1().constData());
    const QString original_content = codec->toUnicode(bytes);
    //Debug::debug() << "[UltimateLyricsProvider] original_content :" << original_content;

    QString lyrics;

    // Check for invalid indicators
    foreach (const QString& indicator, invalid_indicators_) {
      if (original_content.contains(indicator)) {
        emit Finished(request);
        return;
      }
    }

    
    // Apply extract rules
    foreach (const Rule& rule, extract_rules_) {
      // Modify the rule for this request's metadata
      Rule rule_copy(rule);
      for (Rule::iterator it = rule_copy.begin() ; it != rule_copy.end() ; ++it) {
        ReplaceFields(request, &it->first);
      }

      QString content = original_content;
      ApplyExtractRule(rule_copy, &content);
      //Debug::debug() << "Extract rule" << rule_copy << "matched" << content.length();

      if (!content.isEmpty()) {
        lyrics = content;
        break;
      }
    }

    // Apply exclude rules
    foreach (const Rule& rule, exclude_rules_) {
      ApplyExcludeRule(rule, &lyrics);
    }

    //Debug::debug() << "[UltimateLyricsProvider] lyrics :" << lyrics;
    
    if (!lyrics.isEmpty()) {
      //Debug::debug() << "[UltimateLyricsProvider] :" << name_ << " emit InfoReady !!";
      emit InfoReady(request, lyrics);
    }
    else {
      /* WARNING CHANGE FOR YAROCK : only emit Finished in case of No Lyrics WARNING*/
      /* Request are send synchronously if no lyrics is found */
      emit Finished(request);
    }
}


void UltimateLyricsProvider::ApplyExtractRule(const Rule& rule, QString* content) const
{
    foreach (const RuleItem& item, rule) {
      if (item.second.isNull()) {
        *content = ExtractXmlTag(*content, item.first);
      }
      else {
        *content = Extract(*content, item.first, item.second);
      }
    }
}


QString UltimateLyricsProvider::ExtractXmlTag(const QString& source, const QString& tag)
{
    QRegExp re("<(\\w+).*>");
    if (re.indexIn(tag) == -1)
      return QString();

    return Extract(source, tag, "</" + re.cap(1) + ">");
}


QString UltimateLyricsProvider::Extract(const QString& source, const QString& begin, const QString& end)
{
    int begin_idx = source.indexOf(begin);
    if (begin_idx == -1)
      return QString();
    begin_idx += begin.length();

    int end_idx = source.indexOf(end, begin_idx);
    if (end_idx == -1)
      return QString();

    return source.mid(begin_idx, end_idx - begin_idx - 1);
}


void UltimateLyricsProvider::ApplyExcludeRule(const Rule& rule, QString* content) const
{
    foreach (const RuleItem& item, rule) {
      if (item.second.isNull()) {
        *content = ExcludeXmlTag(*content, item.first);
      }
      else {
        *content = Exclude(*content, item.first, item.second);
      }
    }
}


QString UltimateLyricsProvider::ExcludeXmlTag(const QString& source, const QString& tag)
{
    QRegExp re("<(\\w+).*>");
    if (re.indexIn(tag) == -1)
      return source;

    return Exclude(source, tag, "</" + re.cap(1) + ">");
}


QString UltimateLyricsProvider::Exclude(const QString& source, const QString& begin, const QString& end)
{
    int begin_idx = source.indexOf(begin);
    if (begin_idx == -1)
      return source;

    int end_idx = source.indexOf(end, begin_idx + begin.length());
    if (end_idx == -1)
      return source;

    return source.left(begin_idx) + source.right(source.length() - end_idx - end.length());
}


QString UltimateLyricsProvider::FirstChar(const QString& text)
{
    if (text.isEmpty())
      return QString();
    return text[0].toLower();
}


QString UltimateLyricsProvider::TitleCase(const QString& text)
{
    if (text.length() == 0)
      return QString();

    if (text.length() == 1)
      return text[0].toUpper();

    return text[0].toUpper() + text.right(text.length() - 1).toLower();
}


void UltimateLyricsProvider::ReplaceField(const QString& tag, const QString& value,
                                          QString* text) const {
    if (!text->contains(tag))
      return;

    // Apply URL character replacement
    QString value_copy(value);
    foreach (const UrlFormat& format, url_formats_) {
      QRegExp re("[" + QRegExp::escape(format.first) + "]");
      value_copy.replace(re, format.second);
    }

    text->replace(tag, value_copy, Qt::CaseInsensitive);
}


void UltimateLyricsProvider::ReplaceFields(INFO::InfoRequestData request, QString* text) const
{
    INFO::InfoStringHash hash = request.data.value< INFO::InfoStringHash >();

    ReplaceField("{artist}",  hash["artist"].toLower(),          text);
    ReplaceField("{artist2}", NoSpace(hash["artist"].toLower()), text);
    ReplaceField("{album}",   hash["album"].toLower(),           text);
    ReplaceField("{album2}",  NoSpace(hash["album"].toLower()),  text);
    ReplaceField("{title}",   hash["title"].toLower(),           text);
    ReplaceField("{Artist}",  hash["artist"],                    text);
    ReplaceField("{Album}",   hash["album"],                     text);
    ReplaceField("{ARTIST}",  hash["artist"].toUpper(),          text);
    ReplaceField("{year}",    hash["year"],     text);
    ReplaceField("{Title}",   hash["title"],                     text);
    ReplaceField("{Title2}",  TitleCase(hash["title"]),          text);
    ReplaceField("{a}",       FirstChar(hash["artist"]),         text);
    ReplaceField("{track}",   hash["number"],      text);
}

QString UltimateLyricsProvider::NoSpace(const QString& text)
{
    QString ret(text);
    ret.remove(' ');
    return ret;
}

