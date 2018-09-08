package org.martchus.passwordmanager;

import android.content.Intent;
import android.content.ActivityNotFoundException;
import android.net.Uri;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.view.Window;
import android.view.WindowManager.LayoutParams;
import android.support.v4.provider.DocumentFile;
import java.io.FileNotFoundException;
import org.qtproject.qt5.android.bindings.QtActivity;

public class Activity extends QtActivity {
    private final int REQUEST_CODE_PICK_DIR = 1;
    private final int REQUEST_CODE_PICK_EXISTING_FILE = 2;
    private final int REQUEST_CODE_PICK_NEW_FILE = 3;

    /*!
     * \brief Shows the native Android file dialog. Results are handled in onActivityResult().
     */
    public boolean showAndroidFileDialog(boolean existing) {
        String action = existing ? Intent.ACTION_OPEN_DOCUMENT : Intent.ACTION_CREATE_DOCUMENT;
        int requestCode = existing ? REQUEST_CODE_PICK_EXISTING_FILE : REQUEST_CODE_PICK_NEW_FILE;
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

    public void applyTheming() {
        Window window = getWindow();
        window.addFlags(LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        window.clearFlags(LayoutParams.FLAG_TRANSLUCENT_STATUS);
        window.setStatusBarColor(0x000000FF);
        window.setNavigationBarColor(0x000000FF);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
        case REQUEST_CODE_PICK_EXISTING_FILE:
        case REQUEST_CODE_PICK_NEW_FILE:
            boolean existingFile = requestCode == REQUEST_CODE_PICK_EXISTING_FILE;

            if (resultCode != RESULT_OK) {
                onAndroidFileDialogRejected();
                return;
            }

            Uri uri = data.getData();
            if (uri != null) {
                try {
                    DocumentFile file = DocumentFile.fromSingleUri(this, uri);
                    ParcelFileDescriptor fd = getContentResolver().openFileDescriptor(file.getUri(), existingFile ? "r" : "wt");
                    onAndroidFileDialogAcceptedDescriptor(file.getUri().toString(), file.getName(), fd.detachFd(), existingFile);
                } catch (FileNotFoundException e) {
                    onAndroidError("Failed to find selected file.");
                }
                return;
            }

            String fileName = data.getDataString();
            if (fileName != null) {
                onAndroidFileDialogAccepted(fileName, existingFile);
                return;
            }
            onAndroidError("Failed to read result from Android's file dialog.");
            return;
        }

        super.onActivityResult(requestCode, resultCode, data);
    }

    public static native void onAndroidError(String message);
    public static native void onAndroidFileDialogAccepted(String fileName, boolean existing);
    public static native void onAndroidFileDialogAcceptedDescriptor(String nativeUrl, String fileName, int fileDescriptor, boolean existing);
    public static native void onAndroidFileDialogRejected();
}
