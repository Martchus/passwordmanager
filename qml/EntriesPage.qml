import QtQuick 2.4
import QtQuick.Layouts 1.2
import QtQml.Models 2.2
import QtQuick.Controls 2.4 as Controls
import org.kde.kirigami 2.5 as Kirigami

Kirigami.ScrollablePage {
    id: page
    property var main: undefined
    property alias entryModel: delegateModel.model
    property alias rootIndex: delegateModel.rootIndex

    Layout.fillWidth: true
    title: {
        var currentEntryName = entryModel.data(rootIndex)
        return currentEntryName ? currentEntryName : ""
    }
    actions {
        main: Kirigami.Action {
            iconName: "list-add"
            text: qsTr("Add account")
            visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: insertEntry("Account")
            shortcut: "Ctrl+A"
        }
        left: Kirigami.Action {
            iconName: "edit-paste"
            text: qsTr("Paste account")
            visible: !nativeInterface.hasEntryFilter
            enabled: nativeInterface.canPaste && !nativeInterface.hasEntryFilter
            onTriggered: {
                var pastedEntries = nativeInterface.pasteEntries(rootIndex)
                if (pastedEntries.length < 1) {
                    showPassiveNotification(
                                qsTr("Unable to paste the entries here"))
                    return
                }
                showPassiveNotification(
                            qsTr("Pasted ") + pastedEntries.join(", "))
            }
            shortcut: StandardKey.Paste
        }
        right: Kirigami.Action {
            iconName: "folder-add"
            text: qsTr("Add category")
            visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: insertEntry("Node")
            shortcut: "Ctrl+Shift+A"
        }
    }
    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

    // dialog to confirm deletion of an entry
    BasicDialog {
        id: confirmDeletionDialog

        property string entryDesc: "?"
        property int entryIndex: -1

        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel
        title: qsTr("Delete %1?").arg(entryDesc)
        onAccepted: entryModel.removeRows(this.entryIndex, 1, rootIndex)

        ColumnLayout {
            Controls.Label {
                text: " "
            }
        }

        function confirmDeletion(entryName, entryIndex) {
            // skip if undo is supported
            if (nativeInterface.undoStack) {
                entryModel.removeRows(entryIndex, 1, rootIndex)
                return
            }

            var isNode = entryModel.isNode(entryModel.index(entryIndex, 0,
                                                            rootIndex))
            var entryType = isNode ? qsTr("category ") : qsTr("account ")

            this.entryIndex = entryIndex
            this.entryDesc = entryType + entryName
            this.open()
        }
    }

    // dialog to rename an entry
    BasicDialog {
        id: renameDialog
        property string entryDesc: "?"
        property int entryIndex: -1
        property alias newEntryName: entryNameTextField.text
        property bool entryNew: false

        standardButtons: newEntryName.length
                         > 0 ? Controls.Dialog.Ok | Controls.Dialog.Cancel : Controls.Dialog.Cancel
        title: (entryNew ? qsTr("Name for new ") : qsTr("Rename ")) + entryDesc
        onAccepted: {
            entryModel.setData(entryModel.index(this.entryIndex, 0, rootIndex),
                               newEntryName)
        }
        onRejected: {
            if (this.entryNew) {
                entryModel.removeRows(this.entryIndex, 1, rootIndex)
            }
        }

        ColumnLayout {
            Controls.TextField {
                id: entryNameTextField
                Layout.preferredWidth: renameDialog.availableWidth
                placeholderText: qsTr("enter new name here")
                Keys.onPressed: renameDialog.acceptOnReturn(event)
            }
        }

        function renameEntry(entryName, entryIndex) {
            var isNode = entryModel.isNode(entryModel.index(entryIndex, 0,
                                                            rootIndex))
            var entryType = isNode ? qsTr("category ") : qsTr("account ")

            this.entryIndex = entryIndex
            this.entryNew = entryName === null
            if (this.entryNew) {
                this.entryDesc = entryType
                this.newEntryName = ""
            } else {
                this.entryDesc = entryType + entryName
                this.newEntryName = entryName
            }
            entryNameTextField.forceActiveFocus()
            this.open()
        }
    }

    // component representing an entry
    Component {
        id: listDelegateComponent

        Kirigami.SwipeListItem {
            id: listItem
            contentItem: RowLayout {
                Kirigami.ListItemDragHandle {
                    listItem: listItem
                    listView: entriesListView
                    enabled: !nativeInterface.hasEntryFilter
                    // FIXME: not sure why newIndex + 1 is required to be able to move a row at the end
                    onMoveRequested: entryModel.moveRows(
                                         rootIndex, oldIndex, 1, rootIndex,
                                         oldIndex < newIndex ? newIndex + 1 : newIndex)
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    RowLayout {
                        anchors.fill: parent
                        Kirigami.Icon {
                            width: Kirigami.Units.iconSizes.smallMedium
                            height: Kirigami.Units.iconSizes.smallMedium
                            Layout.fillHeight: true
                            source: delegateModel.isNode(
                                        index) ? "folder-symbolic" : "story-editor"
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: model.name
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: {
                            if (mouse.button === Qt.RightButton) {
                                entryContextMenu.popup()
                                return
                            }
                            delegateModel.handleEntryClicked(index, model.name)
                        }
                        onPressAndHold: entryContextMenu.popup()
                    }
                    Controls.Menu {
                        id: entryContextMenu
                        Controls.MenuItem {
                            icon.name: "edit-cut"
                            text: qsTr("Cut")
                            enabled: !nativeInterface.hasEntryFilter
                            onTriggered: {
                                nativeInterface.cutEntry(
                                            entryModel.index(index, 0,
                                                             rootIndex))
                                showPassiveNotification(text + " " + model.name)
                            }
                        }
                        Controls.MenuItem {
                            icon.name: "edit-delete"
                            text: qsTr("Delete")
                            enabled: !nativeInterface.hasEntryFilter
                            onTriggered: confirmDeletionDialog.confirmDeletion(
                                             model.name, index)
                        }
                        Controls.MenuItem {
                            icon.name: "edit-rename"
                            text: qsTr("Rename")
                            enabled: !nativeInterface.hasEntryFilter
                            onTriggered: renameDialog.renameEntry(model.name,
                                                                  index)
                        }
                    }
                }
            }
            actions: [
                Kirigami.Action {
                    iconName: "edit-cut"
                    text: qsTr("Cut")
                    enabled: !nativeInterface.hasEntryFilter
                    onTriggered: {
                        nativeInterface.cutEntry(entryModel.index(index, 0,
                                                                  rootIndex))
                        showPassiveNotification(text + " " + model.name)
                    }
                    shortcut: StandardKey.Cut
                },
                Kirigami.Action {
                    iconName: "edit-delete"
                    text: qsTr("Delete")
                    enabled: !nativeInterface.hasEntryFilter
                    onTriggered: confirmDeletionDialog.confirmDeletion(
                                     model.name, index)
                    shortcut: StandardKey.Delete
                },
                Kirigami.Action {
                    iconName: "edit-rename"
                    text: qsTr("Rename")
                    enabled: !nativeInterface.hasEntryFilter
                    onTriggered: renameDialog.renameEntry(model.name, index)
                    shortcut: "F2"
                }
            ]
        }
    }

    // list view to display one hierarchy level of entry model
    ListView {
        id: entriesListView
        anchors.fill: parent
        moveDisplaced: Transition {
            YAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        model: DelegateModel {
            id: delegateModel

            delegate: Kirigami.DelegateRecycler {
                width: parent ? parent.width : implicitWidth
                sourceComponent: listDelegateComponent
            }

            function isNode(rowNumber) {
                return entryModel.isNode(entryModel.index(rowNumber, 0,
                                                          rootIndex))
            }

            function handleEntryClicked(rowNumber, entryName) {
                var modelIndex = entryModel.index(rowNumber, 0, rootIndex)
                if (entryModel.isNode(modelIndex)) {
                    root.pushStackEntry(entryModel, modelIndex)
                } else {
                    nativeInterface.currentAccountIndex = modelIndex
                    root.pushAccountEdit()
                }
            }
        }
    }

    function insertEntry(entryType) {
        var newIndex = entryModel.rowCount(rootIndex)
        entryModel["setInsertTypeTo" + entryType]()
        entryModel.insertRows(newIndex, 1, rootIndex)
        renameDialog.renameEntry(null, newIndex)
    }
}
