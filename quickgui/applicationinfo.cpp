#include "applicationinfo.h"

#include <qmath.h>

#include <QFile>
#include <QRegExp>
#include <QUrl>
#include <QUrlQuery>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>

using namespace Io;

namespace QtGui {

ApplicationInfo::ApplicationInfo()
{
    m_colors = new QQmlPropertyMap(this);
    m_colors->insert(QLatin1String("white"), QVariant("#ffffff"));
    m_colors->insert(QLatin1String("smokeGray"), QVariant("#eeeeee"));
    m_colors->insert(QLatin1String("paleGray"), QVariant("#d7d6d5"));
    m_colors->insert(QLatin1String("lightGray"), QVariant("#aeadac"));
    m_colors->insert(QLatin1String("darkGray"), QVariant("#35322f"));
    m_colors->insert(QLatin1String("mediumGray"), QVariant("#5d5b59"));
    m_colors->insert(QLatin1String("doubleDarkGray"), QVariant("#1e1b18"));
    m_colors->insert(QLatin1String("blue"), QVariant("#14aaff"));
    m_colors->insert(QLatin1String("yetAnotherBlue"), QVariant("#428bca"));
    m_colors->insert(QLatin1String("darkBlue"), QVariant("#14148c"));
    m_colors->insert(QLatin1String("darkYellow"), QVariant("#dfdc00"));
    m_colors->insert(QLatin1String("darkYellow"), QVariant("#eb881c"));
    m_colors->insert(QLatin1String("almostBlack"), QVariant("#222222"));

    m_constants = new QQmlPropertyMap(this);
    m_constants->insert(QLatin1String("isMobile"), QVariant(isMobile()));

    QRect rect = QGuiApplication::primaryScreen()->geometry();
    m_ratio = isMobile() ? qMin(qMax(rect.width(), rect.height()) / 1136., qMin(rect.width(), rect.height()) / 640.) : .5;
    m_sliderHandleWidth = sizeWithRatio(70);
    m_sliderHandleHeight = sizeWithRatio(87);
    m_sliderGapWidth = sizeWithRatio(100);
    m_isPortraitMode = isMobile() ? rect.height() > rect.width() : false;
    m_hMargin =  m_isPortraitMode ? 20 * ratio() : 50 * ratio();
    m_applicationWidth = isMobile() ? rect.width() : 1120;

    m_constants->insert(QLatin1String("rowDelegateHeight"), QVariant(sizeWithRatio(118)));

    m_fieldModel = new FieldModel(this);

    if(isMobile()) {
        connect(QGuiApplication::primaryScreen(), &QScreen::orientationChanged, this, &ApplicationInfo::notifyPortraitMode);
    }
}

void ApplicationInfo::setApplicationWidth(const int newWidth)
{
    if (newWidth != m_applicationWidth) {
        m_applicationWidth = newWidth;
        emit applicationWidthChanged();
    }
}

QString ApplicationInfo::imagePath(const QString image)
{
    return QStringLiteral("qrc:/qml/images/%1").arg(image);
}

void ApplicationInfo::notifyPortraitMode(Qt::ScreenOrientation orientation)
{
    switch (orientation) {
    case Qt::LandscapeOrientation:
    case Qt::InvertedLandscapeOrientation:
        setIsPortraitMode(false);
        break;
    case Qt::PortraitOrientation:
    case Qt::InvertedPortraitOrientation:
        setIsPortraitMode(true);
        break;
    default:
        break;
    }
}

void ApplicationInfo::setIsPortraitMode(const bool newMode)
{
    if (m_isPortraitMode != newMode) {
        m_isPortraitMode = newMode;
        m_hMargin = m_isPortraitMode ? 20 * ratio() : 50 * ratio();
        emit portraitModeChanged();
        emit hMarginChanged();
    }
}

}
