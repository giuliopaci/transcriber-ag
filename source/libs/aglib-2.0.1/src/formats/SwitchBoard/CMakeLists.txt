# ---------------------------------------------
# --- AGLib (BT Patch) - SwitchBoard Format ---
# ---------------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_SwitchBoard
	SHARED

	# --- Sources ---
	SwitchBoard.cc
	SWBfile.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_SwitchBoard

	# -- Libraries --
	ag
)

