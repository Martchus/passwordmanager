import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
import org.kde.kirigami 2.4 as Kirigami

Controls.Dialog {
    modal: true
    focus: true
    parent: applicationWindow().overlay
    //anchors.centerIn: parent // enable if requiring at least Qt 5.12 instead of setting x and y manually
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    width: Math.min(parent.width, Kirigami.Units.gridUnit * 30)

    function acceptOnReturn(event) {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            this.accept()
        }
    }
}
