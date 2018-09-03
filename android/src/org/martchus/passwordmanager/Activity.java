package org.martchus.passwordmanager;

import android.content.Intent;
import android.content.ActivityNotFoundException;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.support.v4.provider.DocumentFile;
import java.io.FileNotFoundException;
import org.qtproject.qt5.android.bindings.QtActivity;

public class Activity extends QtActivity {
    private final int REQUEST_CODE_PICK_DIR = 1;
    private final int REQUEST_CODE_PICK_EXISTING_FILE = 2;
    private final int REQUEST_CODE_PICK_NEW_FILE = 3;

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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
        case REQUEST_CODE_PICK_EXISTING_FILE:
        case REQUEST_CODE_PICK_NEW_FILE:
            if (resultCode != RESULT_OK) {
                onAndroidFileDialogRejected();
                return;
            }
            boolean existingFile = requestCode == REQUEST_CODE_PICK_EXISTING_FILE;
            Uri uri = data.getData();
            if (uri != null) {
                try {
                    DocumentFile file = DocumentFile.fromSingleUri(this, uri);
                    ParcelFileDescriptor fd = getContentResolver().openFileDescriptor(file.getUri(), existingFile ? "rw" : "wt");
                    onAndroidFileDialogAcceptedHandle(uri.toString(), fd.getFd(), existingFile);
                } catch (FileNotFoundException e) {
                    onAndroidError("Failed to open selected file.");
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
    public static native void onAndroidFileDialogAcceptedHandle(String fileName, int fileHandle, boolean existing);
    public static native void onAndroidFileDialogRejected();
}
