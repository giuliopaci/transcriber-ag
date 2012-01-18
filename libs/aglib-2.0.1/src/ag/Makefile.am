## options
if AGDB
SRC = AGDB.cc db.cc
INC = -DAGDB
LIB = -liodbc
else
SRC = 
LIB =
INC =
endif

## library name
lib_LTLIBRARIES = libag.la

## headers
nobase_include_HEADERS = \
ag/AG.h \
ag/AGAPI.h \
ag/AGException.h \
ag/AGSet.h \
ag/AGTypes.h \
ag/Anchor.h \
ag/Annotation.h \
ag/AnnotationIndex.h \
ag/FeatureMap.h \
ag/Hash.h \
ag/Identifiers.h \
ag/Metadata.h \
ag/Paired.h \
ag/RE.h \
ag/Record.h \
ag/Signal.h \
ag/Timeline.h \
ag/Utilities.h \
ag/Validation.h \
ag/agfio.h \
ag/agfioError.h \
ag/agfio_plugin.h \
ag/agtree.h \
ag/db.h \
ag/regex.h

noinst_HEADERS = \
ag_dlfcn.h \
cclass.h \
cname.h \
engine.ih \
regcomp.ih \
regerror.ih \
regex2.h \
utils.h \
engine.c

## sources
libag_la_SOURCES = \
AG.cc \
AGAPI.cc \
AGException.cc \
AGSet.cc \
Anchor.cc \
Annotation.cc \
AnnotationIndex.cc \
FeatureMap.cc \
Identifiers.cc \
Metadata.cc \
Paired.cc \
RE.cc \
Record.cc \
Signal.cc \
Timeline.cc \
Utilities.cc \
Validation.cc \
agfio.cc \
agfioError.cc \
agfio_plugin.cc \
tree_kernel.cc \
tree_others.cc \
ag_dlfcn.c \
regcomp.c \
regerror.c \
regexec.c \
regfree.c \
$(SRC)

## includes
libag_la_CXXFLAGS = $(INC)

## libraries
libag_la_LIBADD = -lstdc++ -ldl $(LIB)

##
libag_la_LDFLAGS = -no-undefined -version-info 1:0:0

##
EXTRA_libag_la_SOURCES = \
AGDB.cc db.cc

EXTRA_DIST = \
AGDB.cc db.cc

## installation
#install-data-hook:
#	target_dir=$(DESTDIR)$(includedir)/aglib; \
#	$(mkinstalldirs) $$target_dir; \
#	for s in $(noinst_HEADERS); do \
#	  t=$$target_dir/`basename $$s`; \
#	  ${INSTALL_DATA} $$s $$t; \
#	done

