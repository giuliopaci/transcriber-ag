Windows build system
==================

The current build system doesn't work perfectly yet...

### File path convention ###

As Windows doesn't have standard path, the conventions used are the following:

 * the global installation directory can be anywhere, let's call it `${INSTALLDIR}`
 * `libag.dll` and the `agfio\_plugin\_*.dll` are in `${INSTALLDIR}`
 * the main config files are in `${INSTALLDIR}\etc` (equivalent of `/etc/TransAG`)
 * locale files are in `${INSTALLDIR}\locale` (equivalent of `/usr/share/locale`)

# Compiling From Linux (or OSX) #

### Cross-compile from Linux (MXE) ###

First, install [MXE](http://mxe.cc/), the *master* branch. Then build TranscriberAG's dependencies by running, in the repertory where you installed mxe:

 * `make libsndfile gtkmm2 portaudio gettext xerces ffmpeg zlib dlfcn-win32 -j4`

(large amount of compilation time). Then build the project using the newly compiled libraries by running, in the TranscriberAG directory:

 * `mkdir build-mxe`
 * `cd build-mxe`
 * `cmake ../source -DCMAKE_TOOLCHAIN_FILE=/where MXE is installed/usr/TARGET/share/cmake/mxe-conf.cmake -DCMAKE_INSTALL_PREFIX="" -DCMAKE_BUILD_TYPE=MinSizeRel`
 * `make`
 * `make install DESTDIR="../windows/installdir"`
 
(where TARGET is the target you chose from mxe). This will install the .exe and .dll files in `build-mxe/installdir`.

Sometimes you will get an error on the cmake command, running it two times usually solves it (it looks more like a bug in cmake than in the actual code).

### Installer compilation ###

#### InnoSetup Installer ###

Install [InnoSetup](http://www.jrsoftware.org/isdl.php) (works well under wine) and use the file `TranscriberAG.iss`.

#### NSIS Installer ####

This method is not advised for recent Windows versions. First, install [`NSIS`](http://nsis.sourceforge.net/Download) (`sudo aptitude install nsis` under Debian), then build the installer: `makensis TranscriberAG.nsi`.

### Debugging under Windows ###

 * install [Mingw-W64](http://sourceforge.net/projects/mingw-w64/) for `gdb.exe` (the one from mxe doesn't work here, no idea why)
 * make a release in Debug mode (in the cmake command, replace `minSizeRel` by `Debug` as argument of `-DCMAKE_BUILD_TYPE`)
 * install the obtained files in Windows
 * copy the sources repertory of TranscriberAG in some place on your computer. All the directories up to `/` must be present, for instance if your sources are under `/home/user/transag/sources/`, copy them to `C:\\TransAG\\home\\user\\transag\\sources\\`.
 * run `gdb.exe` under Windows:
    * `directory Path\\to\\sources\\` (`C:\\TransAG` in previous example)
    * `file Path\\to\\TranscriberAG.exe`
    * enjoy!
