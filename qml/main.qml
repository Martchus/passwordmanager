import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import martchus.passwordmanager 2.0

import "./pages" as Pages
import "./touch" as Touch

ApplicationWindow {
    id: root
    width: 700
    height: 800
    title: qsTr("Password Manager")
    visible: true

    property string statusBarMessage

    property Component startPage: Pages.StartPage {
        onUpdateStatusBar: statusBarMessage = message
        onNextPage: if (!isLocked) {
                        pageView.push(accountsPage)
                        clearSearchBox()
                    }
    }
    property Component accountsPage: Pages.AccountsPage {
        onUpdateStatusBar: statusBarMessage = message
        onNextPage: if (!isLocked) {
                        pageView.push(fieldsPage)
                    }
        onPreviousPage: {
            pageView.pop()
        }
    }
    property Component fieldsPage: Pages.FieldsPage {
        onUpdateStatusBar: statusBarMessage = message
        onNextPage: if (!isLocked) {
                        pageView.pop()
                    }
        onPreviousPage: {
            pageView.pop()
        }
    }

    StackView {
        id: pageView
        anchors.fill: parent
        focus: true
        Keys.onReleased: {
            if (event.key === Qt.Key_Back ||
                    (event.key === Qt.Key_Left && (event.modifiers & Qt.AltModifier))) {
                if (pageView.depth > 1) {
                    event.accepted = true
                    if (!currentItem.isLocked)
                        currentItem.previousPage()
                } else {
                    if (!currentItem.hasNoSearchText) {
                        event.accepted = true
                        currentItem.clearSearchBox()
                    }
                }
            }
        }
        initialItem: startPage
        delegate: StackViewDelegate {
            pushTransition: StackViewTransition {
                function transitionFinished(properties)
                {
                    properties.exitItem.opacity = 1
                }
                PropertyAnimation {
                    target: enterItem
                    property: "x"
                    from: target.width
                    to: 0
                    duration: 500
                    easing.type: Easing.OutSine
                }
                PropertyAnimation {
                    target: exitItem
                    property: "x"
                    from: 0
                    to: -target.width
                    duration: 500
                    easing.type: Easing.OutSine
                }
            }
            popTransition: StackViewTransition {
                function transitionFinished(properties)
                {
                    properties.exitItem.opacity = 1
                }
                PropertyAnimation {
                    target: enterItem
                    property: "x"
                    from: -target.width
                    to: 0
                    duration: 500
                    easing.type: Easing.OutSine
                }
                PropertyAnimation {
                    target: exitItem
                    property: "x"
                    from: 0
                    to: target.width
                    duration: 500
                    easing.type: Easing.OutSine
                }
            }
            property Component replaceTransition: pushTransition
        }
    }
    statusBar: StatusBar {
        id: statusbar
        width: parent.width
        opacity: label.text !== "" ? 1 : 0
        property real statusBarHeight: 65 * ApplicationInfo.ratio
        height: label.text !== "" ? statusBarHeight : 0
        Behavior on height { NumberAnimation {easing.type: Easing.OutSine}}
        Behavior on opacity { NumberAnimation {}}
        style: StatusBarStyle {
            padding { left: 0; right: 0 ; top: 0 ; bottom: 0}
            property Component background: Rectangle {
                implicitHeight: 65 * ApplicationInfo.ratio
                implicitWidth: root.width
                color: ApplicationInfo.colors.smokeGray
                Rectangle {
                    width: parent.width
                    height: 1
                    color: Qt.darker(parent.color, 1.5)
                }
                Rectangle {
                    y: 1
                    width: parent.width
                    height: 1
                    color: "white"
                }
            }
        }
        Touch.TouchLabel {
            id: label
            y: 32 * ApplicationInfo.ratio - height/2
            width: parent.width // The text will only wrap if an explicit width has been set
            text: statusBarMessage
            textFormat: Text.RichText
            onLinkActivated: Qt.openUrlExternally(link)
            wrapMode: Text.Wrap
            pixelSize: 18
            letterSpacing: -0.15
            color: ApplicationInfo.colors.mediumGray
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            function decreaseFontSizeOnNarrowScreen() {
                if (label.implicitHeight > statusbar.statusBarHeight)
                    pixelSize = Math.floor(pixelSize * statusbar.statusBarHeight/label.implicitHeight)
            }
            onTextChanged: {
                if (text === "")
                    pixelSize = 18
                else
                    decreaseFontSizeOnNarrowScreen()
            }
            onWidthChanged: decreaseFontSizeOnNarrowScreen()
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("Application")
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
    }
}
