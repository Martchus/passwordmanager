import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

BasicDialog {
    id: passwordDialog
    property alias instruction: instructionLabel.text
    property alias password: passwordTextField.text
    property bool newPassword: false
    readonly property bool canAccept: !newPassword
                                      || showCharactersCheckBox.checked
                                      || passwordTextField.text === repeatPasswordTextField.text

    standardButtons: canAccept ? Dialog.Ok
                                 | Dialog.Cancel : Dialog.Cancel

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
    contentItem: ColumnLayout {
        Label {
            id: instructionLabel
            Layout.preferredWidth: passwordDialog.availableWidth
            wrapMode: Label.Wrap
        }

        TextField {
            id: passwordTextField
            Layout.preferredWidth: passwordDialog.availableWidth
            echoMode: showCharactersCheckBox.checked ? TextInput.Normal : TextInput.Password
            placeholderText: newPassword
                ? qsTr("Enter password here, leave empty for no encryption")
                : qsTr("Enter password here")
            Keys.onPressed: (event) => passwordDialog.acceptOnReturn(event)
        }
        TextField {
            id: repeatPasswordTextField
            Layout.preferredWidth: passwordDialog.availableWidth
            visible: passwordDialog.newPassword
            enabled: visible && !showCharactersCheckBox.checked
            echoMode: TextInput.Password
            placeholderText: qsTr("Repeat password")
            Keys.onPressed: passwordDialog.acceptOnReturn(event)
        }
        CheckBox {
            id: showCharactersCheckBox
            text: qsTr("Show characters")
            checked: false
        }
    }
    footer: DialogButtonBox {
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
