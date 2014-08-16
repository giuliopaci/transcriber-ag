# ---------------------------------------
# --- AGLib (BT Patch) - TIMIT Format ---
# ---------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_TIMIT
	SHARED

	# --- Sources ---
	TIMIT.cc
	TIMITfile.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_TIMIT

	# -- Libraries --
	ag
)

