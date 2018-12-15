import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
import QtQuick.Layouts 1.2
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.4 as Kirigami

BasicDialog {
    id: aboutDialog
    standardButtons: Controls.Dialog.Ok
    padding: Kirigami.Units.largeSpacing

    ColumnLayout {
        width: aboutDialog.availableWidth

        Image {
            readonly property double size: Kirigami.Units.gridUnit * 12.2
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: size
            Layout.preferredHeight: size
            source: "qrc:/icons/hicolor/scalable/apps/passwordmanager.svg"
            sourceSize.width: size
            sourceSize.height: size
        }
        Controls.Label {
            Layout.fillWidth: true
            text: app.applicationName
            font.bold: true
            font.pointSize: Qt.application.font.pointSize * 1.2
            horizontalAlignment: Text.AlignHCenter
        }
        Controls.Label {
            Layout.fillWidth: true
            text: app.applicationVersion
            horizontalAlignment: Text.AlignHCenter
        }
        Controls.Label {
            Layout.fillWidth: true
            text: description
            font.italic: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }
        Controls.Label {
            Layout.fillWidth: true
            text: "<a href=\"" + app.organizationDomain + "\">" + app.organizationDomain + "</a>"
            horizontalAlignment: Text.AlignHCenter
            onLinkActivated: openWebsite()
            wrapMode: Text.Wrap
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: parent.openWebsite()
            }
            function openWebsite() {
                Qt.openUrlExternally(app.organizationDomain)
            }
        }
        Controls.Label {
            Layout.fillWidth: true
            text: qsTr("developed by %1").arg(app.organizationName)
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
        }
    }
}
