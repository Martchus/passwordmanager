#include "./controller.h"
#include "./android.h"

#include <passwordfile/io/cryptoexception.h>
#include <passwordfile/io/parsingexception.h>

#include <qtutilities/misc/compat.h>
#include <qtutilities/misc/dialogutils.h>
#include <qtutilities/resources/resources.h>

#include <c++utilities/io/nativefilestream.h>
#include <c++utilities/io/path.h>

#ifndef QT_NO_CLIPBOARD
#include <QClipboard>
#endif
#if defined(CPP_UTILITIES_DEBUG_BUILD) || (defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER))
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
using namespace CppUtilities;
using namespace QtUtilities;

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
    auto actualFilePath = makeStringView(filePath);
    if (filePath.startsWith(QLatin1String("file:"))) {
        actualFilePath = midRef(filePath, 5);
    }
    while (actualFilePath.startsWith(QLatin1String("//"))) {
        actualFilePath = actualFilePath.mid(1);
    }

    // assign full file path and file name
    m_filePath = actualFilePath.toString();
    m_file.setPath(m_filePath.toLocal8Bit().toStdString());
    const auto fileName = CppUtilities::fileName(m_file.path());
    m_fileName = QString::fromLocal8Bit(fileName.data(), static_cast<int>(fileName.size()));
    emit filePathChanged(m_filePath);

    // handle recent files
    if (m_filePath.isEmpty()) {
        return;
    }
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
    if (const auto error = QtUtilities::errorMessageForSettings(m_settings); !error.isEmpty()) {
        emit settingsError(error);
    }
}

void Controller::load()
{
    try {
        m_file.load();
        m_entryModel.setRootEntry(m_file.rootEntry());
        if (!m_entryModel.rootEntry()) {
            emit fileError(tr("An error occurred when opening the file: root element missing"), QStringLiteral("load"));
            return;
        }
        setFileOpen(true);
        updateWindowTitle();
    } catch (const CryptoException &e) {
        if ((m_file.saveOptions() & PasswordFileSaveFlags::Encryption) && m_password.isEmpty()) {
            emit passwordRequired(m_filePath);
        } else {
            // clear password since the password which has been provided likely wasn't correct
            clearPassword();
            emit fileError(tr("A crypto error occurred when opening the file: ") + QString::fromLocal8Bit(e.what()), QStringLiteral("load"));
        }
    } catch (...) {
        if ((m_file.saveOptions() & PasswordFileSaveFlags::Encryption) && m_password.isEmpty()) {
            emit passwordRequired(m_filePath);
        } else {
            // clear password since the password which has been provided might not have been correct (although there was no CryptoException)
            clearPassword();
            emitFileError(tr("loading"));
        }
    }
}

void Controller::create()
{
    try {
        m_file.create();
    } catch (...) {
        emitFileError(tr("creating"));
    }

    m_file.generateRootEntry();
    m_entryModel.setRootEntry(m_file.rootEntry());
    m_password.clear(); // avoid using the password of previously opened file
    setFileOpen(true);
    updateWindowTitle();
}

void Controller::close()
{
    try {
        m_file.close();
        resetFileStatus();
    } catch (...) {
        emitFileError(tr("closing"));
    }
}

void Controller::clear()
{
    try {
        m_file.close();
    } catch (...) {
        emitFileError(tr("closing"));
    }
    m_file.clear();
    resetFileStatus();
}

PasswordFileSaveFlags Controller::prepareSaving()
{
    auto flags = PasswordFileSaveFlags::Compression | PasswordFileSaveFlags::PasswordHashing | PasswordFileSaveFlags::AllowToCreateNewFile;
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
            qDebug() << "Opening new fd for saving, native url: " << m_nativeUrl;
            const auto newFileDescriptor = openFileDescriptorFromAndroidContentUrl(m_nativeUrl, QStringLiteral("wt"));
            if (newFileDescriptor < 0) {
                emit fileError(tr("Unable to open file descriptor for saving the file."), QStringLiteral("save"));
                return;
            }

            m_file.fileStream().open(newFileDescriptor, ios_base::out | ios_base::trunc | ios_base::binary);
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
        emit fileError(tr("A crypto error occurred when saving the file: ") + QString::fromLocal8Bit(e.what()), QStringLiteral("save"));
    } catch (...) {
        emitFileError(tr("saving"));
    }
}

