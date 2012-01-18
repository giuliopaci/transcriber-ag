# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}/Common
	${SRC}/DataModel
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
	TAG
	STATIC

	# --- Sources ---
	SAX_TransAGHandler.cpp
	agfXercesUtils.cpp
	TransAG_StringParser.cpp
)

# --- Libraries ---
add_library(
	agfio_plugin_TransAG
	SHARED

	# --- Sources ---
	TransAG.cpp
	SAX_TransAGHandler.cpp
	agfXercesUtils.cpp
)

add_executable(
	testTransAG

	SAX_TransAGHandler.cpp
	agfXercesUtils.cpp
	testTransAG.cpp
)


# --- Linking ---
set(LINKS Common ag xerces-c)

if (WIN32)
	set(LINKS TransAG ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG ${LINKS})
endif (APPLE)

target_link_libraries(
	TAG

	${LINKS}
)

target_link_libraries(
	agfio_plugin_TransAG

	${LINKS}
)

target_link_libraries(
	testTransAG

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_TransAG

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_TransAG testTransAG
	
	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION	lib/ag
)

