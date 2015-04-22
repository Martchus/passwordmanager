import QtQuick 2.1
import QtQuick.Layouts 1.0
import martchus.passwordmanager 2.0

Rectangle {
    id: rect
    height: ApplicationInfo.constants.rowDelegateHeight
    width: parent.width
    signal clicked
    signal deleteEntry

    color: mouseNext.pressed ? ApplicationInfo.colors.smokeGray : ApplicationInfo.colors.white

    GridLayout {
        id: _grid
        anchors.fill: parent
        flow: Qt.LeftToRight
        rowSpacing: 4 * ApplicationInfo.ratio
        columnSpacing: 0
        columns: 2
        Rectangle {
            Layout.preferredWidth: ApplicationInfo.hMargin
            Layout.fillHeight: true
            opacity: 0
        }
        Loader {
            // sourceComponent:
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        Rectangle {
            id: separator
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.columnSpan: 2
        }
    }
    Rectangle {
        z: 1
        height: 1
        anchors.bottom: parent.bottom
        width: parent.width
        color: ApplicationInfo.colors.paleGray
    }
    MouseArea {
        id: mouseNext
        anchors.left: parent.left
        width: parent.width - 80 * ApplicationInfo.ratio - ApplicationInfo.hMargin
        height: parent.height
        onClicked: rect.clicked()
    }
}
