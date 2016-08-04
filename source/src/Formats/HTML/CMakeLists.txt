# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}
	${AGLIB_INCLUDE_DIRS}
	${GLIBMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${GLIBMM_LIBRARY_DIRS}
	${AGLIB_LIBRARY_DIRS}
)


# --- Libraries ---
add_library(
	agfio_plugin_HTML
	SHARED

	# --- Sources ---
	HTML.cpp
	HTMLfile.cpp
	HTMLWriter.cpp
)


# --- Executables ---
add_executable(tag2html	tag2html.cpp)

# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_HTML

	${LINKS}
)

target_link_libraries(
	tag2html

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_HTML

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_HTML tag2html
	
	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

