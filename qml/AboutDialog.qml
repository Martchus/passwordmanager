import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

BasicDialog {
    id: aboutDialog
    standardButtons: Dialog.Ok
    implicitWidth: 400
    padding: 20
    title: qsTr("About %1").arg(Qt.application.name)
    contentItem: ScrollView {
        contentWidth: availableWidth
        ColumnLayout {
            width: availableWidth
            Image {
                readonly property double size: 200
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: size
                Layout.preferredHeight: size
                source: "qrc:/icons/hicolor/scalable/apps/passwordmanager.svg"
                sourceSize.width: size
                sourceSize.height: size
            }
            Label {
                Layout.fillWidth: true
                text: app.applicationName
                font.bold: true
                font.pointSize: Qt.application.font.pointSize * 1.2
                horizontalAlignment: Text.AlignHCenter
            }
            Label {
                Layout.fillWidth: true
                text: app.applicationVersion
                horizontalAlignment: Text.AlignHCenter
            }
            Label {
                Layout.fillWidth: true
                text: description
                font.italic: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }
            Label {
                Layout.fillWidth: true
                text: "<a href=\"" + app.organizationDomain + "\">" + app.organizationDomain + "</a>"
                textFormat: Text.StyledText
                horizontalAlignment: Text.AlignHCenter
                onLinkActivated: Qt.openUrlExternally(app.organizationDomain)
                wrapMode: Text.Wrap
            }
            Label {
                Layout.fillWidth: true
                text: qsTr("developed by %1").arg(app.organizationName)
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }
        }
    }
}
