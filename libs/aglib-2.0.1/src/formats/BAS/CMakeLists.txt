# -------------------------------------
# --- AGLib (BT Patch) - BAS Format ---
# -------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_BAS
	SHARED

	# --- Sources ---
	BAS.cc
	BASfile.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_BAS

	# -- Libraries --
	ag
)

