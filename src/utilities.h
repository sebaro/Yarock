
#ifndef _UTIL_H_
#define _UTIL_H_

#include <QApplication>
#include <QImage>
#include <QLatin1Char>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStyle>
#include <QUrl>


static QStyle *custom_style = 0;

namespace UTIL {
    int randomInt(int low, int high);

    QString getConfigDir();
    QString getConfigFile();

    static const QString CONFIGDIR  = getConfigDir();

    static const QString CONFIGFILE = getConfigFile();

    QString deltaTimeToString(int seconds);
    QString durationToString(int seconds);

    static inline QString prettyTrackNumber(int number) {
        QString ret;
        //ret.asprintf("%02d", number);
        ret = QString("%1").arg(number, 2, 10, QLatin1Char('0'));
        return ret;
    }

    void urlAddQueryItem(QUrl& url, const QString& key, const QString& value);
    bool urlHasQueryItem(const QUrl& url, const QString& key);

    QPixmap createRoundedImage(const QPixmap& avatar, const QSize& size=QSize(), float frameWidthPct = 0.10);

    QPixmap squareCenterPixmap(const QPixmap& sourceImage );

    QImage artistImageFromByteArray(QByteArray array, int size = 200);

    void drawPlayingIcon(QPainter* painter, int size, int margin, QPoint pos);

    static inline QStyle* getStyle() {
        if(!custom_style) {
            custom_style = QApplication::style();
        }
        return  custom_style;
    }

    static inline QFont alternateFont() {
        QFont font = QApplication::font();
        font.setPointSize(QApplication::font().pointSize() - 1);
        font.setWeight( QFont::Bold );
        return font;
    }

    static inline QFontMetrics alternateFontMetric() {
        return  QFontMetrics(alternateFont());
    }
}

#endif // _UTIL_H_
