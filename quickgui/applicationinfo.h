#ifndef APPLICATIONINFO_H
#define APPLICATIONINFO_H

#include "../model/fieldmodel.h"

#include <QObject>
#include <QQmlPropertyMap>

namespace QtGui {

class ApplicationInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int applicationWidth READ applicationWidth WRITE setApplicationWidth NOTIFY applicationWidthChanged)
    Q_PROPERTY(bool isMobile READ isMobile CONSTANT)
    Q_PROPERTY(QObject *colors READ colors CONSTANT)
    Q_PROPERTY(QObject *constants READ constants CONSTANT)
    Q_PROPERTY(bool isPortraitMode READ isPortraitMode WRITE setIsPortraitMode NOTIFY portraitModeChanged)
    Q_PROPERTY(const QString &currenfFile READ currentFile WRITE setCurrentFile NOTIFY currentFileChanged)
    Q_PROPERTY(FieldModel *fieldModel READ fieldModel)
    Q_PROPERTY(Io::AccountEntry *currentAccountEntry READ currentAccountEntry WRITE setCurrentAccountEntry)
    Q_PROPERTY(QString currentAccountName READ currentAccountName)
    Q_PROPERTY(qreal ratio READ ratio CONSTANT)
    Q_PROPERTY(qreal hMargin READ hMargin NOTIFY hMarginChanged)
    Q_PROPERTY(qreal sliderHandleWidth READ sliderHandleWidth CONSTANT)
    Q_PROPERTY(qreal sliderHandleHeight READ sliderHandleHeight CONSTANT)
    Q_PROPERTY(qreal sliderGapWidth READ sliderGapWidth CONSTANT)

public:
    ApplicationInfo();

    static constexpr bool isMobile();
    QQmlPropertyMap *colors() const;
    QQmlPropertyMap *constants() const;

    int applicationWidth() const;
    void setApplicationWidth(const int newWidth);

    bool isPortraitMode() const;
    void setIsPortraitMode(const bool newMode);

    const QString &currentFile() const;
    void setCurrentFile(const QString &currentFile);
    FieldModel *fieldModel();
    Io::AccountEntry *currentAccountEntry();
    QString currentAccountName() const;
    void setCurrentAccountEntry(Io::AccountEntry *entry);

    qreal hMargin() const;
    qreal ratio() const;
    qreal sliderHandleHeight();
    qreal sliderGapWidth();
    qreal sliderHandleWidth();

    Q_INVOKABLE QString imagePath(const QString image);

protected Q_SLOTS:
    void notifyPortraitMode(Qt::ScreenOrientation);

protected:
    qreal sizeWithRatio(const qreal height);

Q_SIGNALS:
    void currentFileChanged();
    void applicationWidthChanged();
    void portraitModeChanged();
    void hMarginChanged();

private:
    int m_applicationWidth;
    QQmlPropertyMap *m_colors;
    QQmlPropertyMap *m_constants;
    bool m_isPortraitMode;
    QString m_currentFile;
    FieldModel *m_fieldModel;
    qreal m_ratio;
    qreal m_hMargin;
    qreal m_sliderHandleHeight, m_sliderHandleWidth, m_sliderGapWidth;
};

constexpr bool ApplicationInfo::isMobile()
{
#if defined(Q_OS_ANDROID) || defined(Q_OS_IOS) || defined(Q_OS_BLACKBERRY)
    return true;
#else
    return false;
#endif
}

inline QQmlPropertyMap *ApplicationInfo::colors() const
{
    return m_colors;
}

inline QQmlPropertyMap *ApplicationInfo::constants() const
{
    return m_constants;
}

inline int ApplicationInfo::applicationWidth() const
{
    return m_applicationWidth;
}

inline bool ApplicationInfo::isPortraitMode() const
{
    return m_isPortraitMode;
}

inline const QString &ApplicationInfo::currentFile() const
{
    return m_currentFile;
}

inline void ApplicationInfo::setCurrentFile(const QString &currentFile)
{
    if(m_currentFile != currentFile) {
        m_currentFile = currentFile;
        emit currentFileChanged();
    }
}

inline FieldModel *ApplicationInfo::fieldModel()
{
    return m_fieldModel;
}

inline Io::AccountEntry *ApplicationInfo::currentAccountEntry()
{
    return m_fieldModel->accountEntry();
}

inline QString ApplicationInfo::currentAccountName() const
{
    if(m_fieldModel->accountEntry()) {
        return QString::fromStdString(m_fieldModel->accountEntry()->label());
    }
    return QString();
}

inline void ApplicationInfo::setCurrentAccountEntry(Io::AccountEntry *entry)
{
    m_fieldModel->setAccountEntry(entry);
}

inline qreal ApplicationInfo::hMargin() const
{
    return m_hMargin;
}

inline qreal ApplicationInfo::ratio() const
{
    return m_ratio;
}

inline qreal ApplicationInfo::sliderHandleHeight()
{
    return m_sliderHandleHeight;
}

inline qreal ApplicationInfo::sliderGapWidth()
{
    return m_sliderGapWidth;
}

inline qreal ApplicationInfo::sliderHandleWidth()
{
    return m_sliderHandleWidth;
}

inline qreal ApplicationInfo::sizeWithRatio(const qreal height)
{
    return ratio() * height;
}

}

#endif // APPLICATIONINFO_H
