# Password Manager
A simple password manager with Qt 5 GUI and command-line interface using AES-256-CBC encryption via OpenSSL.

## Features
* Cross-platform: tested under GNU/Linux, Android and Windows
* Qt Widgets GUI for desktop platforms
* Qt Quick GUI (using Qt Quick Controls 2 and Kirigami 2) for mobile platforms
* Interactive command-line interface
* Simple architecture: All data is stored in ordinary files with AES-256-CBC applied. No cloud stuff. Use
  eg. Syncthing for synchronization.

## Covered C++/Qt topics
I've mainly started this project to learn C++ and Qt programming. So beside the mentioned features this project
and the underlying libraries serve as an example project covering some interesting C++/Qt topics:

* Basic use of Qt Widgets, Qt Quick and Kirigami 2
* Creating custom Qt models
    * Nested model and model with multiple columns
    * Support Drag & Drop in `QTreeView`
    * Support re-ordering in Qt Quick `ListView`
    * Use nested and table model within Qt QML
    * Integration with Qt Widgets' undo/redo framework
    * Filtering
* Android tweaks
    * Add CMake target to invoke `androiddeployqt`
    * Customize activity
    * Customize gradle project to add additional Java dependency
    * Adjust the window style of the activity
    * Call Java function from C++ and vice verca
    * Show native file dialog
    * Open `content://` URL with `std::iostream`
* Windows specific issues
    * Open an `std::iostream` for files which have non-ASCII characters in the path
* Use of zlib to (de)compress buffer
* Use of OpenSSL for symmetric (de)cryption

## Download / binary repository
I currently provide packages for Arch Linux and Windows. Sources for those packages can be found in a
separate [repository](https://github.com/Martchus/PKGBUILDs). For binaries checkout the release section
on GitHub or my [website](http://martchus.no-ip.biz/website/page.php?name=programming).

## Build instructions
The Password Manager depends on c++utilities and passwordfile. Checkout the README of c++utilities for more details. Note that this project is not built differently than any other CMake project.

### Optional dependencies
* When building any Qt GUI, the library qtutilities is required.
* When building with Qt Widgets GUI support, the following Qt modules are required: core gui widgets
* When building with support for the experimental Qt Quick GUI, the following Qt/KDE modules are required: core gui qml quick quickcontrols2 kirigami

### Building this straight
1. Install (preferably the latest version of) g++ or clang, the required Qt 5 modules and CMake. OpenSSL, iconv and
   zlib are required as well but likely already installed.
2. Get the sources of additional dependencies and the password manager itself. For the lastest version from Git clone the following repositories:  
   ```
   cd $SOURCES
   git clone https://github.com/Martchus/cpp-utilities.git c++utilities
   git clone https://github.com/Martchus/passwordfile.git
   git clone https://github.com/Martchus/qtutilities.git                  # only required for Qt GUI
   git clone https://github.com/Martchus/passwordmanager.git
   git clone https://github.com/Martchus/subdirs.git
   ```
3. Build and install everything in one step:  
   ```
   cd $BUILD_DIR
   cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="/install/prefix" \
    $SOURCES/subdirs/passwordmanager
   make install -j$(nproc)
   ```

#### Concrete example of 3. for building an Android APK under Arch Linux
Create stuff for signing the package (remove `-DANDROID_APK_FORCE_DEBUG=ON` line in the CMake invocation to actually use this):
```
# locate keystore
keystore_dir=/path/to/keystore-dir
keystore_alias=$USER
keystore_url=$keystore_dir/$keystore_alias
#keystore_password=<password>

# create keystore (do only once)
pushd "$keystore_dir"
keytool -genkey -v -keystore keystore_alias.keystore -alias keystore_alias -keyalg RSA -validity 999999
keytool -importkeystore -srckeystore keystore_alias.keystore -destkeystore keystore_alias.keystore -deststoretype pkcs12 # FIXME: make this in one step
popd
```

Build c++utilities, passwordfile, qtutilities and passwordmanager in one step to create an Android APK for arm64-v8a:

```
# specify Android platform
_pkg_arch=aarch64
_android_arch=arm64-v8a
_android_api_level=22

# set project name
_reponame=passwordmanager
_pkgname=passwordmanager

# locate SDK, NDK and further libraries
android_sdk_root=${ANDROID_SDK_ROOT:-/opt/android-sdk}
android_ndk_root=${ANDROID_NDK_ROOT:-/opt/android-ndk}
build_tools_version=$(pacman -Q android-sdk-build-tools | sed 's/.* r\(.*\)-.*/\1/')
other_libs_root=/opt/android-libs/$_pkg_arch
other_libs_include=$other_libs_root/include
root="$android_ndk_root/sysroot;$other_libs_root"

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_SYSTEM_VERSION=$_android_api_level \
    -DCMAKE_ANDROID_ARCH_ABI=$_android_arch \
    -DCMAKE_ANDROID_NDK="$android_ndk_root" \
    -DCMAKE_ANDROID_SDK="$android_sdk_root" \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_INSTALL_PREFIX=$other_libs_root \
    -DCMAKE_PREFIX_PATH="$root" \
    -DCMAKE_FIND_ROOT_PATH="$root;$root/libs" \
    -Diconv_DYNAMIC_INCLUDE_DIR="$other_libs_include" \
    -Diconv_STATIC_INCLUDE_DIR="$other_libs_include" \
    -Dcrypto_DYNAMIC_INCLUDE_DIR="$other_libs_include" \
    -Dcrypto_STATIC_INCLUDE_DIR="$other_libs_include" \
    -Dboost_iostreams_DYNAMIC_INCLUDE_DIR="$other_libs_include" \
    -Dboost_iostreams_STATIC_INCLUDE_DIR="$other_libs_include" \
    -DCLANG_FORMAT_ENABLED=ON \
    -DUSE_NATIVE_FILE_BUFFER=ON \
    -DNO_DOXYGEN=ON \
    -DWIDGETS_GUI=OFF \
    -DQUICK_GUI=ON \
    -DBUILTIN_ICON_THEMES=breeze \
    -DBUILTIN_TRANSLATIONS=ON \
    -DANDROID_APK_FORCE_DEBUG=ON \
    -DANDROID_APK_KEYSTORE_URL="$keystore_url" \
    -DANDROID_APK_KEYSTORE_ALIAS="$keystore_alias" \
    -DANDROID_APK_KEYSTORE_PASSWORD="$keystore_password" \
    $SOURCES/subdirs/$_reponame

make passwordmanager_apk -j$(nproc)
make passwordmanager_deploy_apk # install app on USB-connected phone
```

##### Notes
* The Android packages for the dependencies Qt 5, iconv and OpenSSL are provided in my PKGBUILDs repo.
* The lastest Java I was able to use was version 8 (`jdk8-openjdk` package).

### Manual deployment of Android APK file
1. Find device ID: `adb devices`
2. Install App on phone: `adb -s <DEVICE_ID> install -r $BUILD_DIR/passwordmanager_build_apk/build/outputs/apk/passwordmanager_build_apk-debug.apk`
3. View log: `adb -s <DEVICE_ID> logcat`

### Building without Qt 5 GUI
It is possible to build without the GUI if only the CLI is needed. In this case no Qt dependencies (including qtutilities) are required.

To build without GUI, add the following parameters to the CMake call:
```
-DWIDGETS_GUI=OFF -DQUICK_GUI=OFF
```
