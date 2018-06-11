#ifndef QT_QUICK_GUI_CONTROLLER_H
#define QT_QUICK_GUI_CONTROLLER_H

#include "../model/entryfiltermodel.h"
#include "../model/entrymodel.h"
#include "../model/fieldmodel.h"

#include <passwordfile/io/passwordfile.h>

#include <QObject>
#include <QPersistentModelIndex>

namespace QtGui {

class Controller : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool fileOpen READ isFileOpen NOTIFY fileOpenChanged)
    Q_PROPERTY(EntryModel *entryModel READ entryModel NOTIFY entryModelChanged)
    Q_PROPERTY(EntryFilterModel *entryFilterModel READ entryFilterModel NOTIFY entryFilterModelChanged)
    Q_PROPERTY(FieldModel *fieldModel READ fieldModel NOTIFY fieldModelChanged)
    Q_PROPERTY(QModelIndex currentAccountIndex READ currentAccountIndex WRITE setCurrentAccountIndex NOTIFY currentAccountChanged)
    Q_PROPERTY(QString currentAccountName READ currentAccountName NOTIFY currentAccountChanged)
    Q_PROPERTY(QList<QPersistentModelIndex> cutEntries READ cutEntries WRITE setCutEntries NOTIFY cutEntriesChanged)
    Q_PROPERTY(bool canPaste READ canPaste NOTIFY cutEntriesChanged)

public:
    explicit Controller(const QString &filePath = QString(), QObject *parent = nullptr);

    const QString &filePath() const;
    void setFilePath(const QString &filePath);
    const QString &password() const;
    void setPassword(const QString &password);
    const QString &windowTitle() const;
    bool isFileOpen() const;
    EntryModel *entryModel();
    EntryFilterModel *entryFilterModel();
    FieldModel *fieldModel();
    QModelIndex currentAccountIndex() const;
    void setCurrentAccountIndex(const QModelIndex &accountIndex);
    const QList<QPersistentModelIndex> &cutEntries() const;
    void setCutEntries(const QList<QPersistentModelIndex> &cutEntries);
    QString currentAccountName() const;
    Q_INVOKABLE void cutEntry(const QModelIndex &entryIndex);
    Q_INVOKABLE bool pasteEntries(const QModelIndex &destinationParent, int row = -1);
    bool canPaste() const;

public slots:
    void load();
    void create();
    void close();
    void save();

signals:
    void filePathChanged(const QString &newFilePath);
    void passwordChanged(const QString &newPassword);
    void passwordRequired(const QString &filePath);
    void windowTitleChanged(const QString &windowTitle);
    void fileOpenChanged(bool fileOpen);
    void fileError(const QString &errorMessage);
    void entryModelChanged();
    void entryFilterModelChanged();
    void fieldModelChanged();
    void currentAccountChanged();
    void cutEntriesChanged(const QList<QPersistentModelIndex> &cutEntries);

private:
    void resetFileStatus();
    void updateWindowTitle();
    void setFileOpen(bool fileOpen);
    void emitIoError(const QString &when);

    QString m_filePath;
    QString m_password;
    QString m_windowTitle;
    Io::PasswordFile m_file;
    EntryModel m_entryModel;
    EntryFilterModel m_entryFilterModel;
    FieldModel m_fieldModel;
    QList<QPersistentModelIndex> m_cutEntries;
    bool m_fileOpen;
    bool m_fileModified;
};

inline const QString &Controller::filePath() const
{
    return m_filePath;
}

inline const QString &Controller::password() const
{
    return m_password;
}

inline const QString &Controller::windowTitle() const
{
    return m_windowTitle;
}

inline bool Controller::isFileOpen() const
{
    return m_fileOpen;
}

inline EntryModel *Controller::entryModel()
{
    return &m_entryModel;
}

inline EntryFilterModel *Controller::entryFilterModel()
{
    return &m_entryFilterModel;
}

inline FieldModel *Controller::fieldModel()
{
    return &m_fieldModel;
}

inline QModelIndex Controller::currentAccountIndex() const
{
    return m_fieldModel.accountEntry() ? m_entryModel.index(const_cast<Io::AccountEntry *>(m_fieldModel.accountEntry())) : QModelIndex();
}

inline void Controller::setCurrentAccountIndex(const QModelIndex &accountIndex)
{
    m_fieldModel.setAccountEntry(m_entryModel.isNode(accountIndex) ? nullptr : static_cast<Io::AccountEntry *>(m_entryModel.entry(accountIndex)));
    emit currentAccountChanged();
}

inline const QList<QPersistentModelIndex> &Controller::cutEntries() const
{
    return m_cutEntries;
}

inline void Controller::setCutEntries(const QList<QPersistentModelIndex> &cutEntries)
{
    cutEntriesChanged(m_cutEntries = cutEntries);
}

inline QString Controller::currentAccountName() const
{
    return m_fieldModel.accountEntry() ? QString::fromStdString(m_fieldModel.accountEntry()->label()) : QStringLiteral("?");
}

inline void Controller::cutEntry(const QModelIndex &entryIndex)
{
    cutEntriesChanged(m_cutEntries << QPersistentModelIndex(entryIndex));
}

inline bool Controller::canPaste() const
{
    return !m_cutEntries.isEmpty();
}

} // namespace QtGui

#endif // QT_QUICK_GUI_CONTROLLER_H
