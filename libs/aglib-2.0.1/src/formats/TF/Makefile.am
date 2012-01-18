## library name
lib_LTLIBRARIES = agfio_plugin_TF.la

## headers
noinst_HEADERS = TF.h TFfile.h

## sources
agfio_plugin_TF_la_SOURCES = TF.cc TFfile.cc

## includes
#agfio_plugin_TF_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_TF_la_LIBADD = -lag

##
agfio_plugin_TF_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

