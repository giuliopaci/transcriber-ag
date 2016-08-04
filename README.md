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

#### Linux (Debian/Ubuntu) ####

You'll have to play a bit with apt/aptitude to get the right dependencies, but
globally

`aptitude install build-essential cmake gettext cdbs libxerces-c-dev libavcodec-dev libavformat-dev libswscale-dev libavdevice-dev libavfilter-dev libgtkmm-2.4-dev portaudio19-dev libsndfile1-dev xsltproc libxt-dev`

should do it. On some systems, you might have dependency conflicts; in this case install `libjack-jackd2-dev`. Then

 * `mkdir build`
 * `cd build`
 * `cmake ../source`
 * `make`
 * `sudo make install`
 * `sudo cp -R ../source/etc/TransAG /etc/`


#### Building for Windows ####

See `README.md` in `windows` directory.

## TODO ##

 * debugging Windows build (almost done)
 * separate etc/, share/ and doc/ to be more Debian-compliant (and facilitate a future well-formed Debian package)
 * making Windows binaries smaller (find the good mxe options)
 * include a variable in the conf to change UI language (especially for Windows)
 * remove deprecated functions in ffmpeg, gthread and glib (requires some knowledge, especially for ffmpeg, but feasible)
 * update SoundTouch (1.4 -> 1.8)
 * possibility to link against stock SoundTouch (quite difficult: Debian SoundTouch is compiled with float samples, while the code here expects int16_t...)
 * OSX compilation (should be straightforward, I'm just lacking the OS...)
 * gtkspellmm inclusion
