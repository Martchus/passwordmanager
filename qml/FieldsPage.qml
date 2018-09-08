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

    // component representing a field
    Component {
        id: fieldsListDelegateComponent

        Kirigami.BasicListItem {
            id: fieldsListItem
            contentItem: RowLayout {
                Kirigami.ListItemDragHandle {
                    listItem: fieldsListItem
                    listView: fieldsListView
                    onMoveRequested: fieldsListView.model.moveRows(
                                         fieldsListView.model.index(-1, 0),
                                         oldIndex, 1,
                                         fieldsListView.model.index(-1, 0),
                                         newIndex)
                }
                Controls.TextField {
                    Layout.fillWidth: true
                    text: model.key ? model.key : ""
                    onEditingFinished: fieldsListView.model.setData(
                                           fieldsListView.model.index(index,
                                                                      0), text)
                }
                Controls.TextField {
                    Layout.fillWidth: true
                    text: model.actualValue ? model.actualValue : ""
                    echoMode: model.isPassword
                              && (!activeFocus
                                  || !main.showPasswordsOnFocus) ? TextInput.PasswordEchoOnEdit : TextInput.Normal
                    onEditingFinished: fieldsListView.model.setData(
                                           fieldsListView.model.index(index,
                                                                      1), text)
                }
                Kirigami.Icon {
                    source: "handle-right"
                    width: Kirigami.Units.iconSizes.smallMedium
                    height: Kirigami.Units.iconSizes.smallMedium
                    Layout.fillHeight: true
                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: fieldContextMenu.popup()
                        onPressAndHold: fieldContextMenu.popup()
                    }
                }
                Controls.Menu {
                    id: fieldContextMenu
                    Controls.MenuItem {
                        icon.name: model.isPassword ? "password-show-off" : "password-show-on"
                        text: model.isPassword ? qsTr("Mark as normal field") : qsTr(
                                                     "Mark as password field")
                        onClicked: fieldsListView.model.setData(
                                       fieldsListView.model.index(index, 0),
                                       model.isPassword ? 0 : 1, 0x0100 + 1)
                    }
                    Controls.MenuItem {
                        icon.name: "edit-copy"
                        text: qsTr("Copy password")
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
                        onClicked: fieldsListView.model.insertRows(index + 1, 1)
                    }
                }
            }
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
