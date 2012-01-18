# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}/Common
	${SRC}/DataModel
	${SRC}/Editors
#	${GTKSPELL}
)


# --- Links ---
link_directories()


# --- Libraries ---
add_library(
	SearchModule
	STATIC

	# --- Sources ---
	text/MiniSearch.cpp
	text/SearchReplaceDialog.cpp
	text/SearchManager.cpp
	text/SearchReplaceGeneral.cpp
	tags/TSearchGeneral.cpp
	tags/TSearchDialog.cpp
	tags/TSearchManager.cpp
	highlight/HighlightGeneral.cpp
	SearchGeneral.cpp
)

