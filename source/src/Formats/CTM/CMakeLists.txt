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
	${AGLIB_LIBRARY_DIRS}
	${GLIBMM_LIBRARY_DIRS}
)


# --- Libraries ---
add_library(
	agfio_plugin_CTM
	SHARED

	# --- Sources ---
	CTM.cpp
	CTMfile.cpp
)


# --- Executables ---
add_executable(ctm2tag	ctm2tag.cpp)

add_executable(
    testCTM	

    testCTM.cpp
	CTM.cpp
	CTMfile.cpp
)


# --- Linking ---
set(LINKS Common ag xerces-c)

if (WIN32 AND NOT MINGW)
	set(LINKS TransAG ${LINKS} dl)
endif (WIN32 AND NOT MINGW)

if (APPLE)
	set(LINKS TransAG ${LINKS})
endif (APPLE)

if (MINGW)
	set(LINKS TransAG ${LINKS})
endif (MINGW)

target_link_libraries(
	agfio_plugin_CTM

	${LINKS}
)

target_link_libraries(
	ctm2tag

	TransAG
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)

target_link_libraries(
	testCTM

	TransAG
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)

# --- Install ---
set_target_properties(
	agfio_plugin_CTM

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_CTM ctm2tag testCTM

	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION	lib/ag
)

