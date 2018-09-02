import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    property alias showPasswordsOnFocus: showPasswordsOnFocusSwitch.checked

    header: Kirigami.ApplicationHeader {
    }
    globalDrawer: Kirigami.GlobalDrawer {
        id: leftMenu
        title: qsTr("Password Manager")
        titleIcon: "qrc://icons/hicolor/scalable/apps/passwordmanager.svg"
        visible: !nativeInterface.fileOpen
        topContent: ColumnLayout {
            Layout.fillWidth: true

            Controls.MenuSeparator {
                padding: 0
                topPadding: 8
                bottomPadding: 0
                Layout.fillWidth: true
            }
            Controls.Label {
                padding: 8
                wrapMode: Controls.Label.Wrap
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: 10
                font.pixelSize: 20
                Layout.fillWidth: true
                text: nativeInterface.fileOpen ? nativeInterface.fileName : qsTr(
                                                     "No file opened")
            }
        }
        actions: [
            Kirigami.Action {
                text: qsTr("Create new file")
                iconName: "document-new"
                onTriggered: fileDialog.createNew()
            },
            Kirigami.Action {
                text: qsTr("Open existing file")
                iconName: "document-open"
                onTriggered: fileDialog.openExisting()
            },
            Kirigami.Action {
                text: qsTr("Recently opened ...")
                iconName: "document-open-recent"
                children: createFileActions(nativeInterface.recentFiles)
            },
            Kirigami.Action {
                text: "Save modifications"
                enabled: nativeInterface.fileOpen
                iconName: "document-save"
                onTriggered: nativeInterface.save()
            },
            Kirigami.Action {
                text: "Change password"
                enabled: nativeInterface.fileOpen
                iconName: "dialog-password"
                onTriggered: enterPasswordDialog.askForNewPassword(
                                 "Change password for " + nativeInterface.filePath)
            },
            Kirigami.Action {
                text: "Close file"
                enabled: nativeInterface.fileOpen
                iconName: "document-close"
                onTriggered: nativeInterface.close()
            }
        ]
        Controls.Switch {
            id: showPasswordsOnFocusSwitch
            text: qsTr("Show passwords on focus")
            checked: true
        }

        Controls.Label {
            wrapMode: Controls.Label.Wrap
            Layout.fillWidth: true
            text: nativeInterface.fileOpen ? nativeInterface.filePath : qsTr(
                                                 "No file opened")
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }
    Component.onCompleted: nativeInterface.init()

    PasswordDialog {
        id: enterPasswordDialog
    }

    FileDialog {
        id: fileDialog
        title: selectExisting ? qsTr("Select an existing file") : qsTr(
                                    "Select path for new file")
        onAccepted: {
            if (fileUrls.length < 1) {
                return
            }
            if (selectExisting) {
                nativeInterface.load(fileUrls[0])
            } else {
                nativeInterface.create(fileUrls[0])
            }
        }
        onRejected: {
            showPassiveNotification(qsTr("Canceled file selection"))
        }

        function openExisting() {
            this.selectExisting = true
            this.open()
        }
        function createNew() {
            this.selectExisting = false
            this.open()
        }
    }

    Connections {
        target: nativeInterface
        onFileError: {
            showPassiveNotification(errorMessage)
        }
        onPasswordRequired: {
            enterPasswordDialog.askForExistingPassword(
                        qsTr("Password required to open %1").arg(
                            nativeInterface.filePath))
            leftMenu.resetMenu()
        }
        onFileOpenChanged: {
            clearStack()
            if (!nativeInterface.fileOpen) {
                showPassiveNotification(qsTr("%1 closed").arg(
                                            nativeInterface.fileName))
                return
            }
            var entryModel = nativeInterface.entryModel
            var rootIndex = entryModel.index(0, 0)
            pushStackEntry(entryModel, rootIndex)
            showPassiveNotification(qsTr("%1 opened").arg(
                                        nativeInterface.fileName))
        }
        onFileSaved: {
            showPassiveNotification(qsTr("%1 saved").arg(
                                        nativeInterface.fileName))
        }
    }

    Component {
        id: fileActionComponent
        Kirigami.Action {
            property string filePath
            text: filePath.substring(filePath.lastIndexOf('/') + 1)
            onTriggered: nativeInterface.load(filePath)
        }
    }

    Component {
        id: entriesComponent
        EntriesPage {
            main: root
        }
    }

    function clearStack() {
        pageStack.pop(root.pageStack.initialPage, Controls.StackView.Immediate)
    }

    function pushStackEntry(entryModel, rootIndex) {
        pageStack.push(entriesComponent.createObject(root, {
                                                         entryModel: entryModel,
                                                         rootIndex: rootIndex,
                                                         title: entryModel.data(
                                                                    rootIndex)
                                                     }))
    }

    function createFileActions(files) {
        return files.map(function (filePath) {
            return this.createObject(root, {
                                         filePath: filePath
                                     })
        }, fileActionComponent)
    }
}
