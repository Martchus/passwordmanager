import QtQuick 2.4
import QtQuick.Layouts 1.2
import QtQml.Models 2.2
import QtQuick.Controls 2.1 as Controls
import org.kde.kirigami 2.4 as Kirigami

Kirigami.ScrollablePage {
    id: page

    property alias model: delegateModel.model
    property alias rootIndex: delegateModel.rootIndex

    Layout.fillWidth: true
    title: "?"
    actions {
        main: Kirigami.Action {
            iconName: "list-add"
            text: qsTr("Add account")
            onTriggered: {
                model.setInsertTypeToAccount()
                model.insertRows(model.rowCount(rootIndex), 1, rootIndex)
            }
        }
        left: Kirigami.Action {
            iconName: "edit-paste"
            text: qsTr("Paste account")
            onTriggered: {


                // TODO
            }
        }
        right: Kirigami.Action {
            iconName: "folder-add"
            text: qsTr("Add category")
            onTriggered: {
                model.setInsertTypeToNode()
                model.insertRows(model.rowCount(rootIndex), 1, rootIndex)
            }
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

    BasicDialog {
        id: confirmDeletionDialog
        property string entryDesc: "?"
        property int entryIndex: -1
        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel
        title: qsTr("Delete %1?").arg(entryDesc)

        function confirmDeletion(entryName, entryIndex) {
            var isNode = model.isNode(entryIndex)
            var entryType = isNode ? qsTr("category ") : qsTr("account ")

            this.entryIndex = entryIndex
            this.entryDesc = entryType + entryName
            this.open()
        }

        onAccepted: model.removeRows(this.entryIndex, 1, rootIndex)
    }

    BasicDialog {
        id: renameDialog
        property string entryDesc: "?"
        property int entryIndex: -1
        property alias newEntryName: entryNameTextField.text
        standardButtons: newEntryName.length
                         > 0 ? Controls.Dialog.Ok | Controls.Dialog.Cancel : Controls.Dialog.Cancel
        title: qsTr("Rename ") + entryDesc

        ColumnLayout {
            Controls.TextField {
                id: entryNameTextField
                Layout.preferredWidth: renameDialog.availableWidth
                placeholderText: qsTr("enter new name here")
            }
        }

        function renameEntry(entryName, entryIndex) {
            var isNode = model.isNode(entryIndex)
            var entryType = isNode ? qsTr("category ") : qsTr("account ")

            this.entryIndex = entryIndex
            this.entryDesc = entryType + entryName
            this.newEntryName = entryName
            this.open()
        }

        onAccepted: model.setData(model.index(this.entryIndex, 0, rootIndex),
                                  newEntryName)
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
            delegate: RowLayout {
                Controls.TextField {
                    text: key ? key : ""
                    onEditingFinished: fieldsListView.model.setData(
                                           fieldsListView.model.index(index,
                                                                      0), text)
                }
                Controls.TextField {
                    text: value ? value : ""
                    echoMode: isPassword ? TextInput.PasswordEchoOnEdit : TextInput.Normal
                    onEditingFinished: fieldsListView.model.setData(
                                           fieldsListView.model.index(index,
                                                                      1), text)
                }
            }
        }
    }

    // list view to display one hierarchy level of entry model
    ListView {
        id: listView
        model: DelegateModel {
            id: delegateModel

            function isNode(rowNumber) {
                return model.isNode(model.index(rowNumber, 0, rootIndex))
            }

            function handleEntryClicked(rowNumber, entryName) {
                var modelIndex = model.index(rowNumber, 0, rootIndex)
                if (model.isNode(modelIndex)) {
                    root.pushStackEntry(model, modelIndex)
                } else {
                    nativeInterface.currentAccountIndex = modelIndex
                    fieldsSheet.open()
                }
            }

            delegate: Kirigami.SwipeListItem {
                id: listItem
                contentItem: RowLayout {
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
                        Layout.alignment: verticalCenter
                        text: name

                        MouseArea {
                            anchors.fill: parent
                            onClicked: delegateModel.handleEntryClicked(index,
                                                                        name)
                        }
                    }
                }
                actions: [
                    Kirigami.Action {
                        iconName: "edit-cut"
                        text: qsTr("Cut")
                        onTriggered: showPassiveNotification(text + " " + name)
                    },
                    Kirigami.Action {
                        iconName: "edit-delete"
                        text: qsTr("Delete")
                        onTriggered: confirmDeletionDialog.confirmDeletion(
                                         name, index)
                    },
                    Kirigami.Action {
                        iconName: "edit-rename"
                        text: qsTr("Rename")
                        onTriggered: renameDialog.renameEntry(name, index)
                    }
                ]
            }
        }
    }
}
