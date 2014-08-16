# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}
	${SRC}/MediaComponent
	${LIVEMEDIA_INCLUDE_DIRS}
	${SOUNDTOUCH_INCLUDE_DIRS}
	${GTKMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${LIVEMEDIA_LIBRARY_DIRS}
	${GTKMM_LIBRARY_DIRS}
)


# --- Libraries ---
add_library(
	VideoComponent
	STATIC

	# --- Sources ---
	FrameBrowser/FrameFiller.cpp
	FrameBrowser/FBModelColumns.cpp
	FrameBrowser/FBListView.cpp
	FrameBrowser/FrameBrowser.cpp
	VideoPlayer/ToolBox.cpp
	VideoPlayer/VideoBuffer.cpp
	VideoPlayer/VideoWidget.cpp
	VideoCommon/MediaCommon.cpp	
	VideoManager.cpp
)

target_link_libraries(
	VideoComponent

    MediaComponent
	${GTKMM_LINK_LIBRARIES}
)
