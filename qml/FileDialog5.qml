import QtQuick.Dialogs 1.3

FileDialog {
    id: fileDialog
    property bool createNewFile: false
    title: selectExisting ? qsTr("Select an existing file") : (saveAs ? qsTr("Select path to save file") : qsTr("Select path for new file"))
    onAccepted: {
        if (fileUrls.length < 1) {
            return
        }
        nativeInterface.handleFileSelectionAccepted(fileUrls[0], "",
                                                    this.selectExisting,
                                                    this.createNewFile)
    }
    onRejected: nativeInterface.handleFileSelectionCanceled()

    function show() {
        if (nativeInterface.showNativeFileDialog(this.selectExisting, this.createNewFile)) {
            return
        }
        // fallback to the Qt Quick file dialog if a native implementation is not available
        this.open()
    }
    function openExisting() {
        this.selectExisting = true
        this.createNewFile = false
        this.show()
    }
    function createNew() {
        this.selectExisting = false
        this.createNewFile = true
        this.show()
    }
    function saveAs() {
        this.selectExisting = false
        this.createNewFile = false
        this.show()
    }
}
