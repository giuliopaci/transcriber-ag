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
	${AGLIB_LIBRARY_DIRS}
	${GLIBMM_LIBRARY_DIRS}
)


# --- Libraries ---
add_library(
	agfio_plugin_CHAT
	SHARED

	# --- Sources ---
	CHAT.cpp
	CHATfile.cpp
	CHATParser.cpp
	CHATWriter.cpp
)


# --- Executables ---
add_executable(chat2tag	chat2tag.cpp)
add_executable(tag2chat	tag2chat.cpp)


# --- Linking ---
set(LINKS	Common ag xerces-c)

if (WIN32 AND NOT MINGW)
	set(LINKS TransAG ${LINKS} dl)
endif (WIN32 AND NOT MINGW)

if (APPLE)
	set(LINKS TransAG ${LINKS})
endif (APPLE)

if (MINGW)
	set(LINKS TransAG ${LINKS})
endif (MINGW)

message(STATUS "GLIBMM_LINK_LIBRARIES=${GLIBMM_LINK_LIBRARIES}")
message(STATUS "GLIBMM_LIBRARIES=${GLIBMM_LIBRARIES}")
message(STATUS "AGLIB_INCLUDE_DIRS=${AGLIB_INCLUDE_DIRS}")
message(STATUS "GLIBMM_INCLUDE_DIRS=${GLIBMM_INCLUDE_DIRS}")

target_link_libraries(
	agfio_plugin_CHAT

	${LINKS}
	Common CommonWidgets ag xerces-c
)

target_link_libraries(
	chat2tag

	TransAG
	${LINKS}
	${GLIBMM_LIBRARIES}
)

target_link_libraries(
	tag2chat

	TransAG
	Common
	${LINKS}
	${GLIBMM_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_CHAT

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_CHAT chat2tag tag2chat
	
	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

