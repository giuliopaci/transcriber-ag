# -------------------------------------
# --- AGLib (BT Patch) - LCF Format ---
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
	agfio_plugin_LCF
	SHARED

	# --- Sources ---
	LCF.cc
	LCFfile.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_LCF

	# -- Libraries --
	ag
)

