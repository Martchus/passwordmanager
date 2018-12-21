#include "./controller.h"
#include "./android.h"

#include <passwordfile/io/cryptoexception.h>
#include <passwordfile/io/parsingexception.h>

#include <qtutilities/misc/dialogutils.h>

#include <c++utilities/io/catchiofailure.h>
#include <c++utilities/io/nativefilestream.h>
#include <c++utilities/io/path.h>

#ifndef QT_NO_CLIPBOARD
#include <QClipboard>
#endif
#ifdef DEBUG_BUILD
#include <QDebug>
#endif
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QSettings>
#include <QStringBuilder>

#include <cassert>
#include <stdexcept>

using namespace std;
using namespace Io;
using namespace IoUtilities;
using namespace Dialogs;

namespace QtGui {

Controller::Controller(QSettings &settings, const QString &filePath, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    , m_entryModel(&m_undoStack)
    , m_fieldModel(&m_undoStack)
#endif
    , m_fileOpen(false)
    , m_fileModified(false)
    , m_useNativeFileDialog(false)
    , m_filterAsDialog(
#ifdef Q_OS_ANDROID
          true
#else
          false
#endif
      )
{
    m_fieldModel.setPasswordVisibility(PasswordVisibility::Never);
    m_entryFilterModel.setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_entryFilterModel.setSourceModel(&m_entryModel);
    connect(&m_entryModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &Controller::handleEntriesRemoved);

#ifdef PASSWORD_MANAGER_UNDO_SUPPORT
    connect(&m_undoStack, &QUndoStack::undoTextChanged, this, &Controller::undoTextChanged);
    connect(&m_undoStack, &QUndoStack::redoTextChanged, this, &Controller::redoTextChanged);
#endif

    // share settings with main window
    m_settings.beginGroup(QStringLiteral("mainwindow"));
    m_recentFiles = m_settings.value(QStringLiteral("recententries")).toStringList();
    m_useNativeFileDialog = m_settings.value(QStringLiteral("usenativefiledialog"), m_useNativeFileDialog).toBool();
    connect(this, &Controller::recentFilesChanged, this, &Controller::handleRecentFilesChanged);

    // set initial file path
    setFilePath(filePath);
}

void Controller::setFilePath(const QString &filePath)
{
    // get rid of file:// prefix
    QStringRef actualFilePath(&filePath);
    if (filePath.startsWith(QLatin1String("file:"))) {
        actualFilePath = filePath.midRef(5);
    }
    while (filePath.startsWith(QLatin1String("//"))) {
        actualFilePath = actualFilePath.mid(1);
    }

    // skip if this path is already set
    if (m_filePath == actualFilePath) {
        return;
    }

    // assign full file path and file name
    m_file.clear();
    m_file.setPath(filePath.toLocal8Bit().data());
    m_fileName = QString::fromLocal8Bit(IoUtilities::fileName(m_file.path()).data());
    emit filePathChanged(m_filePath = filePath);

    // clear password so we don't use the password from the previous file
    m_password.clear();

    // handle recent files
    const auto index = m_recentFiles.indexOf(m_filePath);
    if (!index) {
        return;
    }
    if (index < 0) {
        m_recentFiles.prepend(m_filePath);
    } else if (index > 0) {
        m_recentFiles[index].swap(m_recentFiles.first());
    }
    while (m_recentFiles.size() > 10) {
        m_recentFiles.removeLast();
    }
    emit recentFilesChanged(m_recentFiles);
}

void Controller::setPassword(const QString &password)
{
    if (m_password == password) {
        return;
    }
    m_file.setPassword(password.toUtf8().data());
    emit passwordChanged(m_password = password);
}

void Controller::init()
{
    if (!m_filePath.isEmpty()) {
        load();
    }
}

void Controller::load(const QString &filePath)
{
    if (!filePath.isEmpty()) {
        setFilePath(filePath);
    }

    resetFileStatus();
    try {
        m_file.load();
        m_entryModel.setRootEntry(m_file.rootEntry());
        if (!m_entryModel.rootEntry()) {
            emit fileError(tr("An error occured when opening the file: root element missing"), QStringLiteral("load"));
            return;
        }
        setFileOpen(true);
        updateWindowTitle();
    } catch (const CryptoException &e) {
        if (m_file.isEncryptionUsed() && m_password.isEmpty()) {
            emit passwordRequired(m_filePath);
        } else {
            // clear password since the password which has been provided likely wasn't correct
            clearPassword();
            emit fileError(tr("A crypto error occured when opening the file: ") + QString::fromLocal8Bit(e.what()), QStringLiteral("load"));
        }
    } catch (const runtime_error &e) {
        emit fileError(tr("A parsing error occured when opening the file: ") + QString::fromLocal8Bit(e.what()), QStringLiteral("load"));
    } catch (...) {
        emitIoError(tr("loading"));
    }
}

void Controller::create(const QString &filePath)
{
    if (!filePath.isEmpty()) {
        setFilePath(filePath);
    }

    resetFileStatus();
    try {
        if (filePath.isEmpty()) {
            m_file.clear();
        }
        m_file.create();
    } catch (...) {
        emitIoError(tr("creating"));
    }

    m_file.generateRootEntry();
    m_entryModel.setRootEntry(m_file.rootEntry());
    setFileOpen(true);
    updateWindowTitle();
}

void Controller::close()
{
    try {
        m_file.close();
        resetFileStatus();
    } catch (...) {
        emitIoError(tr("closing"));
    }
}

PasswordFileSaveFlags Controller::prepareSaving()
{
    auto flags = PasswordFileSaveFlags::Compression | PasswordFileSaveFlags::PasswordHashing;
    if (!m_password.isEmpty()) {
        flags |= PasswordFileSaveFlags::Encryption;
        const auto passwordUtf8(m_password.toUtf8());
        m_file.setPassword(passwordUtf8.data(), static_cast<size_t>(passwordUtf8.size()));
    } else {
        m_file.clearPassword();
    }
    return flags;
}

void Controller::save()
{
    const auto flags = prepareSaving();
    try {
#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
        if (!m_nativeUrl.isEmpty()) {
            // ensure file is closed
            m_file.close();

            // open new file descriptor to replace existing file and allow writing
            IF_DEBUG_BUILD(qDebug() << "Opening new fd for saving, native url: " << m_nativeUrl;)
            const auto newFileDescriptor = openFileDescriptorFromAndroidContentUrl(m_nativeUrl, QStringLiteral("wt"));
            if (newFileDescriptor < 0) {
                emit fileError(tr("Unable to open file descriptor for saving the file."));
                return;
            }

            m_file.fileStream().openFromFileDescriptor(newFileDescriptor, ios_base::out | ios_base::trunc | ios_base::binary);
            m_file.write(flags);
        } else {
#endif
            // let libpasswordfile handle everything
            m_file.save(flags);
#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
        }
#endif
        emit fileSaved();
    } catch (const CryptoException &e) {
        emit fileError(tr("A crypto error occured when saving the file: ") + QString::fromLocal8Bit(e.what()), QStringLiteral("save"));
    } catch (const runtime_error &e) {
        emit fileError(tr("An internal error occured when saving the file: ") + QString::fromLocal8Bit(e.what()), QStringLiteral("save"));
    } catch (...) {
        emitIoError(tr("saving"));
    }
}

/*!
 * \brief Shows a native file dialog if supported; otherwise returns false.
 * \remarks If supported, this method will load/create the selected file (according to \a existing).
 */
bool Controller::showNativeFileDialog(bool existing)
{
#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
    if (!m_useNativeFileDialog) {
        return false;
    }
    return showAndroidFileDialog(existing);
#else
    Q_UNUSED(existing)
    return false;
#endif
}

void Controller::handleFileSelectionAccepted(const QString &filePath, bool existing)
{
    m_nativeUrl.clear();
    if (existing) {
        load(filePath);
    } else {
        create(filePath);
    }
}

#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
void Controller::handleFileSelectionAcceptedDescriptor(const QString &nativeUrl, const QString &fileName, int fileDescriptor, bool existing)
{
    try {
        m_file.setPath(fileName.toStdString());
        m_file.fileStream().openFromFileDescriptor(fileDescriptor, ios_base::in | ios_base::binary);
        m_file.opened();
    } catch (...) {
        emitIoError(tr("opening from native file descriptor"));
    }
    emit filePathChanged(m_filePath = m_fileName = fileName);
    handleFileSelectionAccepted(QString(), existing);
    m_nativeUrl = nativeUrl;
}
#endif

void Controller::handleFileSelectionCanceled()
{
    emit newNotification(tr("Canceled file selection"));
}

void Controller::handleEntriesRemoved(const QModelIndex &parentIndex, int first, int last)
{
    // handle deletion of root (currently the view doesn't allow this)
    const auto *const parentEntry = m_entryModel.entry(parentIndex);
    if (!parentEntry) {
        emit entryAboutToBeRemoved(m_entryModel.index(0, 0, QModelIndex()));
        setCurrentAccount(nullptr);
        return;
    }

    // assert arguments
    assert(parentEntry->type() == EntryType::Node);
    const auto &childEntries = static_cast<const NodeEntry *>(parentEntry)->children();
    assert(first >= 0 && static_cast<size_t>(first) < childEntries.size());
    assert(last >= 0 && static_cast<size_t>(last) < childEntries.size());

    // iterate from first to last of the deleted entries
    const auto *const currentAccount = this->currentAccount();
    for (; first <= last; ++first) {
        // inform view about deletion
        emit entryAboutToBeRemoved(m_entryModel.index(first, 0, parentIndex));

        // unset current account if it is under the deleted node
        if (!currentAccount) {
            continue;
        }
        const auto *const childEntry = childEntries[static_cast<size_t>(first)];
        switch (childEntry->type()) {
        case EntryType::Account:
            if (currentAccount == static_cast<const AccountEntry *>(childEntry)) {
                setCurrentAccount(nullptr);
            }
            break;
        case EntryType::Node:
            // FIXME: remove const_cast in passwordfile v4
            if (currentAccount->isIndirectChildOf(static_cast<NodeEntry *>(const_cast<Entry *>(childEntry)))) {
                setCurrentAccount(nullptr);
            }
            break;
        default:;
        }
    }
}

void Controller::handleRecentFilesChanged()
{
    m_settings.setValue(QStringLiteral("recententries"), m_recentFiles);
}

QStringList Controller::pasteEntries(const QModelIndex &destinationParentMaybeFromFilterModel, int row)
{
    // skip if no entries have been cut
    if (m_cutEntries.isEmpty()) {
        return QStringList();
    }

    // determine destinationParent and row in the source model
    QModelIndex destinationParent;
    if (destinationParentMaybeFromFilterModel.model() == &m_entryFilterModel) {
        if (row < 0) {
            row = m_entryFilterModel.rowCount(destinationParentMaybeFromFilterModel);
        }
        const auto destinationIndexInFilter = m_entryFilterModel.index(row, 0, destinationParentMaybeFromFilterModel);
        if (destinationIndexInFilter.isValid()) {
            const auto destinationIndex = m_entryFilterModel.mapToSource(destinationIndexInFilter);
            destinationParent = destinationIndex.parent();
            row = destinationIndex.row();
        } else {
            destinationParent = m_entryFilterModel.mapToSource(destinationParentMaybeFromFilterModel);
            row = -1;
        }
    } else {
        destinationParent = destinationParentMaybeFromFilterModel;
    }
    if (row < 0) {
        row = m_entryModel.rowCount(destinationParent);
    }

    // skip if destination is no node
    if (!m_entryModel.isNode(destinationParent)) {
        return QStringList();
    }

    // move the entries
    QStringList successfullyMovedEntries;
    successfullyMovedEntries.reserve(m_cutEntries.size());
    for (const QPersistentModelIndex &cutIndex : m_cutEntries) {
        if (m_entryModel.moveRows(cutIndex.parent(), cutIndex.row(), 1, destinationParent, row)) {
            successfullyMovedEntries << m_entryModel.data(m_entryModel.index(row, 0, destinationParent)).toString();
            ++row;
        }
    }

    // clear the cut entries
    m_cutEntries.clear();
    emit cutEntriesChanged(m_cutEntries);
    return successfullyMovedEntries;
}

bool Controller::copyToClipboard(const QString &text) const
{
#ifndef QT_NO_CLIPBOARD
    auto *const clipboard(QGuiApplication::clipboard());
    if (!clipboard) {
        return false;
    }
    clipboard->setText(text);
    return true;
#else
    return false;
#endif
}

void Controller::resetFileStatus()
{
    m_entryModel.reset();
    m_fieldModel.reset();
    setFileOpen(false);
}

void Controller::updateWindowTitle()
{
    if (m_fileOpen) {
        const QFileInfo file(m_filePath);
        emit windowTitleChanged(m_windowTitle = file.fileName() % QStringLiteral(" - ") % file.dir().path());
    } else {
        emit windowTitleChanged(tr("No file opened."));
    }
}

void Controller::setFileOpen(bool fileOpen)
{
    if (fileOpen != m_fileOpen) {
        emit fileOpenChanged(m_fileOpen = fileOpen);
    }
}

void Controller::emitIoError(const QString &when)
{
    try {
        const auto *const msg = catchIoFailure();
        emit fileError(tr("An IO error occured when %1 the file %2: ").arg(when, m_filePath) + QString::fromLocal8Bit(msg), QStringLiteral("load"));
    } catch (const exception &e) {
        emit fileError(tr("An unknown exception occured when %1 the file %2: ").arg(when, m_filePath) + QString::fromLocal8Bit(e.what()),
            QStringLiteral("load"));
    } catch (...) {
        emit fileError(tr("An unknown error occured when %1 the file %2.").arg(when, m_filePath), QStringLiteral("load"));
    }
}

void Controller::clearRecentFiles()
{
    if (m_recentFiles.isEmpty()) {
        return;
    }
    m_recentFiles.clear();
    emit recentFilesChanged(m_recentFiles);
}

void Controller::setUseNativeFileDialog(bool useNativeFileDialog)
{
    if (m_useNativeFileDialog == useNativeFileDialog) {
        return;
    }
    emit useNativeFileDialogChanged(m_useNativeFileDialog = useNativeFileDialog);
    m_settings.setValue(QStringLiteral("usenativefiledialog"), m_useNativeFileDialog);
}

void Controller::setEntryFilter(const QString &filter)
{
    const auto previousFilter(m_entryFilterModel.filterRegExp().pattern());
    if (filter == previousFilter) {
        return;
    }
    m_entryFilterModel.setFilterRegExp(filter);
    emit entryFilterChanged(filter);
    if (previousFilter.isEmpty() != filter.isEmpty()) {
        emit hasEntryFilterChanged(!filter.isEmpty());
    }
}

} // namespace QtGui
