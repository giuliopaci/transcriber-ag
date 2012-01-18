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

if (WIN32)
	set(LINKS TransAG ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG ${LINKS})
endif (APPLE)

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
	TARGETS	agfio_plugin_CTM ctm2tag 

	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION	lib/ag
)

