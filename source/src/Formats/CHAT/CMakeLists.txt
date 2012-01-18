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
	${AGLIB}/src/ag
     ${GLIBMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${GLIBMM_LIBRARY_DIRS}
	${AGLIB_BUILD}/src/ag
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

if (WIN32)
	set(LINKS TransAG ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_CHAT

	${LINKS}
)

target_link_libraries(
	chat2tag

	TransAG
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)

target_link_libraries(
	tag2chat

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
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

