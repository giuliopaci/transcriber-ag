if AGDB
PRG = CreateAGTables
else
PRG =
endif

bin_PROGRAMS = $(PRG)
CreateAGTables_LDADD = -liodbc
CreateAGTables_SOURCES = CreateAGTables.c

EXTRA_DIST = README
