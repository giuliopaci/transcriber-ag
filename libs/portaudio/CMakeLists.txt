# -----------------------------------
# --- PortAudio Library - Project ---
# -----------------------------------

# --- CMake Internals ---
cmake_minimum_required	(VERSION 2.6)

# --- Project Root Name ---
project					(PORTAUDIO)

# --- Global variables ---
set						(SRC		${PORTAUDIO_SOURCE_DIR})

# --- Directories ---
add_subdirectory		(src)

