Windows build system
==================

The current build system doesn't work perfectly yet...

#### File path convention ####

As Windows doesn't have standard path, the conventions used are the following:

 * the global installation directory can be anywhere, let's call it `${INSTALLDIR}`
 * `libag.dll` and the `agfio\_plugin\_*.dll` are in `${INSTALLDIR}`
 * the main config files are in `${INSTALLDIR}\etc` (equivalent of `/etc/TransAG`)
 * locale files are in `${INSTALLDIR}\locale` (equivalent of `/usr/share/locale`)

### Cross-compile from Linux (MXE) ###

First, install [MXE](http://mxe.cc/), the *master* branch. Then build TranscriberAG's dependencies by running, in the repertory where you installed mxe:

 * `make libsndfile gtkmm2 portaudio gettext xerces ffmpeg zlib dlfcn-win32 -j4`

(large amount of compilation time). Then build the project using the newly compiled libraries by running, in the TranscriberAG directory:

 * `mkdir build-mxe`
 * `cd build-mxe`
 * `cmake ../source -DCMAKE_TOOLCHAIN_FILE=/where MXE is installed/usr/i686-pc-mingw32.static/share/cmake/mxe-conf.cmake -DCMAKE_INSTALL_PREFIX=""`
 * `make`
 * `make install DESTDIR="../windows/installdir"`
 
This will install the .exe and .dll files in `build-mxe/installdir`.

### Installer compilation ###

First, install [`NSIS`](http://nsis.sourceforge.net/Download) (`sudo aptitude install nsis` under Debian), then build the installer: `makensis TranscriberAG.nsi`.
