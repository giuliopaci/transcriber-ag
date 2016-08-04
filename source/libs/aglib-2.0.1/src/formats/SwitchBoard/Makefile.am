## library name
lib_LTLIBRARIES = agfio_plugin_SwitchBoard.la

## headers
noinst_HEADERS = SwitchBoard.h SWBfile.h

## sources
agfio_plugin_SwitchBoard_la_SOURCES = SwitchBoard.cc SWBfile.cc

## includes
#agfio_plugin_SwitchBoard_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_SwitchBoard_la_LIBADD = -lag

##
agfio_plugin_SwitchBoard_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

