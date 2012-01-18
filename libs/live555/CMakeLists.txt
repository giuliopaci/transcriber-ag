# ----------------------------------
# --- AGLib (BT Patch) - Project ---
# ----------------------------------

# --- CMake Internals ---
cmake_minimum_required	(VERSION 2.6)

# --- Project Root Name ---
project					(LIVE555)

# --- Global Package Dependencies ---
find_package			(PkgConfig	REQUIRED)

# --- Global variables ---
set						(SRC		${LIVE555_SOURCE_DIR})

# --- Directories ---
add_subdirectory		(groupsock)
add_subdirectory		(liveMedia)
add_subdirectory		(UsageEnvironment)
add_subdirectory		(BasicUsageEnvironment)

