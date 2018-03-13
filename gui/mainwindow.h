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

namespace Io {
DECLARE_ENUM_CLASS(EntryType, int);
DECLARE_ENUM_CLASS(FieldType, int);
} // namespace Io

namespace MiscUtils {
class RecentMenuManager;
}

namespace Dialogs {
class AboutDialog;
class SettingsDialog;
class QtSettings;
} // namespace Dialogs

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
    explicit MainWindow(QSettings &settings, Dialogs::QtSettings *qtSettings = nullptr, QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    // file management
    bool openFile(const QString &path);
    bool openFile(const QString &path, bool readOnly);
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

private slots:
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
    void showTreeViewContextMenu();
    void showTableViewContextMenu();
    // other
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

    std::unique_ptr<Ui::MainWindow> m_ui;
    Io::PasswordFile m_file;
    FieldModel *m_fieldModel;
    EntryModel *m_entryModel;
    EntryFilterModel *m_entryFilterModel;
    QUndoStack *m_undoStack;
    QUndoView *m_undoView;
    bool m_somethingChanged;
    bool m_readOnly;
    bool m_dontUpdateSelection;
    int m_clearClipboardTimer;
    MiscUtils::RecentMenuManager *m_recentMgr;
    Dialogs::AboutDialog *m_aboutDlg;
    QSettings &m_settings;
    Dialogs::QtSettings *m_qtSettings;
    Dialogs::SettingsDialog *m_settingsDlg;
};

/*!
 * \brief Opens a file with the specified \a path and updates all widgets to show its contents.
 * \returns Returns true on success; otherwise false
 */
inline bool MainWindow::openFile(const QString &path)
{
    return openFile(path, false);
}

} // namespace QtGui

#endif // MAINWINDOW_H
