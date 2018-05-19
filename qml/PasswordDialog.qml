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
            background: Rectangle {
                border.color: "#5d5e6d"
            }
        }
        Controls.TextField {
            id: repeatPasswordTextField
            Layout.preferredWidth: passwordDialog.availableWidth
            visible: passwordDialog.newPassword
                     && !showCharactersCheckBox.checked
            echoMode: TextInput.Password
            placeholderText: qsTr("repeat password")
            background: Rectangle {
                border.color: passwordDialog.canAccept ? "#089900" : "#ff0000"
            }
        }
        Controls.CheckBox {
            id: showCharactersCheckBox
            text: qsTr("Show characters")
            checked: false
        }
    }

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

    function clear() {
        passwordTextField.text = ""
        repeatPasswordTextField.text = ""
    }

    function askForPassword(instruction, newPassword) {
        this.newPassword = newPassword
        this.instruction = instruction
        this.clear()
        this.open()
    }

    function askForExistingPassword(instruction) {
        this.askForPassword(instruction, false)
    }

    function askForNewPassword(instruction) {
        this.askForPassword(instruction, true)
    }
}
