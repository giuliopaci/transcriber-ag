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
	agfio_plugin_STM
	SHARED

	# --- Sources ---
	STM.cpp
	STMParser.cpp
	STMfile.cpp
	STMWriter.cpp
)


# --- Executables ---
add_executable(stm2tag	stm2tag.cpp)
add_executable(tag2stm	tag2stm.cpp)


# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_STM

	${LINKS}
)

target_link_libraries(
	stm2tag

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)

target_link_libraries(
	tag2stm

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_STM

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_STM stm2tag tag2stm
	
	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

