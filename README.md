Transcriber AG (non-official)
===========================

This repository is _not_ the official Transcriber AG repository, which is
[here](http://transag.sourceforge.net/), but an unofficial repository for
people needing to build it from sources, with a few improvements.

##Changelog##

 * adding Tibetan as possible input language
 * minor improvements to make compilation possible
 * 2.0.0 version plus Debian patches
 
## Build and installation ##

You'll have to play a bit with apt/aptitude to get the right dependencies, but
globally

`aptitude install cdbs libxerces-c-dev libavcodec-dev libavformat-dev libswscale-dev libavdevice-dev libgtkmm-2.4-dev portaudio19-dev`

should do it. Then

 * `cmake .`
 * `make`
 * `sudo make install`
