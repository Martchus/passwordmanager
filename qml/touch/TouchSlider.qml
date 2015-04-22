import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import martchus.passwordmanager 2.0

Slider {
    id: slider
    property real sliderHandleHeight: 0.
    property real sliderHandleWidth: 0.
    implicitHeight: sliderHandleHeight + ApplicationInfo.ratio * 25
    style: SliderStyle {
        groove: Rectangle {
            Rectangle {
                id: beforeHandle
                width: styleData.handlePosition
                height: 20 * ApplicationInfo.ratio
                color: ApplicationInfo.colors.blue
                radius: 90
                z: -1
            }
            Rectangle {
                id: afterHandle
                anchors.left: beforeHandle.right
                anchors.right: parent.right
                height: 20 * ApplicationInfo.ratio
                color: ApplicationInfo.colors.darkGray
                radius: 90
                z: -1
            }
        }
        handle: Item {
            width: sliderHandleWidth
            height: sliderHandleHeight
            Image {
                anchors.centerIn: parent
                source: ApplicationInfo.imagePath(control.pressed ? "Pointer_pressed.png" : "Pointer.png")
                width: sliderHandleWidth + 16 * ApplicationInfo.ratio
                height: sliderHandleHeight + 16 * ApplicationInfo.ratio
            }
        }
    }
}
