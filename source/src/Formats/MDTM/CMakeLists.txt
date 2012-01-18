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
	agfio_plugin_MDTM
	SHARED

	# --- Sources ---
	MDTM.cpp
	MDTMfile.cpp
	MDTMParser.cpp
)


# --- Executables ---
add_executable(mdtm2tag	mdtm2tag.cpp)


# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_MDTM

	${LINKS}
)

target_link_libraries(
	mdtm2tag

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_MDTM

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_MDTM mdtm2tag
	
	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

