import QtQuick 2.4
import QtQuick.Layouts 1.2
import QtQml.Models 2.2
import QtQuick.Controls 2.4 as Controls
import org.kde.kirigami 2.5 as Kirigami

Item {
    id: delegate

    property ListView view
    implicitHeight : listItem.implicitHeight

    signal moveRequested(int oldIndex, int newIndex)

    Kirigami.SwipeListItem {
        id: listItem
        visible: !model.isLastRow
        contentItem: RowLayout {
            id: fieldRow
            Kirigami.ListItemDragHandle {
                listItem: listItem
                listView: view
                onMoveRequested: (oldIndex, newIndex) => delegate.moveRequested(oldIndex, newIndex)
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
                            let pieces = []
                            if (model.key) {
                                pieces.push(model.key)
                            }
                            if (model.value) {
                                pieces.push(model.value)
                            }
                            return pieces.join(": ")
                        }
                        color: palette.text
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: (mouse) => {
                        if (mouse.button === Qt.RightButton) {
                            return fieldContextMenu.popup()
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
                        onClicked: view.model.setData(
                                       view.model.index(index,
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
                        onClicked: view.model.removeRows(index, 1)
                    }
                    Controls.MenuItem {
                        icon.name: "list-add"
                        text: qsTr("Insert empty field after this")
                        onClicked: view.model.insertRows(
                                       index + 1, 1)
                    }
                }
            }
        }
        actions: [
            Kirigami.Action {
                icon.name: !model.isPassword ? "password-show-off" : "password-show-on"
                text: model.isPassword ? qsTr(
                                             "Mark as normal field") : qsTr(
                                             "Mark as password field")
                onTriggered: view.model.setData(
                                 view.model.index(index, 0),
                                 model.isPassword ? 0 : 1, 0x0100 + 1)
            },
            Kirigami.Action {
                icon.name: "edit-copy"
                text: model.isPassword ? qsTr("Copy password") : qsTr(
                                             "Copy value")
                onTriggered: showPassiveNotification(
                                 nativeInterface.copyToClipboard(
                                     model.actualValue) ? qsTr("Copied") : qsTr(
                                                              "Unable to access clipboard"))
                shortcut: StandardKey.Cut
            },
            Kirigami.Action {
                icon.name: "edit-delete"
                text: qsTr("Delete field")
                onTriggered: view.model.removeRows(index, 1)
                shortcut: StandardKey.Delete
            },
            Kirigami.Action {
                icon.name: "list-add"
                text: qsTr("Insert empty field after this")
                enabled: !nativeInterface.hasEntryFilter
                onTriggered: view.model.insertRows(index + 1, 1)
            }
        ]
    }
}
