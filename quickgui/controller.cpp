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
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QSettings>
#include <QStringBuilder>

#include <stdexcept>

using namespace std;
using namespace Io;
using namespace IoUtilities;
using namespace Dialogs;

namespace QtGui {

Controller::Controller(QSettings &settings, const QString &filePath, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_fileOpen(false)
    , m_fileModified(false)
    , m_useNativeFileDialog(false)
{
    m_entryFilterModel.setSourceModel(&m_entryModel);

    // share settings with main window
    m_settings.beginGroup(QStringLiteral("mainwindow"));
    m_recentFiles = m_settings.value(QStringLiteral("recententries")).toStringList();
    m_useNativeFileDialog = m_settings.value(QStringLiteral("usenativefiledialog"), m_useNativeFileDialog).toBool();

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

    // handle recent files
#ifdef Q_OS_ANDROID
    if (m_useNativeFileDialog) {
        return; // native file dialog under Android makes it impossible to store URIs persistently
    }
#endif
    auto index = m_recentFiles.indexOf(m_filePath);
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
    m_settings.setValue(QStringLiteral("recententries"), m_recentFiles);
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
        setFileOpen(true);
        updateWindowTitle();
    } catch (const CryptoException &e) {
        if (m_file.isEncryptionUsed() && m_password.isEmpty()) {
            emit passwordRequired(m_filePath);
        } else {
            emit fileError(tr("A crypto error occured when opening the file: ") + QString::fromLocal8Bit(e.what()));
        }
    } catch (const runtime_error &e) {
        emit fileError(tr("A parsing error occured when opening the file: ") + QString::fromLocal8Bit(e.what()));
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
        if (!m_file.isOpen()) {
            m_file.create();
        }
        m_file.generateRootEntry();
        m_entryModel.setRootEntry(m_file.rootEntry());
        setFileOpen(true);
        updateWindowTitle();
    } catch (...) {
        emitIoError(tr("creating"));
    }
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

void Controller::save()
{
    try {
        if (!m_password.isEmpty()) {
            const auto passwordUtf8(m_password.toUtf8());
            m_file.setPassword(string(passwordUtf8.data(), static_cast<size_t>(passwordUtf8.size())));
        } else {
            m_file.clearPassword();
        }
        m_file.save(!m_password.isEmpty());
        emit fileSaved();
    } catch (const CryptoException &e) {
        emit fileError(tr("A crypto error occured when saving the file: ") + QString::fromLocal8Bit(e.what()));
    } catch (const runtime_error &e) {
        emit fileError(tr("An internal error occured when saving the file: ") + QString::fromLocal8Bit(e.what()));
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
    if (existing) {
        load(filePath);
    } else {
        create(filePath);
    }
}

#if defined(Q_OS_ANDROID) && defined(CPP_UTILITIES_USE_NATIVE_FILE_BUFFER)
void Controller::handleFileSelectionAcceptedDescriptor(const QString &filePath, int fileDescriptor, bool existing)
{
    try {
        m_file.setPath(filePath.toStdString());
        m_file.fileStream().openFromFileDescriptor(fileDescriptor, ios_base::in | ios_base::binary);
        m_file.fileStream().seekg(0);
        emitIoError("seeked to begin");
        char buf[4];
        m_file.fileStream().read(buf, 4);
        emitIoError("first 4 byte: " + QString::fromLocal8Bit(buf, 4));
        m_file.opened();
    } catch (...) {
        emitIoError(tr("opening from native file descriptor"));
    }
    handleFileSelectionAccepted(filePath, existing);
}
#endif

void Controller::handleFileSelectionCanceled()
{
    emit newNotification(tr("Canceled file selection"));
}

QStringList Controller::pasteEntries(const QModelIndex &destinationParent, int row)
{
    if (m_cutEntries.isEmpty() || !m_entryModel.isNode(destinationParent)) {
        return QStringList();
    }

    if (row < 0) {
        row = m_entryModel.rowCount(destinationParent);
    }

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
    auto *clipboard(QGuiApplication::clipboard());
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
    setFileOpen(false);
    m_entryModel.reset();
    m_fieldModel.reset();
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
    const auto *const msg = catchIoFailure();
    emit fileError(tr("An IO error occured when %1 the file %2: ").arg(when, m_filePath) + QString::fromLocal8Bit(msg));
}

} // namespace QtGui
