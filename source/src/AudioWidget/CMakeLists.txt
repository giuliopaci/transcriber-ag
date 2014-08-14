# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}
	${SRC}/MediaComponent
	${SRC}/Common
	${LIVEMEDIA}/liveMedia/include
	${LIVEMEDIA}/groupsock/include
	${LIVEMEDIA}/UsageEnvironment/include
	${LIVEMEDIA}/BasicUsageEnvironment/include
	${SOUNDTOUCH_INCLUDE_DIRS}
	${GTKMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${GTKMM_LIBRARY_DIRS}
)


# --- Library Definition ---
add_library(
	AudioWidget
	STATIC
	
	# -- Sources --
	AudioWidget.cpp
	AudioPlayControlWidget.cpp
	AudioSoundTouchControlWidget.cpp
	AudioVZoomControlWidget.cpp
	AudioVolumeControlWidget.cpp
	AudioZoomControlWidget.cpp
	AudioWaveformWidget.cpp
	AudioTrackWidget.cpp
	SegmentWidget.cpp
	SegmentTrackWidget.cpp
	MarkWidget.cpp
	MarkTrackWidget.cpp
	ScaleWidget.cpp
	ScaleTrackWidget.cpp
	AudioSignalView.cpp
	SegmentTooltip.cpp
)


# --- Links ---
set(LINKS	avcodec avformat ${GTKMM_LINK_LIBRARIES})

# --- Video Link Check ---
#if(VIDEO_BUILD)
	set(LINKS	VideoComponent ${LINKS})
#endif(VIDEO_BUILD)


target_link_libraries(
	AudioWidget

	${LINKS}
)

