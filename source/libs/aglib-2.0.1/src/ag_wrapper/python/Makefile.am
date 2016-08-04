## options
if AGDB
LIB1 = -liodbc
else
LIB1 =
endif

if XML
LIB2 = @XERCESC_LIB@
else
LIB2 =
endif

LIB = $(LIB1) $(LIB2)

## library name
lib_LTLIBRARIES = ag.la

## sources
ag_la_SOURCES = ag.cc

## includes
#ag_la_CXXFLAGS = -I../../ag @PYTHON_INC@
AM_CPPFLAGS = -I../../ag @PYTHON_INC@

## libraries
if BIG_BALL
AM_LDFLAGS=
ag_la_LIBADD = ../../ag/*.lo -lstdc++ $(LIB)
else
AM_LDFLAGS=-L../../ag -L../../ag/.libs
ag_la_LIBADD = -lag
endif

##
ag_la_LDFLAGS = -no-undefined -module -version-info 1:0:0

## installation
libdir=@PYTHON_INST@

