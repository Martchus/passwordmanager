#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./passwordgeneratordialog.h"

#include <passwordfile/io/passwordfile.h>

#include <qtutilities/aboutdialog/aboutdialog.h>

#include <c++utilities/io/binaryreader.h>
#include <c++utilities/io/binarywriter.h>

#include <QMainWindow>
#include <QMap>

#include <memory>
#include <iostream>
#include <fstream>

QT_FORWARD_DECLARE_CLASS(QCloseEvent)
QT_FORWARD_DECLARE_CLASS(QTreeWidgetItem)
QT_FORWARD_DECLARE_CLASS(QUndoStack)
QT_FORWARD_DECLARE_CLASS(QUndoView)

namespace Io {
DECLARE_ENUM(EntryType, int)
DECLARE_ENUM(FieldType, int)
}

namespace MiscUtils {
class RecentMenuManager;
}

namespace QtGui {

class FieldModel;
class EntryModel;
class EntryFilterModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // file management
    bool openFile(const QString &path);
    void createFile(const QString &path, const QString &password);
    void createFile(const QString &path);

public Q_SLOTS:
    // file management
    bool createFile();
    void changePassword();
    bool saveFile();
    void exportFile();
    bool closeFile();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void closeEvent(QCloseEvent *event);
    void timerEvent(QTimerEvent *event);

private Q_SLOTS:
    // showing dialogs
    void showAboutDialog();
    void showPassowrdGeneratorDialog();
    void showOpenFileDialog();
    void showSaveFileDialog();
    void showUndoView();
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
    bool m_dontUpdateSelection;
    int m_clearClipboardTimer;
    MiscUtils::RecentMenuManager *m_recentMgr;
};

}

#endif // MAINWINDOW_H
