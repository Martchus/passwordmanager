import QtQuick 2.7
import QtQuick.Templates 2.0 as T2
import QtQuick.Controls 2.1 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    property var fieldsPage: undefined
    property var lastEntriesPage: undefined

    globalDrawer: Kirigami.GlobalDrawer {
        id: leftMenu
        property bool showNoPasswordWarning: nativeInterface.fileOpen
                                             && !nativeInterface.passwordSet

        title: app.applicationName
        titleIcon: "qrc://icons/hicolor/scalable/apps/passwordmanager.svg"
        visible: true
        resetMenuOnTriggered: false
        topContent: ColumnLayout {
            Layout.fillWidth: true

            Item {
                Layout.preferredHeight: 4
            }
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: filterTextField.implicitHeight
                enabled: nativeInterface.fileOpen
                visible: !nativeInterface.filterAsDialog

                Controls.TextField {
                    id: filterTextField
                    anchors.fill: parent
                    placeholderText: qsTr("Filter")
                    onTextChanged: nativeInterface.entryFilter = text
                }
                Kirigami.Icon {
                    source: "edit-clear"
                    anchors.right: parent.right
                    anchors.rightMargin: 6
                    anchors.verticalCenter: parent.verticalCenter
                    width: Kirigami.Units.iconSizes.small
                    height: Kirigami.Units.iconSizes.small
                    visible: filterTextField.text.length !== 0
                    MouseArea {
                        anchors.fill: parent
                        onClicked: filterTextField.text = ""
                    }
                }
            }

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
                Controls.Label {
                    text: qsTr("No password set\nFile will be saved unencrypted!")
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
                id: recentlyOpenedAction
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
                text: qsTr("Save as")
                enabled: nativeInterface.fileOpen
                iconName: "document-save-as"
                onTriggered: fileDialog.saveAs()
                shortcut: StandardKey.SaveAs
            },
            Kirigami.Action {
                text: nativeInterface.passwordSet ? qsTr("Change password") : qsTr(
                                                        "Add password")
                enabled: nativeInterface.fileOpen
                iconName: "document-encrypt"
                onTriggered: enterPasswordDialog.askForNewPassword(
                                 qsTr("Change password for %1").arg(
                                     nativeInterface.filePath))
                shortcut: "Ctrl+P"
            },
            Kirigami.Action {
                text: qsTr("Details")
                enabled: nativeInterface.fileOpen
                iconName: "document-properties"
                onTriggered: {
                    leftMenu.resetMenu()
                    fileSummaryDialog.show()
                }
                shortcut: "Ctrl+I"
            },
            Kirigami.Action {
                text: nativeInterface.entryFilter.length === 0 ? qsTr("Search") : qsTr(
                                                                     "Adjust search")
                enabled: nativeInterface.fileOpen
                visible: nativeInterface.filterAsDialog
                iconName: "search"
                onTriggered: {
                    leftMenu.resetMenu()
                    filterDialog.open()
                }
                shortcut: "Ctrl+F"
            },
            Kirigami.Action {
                text: qsTr("Clear search")
                enabled: nativeInterface.fileOpen
                visible: nativeInterface.filterAsDialog
                         && nativeInterface.entryFilter.length > 0
                iconName: "edit-clear"
                onTriggered: {
                    leftMenu.resetMenu()
                    nativeInterface.entryFilter = ""
                }
                shortcut: "Ctrl+Shift+F"
            },
            Kirigami.Action {
                text: qsTr("Undo \"%1\"").arg(nativeInterface.undoText)
                visible: nativeInterface.undoText.length !== 0
                         && nativeInterface.entryFilter.length === 0
                enabled: visible
                iconName: "edit-undo"
                shortcut: StandardKey.Undo
                onTriggered: nativeInterface.undo()
            },
            Kirigami.Action {
                text: qsTr("Redo \"%1\"").arg(nativeInterface.redoText)
                visible: nativeInterface.redoText.length !== 0
                         && nativeInterface.entryFilter.length === 0
                enabled: visible
                iconName: "edit-redo"
                shortcut: StandardKey.Redo
                onTriggered: nativeInterface.redo()
            },
            Kirigami.Action {
                text: qsTr("Close file")
                enabled: nativeInterface.fileOpen
                iconName: "document-close"
                shortcut: StandardKey.Close
                onTriggered: nativeInterface.close()
            }
        ]
        onBannerClicked: {
            leftMenu.resetMenu()
            aboutDialog.open()
        }

        Controls.Switch {
            text: qsTr("Use native file dialog")
            checked: nativeInterface.useNativeFileDialog
            visible: nativeInterface.supportsNativeFileDialog
            onCheckedChanged: nativeInterface.useNativeFileDialog = checked
        }
    }
    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }
    Component.onCompleted: nativeInterface.init()

    AboutDialog {
        id: aboutDialog
    }

    PasswordDialog {
        id: enterPasswordDialog
        onAboutToShow: leftMenu.close()
        onRejected: {
            if (!nativeInterface.fileOpen) {
                leftMenu.open()
            }
        }
    }

    BasicDialog {
        id: fileSummaryDialog
        standardButtons: Controls.Dialog.Ok
        title: qsTr("File details")

        Controls.Label {
            id: fileSummaryLabel
            text: "No file summary available"
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            width: fileSummaryDialog.availableWidth
        }

        function show() {
            fileSummaryLabel.text = nativeInterface.computeFileSummary()
            this.open()
        }
    }

    FileDialog {
        id: fileDialog
    }

    BasicDialog {
        id: filterDialog
        title: qsTr("Search for categories and accounts")
        onAccepted: nativeInterface.entryFilter = filterDialogTextField.text
        onReset: {
            nativeInterface.entryFilter = ""
            filterDialog.close()
        }
        onVisibleChanged: {
            if (visible) {
                filterDialogTextField.forceActiveFocus()
            }
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Apply search term")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
                enabled: filterDialogTextField.text.length > 0
            }
            Controls.Button {
                text: qsTr("Clear search")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.ResetRole
                enabled: nativeInterface.entryFilter
            }
            Controls.Button {
                text: qsTr("Quit dialog")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
        }

        ColumnLayout {
            Controls.TextField {
                id: filterDialogTextField
                Layout.preferredWidth: filterDialog.availableWidth
                Keys.onPressed: filterDialog.acceptOnReturn(event)
            }
        }
    }

    Connections {
        target: nativeInterface
        onEntryFilterChanged: {
            if (filterTextField.text !== newFilter) {
                filterTextField.text = newFilter
            }
        }
        onFileError: {
            var retryMethod = null
            if (retryAction === "load" || retryAction === "save") {
                retryMethod = retryAction
            }
            if (retryMethod) {
                showPassiveNotification(errorMessage)
            } else {
                showPassiveNotification(errorMessage, 2500, qsTr("Retry"),
                                        function () {
                                            nativeInterface[retryMethod]()
                                        })
            }
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
            initStack()
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
            // get the filter entry index
            if (nativeInterface.hasEntryFilter) {
                removedIndex = nativeInterface.filterEntryIndex(removedIndex)
            }

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
        onHasEntryFilterChanged: {
            if (nativeInterface.fileOpen) {
                pageStack.clear()
                initStack()
            }
        }
    }

    Component {
        id: fileActionComponent
        Kirigami.Action {
            property string filePath
            text: filePath.substring(filePath.lastIndexOf('/') + 1)
            onTriggered: {
                nativeInterface.clear()
                nativeInterface.filePath = filePath
                nativeInterface.load()
            }
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

    function initStack() {
        var entryModel = nativeInterface.hasEntryFilter ? nativeInterface.entryFilterModel : nativeInterface.entryModel
        var rootIndex = entryModel.index(0, 0)
        pushStackEntry(entryModel, rootIndex)
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
