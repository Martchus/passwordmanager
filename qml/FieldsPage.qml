import QtQuick 2.15
import QtQuick.Layouts 1.2
import QtQml.Models 2.2
import QtQuick.Controls 2.4 as Controls
import org.kde.kirigami 2.5 as Kirigami

Kirigami.ScrollablePage {
    id: page
    property var main: undefined

    Layout.fillWidth: true
    title: nativeInterface.currentAccountName
    actions:[
        Kirigami.Action {
            icon.name: "list-add"
            text: qsTr("Add field")
            visible: !nativeInterface.hasEntryFilter
            enabled: !nativeInterface.hasEntryFilter
            onTriggered: {
                const delegateModel = fieldsListView.model
                const row = delegateModel.rowCount() - 1
                fieldDialog.init(delegateModel, row)
                fieldDialog.open()
            }
            shortcut: "Ctrl+Shift+A"
        }
    ]
    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

    // dialog to edit certain field
    BasicDialog {
        id: fieldDialog
        property int entryIndex: -1
        property alias fieldName: fieldNameEdit.text
        property alias fieldValue: fieldValueEdit.text
        property alias isPassword: fieldIsPasswordCheckBox.checked
        title: qsTr("Edit field of %1").arg(nativeInterface.currentAccountName)
        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel

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
                columns: 2
                columnSpacing: 0

                Controls.TextField {
                    id: fieldNameEdit
                    Layout.fillWidth: true
                    text: fieldDialog.fieldName
                    Keys.onPressed: (event) => fieldDialog.acceptOnReturn(event)
                }
                Controls.RoundButton {
                    flat: true
                    icon.name: "username-copy"
                    Layout.preferredWidth: height
                    onClicked: {
                        nativeInterface.copyToClipboard(fieldNameEdit.text)
                        showPassiveNotification(qsTr("Copied field name"))
                    }
                }
                Controls.TextField {
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
                Controls.RoundButton {
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
            }
            RowLayout {
                Layout.preferredWidth: fieldDialog.availableWidth
                Controls.CheckBox {
                    id: fieldIsPasswordCheckBox
                    text: qsTr("Mark as password")
                    checked: false
                }
                Controls.CheckBox {
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
    ListView {
        id: fieldsListView
        implicitWidth: Kirigami.Units.gridUnit * 30
        model: nativeInterface.fieldModel
        reuseItems: true
        moveDisplaced: Transition {
            YAnimator {
                duration: Kirigami.Units.longDuration
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
