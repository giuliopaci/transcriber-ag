## library name
lib_LTLIBRARIES = agfio_plugin_XLabel.la

## headers
noinst_HEADERS = XLabel.h XLabelFile.h

## sources
agfio_plugin_XLabel_la_SOURCES = XLabel.cc XLabelFile.cc

## includes
#agfio_plugin_XLabel_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_XLabel_la_LIBADD = -lag

##
agfio_plugin_XLabel_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

