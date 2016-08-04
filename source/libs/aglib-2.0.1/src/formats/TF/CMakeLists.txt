# ------------------------------------
# --- AGLib (BT Patch) - TF Format ---
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
	agfio_plugin_TF
	SHARED

	# --- Sources ---
	TF.cc
	TFfile.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_TF

	# -- Libraries --
	ag
)

