# ------------------------------------
# --- AGLib (BT Patch) - AG Format ---
# ------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories()


# --- Libraries ---
add_library(
	agfio_plugin_AG
	MODULE

	# --- Sources ---
	AG.cc
	SAX_AGHandlers.cc
	agfXercesUtils.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_AG

	# -- Libraries --
	ag
	xerces-c
)

