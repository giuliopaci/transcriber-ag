# ----------------------------------------
# --- AGLib (BT Patch) - XLabel Format ---
# ----------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_XLabel
	SHARED

	# --- Sources ---
	XLabel.cc
	XLabelFile.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_XLabel

	# -- Libraries --
	ag
)

