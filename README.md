# Password Manager
A simple [password manager](https://en.wikipedia.org/wiki/Password_manager) with Qt GUI and command-line interface using AES-256-CBC encryption via OpenSSL.

## Features
* Cross-platform: tested under GNU/Linux, Android and Windows
* Qt Widgets GUI for desktop platforms
* Qt Quick GUI (using Qt Quick Controls 2 and Kirigami) for mobile platforms
* Interactive command-line interface
* Simple architecture: All data is stored in ordinary files with AES-256-CBC applied. No cloud stuff. Use
  eg. Syncthing for synchronization.

## Covered C++/Qt topics
I've mainly started this project to learn C++ and Qt programming. So beside the mentioned features this project
and the underlying libraries serve as an example project covering some interesting C++/Qt topics:

* Basic use of Qt Widgets, Qt Quick and Kirigami
* Creating custom Qt models
    * Nested model and model with multiple columns
    * Support Drag & Drop in `QTreeView`
    * Support re-ordering in Qt Quick `ListView`
    * Use nested and table model within Qt QML
    * Integration with Qt Widgets' undo/redo framework
    * Filtering
* Android tweaks
    * Create APK via CMake (using `androiddeployqt`)
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
    * there is also a [binary repository](https://martchus.dyn.f3l.de/repo/arch/ownstuff)
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
    * for generic, self-contained binaries checkout the [release section on GitHub](https://github.com/Martchus/passwordmanager/releases)
        * Requires glibc>=2.26, OpenGL and libX11
            * openSUSE Leap 15, Fedora 27, Debian 10 and Ubuntu 18.04 are recent enough (be sure
              the package `libopengl0` is installed on Debian/Ubuntu)
        * Supports X11 and Wayland (set the environment variable `QT_QPA_PLATFORM=xcb` to disable
          native Wayland support if it does not work on your system)
        * Binaries are signed with the GPG key
          [`B9E36A7275FC61B464B67907E06FE8F53CDC6A4C`](https://keyserver.ubuntu.com/pks/lookup?search=B9E36A7275FC61B464B67907E06FE8F53CDC6A4C&fingerprint=on&op=index).
* Windows
    * for binaries checkout the [release section on GitHub](https://github.com/Martchus/tageditor/releases)
        * The Qt 6 based version is stable and preferable but only supports Windows 10 version 1809 and newer.
        * The Qt 5 based version should still work on older versions down to Windows 7 although this is not regularly checked.
        * The Universal CRT needs to be [installed](https://learn.microsoft.com/en-us/cpp/windows/universal-crt-deployment#central-deployment).
        * Binaries are signed with the GPG key
          [`B9E36A7275FC61B464B67907E06FE8F53CDC6A4C`](https://keyserver.ubuntu.com/pks/lookup?search=B9E36A7275FC61B464B67907E06FE8F53CDC6A4C&fingerprint=on&op=index).
    * for mingw-w64 PKGBUILDs checkout [my GitHub repository](https://github.com/Martchus/PKGBUILDs)

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
* When building with support for the experimental Qt Quick GUI, the following Qt/KDE modules are required (version 6.6 or higher): core gui qml quick quickcontrols2 kirigami

To specify the major Qt version to use, set `QT_PACKAGE_PREFIX` (e.g. add `-DQT_PACKAGE_PREFIX:STRING=Qt6`
to the CMake arguments). There's also `KF_PACKAGE_PREFIX` for KDE dependencies. Note that the Qt Quick GUI
always requires the same major Qt version as your KDE modules use.

### Building this straight
0. Install (preferably the latest version of) the GCC toolchain or Clang, the required Qt modules, OpenSSL, iconv,
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
   cmake --build .
   ```
    * If the install directory is not writable, do **not** conduct the build as root. Instead, set `DESTDIR` to a
      writable location (e.g. `DESTDIR="temporary/install/dir" cmake --build .`) and move the files from there to
      the desired location afterwards.
    * Especially for development it is more straight forward to use
      [CMake presets](https://github.com/Martchus/cpp-utilities/blob/master/README.md#cmake-presets).

### Building without Qt GUI
It is possible to build without the GUI if only the CLI is needed. In this case no Qt dependencies (including qtutilities) are required.

To build without GUI, add the following parameters to the CMake call:
```
-DWIDGETS_GUI=OFF -DQUICK_GUI=OFF
```

## Copyright notice and license
Copyright © 2015-2024 Marius Kittler

All code is licensed under [GPL-2-or-later](LICENSE).
