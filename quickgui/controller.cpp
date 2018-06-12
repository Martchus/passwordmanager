#include "./controller.h"

#include <passwordfile/io/cryptoexception.h>
#include <passwordfile/io/parsingexception.h>

#include <qtutilities/misc/dialogutils.h>

#include <c++utilities/io/catchiofailure.h>

#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QStringBuilder>

#include <stdexcept>

using namespace std;
using namespace Io;
using namespace IoUtilities;
using namespace Dialogs;

namespace QtGui {

Controller::Controller(const QString &filePath, QObject *parent)
    : QObject(parent)
    , m_fileOpen(false)
    , m_fileModified(false)
{
    setFilePath(filePath);
    m_entryFilterModel.setSourceModel(&m_entryModel);
}

void Controller::setFilePath(const QString &filePath)
{
    if (m_filePath == filePath) {
        return;
    }
    m_file.clear();
    m_file.setPath(filePath.toLocal8Bit().data());
    emit filePathChanged(m_filePath = filePath);
}

void Controller::setPassword(const QString &password)
{
    if (m_password == password) {
        return;
    }
    m_file.setPassword(password.toUtf8().data());
    emit passwordChanged(m_password = password);
}

void Controller::load()
{
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

void Controller::create()
{
    resetFileStatus();
    try {
        m_file.create();
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

bool Controller::pasteEntries(const QModelIndex &destinationParent, int row)
{
    if (m_cutEntries.isEmpty() || !m_entryModel.isNode(destinationParent)) {
        return false;
    }

    if (row < 0) {
        row = m_entryModel.rowCount(destinationParent);
    }
    bool result = true;
    for (const QPersistentModelIndex &cutIndex : m_cutEntries) {
        if (m_entryModel.moveRows(cutIndex.parent(), cutIndex.row(), 1, destinationParent, row)) {
            ++row;
        } else {
            result = false;
        }
    }

    // clear the cut entries
    m_cutEntries.clear();
    emit cutEntriesChanged(m_cutEntries);
    return true;
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
    emit fileError(tr("An IO error occured when %1 the file: ").arg(when) + QString::fromLocal8Bit(msg));
}

} // namespace QtGui
