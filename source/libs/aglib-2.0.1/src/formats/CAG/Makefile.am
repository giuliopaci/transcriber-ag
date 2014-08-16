if ZLIB
SRC = gzstream.cc
HDR = gzstream.h
LIB = -lz
else
SRC =
HDR =
LIB =
endif

## library name
lib_LTLIBRARIES = agfio_plugin_CAG.la

## headers
noinst_HEADERS = CAG.h $(HDR)

## sources
agfio_plugin_CAG_la_SOURCES = CAG.cc $(SRC)

## includes
#agfio_plugin_CAG_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_CAG_la_LIBADD = -lag $(LIB)

##
agfio_plugin_CAG_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

