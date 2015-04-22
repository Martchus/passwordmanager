import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import martchus.passwordmanager 2.0

ScrollView {
    frameVisible: false
    style: ScrollViewStyle {
        property int handleWidth: 20 * ApplicationInfo.ratio
        transientScrollBars: true
        padding{ top: 4 ; bottom: 4 ; right: 4}
        property bool hovered: false
    }
    Rectangle {
        anchors.fill: parent
        color: ApplicationInfo.colors.white
    }
}
