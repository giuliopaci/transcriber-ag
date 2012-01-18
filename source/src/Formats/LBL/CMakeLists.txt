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
	agfio_plugin_LBL
	SHARED

	# --- Sources ---
	LBL.cpp
	LBLfile.cpp
)


# --- Executables ---
add_executable(lbl2tag	lbl2tag.cpp)


# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_LBL

	${LINKS}
)

target_link_libraries(
	lbl2tag

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)



# --- Install ---
set_target_properties(
	agfio_plugin_LBL

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_LBL lbl2tag
	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

