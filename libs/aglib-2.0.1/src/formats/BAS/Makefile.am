## library name
lib_LTLIBRARIES = agfio_plugin_BAS.la

## headers
noinst_HEADERS = BAS.h BASfile.h

## sources
agfio_plugin_BAS_la_SOURCES = BAS.cc BASfile.cc

## includes
#agfio_plugin_BAS_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_BAS_la_LIBADD = -lag

##
agfio_plugin_BAS_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

