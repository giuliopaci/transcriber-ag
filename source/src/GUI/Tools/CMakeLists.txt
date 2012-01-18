# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}/Editors
	${SRC}/Common
	${SRC}/DataModel
#	${GTKSPELL}
)


# --- Links ---
link_directories()


# --- Libraries ---
add_library(
	ToolsModule
	STATIC

	# --- Sources ---
	Clipboard.cpp
	Doc.cpp
	Explorer_menu.cpp
	TAGCommandLine.cpp
	NoteBook_mod.cpp
)

