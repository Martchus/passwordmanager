import QtQuick 2.15
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
    actions:[
        Kirigami.Action {
            icon.name: "list-add"
            text: qsTr("Add account")
            visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: insertEntry("Account")
            shortcut: "Ctrl+A"
        },
        Kirigami.Action {
            icon.name: "edit-paste"
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
                showPassiveNotification(qsTr("Pasted %1").arg(
                                            pastedEntries.join(", ")))
            }
            shortcut: StandardKey.Paste
        },
        Kirigami.Action {
            icon.name: "folder-add"
            text: qsTr("Add category")
            visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: insertEntry("Node")
            shortcut: "Ctrl+Shift+A"
        }
    ]

    // dialog to confirm deletion of an entry
    BasicDialog {
        id: confirmDeletionDialog

        property string entryDesc: "?"
        property int entryIndex: -1

        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel
        title: qsTr("Delete %1?").arg(entryDesc)
        onAccepted: entryModel.removeRows(this.entryIndex, 1, rootIndex)
        contentItem: ColumnLayout {
            Controls.Label {
                text: " "
            }
        }

        function confirmDeletion(entryName, entryIndex) {
            var isNode = entryModel.isNode(entryModel.index(entryIndex, 0,
                                                            rootIndex))
            var entryType = isNode ? qsTr("category ") : qsTr("account ")
            this.entryIndex = entryIndex
            this.entryDesc = entryType + entryName

            // skip dialog if undo is supported
            if (nativeInterface.undoStack) {
                entryModel.removeRows(entryIndex, 1, rootIndex)
                showPassiveNotification(qsTr("Deleted %1").arg(entryDesc))
                return
            }
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
        title: entryNew ? qsTr("Name for new %1").arg(entryDesc) : qsTr(
                              "Rename %1").arg(entryDesc)
        onAccepted: {
            entryModel.setData(entryModel.index(this.entryIndex, 0, rootIndex),
                               newEntryName)
        }
        onRejected: {
            if (this.entryNew) {
                entryModel.removeRows(this.entryIndex, 1, rootIndex)
            }
        }
        contentItem: ColumnLayout {
            Controls.TextField {
                id: entryNameTextField
                Layout.preferredWidth: renameDialog.availableWidth
                placeholderText: qsTr("enter new name here")
                Keys.onPressed: (event) => renameDialog.acceptOnReturn(event)
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
        reuseItems: true
        model: DelegateModel {
            id: delegateModel
            delegate: EntryDelegate {
                width: entriesListView.width
                view: entriesListView
                onMoveRequested:
                    (oldIndex, newIndex) => {
                        entryModel.moveRows(rootIndex, oldIndex, 1, rootIndex, oldIndex < newIndex ? newIndex + 1 : newIndex)
                    }
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
