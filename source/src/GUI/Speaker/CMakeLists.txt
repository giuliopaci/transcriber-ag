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
	SpeakerModule
	STATIC

	# --- Sources ---
	SpeakerDico_dialog.cpp
	ListModelColumns_language.cpp
	ListModel_Columns.cpp
	SpeakerData.cpp
	DictionaryManager.cpp
	ListView_mod.cpp
)

