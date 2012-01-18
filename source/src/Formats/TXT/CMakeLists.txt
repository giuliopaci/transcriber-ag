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
	TXT
	STATIC

	# --- Sources ---
	TXTWriter.cpp
)


# --- Libraries ---
add_library(
	agfio_plugin_TXT
	SHARED

	# --- Sources ---
	TXT.cpp
	TXTParser.cpp
	TXTfile.cpp
	TXTWriter.cpp
)


# --- Executables ---
add_executable(tag2txt	tag2txt.cpp)

# --- Linking ---
set(LINKS	ag xerces-c)

if (WIN32)
	set(LINKS TransAG Common ${LINKS} dl)
endif (WIN32)

if (APPLE)
	set(LINKS TransAG Common ${LINKS})
endif (APPLE)

target_link_libraries(
	agfio_plugin_TXT

	${LINKS}
)

target_link_libraries(
	TXT

	${LINKS}
)

target_link_libraries(
	tag2txt

	TransAG
	Common
	${LINKS}
	${GLIBMM_LINK_LIBRARIES}
)


# --- Install ---
set_target_properties(
	agfio_plugin_TXT

	PROPERTIES
	PREFIX	""
)

install(
	TARGETS	agfio_plugin_TXT tag2txt
	
	RUNTIME	DESTINATION	bin
	LIBRARY	DESTINATION	lib/ag
)

