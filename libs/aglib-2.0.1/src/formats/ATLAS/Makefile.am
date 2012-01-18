## options
if XML
LIB = @XERCESC_LIB@
else
LIB =
endif

## library name
lib_LTLIBRARIES = agfio_plugin_ATLAS.la

## headers
noinst_HEADERS = ATLAS.h SAX_ATLASHandlers.h agfXercesUtils.h

## sources
agfio_plugin_ATLAS_la_SOURCES = ATLAS_load.cc ATLAS_store.cc SAX_ATLASHandlers.cc agfXercesUtils.cc

## includes
#agfio_plugin_ATLAS_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_ATLAS_la_LIBADD = -lag $(LIB)

##
agfio_plugin_ATLAS_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag

