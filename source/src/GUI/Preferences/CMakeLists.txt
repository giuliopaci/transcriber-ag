# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${GTKMM_INCLUDE_DIRS}
	${SRC}
	${SRC}/Common
#	${GTKSPELL}
)


# --- Links ---
link_directories()


# --- Libraries ---
add_library(
	PreferencesModule
	STATIC

	# --- Sources ---
	TreeViewPreferences.cpp
	TreeModelColumnsPreferences.cpp
	PreferencesDialog.cpp
	PreferencesFrame.cpp
	GeneralFrame.cpp
	DataModelFrame.cpp
	TextEditorFrame.cpp
#	SpellerFrame.cpp
	SpeakersFrame.cpp
	AudioFrame.cpp
	ColorsFrame.cpp
	FontsFrame.cpp
	VideoFrame.cpp
)

