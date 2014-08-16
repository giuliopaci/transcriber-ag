if AGDB
AGDB_TEST = agdb_demo
else
AGDB_TEST =
endif

if XML
AIF_TEST = AIF_test
AIF2TB = aif2tb
else
AIF_TEST =
AIF2TB =
endif

AM_CPPFLAGS = -I../src/ag
AM_LDFLAGS=-L../src/ag
LDADD = -lag

#AIF_test_SOURCES = AIF_test.cc
#BU_test_SOURCES = BU_test.cc
#TIMIT_test_SOURCES = TIMIT_test.cc
#TB_test_SOURCES = TB_test.cc
#SWB_test_SOURCES = SWB_test.cc
#BAS_test_SOURCES = BAS_test.cc
#TF_test_SOURCES = TF_test.cc
#LCF_test_SOURCES = LCF_test.cc
#XLabel_test_SOURCES = XLabel_test.cc
#tb2aif_SOURCES = tb2aif.cc
#aif2tb_SOURCES = aif2tb.cc
agdemo_SOURCES = agdemo.cc
agdb_demo_SOURCES = agdb_demo.cc

#noinst_PROGRAMS = $(AIF_TEST) BU_test TIMIT_test TB_test SWB_test BAS_test \
#	TF_test LCF_test XLabel_test tb2aif $(AIF2TB) $(AGDB_TEST) agdemo
noinst_PROGRAMS = $(AGDB_TEST) agdemo

EXTRA_DIST = ag_wrapper_test.tcl ag_wrapper_test.py atis2wsj.sh

