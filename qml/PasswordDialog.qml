import QtQuick 2.7
import QtQuick.Controls 2.1 as Controls
import QtQuick.Layouts 1.2
import org.kde.kirigami 2.4 as Kirigami

BasicDialog {
    id: passwordDialog
    property alias instruction: instructionLabel.text
    property alias password: passwordTextField.text
    property bool newPassword: false
    readonly property bool canAccept: !newPassword
                                      || showCharactersCheckBox.checked
                                      || passwordTextField.text === repeatPasswordTextField.text

    standardButtons: canAccept ? Controls.Dialog.Ok
                                 | Controls.Dialog.Cancel : Controls.Dialog.Cancel
    title: qsTr("Enter password")
    onAccepted: {
        nativeInterface.password = password
        if (newPassword) {
            showPassiveNotification(
                        qsTr("The new password will be used when saving next time."))
        } else {
            nativeInterface.load()
        }
    }
    onRejected: {
        if (newPassword) {
            showPassiveNotification(
                        qsTr("You aborted. The password has not been altered."))
        }
    }

    ColumnLayout {
        Controls.Label {
            id: instructionLabel
            Layout.preferredWidth: passwordDialog.availableWidth
            wrapMode: Controls.Label.Wrap
        }

        Controls.TextField {
            id: passwordTextField
            Layout.preferredWidth: passwordDialog.availableWidth
            echoMode: showCharactersCheckBox.checked ? TextInput.Normal : TextInput.Password
            placeholderText: qsTr("enter password here, leave empty for no encryption")
            color: "#101010"
            placeholderTextColor: "#505050"
            background: Rectangle {
                border.color: "#5d5e6d"
            }
            Keys.onPressed: passwordDialog.acceptOnReturn(event)
        }
        Controls.TextField {
            id: repeatPasswordTextField
            Layout.preferredWidth: passwordDialog.availableWidth
            visible: passwordDialog.newPassword
                     && !showCharactersCheckBox.checked
            echoMode: TextInput.Password
            placeholderText: qsTr("repeat password")
            color: "#101010"
            placeholderTextColor: "#505050"
            background: Rectangle {
                border.color: passwordDialog.canAccept ? "#089900" : "#ff0000"
            }
            Keys.onPressed: passwordDialog.acceptOnReturn(event)
        }
        Controls.CheckBox {
            id: showCharactersCheckBox
            text: qsTr("Show characters")
            checked: false
        }
    }

    function clear() {
        passwordTextField.text = ""
        repeatPasswordTextField.text = ""
    }

    function askForPassword(instruction) {
        this.instruction = instruction
        clear()
        open()
        passwordTextField.forceActiveFocus()
    }

    function askForExistingPassword(instruction) {
        newPassword = false
        askForPassword(instruction)
    }

    function askForNewPassword(instruction) {
        newPassword = true
        askForPassword(instruction)
    }
}
