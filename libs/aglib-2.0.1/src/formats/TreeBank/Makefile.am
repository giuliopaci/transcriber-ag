## library name
lib_LTLIBRARIES = agfio_plugin_TreeBank.la

## headers
noinst_HEADERS = TreeBank.h TBparser.h TBhandler.h TBEscapeHandler.h TB_UPenn_WSJ_EscapeHandler.h ag_sstream.h

## sources
agfio_plugin_TreeBank_la_SOURCES = TreeBank.cc TBparser.cc TBhandler.cc TB_UPenn_WSJ_EscapeHandler.cc

## includes
#agfio_plugin_TreeBank_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_TreeBank_la_LIBADD = -lag

##
agfio_plugin_TreeBank_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

