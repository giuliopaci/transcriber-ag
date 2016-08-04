# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}
	${AGLIB_INCLUDE_DIRS}
	${GTKMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${GTKMM_LIBRARY_DIRS}
	${AGLIB_LIBRARY_DIRS}
)


# --- Libraries ---
add_library(
	agfio_plugin_SGML
	SHARED

	# --- Sources ---
	SGMLobjects.cpp
	SGML.cpp
	SAX_SGMLHandler.cpp
	agfXercesUtils.cpp
)


# --- Executables ---
#add_executable(sgml2tag	sgml2tag.cpp)


# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common CommonWidgets ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_SGML

	${LINKS}
	${GTKMM_LINK_LIBRARIES}
)

#target_link_libraries(
#	
#	TransAG
#	Common
#	${LINKS}
#	${GTKMM_LINK_LIBRARIES}
#)


# --- Install ---
set_target_properties(
	agfio_plugin_SGML

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_SGML

	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

