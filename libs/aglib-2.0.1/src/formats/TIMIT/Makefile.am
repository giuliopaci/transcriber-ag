## library name
lib_LTLIBRARIES = agfio_plugin_TIMIT.la

## headers
noinst_HEADERS = TIMIT.h TIMITfile.h

## sources
agfio_plugin_TIMIT_la_SOURCES = TIMIT.cc TIMITfile.cc

## includes
#agfio_plugin_TIMIT_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_TIMIT_la_LIBADD = -lag

##
agfio_plugin_TIMIT_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

