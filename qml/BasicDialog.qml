import QtQuick
import QtQuick.Controls.Material

Dialog {
    id: dialog
    modal: true
    clip: true
    focus: true
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    popupType: Popup.Item  // use only Popup.Item for now because context menus (e.g. created by the Breeze style) use only Popup.Item which requires being in a window with enough space
    width: Math.min(Math.max(0, parent.width - additionalSpacing - leftMargin - rightMargin), 800)
    height: Math.min(implicitHeight, Math.max(0, parent.height - additionalSpacing - topMargin - bottomMargin))
    topMargin: parent?.SafeArea.margins.top ?? 0
    leftMargin: parent?.SafeArea.margins.left ?? 0
    rightMargin: parent?.SafeArea.margins.right ?? 0
    bottomMargin: parent?.SafeArea.margins.bottom ?? 0
    onOpened: root.dialogs.push(dialog)
    onClosed: {
        const index = root.dialogs.indexOf(dialog)
        if (index > -1) {
            root.dialogs.splice(index, 1)
        }
    }

    property int additionalSpacing: 20

    function acceptOnReturn(event) {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            this.accept()
        }
    }
}
