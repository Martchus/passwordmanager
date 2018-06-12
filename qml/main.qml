import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    function clearStack() {
        pageStack.pop(root.pageStack.initialPage, Controls.StackView.Immediate)
    }

    function pushStackEntry(entryModel, rootIndex) {
        var title = entryModel.data(rootIndex)
        var entriesComponent = Qt.createComponent("EntriesPage.qml")
        if (entriesComponent.status !== Component.Ready) {
            var errorMessage = "Unable to load EntriesPage.qml: " + entriesComponent.errorString()
            showPassiveNotification(errorMessage)
            console.error(errorMessage)
            return
        }
        var entriesPage = entriesComponent.createObject(root, {
                                                            entryModel: entryModel,
                                                            rootIndex: rootIndex,
                                                            title: title
                                                        })
        pageStack.push(entriesPage)
    }

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
            nativeInterface.filePath = fileUrls[0]
            if (selectExisting) {
                nativeInterface.load()
            } else {
                nativeInterface.create()
            }
        }
        onRejected: {
            showPassiveNotification("Canceled file selection")
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
                        qsTr("Password required to open ") + filePath)
            leftMenu.resetMenu()
        }
        onFileOpenChanged: {
            clearStack()
            if (nativeInterface.fileOpen) {
                var entryModel = nativeInterface.entryModel
                var rootIndex = entryModel.index(0, 0)
                pushStackEntry(entryModel, rootIndex)
                showPassiveNotification(qsTr("File opened"))
            } else {
                showPassiveNotification(qsTr("File closed"))
            }
        }
        onFileSaved: {
            showPassiveNotification(qsTr("File saved"))
        }
    }

    header: Kirigami.ApplicationHeader {
    }
    globalDrawer: Kirigami.GlobalDrawer {
        id: leftMenu
        title: qsTr("Password manager")
        titleIcon: "passwordmanager"
        bannerImageSource: "banner.png"
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

        Controls.Label {
            wrapMode: Controls.Label.Wrap
            Layout.fillWidth: true
            text: {
                if (nativeInterface.fileOpen) {
                    return nativeInterface.filePath
                } else {
                    return qsTr("no file opened")
                }
            }
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    Component.onCompleted: {
        // load file if one has been specified via CLI argument
        if (nativeInterface.filePath.length) {
            nativeInterface.load()
        }
    }
}
