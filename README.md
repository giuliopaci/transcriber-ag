Transcriber AG (non-official)
===========================

This repository is _not_ the official Transcriber AG repository, which is
[here](http://transag.sourceforge.net/), but an unofficial repository for
people needing to build it from sources, with a few improvements.

##Changelog##

Start version is 2.0.0 version plus Debian patches.

 * improving translation system
 * improving CMAKE general build system
 * adding Tibetan as possible input language
 * make compilation possible on modern systems:
    * xerces 2 -> 3
    * small fixes for recent versions of ffmpeg
    * small fixes for recent versions of gtkmm
 
## Build and installation ##

You'll have to play a bit with apt/aptitude to get the right dependencies, but
globally

`aptitude install cdbs libxerces-c-dev libavcodec-dev libavformat-dev libswscale-dev libavdevice-dev libgtkmm-2.4-dev portaudio19-dev libsndfile1-dev xsltproc`

should do it. Then

 * `mkdir build`
 * `cd build`
 * `cmake ../source`
 * `make`
 * `sudo make install`
 * `sudo cp -R ../source/etc/TransAG /etc/`


### Building for Windows ###

See `README.md` in `windows` directory.
