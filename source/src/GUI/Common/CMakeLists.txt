# ---------------------
# --- TranscriberAG ---
# ---------------------

# --- Includes ---
include_directories(
	${GTKMM_INCLUDE_DIRS}
	${SRC}/Common
	${SRC}/MediaComponent
#	${GTKSPELL}
)


# --- Links ---
link_directories(
	${GTKMM_LIBRARY_DIRS}
#	${GTKSPELL}/build/gtkspell
)


# --- Libraries ---
add_library(
	CommonModule
	STATIC

	# --- Sources ---
	Explorer_fileHelper.cpp
	Explorer_popup.cpp
	LanguagesParser.cpp
	Configuration.cpp
)


if (WIN32)
	set(WIN32_LINKS wsock32 ws2_32)
endif (WIN32)


# --- Target ---
target_link_libraries(
	CommonModule

	# -- Links --
	Common
#	gtkspell
#	aspell
	${WIN32_LINKS}
)

