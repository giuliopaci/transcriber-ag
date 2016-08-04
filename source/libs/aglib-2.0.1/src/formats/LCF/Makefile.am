## library name
lib_LTLIBRARIES = agfio_plugin_LCF.la

## headers
noinst_HEADERS = LCF.h LCFfile.h

## sources
agfio_plugin_LCF_la_SOURCES = LCF.cc LCFfile.cc

## includes
#agfio_plugin_LCF_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_LCF_la_LIBADD = -lag

##
agfio_plugin_LCF_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

