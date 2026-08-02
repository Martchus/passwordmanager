import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material

ApplicationWindow {
    id: root
    property var fieldsPage: undefined
    property real lastBackEvent: 0
    property list<Dialog> dialogs
    property list<Action> mainActions: [
        Action {
            text: qsTr("Open existing file")
            icon.name: "document-open-symbolic"
            onTriggered: fileDialog.openExisting()
        },
        Action {
            text: qsTr("About")
            icon.name: "help-about-symbolic"
            onTriggered: aboutDialog.open()
        }
    ]

    Material.primary: "#2c714a"
    Material.accent: "#2c8352"
    Material.theme: nativeInterface.darkModeEnabled ? Material.Dark : Material.Light

    width: 600
    height: 800
    visible: true
    flags: Qt.Window | Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
    leftPadding: 0
    rightPadding: 0

    Component.onCompleted: {
        nativeInterface.init()

        // handle global keyboard and mouse events
        root.contentItem.forceActiveFocus(Qt.ActiveWindowFocusReason);
        [root.contentItem, root.header].forEach(item => item.Keys.released.connect((event) => {
            const key = event.key
            if (key === Qt.Key_Back || (key === Qt.Key_Backspace && typeof root.activeFocusItem.getText !== "function")) {
                event.accepted = true
                if (root.dialogs.length) {
                    root.dialogs[root.dialogs.length - 1].close()
                } else if (pageStack.depth >= 2) {
                    pageStack.pop()
                } else {
                    const now = Date.now() / 1000
                    if ((now - root.lastBackEvent) < 2) {
                        root.close()
                    } else {
                        root.lastBackEvent = now
                        root.showPassiveNotification(qsTr("Tap again to quit"), 2000)
                    }
                }
            }
        }));
    }

    onActiveFocusItemChanged: {
        if (root.activeFocusItem?.toString().startsWith("QQuickPopupItem")) {
            root.contentItem.forceActiveFocus(Qt.ActiveWindowFocusReason);
        }
    }

    header: ToolBar {
        leftPadding: 0
        Material.theme: Material.Light
        Material.background: Material.primary
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: pageStack.anchors.leftMargin
            Material.theme: Material.Dark
            ToolButton {
                icon.name: pageStack.depth > 1 ? "go-previous-symbolic" : "application-menu-symbolic"
                onPressAndHold: nativeInterface.performHapticFeedback()
                onClicked: {
                    if (pageStack.depth > 1) {
                        pageStack.pop()
                    } else {
                        if (leftMenu.opened) {
                            leftMenu.close()
                        } else {
                            leftMenu.open()
                        }
                    }
                }
            }

            Label {
                text: pageStack.currentItem ? pageStack.currentItem.title : qsTr("Password Manager")
                Layout.fillWidth: true
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }

            Repeater {
                model: pageStack.currentItem ? pageStack.currentItem.actions : mainActions
                delegate: ToolButton {
                    action: modelData
                    visible: modelData.visible !== undefined ? modelData.visible : true
                    display: root.width < 500 ? AbstractButton.IconOnly : AbstractButton.TextBesideIcon
                    onPressAndHold: nativeInterface.performHapticFeedback()
                    ToolTip.visible: (hovered || pressed) && text.length > 0 && display === AbstractButton.IconOnly
                    ToolTip.text: text
                }
            }
        }
    }

    Drawer {
        id: leftMenu
        width: Math.min(parent.width * 0.8, 300)
        height: parent.height
        interactive: inPortrait || parent.width < 600
        modal: interactive
        position: initialPosition
        visible: !interactive

        readonly property bool inPortrait: parent.width < parent.height
        readonly property bool showNoPasswordWarning: nativeInterface.fileOpen
                                             && !nativeInterface.passwordSet
        readonly property double initialPosition: interactive ? 0 : 1
        readonly property int effectiveWidth: !interactive ? width : 0

        function closeIfInteractive() {
            return leftMenu.interactive && leftMenu.close();
        }

        CustomFlickable {
            anchors.fill: parent
            clip: true
            contentHeight: drawerLayout.height
            ColumnLayout {
                id: drawerLayout
                width: parent.width
                spacing: 10

                Label {
                    id: fileNameLabel
                    padding: 10
                    wrapMode: Text.Wrap
                    font.bold: true
                    font.pixelSize: 18
                    Layout.fillWidth: true
                    text: nativeInterface.fileOpen ? nativeInterface.fileName : qsTr("No file opened")

                    MouseArea {
                        id: fileNameMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                    }

                    ToolTip {
                        z: 1000
                        text: nativeInterface.filePath
                        visible: text ? fileNameMouseArea.containsMouse : false
                        delay: Qt.styleHints.mousePressAndHoldInterval
                        onAboutToShow: {
                            x = fileNameMouseArea.mouseX + 10
                            y = fileNameMouseArea.mouseY + 10
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.preferredHeight: filterTextField.implicitHeight
                    enabled: nativeInterface.fileOpen
                    visible: !nativeInterface.filterAsDialog
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10

                    SearchField {
                        id: filterTextField
                        anchors.fill: parent
                        //placeholderText: qsTr("Filter")
                        onTextChanged: nativeInterface.entryFilter = text
                        onClearButtonPressed: filterTextField.text = ""
                    }
                }

                RowLayout {
                    visible: leftMenu.showNoPasswordWarning
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10

                    Label {
                        text: qsTr("No password set\nFile will be saved unencrypted!")
                        color: Material ? Material.color(Material.red) : "red"
                    }
                }

                MenuSeparator {
                    Layout.fillWidth: true
                }

                // Actions represented as delegates
                ItemDelegate {
                    text: qsTr("Create new file")
                    icon.name: "document-new-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        fileDialog.createNew()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: qsTr("Open existing file")
                    icon.name: "document-open-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        fileDialog.openExisting()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    id: recentlyOpenedDelegate
                    text: qsTr("Recently opened ...")
                    icon.name: "document-open-recent-symbolic"
                    Layout.fillWidth: true
                    visible: nativeInterface.recentFiles.length > 0
                    onClicked: recentFilesColumn.visible = !recentFilesColumn.visible
                }

                ColumnLayout {
                    id: recentFilesColumn
                    visible: false
                    Layout.fillWidth: true
                    Layout.leftMargin: 20

                    Repeater {
                        model: nativeInterface.recentFiles
                        delegate: ItemDelegate {
                            text: modelData.substring(modelData.lastIndexOf('/') + 1)
                            icon.name: "document-open-symbolic"
                            Layout.fillWidth: true
                            onClicked: {
                                Qt.callLater(root.openRecentFile, modelData)
                                leftMenu.closeIfInteractive()
                            }
                        }
                    }

                    ItemDelegate {
                        text: qsTr("Clear recently opened files")
                        icon.name: "edit-clear-symbolic"
                        Layout.fillWidth: true
                        onClicked: {
                            nativeInterface.clearRecentFiles()
                            leftMenu.closeIfInteractive()
                        }
                    }
                }

                ItemDelegate {
                    text: nativeInterface.fileOpen ? qsTr("Save modifications") : qsTr("Save")
                    enabled: nativeInterface.fileOpen
                    icon.name: "document-save-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        nativeInterface.save()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: qsTr("Save as")
                    enabled: nativeInterface.fileOpen
                    icon.name: "document-save-as-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        fileDialog.saveAs()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: nativeInterface.passwordSet ? qsTr("Change password") : qsTr("Add password")
                    enabled: nativeInterface.fileOpen
                    icon.name: "document-encrypt-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        enterPasswordDialog.askForNewPassword(
                                         qsTr("Change password for %1").arg(
                                             nativeInterface.filePath))
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: qsTr("Details")
                    enabled: nativeInterface.fileOpen
                    icon.name: "document-properties-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        fileSummaryDialog.show()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: nativeInterface.entryFilter.length === 0 ? qsTr("Search") : qsTr("Adjust search")
                    enabled: nativeInterface.fileOpen
                    visible: nativeInterface.filterAsDialog
                    icon.name: "search-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        filterDialog.open()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: qsTr("Clear search")
                    enabled: nativeInterface.fileOpen
                    visible: nativeInterface.filterAsDialog && nativeInterface.entryFilter.length > 0
                    icon.name: "edit-clear-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        nativeInterface.entryFilter = ""
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: qsTr("Undo \"%1\"").arg(nativeInterface.undoText)
                    visible: nativeInterface.undoText.length !== 0 && nativeInterface.entryFilter.length === 0
                    enabled: visible
                    icon.name: "edit-undo-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        nativeInterface.undo()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: qsTr("Redo \"%1\"").arg(nativeInterface.redoText)
                    visible: nativeInterface.redoText.length !== 0 && nativeInterface.entryFilter.length === 0
                    enabled: visible
                    icon.name: "edit-redo-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        nativeInterface.redo()
                        leftMenu.closeIfInteractive()
                    }
                }

                ItemDelegate {
                    text: qsTr("Close file")
                    enabled: nativeInterface.fileOpen
                    icon.name: "document-close-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        nativeInterface.close()
                        leftMenu.closeIfInteractive()
                    }
                }

                MenuSeparator {
                    Layout.fillWidth: true
                }

                ItemDelegate {
                    text: qsTr("About")
                    icon.name: "help-about-symbolic"
                    Layout.fillWidth: true
                    onClicked: {
                        aboutDialog.open()
                        leftMenu.closeIfInteractive()
                    }
                }
            }
        }
    }

    StackView {
        id: pageStack
        anchors.fill: parent
        anchors.leftMargin: leftMenu.visible ? leftMenu.effectiveWidth : parent.SafeArea.margins.left
        anchors.rightMargin: parent.SafeArea.margins.right
        clip: true
    }

    AboutDialog {
        id: aboutDialog
    }

    PasswordDialog {
        id: enterPasswordDialog
        onAboutToShow: leftMenu.closeIfInteractive()
        onRejected: {
            if (!nativeInterface.fileOpen) {
                leftMenu.open()
            }
        }
    }

    BasicDialog {
        id: fileSummaryDialog
        standardButtons: Dialog.Ok
        title: qsTr("File details")
        contentItem: TextArea {
            id: fileSummaryLabel
            readOnly: true
            text: "No file summary available"
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            width: fileSummaryDialog.availableWidth
        }

        function show() {
            fileSummaryLabel.text = nativeInterface.computeFileSummary()
            this.open()
        }
    }

    FileDialog {
        id: fileDialog
    }

    BasicDialog {
        id: filterDialog
        title: qsTr("Search for categories and accounts")
        onAccepted: applyFilter()
        onApplied: applyFilter()
        onReset: resetFilter()
        onVisibleChanged: {
            if (visible) {
                filterDialogTextField.forceActiveFocus()
            }
        }
        footer: DialogButtonBox {
            standardButtons: filterDialogTextField.text.length > 0
                ? DialogButtonBox.Apply | DialogButtonBox.Reset | DialogButtonBox.Cancel
                : DialogButtonBox.Reset | DialogButtonBox.Cancel
        }
        contentItem: ColumnLayout {
            TextField {
                id: filterDialogTextField
                Layout.preferredWidth: filterDialog.availableWidth
                Keys.onPressed: filterDialog.acceptOnReturn(event)
            }
        }
        function applyFilter() {
            nativeInterface.entryFilter = filterDialogTextField.text
            close()
        }
        function resetFilter() {
            nativeInterface.entryFilter = ""
            close()
        }
    }

    Connections {
        target: nativeInterface
        function onEntryFilterChanged(newFilter) {
            if (filterTextField.text !== newFilter) {
                filterTextField.text = newFilter
            }
        }
        function onFileError(errorMessage, retryAction) {
            var retryMethod = null
            if (retryAction === "load" || retryAction === "save") {
                retryMethod = retryAction
            }
            if (retryMethod === null) {
                showPassiveNotification(errorMessage)
            } else {
                showPassiveNotification(errorMessage, 2500, qsTr("Retry"),
                                        function () {
                                            nativeInterface[retryMethod]()
                                        })
            }
        }
        function onSettingsError(errorMessage) {
            showPassiveNotification(errorMessage)
        }
        function onPasswordRequired(filePath) {
            enterPasswordDialog.askForExistingPassword(
                        qsTr("Password required to open %1").arg(
                            nativeInterface.filePath))
            leftMenu.closeIfInteractive()
        }
        function onFileOpenChanged(fileOpen) {
            clearStack()
            if (!nativeInterface.fileOpen) {
                showPassiveNotification(qsTr("%1 closed").arg(
                                            nativeInterface.fileName))
                return
            }
            initStack()
            showPassiveNotification(qsTr("%1 opened").arg(
                                        nativeInterface.fileName))
            leftMenu.closeIfInteractive()
        }
        function onFileSaved() {
            showPassiveNotification(qsTr("%1 saved").arg(
                                        nativeInterface.fileName))
        }
        function onNewNotification(message) {
            showPassiveNotification(message)
        }
        function onCurrentAccountChanged() {
            // remove the fields page if the current account has been removed
            if (!nativeInterface.hasCurrentAccount) {
                pageStack.pop(null)
            }
        }
        function onEntryAboutToBeRemoved(removedIndex) {
            // under TreeView model, the active fields page is handled by onCurrentAccountChanged
        }
        function onHasEntryFilterChanged(hasEntryFilter) {
            if (nativeInterface.fileOpen) {
                pageStack.clear()
                initStack()
            }
        }
    }

    Component {
        id: entriesComponent
        EntriesPage {
            main: root
        }
    }

    Component {
        id: fieldsComponent
        FieldsPage {
            main: root
        }
    }

    Shortcut {
        sequence: "Ctrl+M"
        onActivated: leftMenu.visible = !leftMenu.visible
    }

    function initStack() {
        pageStack.push(entriesComponent.createObject(root))
    }

    function clearStack() {
        pageStack.clear()
    }

    function pushAccountEdit() {
        // lazy-initialize fieldsPage
        if (!fieldsPage) {
            fieldsPage = fieldsComponent.createObject(root)
        }
        // remove fieldsPage if already shown to prevent warning
        if (pageStack.get(pageStack.depth - 1) === fieldsPage) {
            pageStack.pop()
        }
        pageStack.push(fieldsPage)
    }

    function openRecentFile(filePath) {
        nativeInterface.clear()
        nativeInterface.filePath = filePath
        nativeInterface.load()
        recentFilesColumn.visible = false
    }

    // passive notification (toast popup implementation)
    Popup {
        id: toastPopup
        parent: Overlay.overlay
        x: parent ? (parent.width - width) / 2 : 0
        y: parent ? (parent.height - height - 60) : 0
        padding: 12
        leftPadding: 16
        rightPadding: 16
        width: Math.min(parent ? parent.width - 40 : 360, 400)
        modal: false
        focus: false
        closePolicy: Popup.NoAutoClose

        background: Rectangle {
            color: nativeInterface.darkModeEnabled ? "#333333" : "#f0f0f0"
            radius: 10
            border.color: nativeInterface.darkModeEnabled ? "#555555" : "#cccccc"
            border.width: 1
        }

        contentItem: RowLayout {
            spacing: 12

            Label {
                id: toastLabel
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                font.pixelSize: 13
                font.weight: Font.Medium
                color: nativeInterface.darkModeEnabled ? "#ffffff" : "#333333"
                verticalAlignment: Text.AlignVCenter
            }
            Button {
                id: toastActionButton
                visible: false
                flat: true
                Layout.alignment: Qt.AlignVCenter
                onClicked: {
                    toastPopup.close()
                    if (toastPopup.actionCallback) {
                        toastPopup.actionCallback()
                    }
                }
            }
        }

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 200; easing.type: Easing.OutCubic }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 200; easing.type: Easing.InCubic }
        }

        Timer {
            id: toastTimer
            interval: 3000
            onTriggered: toastPopup.close()
        }

        property var actionCallback: null

        function show(message, duration, actionText, actionCallbackFunc) {
            toastLabel.text = message
            if (actionText && actionCallbackFunc) {
                toastActionButton.text = actionText
                toastActionButton.visible = true
                toastPopup.actionCallback = actionCallbackFunc
            } else {
                toastActionButton.visible = false
                toastPopup.actionCallback = null
            }
            toastTimer.interval = duration ? duration : 3000
            toastTimer.restart()
            toastPopup.open()
        }
    }

    function showPassiveNotification(message, duration, actionText, actionCallbackFunc) {
        return (!actionText && nativeInterface.showToast(message)) || toastPopup.show(message, duration, actionText, actionCallbackFunc)
    }
    function openFilterDialog() {
        filterDialog.open()
    }
}
