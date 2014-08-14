# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

if (NOT VIDEO_BUILD)
	 set (G_LIBRARY_DIRS 
    	 ${GLIBMM_LIBRARY_DIRS}
    )
	set (G_INCLUDE_DIRS 
	     ${GLIBMM_INCLUDE_DIRS}
	 )
else (NOT VIDEO_BUILD)
    set (VIDEO_SRC
	    video/VideoWriter.cpp
	    video/Thumbnailer.cpp
	    )
	 set (G_LIBRARY_DIRS 
	     ${GTKMM_LIBRARY_DIRS}
	 )
	set (G_INCLUDE_DIRS 
	     ${GTKMM_INCLUDE_DIRS}
	 )
endif (NOT VIDEO_BUILD)

# --- Includes ---
include_directories(
	${LIVEMEDIA}/liveMedia/include
	${LIVEMEDIA}/groupsock/include
	${LIVEMEDIA}/UsageEnvironment/include
	${LIVEMEDIA}/BasicUsageEnvironment/include
	${SOUNDTOUCH_INCLUDE_DIRS}
	${G_INCLUDE_DIRS}
    ${SRC}/MediaComponent
	${SRC}/Common
	${SRC}
)

if (WIN32)
     set(WIN32_LINKS wsock32 ws2_32 dl)
endif(WIN32)

# --- Links ---
link_directories(
	${LIVEMEDIA_BUILD}/liveMedia
	${LIVEMEDIA_BUILD}/groupsock
	${LIVEMEDIA_BUILD}/UsageEnvironment
	${LIVEMEDIA_BUILD}/BasicUsageEnvironment
	${G_LIBRARY_DIRS}
    ${SOUNDTOUCH_LIBRARY_DIRS}
)

SET(APP_BUILD_MODE	"")



# --- Library Definition ---
add_library(
	MediaComponent
	STATIC
	
	# -- Sources --
	base/Guesser.cpp
	filters/MuteFilter.cpp
	filters/SoundTouchFilter.cpp
	filters/VolumeFilter.cpp
	io/FFMpegHandler.cpp
	io/RTSPHandler.cpp
	io/SilentHandler.cpp
	io/SndFileHandler.cpp
	io/PortAudioStream.cpp
	rtsp/BufferSink.cpp
	rtsp/RTSPSession.cpp
	tools/AudioTools.cpp
	${VIDEO_SRC}
)


# --- Link ---
target_link_libraries(
	MediaComponent
	
	Common
	SoundTouch
    liveMedia
	${FFMPEG_LIBRARIES}
)


# --- Executables ---
add_executable(
	MediumGuesser

	# --- Sources ---
	apps/MediumGuesser.cpp
)


# --- Link ---
target_link_libraries(
	MediumGuesser
	
	MediaComponent
	Common
	SoundTouch
    liveMedia
    groupsock
    BasicUsageEnvironment
    UsageEnvironment
	${FFMPEG_LIBRARIES}
	${GLIBMM_LIBRARIES}
	${SNDFILE_LIBRARIES}
	gthread-2.0
	portaudio
	sndfile
	${GTKMM_LINK_LIBRARIES}
	${APPLE_LINKS}
	${WIN32_LINKS}
)

# --- Executables ---

#--------------------------#
#   MultimediaRTSPServer   #
#--------------------------#
add_executable(
	MultimediaRTSPServer

	# --- Sources ---
	server/MultimediaRTSPServer.cpp
	server/main.cpp
)


# --- Link ---
target_link_libraries(
	MultimediaRTSPServer
	
    liveMedia
    groupsock
    BasicUsageEnvironment
    UsageEnvironment
    ${WIN32_LINKS}        
)


#--------------#
#   GenPeaks   #
#--------------#
add_executable(
	GenPeaks

	# --- Sources ---
	apps/GenPeaks.cpp
)


# --- Link ---
target_link_libraries(
	GenPeaks
	
	MediaComponent
	Common
	SoundTouch
    liveMedia
    BasicUsageEnvironment
    UsageEnvironment
    groupsock
	${FFMPEG_LIBRARIES}
	${GLIBMM_LIBRARIES}
	${SNDFILE_LIBRARIES}
	portaudio
	sndfile
	${GTKMM_LINK_LIBRARIES}
	${APPLE_LINKS}
	${WIN32_LINKS}
)


#--------------#
#   MMPlayer   #
#--------------#
add_executable(
	MMPlayer

	# --- Sources ---
	apps/MMPlayer.cpp
)


# --- Link ---
target_link_libraries(
	MMPlayer
	
	MediaComponent
	Common
	SoundTouch
    liveMedia
    BasicUsageEnvironment
    UsageEnvironment
    groupsock
	${FFMPEG_LIBRARIES}
	${GLIBMM_LIBRARIES}
	${SNDFILE_LIBRARIES}
	portaudio
	sndfile
	gthread-2.0
	${GTKMM_LINK_LIBRARIES}
	${APPLE_LINKS}
	${WIN32_LINKS}
)


#---------------#
#   AudioTool   #
#---------------#
add_executable(
	audioTool

	# --- Sources ---
	apps/AudioToolsMain.cpp
)

# --- Link ---
target_link_libraries(
	audioTool
	
	MediaComponent
	Common
	SoundTouch
    liveMedia
    BasicUsageEnvironment
    UsageEnvironment
    groupsock
	${FFMPEG_LIBRARIES}
	${GLIBMM_LIBRARIES}
	${SNDFILE_LIBRARIES}
	portaudio
	sndfile
	gthread-2.0
	${GTKMM_LINK_LIBRARIES}
	${APPLE_LINKS}
	${WIN32_LINKS}
)


if (VIDEO_BUILD)
#---------------------#
#   ThumbnailerTest   #
#---------------------#
add_executable(
	ThumbnailerTest

	# --- Sources ---
	apps/ThumbnailerTest.cpp
)


# --- Link ---
target_link_libraries(
	ThumbnailerTest
	
	MediaComponent
	Common
	SoundTouch
    liveMedia
    BasicUsageEnvironment
    UsageEnvironment
    groupsock
	${FFMPEG_LIBRARIES}
	${SNDFILE_LIBRARIES}
	${GLIBMM_LIBRARIES}
	gthread-2.0
	portaudio
	sndfile
	${APPLE_LINKS}
	${WIN32_LINKS}
)
install(
	TARGETS	GenPeaks MMPlayer MediumGuesser MultimediaRTSPServer ThumbnailerTest audioTool
	
	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION lib
)
endif (VIDEO_BUILD)

