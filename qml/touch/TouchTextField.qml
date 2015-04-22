import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import martchus.passwordmanager 2.0

TextField {
    id: textfield
    signal clearButtonClicked
    implicitWidth: parent.width
    property bool isEmpty: true
    style: TextFieldStyle {
        renderType: ApplicationInfo.isMobile ? Text.QtRendering : Text.NativeRendering
        background: Rectangle {
            radius: 8
            border.width: 1
            border.color: Qt.darker(ApplicationInfo.colors.blue, 1.6)
            color: ApplicationInfo.colors.white
            gradient: Gradient {
                GradientStop { position: 0 ; color: "#ddd"}
                GradientStop { position: 0.05 ; color: "#fff"}
            }
            implicitHeight: 60 * ApplicationInfo.ratio
            opacity: 1
        }
        padding.left : (12 + 50) * ApplicationInfo.ratio
        padding.right: (12 + 50) * ApplicationInfo.ratio
        font.pixelSize: 28 * ApplicationInfo.ratio
        font.family: "Open Sans"
        font.letterSpacing: -0.25 * ApplicationInfo.ratio
        selectedTextColor : ApplicationInfo.colors.lightGray
        selectionColor : ApplicationInfo.colors.darkBlue
        textColor : ApplicationInfo.colors.mediumGray
    }

    onClearButtonClicked: text = ""

    Item {
        id: item
        anchors.left: parent.left
        anchors.top: parent.top
        height: parent.height
        width: parent.height
        Image {
            opacity: 0.9
            anchors.centerIn: item
            height: iconSize
            width: iconSize
            source: ApplicationInfo.imagePath("magnifier.png")
            property int iconSize: 50 * ApplicationInfo.ratio
        }
    }

    onTextChanged: isEmpty = (text === "")
    inputMethodHints: Qt.ImhNoPredictiveText
    MouseArea {
        z: 2
        opacity: !textfield.isEmpty ? 1 : 0
        Behavior on opacity {NumberAnimation{}}
        anchors.right: parent.right
        anchors.rightMargin: 4 * ApplicationInfo.ratio
        anchors.top: parent.top
        height: parent.height
        width: parent.height
        Image {
            anchors.centerIn: parent
            source: ApplicationInfo.imagePath("clear.png")
            property int iconSize: 40 * ApplicationInfo.ratio
            opacity: parent.pressed ? 1 : 0.9
            width: iconSize
            height: iconSize
        }
        onClicked: textfield.clearButtonClicked()
    }
}

