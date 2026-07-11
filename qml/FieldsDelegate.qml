import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

ItemDelegate {
    id: delegate

    property ListView view
    implicitHeight: Math.max(48, contentItem.implicitHeight + topPadding + bottomPadding)
    rightPadding: dragHandle.width

    signal moveRequested(int oldIndex, int newIndex)

    visible: !model.isLastRow

    ToolButton {
        id: dragHandle
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        width: 48
        height: parent.height
        icon.name: "handle-sort"
        flat: true
        z: 10
        enabled: !nativeInterface.hasEntryFilter

        background: Rectangle {
            color: (dragArea.dragging)
                   ? (nativeInterface.darkModeEnabled ? "#444444" : "#e0e0e0")
                   : "transparent"
            radius: 0
        }

        MouseArea {
            id: dragArea
            anchors.fill: parent
            cursorShape: dragging ? Qt.ClosedHandCursor : Qt.OpenHandCursor

            property real startY: 0
            property bool dragging: false

            onPressed: (mouse) => {
                mouse.preventSteal = true
                startY = mapToItem(view, mouse.x, mouse.y).y
                dragging = true
                view.interactive = false
            }

            onPositionChanged: (mouse) => {
                mouse.preventSteal = true
                if (!dragging) return
                var currentY = mapToItem(view, mouse.x, mouse.y).y
                var deltaY = currentY - startY
                var itemHeight = delegate.height
                if (deltaY > itemHeight / 2) {
                    if (index < view.count - 2) {
                        delegate.moveRequested(index, index + 1)
                        startY += itemHeight
                    }
                } else if (deltaY < -itemHeight / 2) {
                    if (index > 0) {
                        delegate.moveRequested(index, index - 1)
                        startY -= itemHeight
                    }
                }
            }

            onReleased: (mouse) => {
                dragging = false
                view.interactive = true
            }

            onCanceled: {
                dragging = false
                view.interactive = true
            }
        }
    }

    contentItem: RowLayout {
        id: fieldRow
        spacing: 10
        Label {
            Layout.fillWidth: true
            elide: Text.ElideRight
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
            verticalAlignment: Text.AlignVCenter
        }
    }

    Item {
        id: clickZone
        anchors.left: parent.left
        anchors.right: dragHandle.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                fieldDialog.init(model, index)
                fieldDialog.open()
            }
            onLongPressed: fieldContextMenu.showCenteredIn(dragHandle)
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: fieldContextMenu.showCenteredIn(dragHandle)
        }
    }

    CustomMenu {
        id: fieldContextMenu
        MenuItem {
            icon.name: !model.isPassword ? "password-show-off-symbolic" : "password-show-on-symbolic"
            text: model.isPassword ? qsTr("Mark as normal field") : qsTr("Mark as password field")
            onClicked: view.model.setData(view.model.index(index, 0), model.isPassword ? 0 : 1, 0x0100 + 1)
        }
        MenuItem {
            icon.name: "edit-copy-symbolic"
            text: model.isPassword ? qsTr("Copy password") : qsTr("Copy value")
            onClicked: showPassiveNotification(nativeInterface.copyToClipboard(model.actualValue) ? qsTr("Copied") : qsTr("Unable to access clipboard"))
        }
        MenuItem {
            icon.name: "clock-symbolic"
            text: qsTr("Copy TOTP")
            visible: model.actualValue.startsWith("otpauth:")
            height: visible ? implicitHeight : 0
            onClicked: showPassiveNotification(nativeInterface.copyTOTP(model.actualValue))
        }
        MenuItem {
            icon.name: "edit-delete-symbolic"
            text: qsTr("Delete field")
            onClicked: view.model.removeRows(index, 1)
        }
        MenuItem {
            icon.name: "list-add-symbolic"
            text: qsTr("Insert empty field after this")
            onClicked: view.model.insertRows(index + 1, 1)
        }
        MenuSeparator {
            visible: goUpItem.visible || goDownItem.visible
            height: visible ? implicitHeight : 0
        }
        MenuItem {
            id: goUpItem
            icon.name: "go-up-symbolic"
            text: qsTr("Move up")
            visible: index > 0
            height: visible ? implicitHeight : 0
            onClicked: delegate.moveRequested(index, index - 1)
        }
        MenuItem {
            id: goDownItem
            icon.name: "go-down-symbolic"
            text: qsTr("Move down")
            visible: index < view.count - 2
            height: visible ? implicitHeight : 0
            onClicked: delegate.moveRequested(index, index + 1)
        }
    }
}
