# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}
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
	TAG_compat
	STATIC

	# --- Sources ---
	SAX_TransAGHandler.cpp
	agfXercesUtils.cpp
	TransAG_StringParser.cpp
)

# --- Libraries ---
add_library(
	agfio_plugin_TransAG_compat
	SHARED

	# --- Sources ---
	TransAG_compat.cpp
	SAX_TransAGHandler.cpp
	agfXercesUtils.cpp
)

add_executable(compat2tag	compat2tag.cpp)

target_link_libraries(
	compat2tag

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


add_executable(
	testTransAG_compat

	SAX_TransAGHandler.cpp
	agfXercesUtils.cpp
	testTransAG_compat.cpp
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
	TAG_compat

	${LINKS}
)

target_link_libraries(
	agfio_plugin_TransAG_compat

	${LINKS}
)

target_link_libraries(
	testTransAG_compat

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_TransAG_compat

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_TransAG_compat testTransAG_compat compat2tag
	
	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION	lib/ag
)

