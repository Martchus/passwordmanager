import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    property alias showPasswordsOnFocus: showPasswordsOnFocusSwitch.checked
    property var fieldsPage: undefined
    property var lastEntriesPage: undefined

    header: Kirigami.ApplicationHeader {
        backButtonEnabled: false
        minimumHeight: 0
        preferredHeight: Kirigami.Units.gridUnit * 2.3
        maximumHeight: Kirigami.Units.gridUnit * 3

        /*
        Controls.TextField {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            placeholderText: qsTr("Filter")
            width: Kirigami.Units.gridUnit * 8
        }
        */
    }
    globalDrawer: Kirigami.GlobalDrawer {
        id: leftMenu
        property bool showNoPasswordWarning: nativeInterface.fileOpen
                                             && !nativeInterface.passwordSet

        title: qsTr("Password Manager")
        titleIcon: "qrc://icons/hicolor/scalable/apps/passwordmanager-black.svg"
        visible: true
        resetMenuOnTriggered: false
        topContent: ColumnLayout {
            Layout.fillWidth: true

            Controls.MenuSeparator {
                padding: 0
                topPadding: 8
                bottomPadding: 0
                Layout.fillWidth: true
            }
            Controls.Label {
                id: fileNameLabel
                padding: 8
                wrapMode: Controls.Label.Wrap
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: 10
                font.pixelSize: 20
                Layout.fillWidth: true
                text: nativeInterface.fileOpen ? nativeInterface.fileName : qsTr(
                                                     "No file opened")
                MouseArea {
                    id: fileNameMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }
                Controls.ToolTip {
                    z: 1000
                    text: nativeInterface.filePath
                    visible: text ? fileNameMouseArea.containsMouse : false
                    delay: Qt.styleHints.mousePressAndHoldInterval
                    onAboutToShow: {
                        x = fileNameMouseArea.mouseX + 10
                        y = fileNameMouseArea.mouseY + 10
                    }
                }
            }
            RowLayout {
                visible: leftMenu.showNoPasswordWarning

                Item {
                    Layout.preferredWidth: 2
                }
                Kirigami.Icon {
                    source: "emblem-warning"
                    width: Kirigami.Units.iconSizes.small
                    height: Kirigami.Units.iconSizes.small
                }
                Controls.Label {
                    text: qsTr("No password set\nFile will be saved unencrypted!")
                    font.bold: true
                }
            }
            Item {
                visible: leftMenu.showNoPasswordWarning
                height: 4
            }
        }
        actions: [
            Kirigami.Action {
                text: qsTr("Create new file")
                iconName: "document-new"
                onTriggered: fileDialog.createNew()
                shortcut: StandardKey.New
            },
            Kirigami.Action {
                text: qsTr("Open existing file")
                iconName: "document-open"
                onTriggered: fileDialog.openExisting()
                shortcut: StandardKey.Open
            },
            Kirigami.Action {
                text: qsTr("Recently opened ...")
                iconName: "document-open-recent"
                children: createRecentlyOpenedActions(
                              nativeInterface.recentFiles)
                visible: nativeInterface.recentFiles.length > 0
                shortcut: "Ctrl+R"
            },
            Kirigami.Action {
                text: qsTr("Save modifications")
                enabled: nativeInterface.fileOpen
                iconName: "document-save"
                onTriggered: nativeInterface.save()
                shortcut: StandardKey.Save
            },
            Kirigami.Action {
                text: nativeInterface.passwordSet ? qsTr("Change password") : qsTr(
                                                        "Add password")
                enabled: nativeInterface.fileOpen
                iconName: "document-encrypt"
                onTriggered: enterPasswordDialog.askForNewPassword(
                                 "Change password for " + nativeInterface.filePath)
                shortcut: "Ctrl+P"
            },
            Kirigami.Action {
                text: "Close file"
                enabled: nativeInterface.fileOpen
                iconName: "document-close"
                onTriggered: nativeInterface.close()
                shortcut: StandardKey.Close
            }
        ]
        Controls.Switch {
            text: qsTr("Use native file dialog")
            checked: nativeInterface.useNativeFileDialog
            visible: nativeInterface.supportsNativeFileDialog
            onCheckedChanged: nativeInterface.useNativeFileDialog = checked
        }
        Controls.Switch {
            id: showPasswordsOnFocusSwitch
            text: qsTr("Show passwords on focus")
            checked: true
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }
    Component.onCompleted: nativeInterface.init()

    PasswordDialog {
        id: enterPasswordDialog
        onAboutToShow: leftMenu.close()
        onRejected: {
            if (!nativeInterface.fileOpen) {
                leftMenu.open()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: selectExisting ? qsTr("Select an existing file") : qsTr(
                                    "Select path for new file")
        onAccepted: {
            if (fileUrls.length < 1) {
                return
            }
            nativeInterface.handleFileSelectionAccepted(fileUrls[0],
                                                        this.selectExisting)
        }
        onRejected: nativeInterface.handleFileSelectionCanceled()

        function show() {
            if (nativeInterface.showNativeFileDialog(this.selectExisting)) {
                return
            }
            // fallback to the Qt Quick file dialog if a native implementation is not available
            this.open()
        }
        function openExisting() {
            this.selectExisting = true
            this.show()
        }
        function createNew() {
            this.selectExisting = false
            this.show()
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
            leftMenu.close()
        }
        onFileSaved: {
            showPassiveNotification(qsTr("%1 saved").arg(
                                        nativeInterface.fileName))
        }
        onNewNotification: {
            showPassiveNotification(message)
        }
        onCurrentAccountChanged: {
            // remove the fields page if the current account has been removed
            if (!nativeInterface.hasCurrentAccount) {
                pageStack.pop(lastEntriesPage)
            }
        }
        onEntryAboutToBeRemoved: {
            // remove all possibly open stack pages of the removed entry and its children
            for (var i = pageStack.depth - 1; i >= 0; --i) {
                var stackPage = pageStack.get(i)
                if (!stackPage) {
                    continue
                }
                if (stackPage.rootIndex === removedIndex) {
                    pageStack.pop(lastEntriesPage = pageStack.get(i - 1))
                    return
                }
            }
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
        id: clearRecentFilesActionComponent
        Kirigami.Action {
            text: qsTr("Clear recently opened files")
            iconName: "edit-clear"
            onTriggered: {
                nativeInterface.clearRecentFiles()
                leftMenu.resetMenu()
            }
        }
    }

    Component {
        id: entriesComponent
        EntriesPage {
            main: root
        }
    }

    Component {
        id: fieldsComponent
        FieldsPage {
            main: root
        }
    }

    Shortcut {
        sequence: "Ctrl+M"
        onActivated: leftMenu.visible = !leftMenu.visible
    }

    function clearStack() {
        pageStack.pop(lastEntriesPage = root.pageStack.initialPage,
                      Controls.StackView.Immediate)
    }

    function pushStackEntry(entryModel, rootIndex) {
        pageStack.push(lastEntriesPage = entriesComponent.createObject(root, {
                                                                           "entryModel": entryModel,
                                                                           "rootIndex": rootIndex
                                                                       }))
    }

    function pushAccountEdit() {
        // lazy-initialize fieldsPage
        if (!fieldsPage) {
            fieldsPage = fieldsComponent.createObject(root)
        }
        // remove fieldsPage if already shown to prevent warning
        if (pageStack.get(pageStack.depth - 1) === fieldsPage) {
            pageStack.pop(lastEntriesPage)
        }
        pageStack.push(fieldsPage)
    }

    function createFileActions(files) {
        return files.map(function (filePath) {
            return this.createObject(root, {
                                         "filePath": filePath
                                     })
        }, fileActionComponent)
    }

    function createRecentlyOpenedActions(files) {
        var actions = createFileActions(files)
        actions.push(clearRecentFilesActionComponent.createObject(root))
        return actions
    }
}
