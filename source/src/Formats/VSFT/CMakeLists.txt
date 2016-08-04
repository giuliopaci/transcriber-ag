# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}
	${SRC}/MediaComponent
	${LIVEMEDIA_INCLUDE_DIRS}
	${AGLIB_INCLUDE_DIRS}
	${GLIBMM_INCLUDE_DIRS}
)

# --- Links ---
link_directories(
	${GLIBMM_LIBRARY_DIRS}
	${SRC}/MediaComponent
#	${SOUNDTOUCH_LIBRARY_DIRS}
	${AGLIB_LIBRARY_DIRS}
	${LIVEMEDIA_LIBRARY_DIRS}
)


# --- Libraries ---
add_library(
	agfio_plugin_VSFT
	SHARED

	# --- Sources ---
	VSFT.cpp
	VSFTfile.cpp
)


# --- Executables ---
add_executable(vsft2tag	vsft2tag.cpp)

# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} sndfile dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS} sndfile)
endif (APPLE)

target_link_libraries(
	agfio_plugin_VSFT

	${LINKS}
)

target_link_libraries(
	vsft2tag
	
	TransAG
	Common
	MediaComponent
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
	${LIVEMEDIA_LIBRARIES}
	${FFMPEG_LIBRARIES}
	${SNDFILE_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_VSFT

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_VSFT vsft2tag

	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