/*!
 * \brief Shows a native file dialog if supported; otherwise returns false.
 * \remarks If supported, this method will load/create the selected file (according to \a existing).
 */
bool Controller::showNativeFileDialog(bool existing, bool createNew)
{
#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
    if (!m_useNativeFileDialog) {
        return false;
    }
    return showAndroidFileDialog(existing, createNew);
#else
    Q_UNUSED(existing)
    Q_UNUSED(createNew)
    return false;
#endif
}

void Controller::handleFileSelectionAccepted(const QString &filePath, const QString &nativeUrl, bool existing, bool createNew)
{
    m_nativeUrl = nativeUrl;

    // assign the "ordinary" file path if one has been passed; otherwise the caller is responsible for handling this
    const auto saveAs = !existing && !createNew;
    if (!filePath.isEmpty()) {
        // clear leftovers from possibly previously opened file unless we want to save the current file under a different location
        if (!saveAs) {
            m_file.clear();
        }
        setFilePath(filePath);
    }
    cout << "path is still " << m_file.path() << " (2)" << endl;

    if (!saveAs) {
        resetFileStatus();
    }

    if (existing) {
        load();
    } else if (createNew) {
        create();
    } else if (saveAs) {
        save();
    }
}

#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
void Controller::handleFileSelectionAcceptedDescriptor(
    const QString &nativeUrl, const QString &fileName, int fileDescriptor, bool existing, bool createNew)
{
    qDebug() << "Opening file descriptor for native url: " << nativeUrl;
    qDebug() << "(existing: " << existing << ", create new: " << createNew << ")";

    try {
        // clear leftovers from possibly previously opened file unless we want to save the current file under a different location
        if (existing || createNew) {
            m_file.clear();
        }
        m_file.setPath(fileName.toStdString());
        m_file.fileStream().open(fileDescriptor, ios_base::in | ios_base::binary);
        m_file.opened();
    } catch (...) {
        emitFileError(existing ? QStringLiteral("load") : (createNew ? QStringLiteral("create") : QStringLiteral("save")));
    }

    emit filePathChanged(m_filePath = m_fileName = fileName);
    handleFileSelectionAccepted(QString(), nativeUrl, existing, createNew);
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
            if (currentAccount->isIndirectChildOf(static_cast<const NodeEntry *>(childEntry))) {
                setCurrentAccount(nullptr);
            }
            break;
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

void Controller::emitFileError(const QString &when)
{
    try {
        throw;
    } catch (const std::ios_base::failure &failure) {
        emit fileError(
            tr("An IO error occurred when %1 the file %2: ").arg(when, m_filePath) + QString::fromLocal8Bit(failure.what()), QStringLiteral("load"));
    } catch (const runtime_error &e) {
        emit fileError(tr("An error occurred when %1 the file: ").arg(when) + QString::fromLocal8Bit(e.what()), QStringLiteral("save"));
    } catch (const exception &e) {
        emit fileError(tr("An unknown exception occurred when %1 the file %2: ").arg(when, m_filePath) + QString::fromLocal8Bit(e.what()),
            QStringLiteral("load"));
    } catch (...) {
        emit fileError(tr("An unknown error occurred when %1 the file %2.").arg(when, m_filePath), QStringLiteral("load"));
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    const auto previousFilter(m_entryFilterModel.filterRegularExpression().pattern());
#else
    const auto previousFilter(m_entryFilterModel.filterRegExp().pattern());
#endif
    if (filter == previousFilter) {
        return;
    }
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    m_entryFilterModel.setFilterRegularExpression(filter);
#else
    m_entryFilterModel.setFilterRegExp(filter);
#endif
    emit entryFilterChanged(filter);
    if (previousFilter.isEmpty() != filter.isEmpty()) {
        emit hasEntryFilterChanged(!filter.isEmpty());
    }
}

} // namespace QtGui
