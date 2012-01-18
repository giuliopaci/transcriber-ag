# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}/Common
#	${GTKSPELL}
)


# --- Links ---
link_directories()


# --- Libraries ---
add_library(
	TreeModule
	STATIC

	# --- Sources ---
	Explorer_tree.cpp
	TreeModel_Columns.cpp
	TreeView_mod.cpp
	Explorer_tooltip.cpp
	TreeManager.cpp
)

