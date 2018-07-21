# Password Manager
A simple password manager with Qt 5 GUI using AES-256-CBC encryption via OpenSSL.

## Download / binary repository
I currently provide packages for Arch Linux and Windows. Sources for those packages can be found in a
separate [repository](https://github.com/Martchus/PKGBUILDs). For binaries checkout my
[website](http://martchus.no-ip.biz/website/page.php?name=programming).

## Build instructions
The Password Manager depends on c++utilities and passwordfile. So checkout the README of c++utilities for more details. Note that this project is not built differently than any other CMake project.

### Optional dependencies
* When building any Qt GUI, the library qtutilities is required.
* When building with Qt Widgets GUI support, the following Qt modules are required: core gui widgets
* When building with Qt Quick GUI support, the following Qt/KDE modules are required: core gui declarative quickcontrols2 kirigami

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
Build c++utilities, passwordfile, qtutilities and passwordmanager in one step to create an Android APK for arm64-v8a:

```
_reponame=passwordmanager
_pkgname=passwordmanager
_android_arch=arm64-v8a
_android_toolchain=aarch64-linux-android
_android_api_level=21
android_sdk_root=${ANDROID_SDK_ROOT:-/opt/android-sdk}
android_ndk_root=${ANDROID_NDK_ROOT:-/opt/android-ndk}
qt_version=$(pacman -Q "android-qt5-$_android_arch" | sed 's/.* \(.*\)-.*/\1/')
build_tools_version=$(pacman -Q android-sdk-build-tools | sed 's/.* r\(.*\)-.*/\1/')
qt_root=/opt/android-qt5/$qt_version/$_android_arch
other_libs_root=/opt/android-libs/$_android_arch
openssl_root=$other_libs_root
root="$android_ndk_root/sysroot;$other_libs_root;$qt_root"

cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=/usr/share/ECM/toolchain/Android.cmake \
    -DCMAKE_SYSTEM_NAME=Android \
    -DCMAKE_SYSTEM_VERSION=$_android_api_level \
    -DCMAKE_ANDROID_ARCH_ABI=$_android_arch \
    -DCMAKE_ANDROID_NDK="$android_ndk_root" \
    -DCMAKE_ANDROID_SDK="$android_sdk_root" \
    -DCMAKE_ANDROID_STL_TYPE=gnustl_shared \
    -DCMAKE_INSTALL_PREFIX=$other_libs_root \
    -DCMAKE_PREFIX_PATH="$root" \
    -DCMAKE_FIND_ROOT_PATH="$root" \
    -DNO_DOXYGEN=ON \
    -DWIDGETS_GUI=OFF \
    -DQUICK_GUI=ON \
    -DANDROID_API_LEVEL=$_android_api_level \
    -DANDROID_ABI=$_android_arch \
    -DANDROID_ARCHITECTURE=${_android_arch%-*} \
    -DANDROID_SDK_ROOT="$android_sdk_root" \
    -DANDROID_APK_DIR=$SOURCES/passwordmanager/android \
    -DANDROID_TOOLCHAIN=$_android_toolchain \
    -DANDROID_COMPILER_PREFIX=$_android_toolchain \
    -DANDROID_SDK_BUILD_TOOLS_REVISION="$build_tools_version" \
    -DQTANDROID_EXPORTED_TARGET=passwordmanager \
    -Diconv_DYNAMIC_INCLUDE_DIR="$other_libs_root/include" \
    -Diconv_STATIC_INCLUDE_DIR="$other_libs_root/include" \
    -Dcrypto_DYNAMIC_INCLUDE_DIR="$openssl_root/include" \
    -Dcrypto_STATIC_INCLUDE_DIR="$openssl_root/include" \
    -Dcrypto_DYNAMIC_LIB="$openssl_root/lib/libcrypto.so" \
    -Dcrypto_STATIC_LIB="$openssl_root/lib/libcrypto.a" \
    -DQt5Core_DIR="$qt_root/lib/cmake/Qt5Core" \
    -DECM_ADDITIONAL_FIND_ROOT_PATH="$PWD/c++utilities;$PWD/passwordfile;$PWD/qtutilities;$root;$other_libs_root/lib;$openssl_root/lib" \
    -DANDROID_EXTRA_LIBS="$openssl_root/lib/libcrypto.so;$openssl_root/lib/libssl.so;$other_libs_root/lib/libiconv.so;$other_libs_root/lib/libKF5Kirigami2.so" \
    $SOURCES/subdirs/passwordmanager

make create-apk -j$(nproc)
```

##### Notes
* The Android packages for the dependencies Qt 5, iconv and OpenSSL are provided in my PKGBUILDs repo.
* The lastest Java I was able to use was version 8 (`jdk8-openjdk` package).
