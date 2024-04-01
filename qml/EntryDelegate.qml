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
        alwaysVisibleActions: true // default is broken in mobile/tablet mode
        contentItem: RowLayout {
            Kirigami.ListItemDragHandle {
                listItem: listItem
                listView: view
                enabled: !nativeInterface.hasEntryFilter
                onMoveRequested:
                    (oldIndex, newIndex) => delegate.moveRequested(oldIndex, newIndex)
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
                                    index) ? "folder-symbolic" : "view-list-details-symbolic"
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: model.name
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: (mouse) => {
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
                            showPassiveNotification(qsTr("Cut %1").arg(
                                                        model.name))
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
                icon.name: "edit-cut"
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
                icon.name: "edit-delete"
                text: qsTr("Delete")
                enabled: !nativeInterface.hasEntryFilter
                onTriggered: confirmDeletionDialog.confirmDeletion(
                                 model.name, index)
                shortcut: StandardKey.Delete
            },
            Kirigami.Action {
                icon.name: "edit-rename"
                text: qsTr("Rename")
                enabled: !nativeInterface.hasEntryFilter
                onTriggered: renameDialog.renameEntry(model.name, index)
                shortcut: "F2"
            }
        ]
    }
}
