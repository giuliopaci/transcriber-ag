# -------------------------------------
# --- AGLib (BT Patch) - CAG Format ---
# -------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
	${ZLIB_INCLUDE_DIR}
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_CAG
	SHARED

	# --- Sources ---
	CAG.cc
	gzstream.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_CAG

	# -- Libraries --
	ag
	z
	${ZLIB_LIBRARIES}
)

