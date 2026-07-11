import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

Page {
    id: page
    property var main: undefined

    title: nativeInterface.currentAccountName

    property list<Action> actions: [
        Action {
            icon.name: "list-add-symbolic"
            text: qsTr("Add field")
            property bool visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: {
                const delegateModel = fieldsListView.model
                const row = delegateModel.rowCount() - 1
                fieldDialog.init(delegateModel, row)
                fieldDialog.open()
            }
        }
    ]

    // dialog to edit certain field
    BasicDialog {
        id: fieldDialog
        property int entryIndex: -1
        property alias fieldName: fieldNameEdit.text
        property alias fieldValue: fieldValueEdit.text
        property alias isPassword: fieldIsPasswordCheckBox.checked
        title: qsTr("Edit field of %1").arg(nativeInterface.currentAccountName)
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            var column0 = fieldsListView.model.index(entryIndex, 0)
            var column1 = fieldsListView.model.index(entryIndex, 1)
            fieldsListView.model.setData(column0, fieldName)
            fieldsListView.model.setData(column1, fieldValue)
            fieldsListView.model.setData(column0, isPassword ? 1 : 0,
                                         0x0100 + 1)
        }
        contentItem: ColumnLayout {
            GridLayout {
                Layout.preferredWidth: fieldDialog.availableWidth
                columns: 3
                columnSpacing: 0

                TextField {
                    id: fieldNameEdit
                    Layout.fillWidth: true
                    text: fieldDialog.fieldName
                    Keys.onPressed: (event) => fieldDialog.acceptOnReturn(event)
                }
                RoundButton {
                    flat: true
                    icon.name: "username-copy"
                    Layout.preferredWidth: height
                    Layout.columnSpan: 2
                    onClicked: {
                        nativeInterface.copyToClipboard(fieldNameEdit.text)
                        showPassiveNotification(qsTr("Copied field name"))
                    }
                }
                TextField {
                    id: fieldValueEdit
                    property bool hideCharacters: fieldDialog.isPassword
                                                  && !showCharactersCheckBox.checked

                    Layout.fillWidth: true
                    // ensure height is always the same, regardless of echo mode (under Android the
                    // bullet points for PasswordEchoOnEdit have a different size causing a different
                    // height)
                    Layout.preferredHeight: fieldNameEdit.height
                    text: fieldDialog.fieldValue
                    echoMode: hideCharacters ? TextInput.PasswordEchoOnEdit : TextInput.Normal
                    // fix ugly bullet points under Android
                    font.pointSize: hideCharacters ? fieldNameEdit.font.pointSize
                                                     * 0.5 : fieldNameEdit.font.pointSize
                    Keys.onPressed: (event) => fieldDialog.acceptOnReturn(event)
                }
                RoundButton {
                    flat: true
                    icon.name: "password-copy"
                    Layout.preferredWidth: height
                    onClicked: {
                        nativeInterface.copyToClipboard(fieldValueEdit.text)
                        showPassiveNotification(
                                    fieldDialog.isPassword ? qsTr("Copied password") : qsTr(
                                                                 "Copied value"))
                    }
                }
                RoundButton {
                    flat: true
                    icon.name: "clock-symbolic"
                    Layout.preferredWidth: height
                    visible: fieldValueEdit.text.startsWith("otpauth:")
                    onClicked: showPassiveNotification(nativeInterface.copyTOTP(fieldValueEdit.text))
                }
            }
            GridLayout {
                Layout.preferredWidth: fieldDialog.availableWidth
                columns: width > 350 ? 2 : 1
                CheckBox {
                    id: fieldIsPasswordCheckBox
                    text: qsTr("Mark as password")
                    checked: false
                }
                CheckBox {
                    id: showCharactersCheckBox
                    text: qsTr("Show characters")
                    checked: false
                    visible: fieldDialog.isPassword
                }
            }
        }

        function init(model, index) {
            entryIndex = index
            fieldName = model.key ? model.key : ""
            fieldValue = model.actualValue ? model.actualValue : ""
            isPassword = model.isPassword ? true : false
        }
    }

    // list view to edit the currently selected account
    CustomListView {
        id: fieldsListView
        anchors.fill: parent
        model: nativeInterface.fieldModel
        reuseItems: true
        moveDisplaced: Transition {
            YAnimator {
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }
        delegate: FieldsDelegate {
            width: fieldsListView.width
            view: fieldsListView
            onMoveRequested:
                (oldIndex, newIndex) => {
                    const model = fieldsListView.model
                    const invalidIndex = model.index(-1, 0)
                    model.moveRows(invalidIndex, oldIndex, 1, invalidIndex, newIndex)
                }
        }
    }
}
