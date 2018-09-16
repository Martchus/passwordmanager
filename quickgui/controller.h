#ifndef QT_QUICK_GUI_CONTROLLER_H
#define QT_QUICK_GUI_CONTROLLER_H

#include "../model/entryfiltermodel.h"
#include "../model/entrymodel.h"
#include "../model/fieldmodel.h"

#include <passwordfile/io/passwordfile.h>

#include <QObject>
#include <QPersistentModelIndex>

QT_FORWARD_DECLARE_CLASS(QSettings)

namespace QtGui {

class Controller : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY filePathChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword RESET clearPassword NOTIFY passwordChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle NOTIFY windowTitleChanged)
    Q_PROPERTY(bool fileOpen READ isFileOpen NOTIFY fileOpenChanged)
    Q_PROPERTY(bool passwordSet READ isPasswordSet NOTIFY passwordChanged)
    Q_PROPERTY(EntryModel *entryModel READ entryModel NOTIFY entryModelChanged)
    Q_PROPERTY(EntryFilterModel *entryFilterModel READ entryFilterModel NOTIFY entryFilterModelChanged)
    Q_PROPERTY(FieldModel *fieldModel READ fieldModel NOTIFY fieldModelChanged)
    Q_PROPERTY(Io::AccountEntry *currentAccount READ currentAccount WRITE setCurrentAccount NOTIFY currentAccountChanged)
    Q_PROPERTY(QModelIndex currentAccountIndex READ currentAccountIndex WRITE setCurrentAccountIndex NOTIFY currentAccountChanged)
    Q_PROPERTY(QString currentAccountName READ currentAccountName NOTIFY currentAccountChanged)
    Q_PROPERTY(bool hasCurrentAccount READ hasCurrentAccount NOTIFY currentAccountChanged)
    Q_PROPERTY(QList<QPersistentModelIndex> cutEntries READ cutEntries WRITE setCutEntries NOTIFY cutEntriesChanged)
    Q_PROPERTY(bool canPaste READ canPaste NOTIFY cutEntriesChanged)
    Q_PROPERTY(QStringList recentFiles READ recentFiles RESET clearRecentFiles NOTIFY recentFilesChanged)
    Q_PROPERTY(bool useNativeFileDialog READ useNativeFileDialog WRITE setUseNativeFileDialog NOTIFY useNativeFileDialogChanged)
    Q_PROPERTY(bool supportsNativeFileDialog READ supportsNativeFileDialog NOTIFY supportsNativeFileDialogChanged)

public:
    explicit Controller(QSettings &settings, const QString &filePath = QString(), QObject *parent = nullptr);

    const QString &filePath() const;
    const QString &fileName() const;
    void setFilePath(const QString &filePath);
    const QString &password() const;
    void setPassword(const QString &password);
    Q_INVOKABLE void clearPassword();
    const QString &windowTitle() const;
    bool isFileOpen() const;
    bool isPasswordSet() const;
    EntryModel *entryModel();
    EntryFilterModel *entryFilterModel();
    FieldModel *fieldModel();
    Io::AccountEntry *currentAccount();
    void setCurrentAccount(Io::AccountEntry *entry);
    QModelIndex currentAccountIndex() const;
    void setCurrentAccountIndex(const QModelIndex &accountIndex);
    bool hasCurrentAccount() const;
    const QList<QPersistentModelIndex> &cutEntries() const;
    void setCutEntries(const QList<QPersistentModelIndex> &cutEntries);
    QString currentAccountName() const;
    Q_INVOKABLE void cutEntry(const QModelIndex &entryIndex);
    Q_INVOKABLE QStringList pasteEntries(const QModelIndex &destinationParent, int row = -1);
    Q_INVOKABLE bool copyToClipboard(const QString &text) const;
    bool canPaste() const;
    const QStringList &recentFiles() const;
    Q_INVOKABLE void clearRecentFiles();
    bool useNativeFileDialog() const;
    void setUseNativeFileDialog(bool useNativeFileDialog);
    bool supportsNativeFileDialog() const;

public slots:
    void init();
    void load(const QString &filePath = QString());
    void create(const QString &filePath = QString());
    void close();
    void save();
    bool showNativeFileDialog(bool existing);
    void handleFileSelectionAccepted(const QString &filePath, bool existing);
#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
    void handleFileSelectionAcceptedDescriptor(const QString &nativeUrl, const QString &fileName, int fileDescriptor, bool existing);
#endif
    void handleFileSelectionCanceled();

signals:
    void filePathChanged(const QString &newFilePath);
    void passwordChanged(const QString &newPassword);
    void passwordRequired(const QString &filePath);
    void windowTitleChanged(const QString &windowTitle);
    void fileOpenChanged(bool fileOpen);
    void fileError(const QString &errorMessage);
    void fileSaved();
    void entryModelChanged();
    void entryFilterModelChanged();
    void fieldModelChanged();
    void currentAccountChanged();
    void cutEntriesChanged(const QList<QPersistentModelIndex> &cutEntries);
    void recentFilesChanged(const QStringList &recentFiles);
    void newNotification(const QString &message);
    void useNativeFileDialogChanged(bool useNativeFileDialog);
    void supportsNativeFileDialogChanged();
    void entryAboutToBeRemoved(const QModelIndex &removedIndex);

private slots:
    void handleEntriesRemoved(const QModelIndex &parentIndex, int first, int last);
    void handleRecentFilesChanged();

private:
    void resetFileStatus();
    void updateWindowTitle();
    void setFileOpen(bool fileOpen);
    void emitIoError(const QString &when);

    QSettings &m_settings;
    QString m_filePath;
    QString m_fileName;
    QString m_password;
    QString m_windowTitle;
    Io::PasswordFile m_file;
    EntryModel m_entryModel;
    EntryFilterModel m_entryFilterModel;
    FieldModel m_fieldModel;
    QList<QPersistentModelIndex> m_cutEntries;
    QStringList m_recentFiles;
    QString m_nativeUrl;
    bool m_fileOpen;
    bool m_fileModified;
    bool m_useNativeFileDialog;
};

inline const QString &Controller::filePath() const
{
    return m_filePath;
}

inline const QString &Controller::fileName() const
{
    return m_fileName;
}

inline const QString &Controller::password() const
{
    return m_password;
}

inline void Controller::clearPassword()
{
    setPassword(QString());
}

inline const QString &Controller::windowTitle() const
{
    return m_windowTitle;
}

inline bool Controller::isFileOpen() const
{
    return m_fileOpen;
}

inline bool Controller::isPasswordSet() const
{
    return !m_password.isEmpty();
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

inline Io::AccountEntry *Controller::currentAccount()
{
    return m_fieldModel.accountEntry();
}

inline void Controller::setCurrentAccount(Io::AccountEntry *entry)
{
    m_fieldModel.setAccountEntry(entry);
    emit currentAccountChanged();
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

inline bool Controller::hasCurrentAccount() const
{
    return m_fieldModel.accountEntry() != nullptr;
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

inline const QStringList &Controller::recentFiles() const
{
    return m_recentFiles;
}

inline bool Controller::useNativeFileDialog() const
{
    return m_useNativeFileDialog;
}

inline bool Controller::supportsNativeFileDialog() const
{
#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
    return true;
#else
    return false;
#endif
}

} // namespace QtGui

#endif // QT_QUICK_GUI_CONTROLLER_H
