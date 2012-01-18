# ---------------------------------------
# --- AGLib (BT Patch) - ATLAS Format ---
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
	agfio_plugin_ATLAS
	SHARED

	# --- Sources ---
	ATLAS_load.cc
	ATLAS_store.cc
	SAX_ATLASHandlers.cc
	agfXercesUtils.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_ATLAS

	# -- Libraries --
	xerces-c
	ag
)

