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
	agfio_plugin_VRXML
	SHARED

	# --- Sources ---
	VRXML.cpp
	SAX_VRXMLHandler.cpp
	agfXercesUtils.cpp
)


# --- Executables ---
add_executable(vrxml2tag	vrxml2tag.cpp)


# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_VRXML

	${LINKS}
)

target_link_libraries(
	vrxml2tag

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_VRXML

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_VRXML vrxml2tag

	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

