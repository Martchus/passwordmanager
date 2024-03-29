package org.martchus.passwordmanager;

import android.content.Intent;
import android.content.ActivityNotFoundException;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import androidx.documentfile.provider.DocumentFile;
import java.io.FileNotFoundException;
import org.qtproject.qt.android.bindings.QtActivity;

public class Activity extends QtActivity {
    private final int REQUEST_CODE_OPEN_EXISTING_FILE = 1;
    private final int REQUEST_CODE_CREATE_NEW_FILE = 2;
    private final int REQUEST_CODE_SAVE_FILE_AS = 3;

    /*!
     * \brief Shows the native Android file dialog. Results are handled in onActivityResult().
     */
    public boolean showAndroidFileDialog(boolean existing, boolean createNew) {
        String action = existing ? Intent.ACTION_OPEN_DOCUMENT : Intent.ACTION_CREATE_DOCUMENT;
        int requestCode = existing ? REQUEST_CODE_OPEN_EXISTING_FILE : (createNew ? REQUEST_CODE_CREATE_NEW_FILE : REQUEST_CODE_SAVE_FILE_AS);
        Intent intent = new Intent(action);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        try {
            startActivityForResult(intent, requestCode);
            return true;
        } catch (ActivityNotFoundException e) {
            return false;
        }
    }

    /*!
     * \brief Opens a native file descriptor for the specified \a contentUri (content://...) with the specified \a mode.
     * \returns Returns the file descriptor or -1 on error.
     * \remarks \a mode must be "r" and/or "w" for read and/or write. Appending "t" for truncate is possible as well.
     */
    public int openFileDescriptorFromAndroidContentUri(String contentUri, String mode) {
        try {
            DocumentFile file = DocumentFile.fromSingleUri(this, Uri.parse(contentUri));
            ParcelFileDescriptor fd = getContentResolver().openFileDescriptor(file.getUri(), mode);
            return fd.detachFd();
        } catch (FileNotFoundException e) {
            return -1;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
        case REQUEST_CODE_OPEN_EXISTING_FILE:
        case REQUEST_CODE_CREATE_NEW_FILE:
        case REQUEST_CODE_SAVE_FILE_AS:
            boolean createNew = requestCode == REQUEST_CODE_CREATE_NEW_FILE;
            boolean existingFile = requestCode == REQUEST_CODE_OPEN_EXISTING_FILE;
            boolean saveAs = requestCode == REQUEST_CODE_SAVE_FILE_AS;

            if (resultCode != RESULT_OK) {
                onAndroidFileDialogRejected();
                return;
            }

            Uri uri = data.getData();
            if (uri != null) {
                try {
                    DocumentFile file = DocumentFile.fromSingleUri(this, uri);
                    ParcelFileDescriptor fd = getContentResolver().openFileDescriptor(file.getUri(), existingFile ? "r" : "wt");
                    onAndroidFileDialogAcceptedDescriptor(file.getUri().toString(), file.getName(), fd.detachFd(), existingFile, createNew);
                } catch (FileNotFoundException e) {
                    onAndroidError("Failed to find selected file.");
                }
                return;
            }

            String fileName = data.getDataString();
            if (fileName != null) {
                onAndroidFileDialogAccepted(fileName, existingFile, createNew);
                return;
            }
            onAndroidError("Failed to read result from Android's file dialog.");
            return;
        }

        super.onActivityResult(requestCode, resultCode, data);
    }

    public static native void onAndroidError(String message);
    public static native void onAndroidFileDialogAccepted(String fileName, boolean existing, boolean createNew);
    public static native void onAndroidFileDialogAcceptedDescriptor(String nativeUrl, String fileName, int fileDescriptor, boolean existing, boolean createNew);
    public static native void onAndroidFileDialogRejected();
}
