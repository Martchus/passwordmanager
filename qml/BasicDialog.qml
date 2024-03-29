import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
import org.kde.kirigami 2.4 as Kirigami

Controls.Dialog {
    modal: true
    focus: true
    parent: applicationWindow().overlay
    anchors.centerIn: parent
    width: Math.min(parent.width, Kirigami.Units.gridUnit * 30)

    function acceptOnReturn(event) {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            this.accept()
        }
    }
}
