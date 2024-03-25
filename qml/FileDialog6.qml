import QtQuick.Dialogs 6.2 as Dialogs

Dialogs.FileDialog {
    id: fileDialog
    property bool createNewFile: false
    property bool selectExisting: fileMode !== Dialogs.FileDialog.SaveFile
    title: selectExisting ? qsTr("Select an existing file") : (!createNewFile ? qsTr("Select path to save file") : qsTr("Select path for new file"))
    onAccepted: {
        if (selectedFiles.length < 1) {
            return
        }
        nativeInterface.handleFileSelectionAccepted(selectedFiles[0], "",
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
        this.fileMode = Dialogs.FileDialog.OpenFile
        this.createNewFile = false
        this.show()
    }
    function createNew() {
        this.fileMode = Dialogs.FileDialog.SaveFile
        this.createNewFile = true
        this.show()
    }
    function saveAs() {
        this.fileMode = Dialogs.FileDialog.SaveFile
        this.createNewFile = false
        this.show()
    }
}
