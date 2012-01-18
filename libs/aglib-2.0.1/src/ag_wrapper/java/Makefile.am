## options
if AGDB
LIB1 = -liodbc
else
LIB1 =
endif

if XML
LIB2 = @XERCESC_LIB@
else
INC2 =
endif

LIB = $(LIB1) $(LIB2)

## library name
lib_LTLIBRARIES = libjag.la

## sources
libjag_la_SOURCES = jag.cc

## includes
#libjag_la_CXXFLAGS = -I../../ag
AM_CPPFLAGS = -I../../ag

## libraries
if BIG_BALL
AM_LDFLAGS=
libjag_la_LIBADD = ../../ag/*.lo -lstdc++ $(LIB)
else
AM_LDFLAGS=-L../../ag
libjag_la_LIBADD = -lag
endif

##
libjag_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=@JAVA_INST@
if AG_JAVA
install-data-hook:
	$(INSTALL_DATA) $(srcdir)/jag.class @JAVA_INST@; \
	$(INSTALL_DATA) $(srcdir)/jag.java @JAVA_INST@
endif

## distribution
EXTRA_DIST = jag.class jag.java

