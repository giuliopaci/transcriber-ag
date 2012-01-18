# -------------------------------------
# --- PutPixel API : sample project ---
# -------------------------------------

# --- CMake Internals ---
cmake_minimum_required	(VERSION 2.6)

# --- Project Root Name ---
project					(PutPixel)


# --- Global Package Dependencies ---
find_package			(PkgConfig	REQUIRED)
find_package			(Gettext	REQUIRED)

# -----------------------
# --- Project Options ---
# -----------------------
option(DEBUG		"Sets Debug Mode"		OFF)

# --- Debug Mode ---
if (DEBUG)
	set(CMAKE_BUILD_TYPE Debug)
endif(DEBUG)


# --- Unix Settings ---
if (UNIX)
#	find_package			(ASPELL			REQUIRED)
	find_package			(X11			REQUIRED)
	pkg_check_modules		(GNOMECANVASMM	libgnomecanvasmm-2.6)
	

	# -- Install --
	set(CMAKE_INSTALL_PREFIX	/usr/local)
endif (UNIX)

# --- Global variables ---
set	(SRC	${PutPixel_SOURCE_DIR}/src)

# --- Project Options ---
add_subdirectory(src)
