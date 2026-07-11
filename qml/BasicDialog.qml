import QtQuick
import QtQuick.Controls.Material

Dialog {
    modal: true
    focus: true
    parent: root.overlay
    anchors.centerIn: parent
    width: Math.min(parent.width - 40, 500)

    function acceptOnReturn(event) {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            this.accept()
        }
    }
}
