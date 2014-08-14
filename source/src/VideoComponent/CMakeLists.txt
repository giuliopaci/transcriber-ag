# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}
	${SRC}/MediaComponent
	${LIVEMEDIA}/liveMedia/include
	${LIVEMEDIA}/groupsock/include
	${LIVEMEDIA}/UsageEnvironment/include
	${LIVEMEDIA}/BasicUsageEnvironment/include
	${SOUNDTOUCH_INCLUDE_DIRS}
	${GTKMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${LIVEMEDIA}/liveMedia
	${LIVEMEDIA}/groupsock
	${LIVEMEDIA}/UsageEnvironment
	${LIVEMEDIA}/BasicUsageEnvironment
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
