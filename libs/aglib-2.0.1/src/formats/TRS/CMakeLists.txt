# ---------------------------(---------
# --- AGLib (BT Patch) - TRS Format ---
# --------------------------(----------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_TRS
	SHARED

	# --- Sources ---
	TRS.cc
	SAX_TRSHandlers.cc
	agfXercesUtils.cc
	iso639.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_TRS

	# -- Libraries --
	xerces-c
	ag
)

