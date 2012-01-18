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
	agfio_plugin_TRS
	SHARED

	# --- Sources ---
	TRS.cpp
	SAX_TRSHandlers.cpp
	agfXercesUtils.cpp
)

add_executable(trs2tag	trs2tag.cpp)

# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_TRS

	${LINKS}
)

target_link_libraries(
	trs2tag

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)

# --- Install ---
set_target_properties(
	agfio_plugin_TRS

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS		agfio_plugin_TRS	trs2tag
    RUNTIME	DESTINATION	bin
    LIBRARY	DESTINATION	lib/ag
)

