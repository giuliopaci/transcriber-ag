# ------------------------------------
# --- AGLib (BT Patch) - BU Format ---
# ------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_BU
	SHARED

	# --- Sources ---
	BU.cc
	BUpos.cc
	XLabelFile.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_BU

	# -- Libraries --
	ag
)

