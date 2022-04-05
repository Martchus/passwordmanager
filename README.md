# Password Manager
A simple [password manager](https://en.wikipedia.org/wiki/Password_manager) with Qt GUI and command-line interface using AES-256-CBC encryption via OpenSSL.

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

Note that some of the mentioned points are actually implemented the underlying libraries
[c++utilities](http://github.com/Martchus/cpp-utilities), [qtutilities](http://github.com/Martchus/qtutilities)
and [passwordfile](http://github.com/Martchus/passwordfile).

## Download
### Source
See the release section on GitHub.

### Packages and binaries
* Arch Linux
    * for PKGBUILDs checkout [my GitHub repository](https://github.com/Martchus/PKGBUILDs) or
      [the AUR](https://aur.archlinux.org/packages?SeB=m&K=Martchus)
    * there is also a [binary repository](https://martchus.no-ip.biz/repo/arch/ownstuff)
* Tumbleweed, Leap, Fedora
    * RPM \*.spec files and binaries are available via openSUSE Build Service
        * remarks
            * Be sure to add the repository that matches the version of your OS and to keep it
              in sync when upgrading.
            * The linked download pages might be incomplete, use the repositories URL for a full
              list.
        * latest releases: [download page](https://software.opensuse.org/download.html?project=home:mkittler&package=passwordmanager),
          [repositories URL](https://download.opensuse.org/repositories/home:/mkittler),
          [project page](https://build.opensuse.org/project/show/home:mkittler)
        * Git master: [download page](https://software.opensuse.org/download.html?project=home:mkittler:vcs&package=passwordmanager),
          [repositories URL](https://download.opensuse.org/repositories/home:/mkittler:/vcs),
          [project page](https://build.opensuse.org/project/show/home:mkittler:vcs)
* Other GNU/Linux systems
    * [AppImage repository for releases on the openSUSE Build Service](https://download.opensuse.org/repositories/home:/mkittler:/appimage/AppImage)
    * [AppImage repository for builds from Git master the openSUSE Build Service](https://download.opensuse.org/repositories/home:/mkittler:/appimage:/vcs/AppImage/)
* Windows
    * for mingw-w64 PKGBUILDs checkout [my GitHub repository](https://github.com/Martchus/PKGBUILDs)
    * for binaries checkout the [release section on GitHub](https://github.com/Martchus/tageditor/releases)
        * the Qt 6 based version is stable and preferable but only supports Windows 10
        * the Qt 5 based version should still work on older versions down to Windows 7 although this is not regularly checked

## Build instructions
The application depends on [c++utilities](https://github.com/Martchus/cpp-utilities) and
[passwordfile](https://github.com/Martchus/passwordfile) and is built the same way as these libraries.
For basic instructions checkout the README file of [c++utilities](https://github.com/Martchus/cpp-utilities).
When the Qt GUI is enabled, Qt and [qtutilities](https://github.com/Martchus/qtutilities) are required, too.

To avoid building c++utilities/passwordfile/qtutilities separately, follow the instructions under
"Building this straight". There's also documentation about
[various build variables](https://github.com/Martchus/cpp-utilities/blob/master/doc/buildvariables.md) which
can be passed to CMake to influence the build.

### Optional dependencies
* When building any Qt GUI, the library qtutilities is required.
* When building with Qt Widgets GUI support, the following Qt modules are required (version 5.6 or higher): core gui widgets
* When building with support for the experimental Qt Quick GUI, the following Qt/KDE modules are required (version 5.12 or higher): core gui qml quick quickcontrols2 kirigami

### Building this straight
0. Install (preferably the latest version of) the CGG toolchain or Clang, the required Qt modules, OpenSSL, iconv,
   zlib, CMake and Ninja.
1. Get the sources of additional dependencies and the password manager itself. For the latest version from Git clone the following repositories:
   ```
   cd "$SOURCES"
   git clone https://github.com/Martchus/cpp-utilities.git c++utilities
   git clone https://github.com/Martchus/passwordfile.git
   git clone https://github.com/Martchus/qtutilities.git                  # only required for Qt GUI
   git clone https://github.com/Martchus/passwordmanager.git
   git clone https://github.com/Martchus/subdirs.git
   ```
2. Build and install everything in one step:
   ```
   cd "$BUILD_DIR"
   cmake \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="/install/prefix" \
    "$SOURCES/subdirs/passwordmanager"
   ninja install
   ```
    * If the install directory is not writable, do **not** conduct the build as root. Instead, set `DESTDIR` to a
      writable location (e.g. `DESTDIR="temporary/install/dir" ninja install`) and move the files from there to
      the desired location afterwards.

#### Concrete example of 3. for building an Android APK under Arch Linux
Create stuff for signing the package (remove `-DANDROID_APK_FORCE_DEBUG=ON` line in the CMake invocation to actually use this):
```
# locate keystore
keystore_dir=/path/to/keystore-dir
keystore_alias=$USER
keystore_url=$keystore_dir/$keystore_alias

# make up some password to protect the store; enter this on keytool invocation
keystore_password=<password>

# create keystore (do only once)
pushd "$keystore_dir"
keytool -genkey -v -keystore "$keystore_alias" -alias "$keystore_alias" -keyalg RSA -keysize 2048 -validity 10000
popd
```

Build c++utilities, passwordfile, qtutilities and passwordmanager in one step to create an Android APK for arm64-v8a:

```
# specify Android platform
_pkg_arch=aarch64
_android_arch=arm64-v8a
_android_arch2=arm64
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

# use Java 8 which seems to be the latest version which works
export PATH=/usr/lib/jvm/java-8-openjdk/jre/bin/:$PATH

# configure with the toolchain file provided by the Android NDK (still WIP)
# note: This configuration is likely required in the future to resolve https://gitlab.kitware.com/cmake/cmake/issues/18739. But for now
#       better keep using CMake's internal Android support because this config has its own pitfalls (see CMAKE_CXX_FLAGS).
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_ABI=$_android_arch \
    -DANDROID_PLATFORM=$_android_api_level \
    -DCMAKE_TOOLCHAIN_FILE=$android_ndk_root/build/cmake/android.toolchain.cmake \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_SYSTEM_VERSION=$_android_api_level \
    -DCMAKE_ANDROID_ARCH_ABI=$_android_arch \
    -DCMAKE_ANDROID_NDK="$android_ndk_root" \
    -DCMAKE_ANDROID_SDK="$android_sdk_root" \
    -DCMAKE_ANDROID_STL_TYPE=c++_shared \
    -DCMAKE_INSTALL_PREFIX=$other_libs_root \
    -DCMAKE_PREFIX_PATH="$root" \
    -DCMAKE_FIND_ROOT_PATH="$root;$root/libs" \
    -DCMAKE_CXX_FLAGS="-include $android_ndk_root/sysroot/usr/include/math.h -include $android_ndk_root/sources/cxx-stl/llvm-libc++/include/math.h -I$other_libs_include" \
    -DBUILD_SHARED_LIBS=ON \
    -DZLIB_LIBRARY="$android_ndk_root/platforms/android-$_android_api_level/arch-$_android_arch2/usr/lib/libz.so" \
    -DCLANG_FORMAT_ENABLED=ON \
    -DUSE_NATIVE_FILE_BUFFER=ON \
    -DUSE_STANDARD_FILESYSTEM=OFF \
    -DNO_DOXYGEN=ON \
    -DWIDGETS_GUI=OFF \
    -DQUICK_GUI=ON \
    -DBUILTIN_ICON_THEMES=breeze \
    -DBUILTIN_TRANSLATIONS=ON \
    -DANDROID_APK_TOOLCHAIN_VERSION=4.9 \
    -DANDROID_APK_CXX_STANDARD_LIBRARY="$android_ndk_root/platforms/android-$_android_api_level/arch-$_android_arch2/usr/lib/libstdc++.so" \
    -DANDROID_APK_FORCE_DEBUG=ON \
    -DANDROID_APK_KEYSTORE_URL="$keystore_url" \
    -DANDROID_APK_KEYSTORE_ALIAS="$keystore_alias" \
    -DANDROID_APK_KEYSTORE_PASSWORD="$keystore_password" \
    -DANDROID_APK_APPLICATION_ID_SUFFIX=".unstable" \
    -DANDROID_APK_APPLICATION_LABEL="Password Manager (unstable)" \
    $SOURCES/subdirs/$_reponame

# configure with CMake's internal Android support
# note: Requires workaround with Android NDK r19: https://gitlab.kitware.com/cmake/cmake/issues/18739#note_498676
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
    -DCMAKE_CXX_FLAGS="-D__ANDROID_API__=$_android_api_level" \
    -DCLANG_FORMAT_ENABLED=ON \
    -DBUILD_SHARED_LIBS=ON \
    -DUSE_NATIVE_FILE_BUFFER=ON \
    -DUSE_STANDARD_FILESYSTEM=OFF \
    -DNO_DOXYGEN=ON \
    -DWIDGETS_GUI=OFF \
    -DQUICK_GUI=ON \
    -DBUILTIN_ICON_THEMES=breeze \
    -DBUILTIN_TRANSLATIONS=ON \
    -DANDROID_APK_FORCE_DEBUG=ON \
    -DANDROID_APK_KEYSTORE_URL="$keystore_url" \
    -DANDROID_APK_KEYSTORE_ALIAS="$keystore_alias" \
    -DANDROID_APK_KEYSTORE_PASSWORD="$keystore_password" \
    -DANDROID_APK_APPLICATION_ID_SUFFIX=".unstable" \
    -DANDROID_APK_APPLICATION_LABEL="Password Manager (unstable)" \
    $SOURCES/subdirs/$_reponame

# build all binaries and make APK file using all CPU cores
make passwordmanager_apk -j$(nproc)

# install app on USB-connected phone
make passwordmanager_deploy_apk
```

##### Notes
* The Android packages for the dependencies Qt, iconv, OpenSSL and Kirigami 2 are provided in
  my [PKGBUILDs](http://github.com/Martchus/PKGBUILDs) repo.
* The latest Java I was able to use was version 8 (`jdk8-openjdk` package).

### Manual deployment of Android APK file
1. Find device ID: `adb devices`
2. Install App on phone: `adb -s <DEVICE_ID> install -r $BUILD_DIR/passwordmanager_build_apk/build/outputs/apk/passwordmanager_build_apk-debug.apk`
3. View log: `adb -s <DEVICE_ID> logcat`

### Building without Qt GUI
It is possible to build without the GUI if only the CLI is needed. In this case no Qt dependencies (including qtutilities) are required.

To build without GUI, add the following parameters to the CMake call:
```
-DWIDGETS_GUI=OFF -DQUICK_GUI=OFF
```

## Copyright notice and license
Copyright © 2015-2022 Marius Kittler

All code is licensed under [GPL-2-or-later](LICENSE).
