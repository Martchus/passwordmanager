import QtQuick 2.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.0
import martchus.passwordmanager 2.0

Button {
    id: button
    property int pixelSize: 34
    property real letterSpacing: -0.25
    property string text: ""

    style: ButtonStyle {
        label: Label {
            id: label
            font.family: "Open Sans"
            font.pixelSize: pixelSize * ApplicationInfo.ratio * 1.1 // increasing fonts
            font.letterSpacing: letterSpacing * ApplicationInfo.ratio
            color: ApplicationInfo.colors.doubleDarkGray
            verticalAlignment: Text.AlignBottom
            horizontalAlignment: Text.AlignLeft
            elide: Text.ElideRight
            linkColor: ApplicationInfo.colors.blue
            renderType: ApplicationInfo.isMobile ? Text.QtRendering : Text.NativeRendering
            text: button.text

            Text {
                id: text
                visible: false
                font.family: label.font.family
                font.pixelSize: label.font.pixelSize
                font.letterSpacing: label.font.letterSpacing
                wrapMode: label.wrapMode
                elide: label.elide
                text: label.text
            }
        }
    }

    property int maximumWidth: (ApplicationInfo.constants.isMobile ? ApplicationInfo.applicationWidth : 1120) / 4
    Layout.minimumWidth: Math.min(Layout.maximumWidth, implicitWidth + 1)

}
