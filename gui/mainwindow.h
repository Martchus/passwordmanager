#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./passwordgeneratordialog.h"

#include <passwordfile/io/passwordfile.h>

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <QMainWindow>
#include <QMap>

#include <memory>

QT_FORWARD_DECLARE_CLASS(QCloseEvent)
QT_FORWARD_DECLARE_CLASS(QTreeWidgetItem)
QT_FORWARD_DECLARE_CLASS(QUndoStack)
QT_FORWARD_DECLARE_CLASS(QUndoView)
QT_FORWARD_DECLARE_CLASS(QSettings)

#define PASSWORD_MANAGER_ENUM_CLASS enum class
namespace Io {
PASSWORD_MANAGER_ENUM_CLASS EntryType : int;
PASSWORD_MANAGER_ENUM_CLASS FieldType : int;
} // namespace Io
#undef PASSWORD_MANAGER_ENUM_CLASS

namespace QtUtilities {
class RecentMenuManager;
class AboutDialog;
class SettingsDialog;
class QtSettings;
} // namespace QtUtilities

namespace QtGui {

class FieldModel;
class EntryModel;
class EntryFilterModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QSettings &settings, QtUtilities::QtSettings *qtSettings = nullptr, QWidget *parent = nullptr);
    ~MainWindow() override;

public Q_SLOTS:
    // file management
    bool openFile(const QString &path);
    bool openFile(const QString &path, Io::PasswordFileOpenFlags openFlags);
    void createFile(const QString &path, const QString &password);
    void createFile(const QString &path);
    bool createFile();
    void changePassword();
    bool saveFile();
    void exportFile();
    bool closeFile();
    // show dialogs
    void showOpenFileDialog();
    void showSaveFileDialog();
    void showSettingsDialog();
    void showAboutDialog();
    void showPassowrdGeneratorDialog();
    void showUndoView();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private Q_SLOTS:
    // file management
    bool showFile();
    // account/categories management
    void addAccount();
    void addCategory();
    void addEntry(Io::EntryType type);
    void removeEntry();
    void applyFilter(const QString &filterText);
    // row management
    void accountSelected(const QModelIndex &selected, const QModelIndex &);
    void insertRow();
    void removeRows();
    void markAsPasswordField();
    void markAsNormalField();
    void setFieldType(Io::FieldType fieldType);
    void setPasswordVisibility(QAction *selectedAction);
    QString selectedFieldsString() const;
    void insertFields(const QString &fieldsString);
    void copyFieldsForXMilliSeconds(int x = 5000);
    void copyFields();
    void insertFieldsFromClipboard();
    // showing context menus
    void showTreeViewContextMenu(const QPoint &pos);
    void showTableViewContextMenu(const QPoint &pos);
    // other
    void showFileDetails();
    void showContainingDirectory();
    void clearClipboard();
    void setSomethingChanged();
    void setSomethingChanged(bool somethingChanged);

private:
    // showing conditional messages/prompts
    bool askForCreatingFile();
    bool showNoFileOpened();
    bool showNoAccount();
    // other
    void updateUiStatus();
    void updateWindowTitle();
    void applyDefaultExpanding(const QModelIndex &parent);
    Io::PasswordFileSaveFlags saveOptions() const;

    std::unique_ptr<Ui::MainWindow> m_ui;
    Io::PasswordFile m_file;
    FieldModel *m_fieldModel;
    EntryModel *m_entryModel;
    EntryFilterModel *m_entryFilterModel;
    QUndoStack *m_undoStack;
    QUndoView *m_undoView;
    bool m_somethingChanged;
    Io::PasswordFileOpenFlags m_openFlags;
    bool m_dontUpdateSelection;
    int m_clearClipboardTimer;
    QtUtilities::RecentMenuManager *m_recentMgr;
    QtUtilities::AboutDialog *m_aboutDlg;
    QSettings &m_settings;
    QtUtilities::QtSettings *m_qtSettings;
    QtUtilities::SettingsDialog *m_settingsDlg;
};

/*!
 * \brief Opens a file with the specified \a path and updates all widgets to show its contents.
 * \returns Returns true on success; otherwise false
 */
inline bool MainWindow::openFile(const QString &path)
{
    return openFile(path, Io::PasswordFileOpenFlags::Default);
}

} // namespace QtGui

#endif // MAINWINDOW_H
