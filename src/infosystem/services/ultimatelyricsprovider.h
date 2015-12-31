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

//! this file is based on file from Clementine Player Project
//! thanks for all good works for Clementine developers ;)

#ifndef _ULTIMATE_LYRICS_PROVIDER_H
#define _ULTIMATE_LYRICS_PROVIDER_H

#include <QObject>
#include <QPair>
#include <QStringList>

#include "info_system.h"
/*
********************************************************************************
*                                                                              *
*    Class UltimateLyricsProvider                                              *
*                                                                              *
********************************************************************************
*/

class UltimateLyricsProvider : public QObject {
  Q_OBJECT

public:
  UltimateLyricsProvider();

  static const int kRedirectLimit;

  typedef QPair<QString, QString> RuleItem;
  typedef QList<RuleItem> Rule;
  typedef QPair<QString, QString> UrlFormat;

  void set_name(const QString& name) { name_ = name; }
  void set_title(const QString& title) { title_ = title; }
  void set_url(const QString& url) { url_ = url; }
  void set_charset(const QString& charset) { charset_ = charset; }

  void add_url_format(const QString& replace, const QString& with) {
    url_formats_ << UrlFormat(replace, with); }

  void add_extract_rule(const Rule& rule) { extract_rules_ << rule; }
  void add_exclude_rule(const Rule& rule) { exclude_rules_ << rule; }
  void add_invalid_indicator(const QString& indicator) { invalid_indicators_ << indicator; }

  QString name() const { return name_; }

  void FetchInfo(INFO::InfoRequestData request);
  QString lyricsUrl() {return lyrics_url_;}

private slots:
  void signalFinish();
  void LyricsFetched(QByteArray bytes);

private:
  void ApplyExtractRule(const Rule& rule, QString* content) const;
  void ApplyExcludeRule(const Rule& rule, QString* content) const;

  static QString ExtractXmlTag(const QString& source, const QString& tag);
  static QString Extract(const QString& source, const QString& begin, const QString& end);
  static QString ExcludeXmlTag(const QString& source, const QString& tag);
  static QString Exclude(const QString& source, const QString& begin, const QString& end);
  static QString FirstChar(const QString& text);
  static QString TitleCase(const QString& text);
  static QString NoSpace(const QString& text);

  void ReplaceField(const QString& tag, const QString& value, QString* text) const;
  void ReplaceFields(INFO::InfoRequestData request, QString* text) const;

private:
  QMap<QObject*, INFO::InfoRequestData> m_requests;

  QString name_;
  QString title_;
  QString url_;        // base url
  QString lyrics_url_; // base url + metadata
  QString charset_;

  QList<UrlFormat> url_formats_;
  QList<Rule> extract_rules_;
  QList<Rule> exclude_rules_;
  QStringList invalid_indicators_;

signals:
  void InfoReady(INFO::InfoRequestData request, const QString &lyrics);
  void Finished(INFO::InfoRequestData request);
};

#endif // _ULTIMATE_LYRICS_PROVIDER_H
