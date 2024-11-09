
#include "utilities.h"
#include "settings.h"

#include <stdlib.h>

#include <QBrush>
#include <QByteArray>
#include <QDir>
#include <QImage>
#include <QLatin1Char>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QPixmap>
#include <QRandomGenerator>
#include <QRect>
#include <QString>
#include <QUrl>
#include <QUrlQuery>


int UTIL::randomInt(int low, int high) {
    // Random number between low and high
    return QRandomGenerator::global()->generate() % ((high + 1) - low) + low;
}

QString UTIL::getConfigDir() {
    QString configdir;

    #ifdef TEST_FLAG
    if (getenv("XDG_CONFIG_HOME")==NULL) {
        configdir = getenv("HOME");
        configdir.append("/.config/yarockTEST");
    }
    else {
        configdir = getenv("XDG_CONFIG_HOME");
        configdir.append("/.yarockTEST");
    }
    #else
    if (getenv("XDG_CONFIG_HOME")==NULL) {
        configdir = getenv("HOME");
        configdir.append("/.config/yarock");
    }
    else {
        configdir = getenv("XDG_CONFIG_HOME");
        configdir.append("/.yarock");
    }
    #endif

    return configdir;
}


QString UTIL::getConfigFile() {
    return QString(UTIL::getConfigDir() + QDir::separator() + "yarock-1.conf");
}

QString UTIL::deltaTimeToString(int seconds) {
    return (seconds >= 0 ? "+" : "-") + UTIL::durationToString(seconds);
}

QString UTIL::durationToString(int seconds) {
    int sec     = qAbs(seconds);
    int hours   = sec / (60*60);
    int minutes = (sec / 60) % 60;
    sec %= 60;

    QString ret;
    if (hours) {
        //ret.asprintf("%d:%02d:%02d", hours, minutes, sec);
        ret = QString("%1:%2:%3").arg(hours).arg(minutes, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
    }
    else {
        //ret.asprintf("%d:%02d", minutes, sec);
        ret = QString("%1:%2").arg(minutes).arg(sec, 2, 10, QLatin1Char('0'));
    }

    return ret;
}

void UTIL::urlAddQueryItem(QUrl& url, const QString& key, const QString& value) {
    QUrlQuery urlQuery( url );
    urlQuery.addQueryItem( key, value );
    url.setQuery( urlQuery );
}

bool UTIL::urlHasQueryItem(const QUrl& url, const QString& key) {
    return QUrlQuery( url ).hasQueryItem( key );
}

QPixmap UTIL::createRoundedImage(const QPixmap& pixmap, const QSize& size, float frameWidthPct) {
    int height;
    int width;

    if (!size.isEmpty()) {
        height = size.height();
        width = size.width();
    }
    else {
        height = pixmap.height();
        width = pixmap.width();
    }

    if (!height || !width)
        return QPixmap();

    QPixmap scaledAvatar = pixmap.scaled( width, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    if (frameWidthPct == 0.00)
        return scaledAvatar;

    QPixmap frame( width, height );
    frame.fill( Qt::transparent );

    QPainter painter( &frame );
    painter.setRenderHint( QPainter::Antialiasing );

    QRect outerRect( 0, 0, width, height );
    QBrush brush( scaledAvatar );
    QPen pen;
    pen.setColor( Qt::transparent );
    pen.setJoinStyle( Qt::RoundJoin );

    painter.setBrush( brush );
    painter.setPen( pen );
    painter.drawRoundedRect( outerRect, frameWidthPct * 100.0, frameWidthPct * 100.0, Qt::RelativeSize );

    return frame;
}

QPixmap UTIL::squareCenterPixmap(const QPixmap& sourceImage) {
    if (sourceImage.width() != sourceImage.height())     {
        const int sqwidth = qMin( sourceImage.width(), sourceImage.height() );
        const int delta = abs( sourceImage.width() - sourceImage.height() );

        if (sourceImage.width() > sourceImage.height())         {
            return sourceImage.copy( delta / 2, 0, sqwidth, sqwidth );
        }
        else {
            return sourceImage.copy( 0, delta / 2, sqwidth, sqwidth );
        }
    }

    return sourceImage;
}

QImage UTIL::artistImageFromByteArray(QByteArray array, int size /*= 200 */) {
    QImage image = QImage::fromData( array );

    int ITEM_HEIGHT = size;
    int ITEM_WIDTH  = size;
    int MAX_SIZE    = ITEM_HEIGHT*2;

    const int width = image.size().width();
    const int height = image.size().height();
    if (width > MAX_SIZE || height > MAX_SIZE)
      image = image.scaled(MAX_SIZE, MAX_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    int xOffset = 0;
    int wDiff = image.width() - ITEM_WIDTH;
    if (wDiff > 0) xOffset = wDiff / 2;
    int yOffset = 0;
    int hDiff = image.height() - ITEM_HEIGHT;
    if (hDiff > 0) yOffset = hDiff / 4;

    return image.copy(xOffset, yOffset, ITEM_WIDTH, ITEM_HEIGHT);
}


void UTIL::drawPlayingIcon(QPainter* painter, int size, int margin, QPoint pos) {
    painter->save();
    painter->setPen( SETTINGS()->_baseColor );
    painter->setBrush(QBrush( SETTINGS()->_baseColor ));

    double raHeadHeight = size-2*margin;
    double raHeadWidth = size-2*margin -(raHeadHeight*0.3);

    QPointF raStartPoint = QPointF(pos) + QPointF(margin,margin);

    QPointF raArrowPoints[3];

    double x1 = raStartPoint.x();
    double y1 = raStartPoint.y();
    double x2 = raStartPoint.x();
    double y2 = raStartPoint.y() + raHeadHeight;
    double x3 = raStartPoint.x() + raHeadWidth;
    double y3 = raStartPoint.y() + raHeadHeight/2;

    raArrowPoints[0]=QPointF(x1,y1);
    raArrowPoints[1]=QPointF(x2,y2);
    raArrowPoints[2]=QPointF(x3,y3);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPolygon(raArrowPoints,3);
    painter->restore();
}
