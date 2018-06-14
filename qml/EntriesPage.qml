import QtQuick 2.4
import QtQuick.Layouts 1.2
import QtQml.Models 2.2
import QtQuick.Controls 2.1 as Controls
import org.kde.kirigami 2.5 as Kirigami

Kirigami.ScrollablePage {
    id: page
    property var main: undefined
    property alias entryModel: delegateModel.model
    property alias rootIndex: delegateModel.rootIndex

    Layout.fillWidth: true
    title: "?"
    actions {
        main: Kirigami.Action {
            iconName: "list-add"
            text: qsTr("Add account")
            onTriggered: insertEntry("Account")
        }
        left: Kirigami.Action {
            iconName: "edit-paste"
            text: qsTr("Paste account")
            enabled: nativeInterface.canPaste
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
        }
        right: Kirigami.Action {
            iconName: "folder-add"
            text: qsTr("Add category")
            onTriggered: insertEntry("Node")
        }
    }
    onBackRequested: {
        if (fieldsSheet.sheetOpen) {
            event.accepted = true
            fieldsSheet.close()
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

        function confirmDeletion(entryName, entryIndex) {
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
            this.open()
        }
    }

    // component representing a field
    Component {
        id: fieldsListDelegateComponent

        RowLayout {
            id: fieldsListItem
            width: fieldsSheet.width

            Kirigami.ListItemDragHandle {
                listItem: fieldsListItem
                listView: fieldsListView
                onMoveRequested: fieldsListView.model.moveRows(
                                     fieldsListView.model.index(-1, 0),
                                     oldIndex, 1,
                                     fieldsListView.model.index(-1,
                                                                0), newIndex)
            }
            Controls.TextField {
                text: model.key ? model.key : ""
                onEditingFinished: fieldsListView.model.setData(
                                       fieldsListView.model.index(index, 0),
                                       text)
                Layout.fillWidth: true
            }
            Controls.TextField {
                text: model.actualValue ? model.actualValue : ""
                echoMode: model.isPassword
                          && (!activeFocus
                              || !main.showPasswordsOnFocus) ? TextInput.PasswordEchoOnEdit : TextInput.Normal
                onEditingFinished: fieldsListView.model.setData(
                                       fieldsListView.model.index(index, 1),
                                       text)
                Layout.fillWidth: true
            }
            Controls.Button {
                id: fieldsListItemToolButton
                text: qsTr("⋮")
                flat: true
                onClicked: fieldsListItemMenu.open()
                Layout.maximumWidth: Kirigami.Units.iconSizes.medium
                Layout.maximumHeight: Kirigami.Units.iconSizes.large

                Controls.Menu {
                    id: fieldsListItemMenu
                    y: fieldsListItemToolButton.height

                    Controls.MenuItem {
                        icon.name: model.isPassword ? "password-show-off" : "password-show-on"
                        text: model.isPassword ? qsTr("Mark as normal field") : qsTr(
                                                     "Mark as password field")
                        onClicked: fieldsListView.model.setData(
                                       fieldsListView.model.index(index, 0),
                                       model.isPassword ? 0 : 1, 0x0100 + 1)
                    }
                    Controls.MenuItem {
                        icon.name: "edit-delete"
                        text: qsTr("Delete field")
                        onClicked: fieldsListView.model.removeRows(index, 1)
                    }
                    Controls.MenuItem {
                        icon.name: "list-add"
                        text: qsTr("Insert empty field after this")
                        onClicked: fieldsListView.model.insertRows(index + 1, 1)
                    }
                }
            }
        }
    }

    // "sheet" to display field model
    Kirigami.OverlaySheet {
        id: fieldsSheet
        parent: applicationWindow().overlay
        header: Kirigami.Heading {
            text: qsTr("Edit account ") + nativeInterface.currentAccountName
        }

        ListView {
            id: fieldsListView
            implicitWidth: Kirigami.Units.gridUnit * 30
            model: nativeInterface.fieldModel
            moveDisplaced: Transition {
                YAnimator {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            delegate: Kirigami.DelegateRecycler {
                width: parent ? parent.width : implicitWidth
                sourceComponent: fieldsListDelegateComponent
            }
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
                    onMoveRequested: entryModel.moveRows(rootIndex, oldIndex,
                                                         1, rootIndex, newIndex)
                }
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
                    height: Math.max(implicitHeight,
                                     Kirigami.Units.iconSizes.smallMedium)
                    text: model.name

                    MouseArea {
                        anchors.fill: parent
                        onClicked: delegateModel.handleEntryClicked(index,
                                                                    model.name)
                    }
                }
            }
            actions: [
                Kirigami.Action {
                    iconName: "edit-cut"
                    text: qsTr("Cut")
                    onTriggered: {
                        nativeInterface.cutEntry(entryModel.index(index, 0,
                                                                  rootIndex))
                        showPassiveNotification(text + " " + model.name)
                    }
                },
                Kirigami.Action {
                    iconName: "edit-delete"
                    text: qsTr("Delete")
                    onTriggered: confirmDeletionDialog.confirmDeletion(
                                     model.name, index)
                },
                Kirigami.Action {
                    iconName: "edit-rename"
                    text: qsTr("Rename")
                    onTriggered: renameDialog.renameEntry(model.name, index)
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
                    fieldsSheet.open()
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
