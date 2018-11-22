import QtQuick 2.4
import QtQuick.Layouts 1.2
import QtQml.Models 2.2
import QtQuick.Controls 2.4 as Controls
import org.kde.kirigami 2.5 as Kirigami

Kirigami.ScrollablePage {
    id: page
    property var main: undefined

    Layout.fillWidth: true
    title: nativeInterface.currentAccountName
    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

    // dialog to edit certain field
    BasicDialog {
        id: fieldDialog
        property int entryIndex: -1
        property alias fieldName: fieldNameEdit.text
        property alias fieldValue: fieldValueEdit.text
        property alias isPassword: fieldIsPasswordCheckBox.checked
        title: qsTr("Edit field of ") + nativeInterface.currentAccountName
        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel
        onAccepted: {
            var column0 = fieldsListView.model.index(entryIndex, 0)
            var column1 = fieldsListView.model.index(entryIndex, 1)
            fieldsListView.model.setData(column0, fieldName)
            fieldsListView.model.setData(column1, fieldValue)
            fieldsListView.model.setData(column0, isPassword ? 1 : 0,
                                                               0x0100 + 1)
        }

        ColumnLayout {
            Controls.TextField {
                id: fieldNameEdit
                Layout.preferredWidth: fieldDialog.availableWidth
                text: fieldDialog.fieldName
                Keys.onPressed: fieldDialog.acceptOnReturn(event)
            }
            Controls.TextField {
                id: fieldValueEdit
                Layout.preferredWidth: fieldDialog.availableWidth
                text: fieldDialog.fieldValue
                echoMode: fieldDialog.isPassword
                          && !showCharactersCheckBox.checked ? TextInput.PasswordEchoOnEdit : TextInput.Normal
                Keys.onPressed: fieldDialog.acceptOnReturn(event)
            }
            RowLayout {
                Layout.preferredWidth: fieldDialog.availableWidth
                Controls.CheckBox {
                    id: fieldIsPasswordCheckBox
                    text: qsTr("Mark as password")
                    checked: false
                }
                Controls.CheckBox {
                    id: showCharactersCheckBox
                    text: qsTr("Show characters")
                    checked: false
                    visible: fieldDialog.isPassword
                }
            }
        }

        function init(model, index) {
            entryIndex = index
            fieldName = model.key ? model.key : ""
            fieldValue = model.actualValue ? model.actualValue : ""
            isPassword = model.isPassword ? true : false
        }
    }

    // component representing a field
    Component {
        id: fieldsListDelegateComponent

        Kirigami.SwipeListItem {
            id: fieldsListItem
            contentItem: RowLayout {
                id: fieldRow
                readonly property bool isLast: model.isLastRow

                Kirigami.ListItemDragHandle {
                    listItem: fieldsListItem
                    listView: fieldsListView
                    onMoveRequested: fieldsListView.model.moveRows(
                                         fieldsListView.model.index(-1, 0),
                                         oldIndex, 1,
                                         fieldsListView.model.index(-1, 0),
                                         newIndex)
                    opacity: fieldRow.isLast ? 0 : 100
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    RowLayout {
                        anchors.fill: parent
                        Controls.Label {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            text: {
                                if (fieldRow.isLast) {
                                    return qsTr("click to append new field")
                                }

                                var pieces = []
                                if (model.key) {
                                    pieces.push(model.key)
                                }
                                if (model.value) {
                                    pieces.push(model.value)
                                }
                                return pieces.join(": ")
                            }
                            color: fieldRow.isLast ? "gray" : palette.text
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: {
                            if (mouse.button === Qt.RightButton) {
                                fieldContextMenu.popup()
                                return
                            }
                            fieldDialog.init(model, index)
                            fieldDialog.open()
                        }
                        onPressAndHold: fieldContextMenu.popup()
                    }
                    Controls.Menu {
                        id: fieldContextMenu
                        Controls.MenuItem {
                            icon.name: !model.isPassword ? "password-show-off" : "password-show-on"
                            text: model.isPassword ? qsTr("Mark as normal field") : qsTr(
                                                         "Mark as password field")
                            onClicked: fieldsListView.model.setData(
                                           fieldsListView.model.index(index,
                                                                      0),
                                           model.isPassword ? 0 : 1, 0x0100 + 1)
                        }
                        Controls.MenuItem {
                            icon.name: "edit-copy"
                            text: model.isPassword ? qsTr("Copy password") : qsTr(
                                                         "Copy value")
                            onClicked: showPassiveNotification(
                                           nativeInterface.copyToClipboard(
                                               model.actualValue) ? qsTr("Copied") : qsTr(
                                                                        "Unable to access clipboard"))
                        }
                        Controls.MenuItem {
                            icon.name: "edit-delete"
                            text: qsTr("Delete field")
                            onClicked: fieldsListView.model.removeRows(index, 1)
                        }
                        Controls.MenuItem {
                            icon.name: "list-add"
                            text: qsTr("Insert empty field after this")
                            onClicked: fieldsListView.model.insertRows(
                                           index + 1, 1)
                        }
                    }
                }
            }
            actions: [
                Kirigami.Action {
                    iconName: !model.isPassword ? "password-show-off" : "password-show-on"
                    text: model.isPassword ? qsTr(
                                                 "Mark as normal field") : qsTr(
                                                 "Mark as password field")
                    onTriggered: fieldsListView.model.setData(
                                     fieldsListView.model.index(index, 0),
                                     model.isPassword ? 0 : 1, 0x0100 + 1)
                    visible: !fieldRow.isLast
                },
                Kirigami.Action {
                    iconName: "edit-copy"
                    text: model.isPassword ? qsTr("Copy password") : qsTr(
                                                 "Copy value")
                    onTriggered: showPassiveNotification(
                                     nativeInterface.copyToClipboard(
                                         model.actualValue) ? qsTr("Copied") : qsTr(
                                                                  "Unable to access clipboard"))
                    shortcut: StandardKey.Cut
                    visible: !fieldRow.isLast
                },
                Kirigami.Action {
                    iconName: "edit-delete"
                    text: qsTr("Delete field")
                    onTriggered: fieldsListView.model.removeRows(index, 1)
                    shortcut: StandardKey.Delete
                    visible: !fieldRow.isLast
                },
                Kirigami.Action {
                    iconName: "list-add"
                    text: qsTr("Insert empty field after this")
                    enabled: !nativeInterface.hasEntryFilter
                    onTriggered: fieldsListView.model.insertRows(index + 1, 1)
                    visible: !fieldRow.isLast
                }
            ]
        }
    }

    // list view to edit the currently selected account
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
