import QtQuick 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Dialogs 1.2
import martchus.passwordmanager 2.0

import "../touch" as Touch

BasicPage {
    id: startPage
    title1: qsTr("Password Manager")

    searchFieldVisisble: true

    FileDialog {
        id: fileDialog
        title: qsTr("Select a password file")
        selectExisting: true
        onAccepted: {
            ApplicationInfo.currentFile = fileUrl
            nextPage()
        }
    }

    ColumnLayout {
        anchors.top: topRect.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 5

        Touch.TouchButton {
            text: qsTr("Create a new password file")
            Layout.fillWidth: true
        }

        Touch.TouchButton {
            text: qsTr("Open an existing password file")
            onClicked: {
                ApplicationInfo.currentFile = "./test"
                nextPage() // fileDialog.visible = true
            }
            Layout.fillWidth: true
        }

        Touch.TouchLabel {
            text: qsTr("Recently opened files")
        }

        Touch.TouchLabel {
            text: qsTr("Test 1")
            //Layout.fillWidth: true
            color: ApplicationInfo.colors.lightGray
        }
        Touch.TouchLabel {
            text: qsTr("Test 2")
            //Layout.fillWidth: true
            color: ApplicationInfo.colors.lightGray
        }
        Touch.TouchLabel {
            text: qsTr("Test 3")
            //Layout.fillWidth: true
            color: ApplicationInfo.colors.lightGray
        }
    }
}
