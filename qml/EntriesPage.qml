import QtQuick
import QtQuick.Layouts
import QtQml.Models
import QtQuick.Controls.Material

Page {
    id: page
    property var main: undefined
    property var entryModel: nativeInterface.hasEntryFilter ? nativeInterface.entryFilterModel : nativeInterface.entryModel

    title: qsTr("Accounts")

    property list<Action> actions: [
        Action {
            icon.name: "list-add-symbolic"
            text: qsTr("Add account")
            property bool visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: insertEntry("Account")
        },
        Action {
            icon.name: "folder-add-symbolic"
            text: qsTr("Add category")
            property bool visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: insertEntry("Node")
        },
        Action {
            icon.name: "edit-paste-symbolic"
            text: qsTr("Paste")
            property bool visible: nativeInterface.canPaste && !nativeInterface.hasEntryFilter
            enabled: visible
            onTriggered: {
                var targetFolderIndex = treeView.selectionModel.currentIndex
                if (!targetFolderIndex || !targetFolderIndex.valid) {
                    targetFolderIndex = entryModel.index(0, 0)
                } else if (!entryModel.isNode(targetFolderIndex)) {
                    targetFolderIndex = entryModel.parent(targetFolderIndex)
                }
                var pastedEntries = nativeInterface.pasteEntries(targetFolderIndex)
                if (pastedEntries.length < 1) {
                    showPassiveNotification(qsTr("Unable to paste the entries here"))
                    return
                }
                showPassiveNotification(qsTr("Pasted %1").arg(pastedEntries.join(", ")))
            }
        }
    ]

    // dialog to confirm deletion of an entry
    BasicDialog {
        id: confirmDeletionDialog
        property string entryDesc: "?"
        property var entryModelIndex: undefined

        standardButtons: Dialog.Ok | Dialog.Cancel
        title: qsTr("Delete %1?").arg(entryDesc)
        onAccepted: {
            if (entryModelIndex && entryModelIndex.valid) {
                var parentIndex = entryModel.parent(entryModelIndex)
                entryModel.removeRows(entryModelIndex.row, 1, parentIndex)
            }
        }
        contentItem: ColumnLayout {
            Label {
                text: " "
            }
        }

        function confirmDeletion(entryName, modelIndex) {
            var isNode = entryModel.isNode(modelIndex)
            var entryType = isNode ? qsTr("category ") : qsTr("account ")
            this.entryModelIndex = modelIndex
            this.entryDesc = entryType + entryName

            // skip dialog if undo is supported
            if (nativeInterface.undoStack) {
                var parentIndex = entryModel.parent(modelIndex)
                entryModel.removeRows(modelIndex.row, 1, parentIndex)
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
        property var entryModelIndex: undefined
        property alias newEntryName: entryNameTextField.text
        property bool entryNew: false

        standardButtons: newEntryName.length > 0 ? Dialog.Ok | Dialog.Cancel : Dialog.Cancel
        title: entryNew ? qsTr("Name for new %1").arg(entryDesc) : qsTr("Rename %1").arg(entryDesc)
        onAccepted: {
            if (entryModelIndex && entryModelIndex.valid) {
                entryModel.setData(entryModelIndex, newEntryName, Qt.DisplayRole)
            }
        }
        onRejected: {
            if (this.entryNew && entryModelIndex && entryModelIndex.valid) {
                var parentIndex = entryModel.parent(entryModelIndex)
                entryModel.removeRows(entryModelIndex.row, 1, parentIndex)
            }
        }
        contentItem: ColumnLayout {
            TextField {
                id: entryNameTextField
                Layout.preferredWidth: renameDialog.availableWidth
                placeholderText: qsTr("New name")
                Keys.onPressed: (event) => renameDialog.acceptOnReturn(event)
            }
        }

        function renameEntry(entryName, modelIndex, isNew) {
            var isNode = entryModel.isNode(modelIndex)
            var entryType = isNode ? qsTr("category ") : qsTr("account ")

            this.entryModelIndex = modelIndex
            this.entryNew = isNew
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

    TreeView {
        id: treeView
        anchors.fill: parent
        model: page.entryModel

        boundsMovement: Flickable.StopAtBounds
        boundsBehavior: Flickable.DragAndOvershootBounds
        transformOrigin: treeView.verticalOvershoot >= 0 ? Item.Top : Item.Bottom
        transform: Scale {
            origin.y: treeView.verticalOvershoot > 0 ? treeView.height : 0
            yScale: 1 + Math.log(Math.abs(treeView.verticalOvershoot) + 1) * 0.01
        }

        columnWidthProvider: function(column) {
            return treeView.width
        }

        onWidthChanged: {
            treeView.forceLayout()
        }

        selectionModel: ItemSelectionModel {
            model: page.entryModel
        }

        delegate: TreeViewDelegate {
            id: treeDelegate
            width: treeView.width
            rightPadding: 48

            readonly property var currentModelIndex: treeDelegate.treeView.index(treeDelegate.row, treeDelegate.column)

            icon.name: entryModel.isNode(currentModelIndex) ? "folder-symbolic" : "view-list-details-symbolic"

            background: Rectangle {
                color: (treeDelegate.current || treeDelegate.selected)
                       ? (nativeInterface.darkModeEnabled ? "#444444" : "#e0e0e0")
                       : "transparent"
            }

            ToolButton {
                id: dragHandle
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 48
                height: parent.height
                icon.name: "handle-sort"
                flat: true
                z: 10
                enabled: !nativeInterface.hasEntryFilter && currentModelIndex !== entryModel.index(0, 0)
                visible: enabled

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
                        startY = mapToItem(treeView, mouse.x, mouse.y).y
                        dragging = true
                        treeView.interactive = false
                        onPressAndHold: nativeInterface.performHapticFeedback()
                    }

                    onPositionChanged: (mouse) => {
                        mouse.preventSteal = true
                        if (!dragging) return
                        var currentY = mapToItem(treeView, mouse.x, mouse.y).y
                        var deltaY = currentY - startY
                        var itemHeight = treeDelegate.height

                        var parentIndex = entryModel.parent(currentModelIndex)
                        var siblingRow = currentModelIndex.row

                        if (deltaY > itemHeight / 2) {
                            if (siblingRow < entryModel.rowCount(parentIndex) - 1) {
                                entryModel.moveRows(parentIndex, siblingRow, 1, parentIndex, siblingRow + 2)
                                startY += itemHeight
                            }
                        } else if (deltaY < -itemHeight / 2) {
                            if (siblingRow > 0) {
                                entryModel.moveRows(parentIndex, siblingRow, 1, parentIndex, siblingRow - 1)
                                startY -= itemHeight
                            }
                        }
                    }

                    onReleased: (mouse) => {
                        dragging = false
                        treeView.interactive = true
                    }

                    onCanceled: {
                        dragging = false
                        treeView.interactive = true
                    }
                }
            }

            MouseArea {
                anchors.left: parent.left
                anchors.right: dragHandle.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: (mouse) => {
                    treeView.selectionModel.setCurrentIndex(currentModelIndex, ItemSelectionModel.ClearAndSelect)
                    if (mouse.button === Qt.RightButton) {
                        entryContextMenu.showCenteredIn(dragHandle)
                    } else {
                        if (entryModel.isNode(currentModelIndex)) {
                            treeView.toggleExpanded(treeDelegate.row)
                        } else {
                            nativeInterface.currentAccountIndex = currentModelIndex
                            root.pushAccountEdit()
                        }
                    }
                }
                onDoubleClicked: (mouse) => {
                    if (mouse.button === Qt.LeftButton && entryModel.isNode(currentModelIndex)) {
                        treeView.toggleExpanded(treeDelegate.row)
                    }
                }
                onPressAndHold: {
                    treeView.selectionModel.setCurrentIndex(currentModelIndex, ItemSelectionModel.ClearAndSelect)
                    entryContextMenu.showCenteredIn(dragHandle)
                }
            }

            CustomMenu {
                id: entryContextMenu
                enabled: !nativeInterface.hasEntryFilter
                MenuItem {
                    icon.name: "edit-cut-symbolic"
                    text: qsTr("Cut")
                    onTriggered: {
                        nativeInterface.cutEntry(currentModelIndex)
                        showPassiveNotification(qsTr("Cut %1").arg(model.display))
                    }
                }
                MenuItem {
                    icon.name: "edit-paste-symbolic"
                    text: qsTr("Paste")
                    visible: nativeInterface.canPaste
                    height: visible ? implicitHeight : 0
                    onTriggered: {
                        var targetFolderIndex = currentModelIndex
                        if (!entryModel.isNode(targetFolderIndex)) {
                            targetFolderIndex = entryModel.parent(targetFolderIndex)
                        }
                        var pastedEntries = nativeInterface.pasteEntries(targetFolderIndex)
                        if (pastedEntries.length < 1) {
                            showPassiveNotification(qsTr("Unable to paste the entries here"))
                            return
                        }
                        showPassiveNotification(qsTr("Pasted %1").arg(pastedEntries.join(", ")))
                    }
                }
                MenuItem {
                    icon.name: "edit-delete-symbolic"
                    text: qsTr("Delete")
                    onTriggered: confirmDeletionDialog.confirmDeletion(model.display, currentModelIndex)
                }
                MenuItem {
                    icon.name: "edit-rename-symbolic"
                    text: qsTr("Rename")
                    onTriggered: renameDialog.renameEntry(model.display, currentModelIndex, false)
                }
                MenuItem {
                    icon.name: "go-up-symbolic"
                    text: qsTr("Move up")
                    visible: currentModelIndex.row > 0
                    height: visible ? implicitHeight : 0
                    onTriggered: {
                        var parentIndex = entryModel.parent(currentModelIndex)
                        var row = currentModelIndex.row
                        entryModel.moveRows(parentIndex, row, 1, parentIndex, row - 1)
                    }
                }
                MenuItem {
                    icon.name: "go-down-symbolic"
                    text: qsTr("Move down")
                    visible: currentModelIndex.row < entryModel.rowCount(entryModel.parent(currentModelIndex)) - 1
                    height: visible ? implicitHeight : 0
                    onTriggered: {
                        var parentIndex = entryModel.parent(currentModelIndex)
                        var row = currentModelIndex.row
                        entryModel.moveRows(parentIndex, row, 1, parentIndex, row + 2)
                    }
                }
            }
        }

        onExpanded: (row) => {
            var modelIndex = treeView.index(row, 0)
            if (treeView.model.isNode(modelIndex)) {
                treeView.model.setData(modelIndex, true, 0x0100 + 2)
            }
        }
        onCollapsed: (row) => {
            var modelIndex = treeView.index(row, 0)
            if (treeView.model.isNode(modelIndex)) {
                treeView.model.setData(modelIndex, false, 0x0100 + 2)
            }
        }

        Component.onCompleted: {
            // Expand root node automatically
            treeView.expand(0)
        }
    }

    function insertEntry(entryType) {
        var targetFolderIndex = treeView.selectionModel.currentIndex
        if (!targetFolderIndex || !targetFolderIndex.valid) {
            targetFolderIndex = entryModel.index(0, 0)
        } else if (!entryModel.isNode(targetFolderIndex)) {
            targetFolderIndex = entryModel.parent(targetFolderIndex)
        }

        var newIndex = entryModel.rowCount(targetFolderIndex)
        entryModel["setInsertTypeTo" + entryType]()
        entryModel.insertRows(newIndex, 1, targetFolderIndex)

        var insertedIndex = entryModel.index(newIndex, 0, targetFolderIndex)
        renameDialog.renameEntry(null, insertedIndex, true)

        var r = treeView.rowAtIndex(targetFolderIndex)
        if (r >= 0) {
            treeView.expand(r)
        }
    }
}
