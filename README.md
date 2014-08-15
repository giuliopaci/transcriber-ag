Transcriber AG (non-official)
===========================

This repository is _not_ the official Transcriber AG repository, which is
[here](http://transag.sourceforge.net/), but an unofficial repository for
people needing to build it from sources, with a few improvements.

##Changelog##

 * improving translation system
 * adding Tibetan as possible input language
 * minor improvements to make compilation possible
 * 2.0.0 version plus Debian patches
 
## Build and installation ##

You'll have to play a bit with apt/aptitude to get the right dependencies, but
globally

`aptitude install cdbs libxerces-c-dev libavcodec-dev libavformat-dev libswscale-dev libavdevice-dev libgtkmm-2.4-dev portaudio19-dev libsndfile1-dev`

should do it. Then

 * `cd libs`
 * `cmake .`
 * `make`
 * `sudo make install`
 * `cd ../source`
 * `cmake .`
 * `make`
 * `sudo make install`
 * `cd ..`
 * `sudo cp -R etc/TransAG /etc/`



### Building for Windows ###

#### MXE ####

(Not fully tested, only under Debian/sid and with MXE branch *master*): install [MXE](http://mxe.cc/) and build TranscriberAG's dependencies by running, in the repertory where you installed mxe:

 * `make libsndfile gtkmm2 portaudio xerces ffmpeg zlib dlfcn-win32 -j4`

(at this point, you can take a good nap, you have a large amount of compilation time). Then build the project using the newly compiled libraries by running, in the TranscriberAG directory:

 * `mkdir lib/build-mingw`
 * `cd lib/build-mingw`
 * `cmake .. -DCMAKE_TOOLCHAIN_FILE=/where MXE is installed/usr/i686-pc-mingw32/share/cmake/mxe-conf.cmake`
 * `make`

