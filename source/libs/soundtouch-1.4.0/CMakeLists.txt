# ----------------------------
# --- SoundTouch - Project ---
# ----------------------------

# --- CMake Internals ---
cmake_minimum_required	(VERSION 2.6)

# --- Project Root Name ---
project					(SoundTouch)

# --- Global Package Dependencies ---
find_package			(PkgConfig	REQUIRED)

# --- Global variables ---
set						(SRC		${SoundTouch_SOURCE_DIR})

# --- Directories ---
add_subdirectory		(include)
add_subdirectory		(source)

