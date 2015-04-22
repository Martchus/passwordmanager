import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import martchus.passwordmanager 2.0

import "../touch" as Touch

Item {
    id: page
    signal updateStatusBar(string message)
    signal nextPage
    signal previousPage
    signal clearSearchBox

    property Component pageComponent
    property bool isLocked: Stack.status !== Stack.Active
    property string title1
    property string title2
    property string title3

    property alias searchText: searchField.text
    property alias hasNoSearchText: searchField.isEmpty
    signal searchBoxReturn
    property string statusBarMessageDefault

    property alias topRect: topRect
    property color topRectColor: ApplicationInfo.colors.yetAnotherBlue

    property bool searchFieldVisisble: false

    Binding {
        target: ApplicationInfo
        property: "isPortraitMode"
        value: page.height > page.width
        when: !ApplicationInfo.isMobile
    }

    Binding {
        target: ApplicationInfo
        property: "applicationWidth"
        value: page.width
    }

    Rectangle {
        id: topRect
        z: 2 // so flickable doesn't draw on top
        anchors.top: parent.top
        height: 80 * ApplicationInfo.ratio
        width: parent.width
        color: page.Stack.index !== 0 && mouseBack.pressed ?
                   Qt.lighter(topRectColor, 1.2) : topRectColor
        Rectangle {
            color: Qt.lighter(parent.color, 1.2)
            height: 1
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 1
            width: parent.width
        }
        Rectangle {
            z: 2 // so flickable doesn't draw on top
            height: 1
            width: parent.width
            color: Qt.darker(ApplicationInfo.colors.blue, 1.6)
            anchors.bottom: parent.bottom
        }

        RowLayout {
            id: titleRow
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 0
            anchors.verticalCenter: parent.verticalCenter
            Separator {
                Layout.fillWidth: false
                Layout.preferredWidth: ApplicationInfo.hMargin
            }
            Image {
                source: ApplicationInfo.imagePath("backarrow.png")
                Layout.preferredWidth: 22 * ApplicationInfo.ratio
                Layout.preferredHeight: 35 * ApplicationInfo.ratio
                visible: page.Stack.index > 0
                Accessible.role: Accessible.Button
                Accessible.name: qsTr("Back")
                function accessiblePressAction () { backPressed() }
            }
            Rectangle {
                opacity: 0
                Layout.preferredWidth: 20 * ApplicationInfo.ratio
                Layout.fillHeight: true
                visible: page.Stack.index > 0
            }
            Touch.TouchLabel {
                id: t1
                text: title1 + " "
                color: ApplicationInfo.colors.white
                pixelSize: 30
                font.weight: Font.Bold
                Layout.maximumWidth: ApplicationInfo.applicationWidth - t3.implicitWidth - 2 * ApplicationInfo.hMargin - 5 * ApplicationInfo.ratio - 42 * ApplicationInfo.ratio
                Layout.alignment: Qt.AlignBaseline
            }
            Touch.TouchLabel {
                text: "- " + title2
                color: ApplicationInfo.colors.white
                visible: title2 !== ""
                pixelSize: 22
                letterSpacing: -0.15
                Layout.alignment: Qt.AlignBaseline
                Layout.maximumWidth: freeSpace > implicitWidth ? freeSpace : 0
                property real freeSpace: ApplicationInfo.applicationWidth - t1.width - t3.implicitWidth - 2 * ApplicationInfo.hMargin - 5 * ApplicationInfo.ratio - 42 * ApplicationInfo.ratio
            }
            Item {
                Layout.fillWidth: true
                height: 0
            }
            Touch.TouchLabel {
                id: t3
                text: title3
                color: ApplicationInfo.colors.white
                visible: title3 !== ""
                pixelSize: 22
                letterSpacing: -0.15
                Layout.alignment: Qt.AlignBaseline
            }
            Separator {
                Layout.fillWidth: false
                Layout.preferredWidth: ApplicationInfo.hMargin
            }
        }
        Rectangle {
            width: parent.width
            height: 5
            anchors.top: parent.bottom
            gradient: Gradient {
                GradientStop {position: 0 ; color: "#40000000"}
                GradientStop {position: 1 ; color: "#00000000"}
            }
        }
        MouseArea {
            id: mouseBack
            anchors.fill: parent
            onClicked: backPressed()
        }
    }

    function backPressed() {
        if (!isLocked) page.previousPage()
    }

    Touch.TouchTextField {
        id: searchField
        z: 2
        visible: searchFieldVisisble
        anchors.right: topRect.right
        anchors.top: topRect.top
        anchors.bottom: topRect.bottom
        width: ApplicationInfo.isPortraitMode ?
                   parent.width - t1.implicitWidth - ApplicationInfo.ratio * 50 :
                   parent.width/2.5
        anchors.leftMargin: topRect.width/2
        anchors.rightMargin: 20 * ApplicationInfo.ratio
        anchors.margins: 12 * ApplicationInfo.ratio

        placeholderText: qsTr("filter")
        Layout.fillWidth: true
        Keys.onReturnPressed: page.searchBoxReturn()
        Keys.onEnterPressed: page.searchBoxReturn()
        onClearButtonClicked: page.clearSearchBox()
    }

    Loader {
        sourceComponent: pageComponent
        anchors.top: topRect.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        Rectangle {
            z: -1
            anchors.fill: parent
            color: ApplicationInfo.colors.white
        }
    }
}
