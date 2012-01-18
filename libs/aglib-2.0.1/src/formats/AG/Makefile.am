## options
if XML
LIB = @XERCESC_LIB@
else
LIB =
endif

## library name
lib_LTLIBRARIES = agfio_plugin_AG.la

## headers
noinst_HEADERS = AG.h SAX_AGHandlers.h agfXercesUtils.h

## sources
agfio_plugin_AG_la_SOURCES = AG.cc SAX_AGHandlers.cc agfXercesUtils.cc

## includes
#agfio_plugin_AG_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS=-I../../ag

## libraries
AM_LDFLAGS=-L../../ag -L../../ag/.libs
agfio_plugin_AG_la_LIBADD = -lag $(LIB)

##
agfio_plugin_AG_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=${prefix}/lib/ag
install-data-hook:
	(cd $(libdir); for f in agfio_plugin_AG.*; do \
	  ff=`echo $$f | sed 's/AG/AIF/'`; \
	  mv $$f $$ff; \
	done; \
	rm agfio_plugin_AIF.la)
	$(LIBTOOL) --mode=install $(libLTLIBRARIES_INSTALL) $(INSTALL_STRIP_FLAG) agfio_plugin_AG.la $(DESTDIR)$(libdir)/agfio_plugin_AG.la

