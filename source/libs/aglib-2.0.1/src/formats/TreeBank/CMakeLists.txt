# ------------------------------------------
# --- AGLib (BT Patch) - TreeBank Format ---
# ------------------------------------------

# --- Includes ---
include_directories(
	${SRC}/ag
)


# --- Links ---
link_directories(
)


# --- Libraries ---
add_library(
	agfio_plugin_TreeBank
	SHARED

	# --- Sources ---
	TreeBank.cc
	TBparser.cc
	TBhandler.cc
	TB_UPenn_WSJ_EscapeHandler.cc
)


# --- Linking ---
target_link_libraries(
	agfio_plugin_TreeBank

	# -- Libraries --
	ag
)

