# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- CMake Internals ---
cmake_minimum_required	(VERSION 2.6)
include (CPack)

# --- Project Root Name ---
project					(TranscriberAG)


# --- Global Package Dependencies ---
find_package			(PkgConfig	REQUIRED)
find_package			(Gettext	REQUIRED)

# -----------------------
# --- Project Options ---
# -----------------------
option(VIDEO_BUILD	"Adds Video Player Module to Build Rules"	ON)
option(DEBUG		"Sets Debug Mode"							OFF)
option(LIBTAG		"Sets libtag only  mode"					OFF)
option(MEDIA_TOOLS	"(use with LIBTAG=ON) Adds (audio) Media tools to build rules"	OFF)

# --- Debug Mode ---
if (DEBUG)
	set(CMAKE_BUILD_TYPE Debug)
	add_definitions(-D_DEBUGMODE)
	message(-- DEBUG MODE [ON])
endif(DEBUG)
if(LIBTAG)
    set(VIDEO_BUILD OFF)
endif(LIBTAG)


# --- Unix Settings ---
if (UNIX)
	find_package			(X11		REQUIRED)
	pkg_check_modules		(GLIBMM		glibmm-2.4)
    if (NOT LIBTAG)
#	    find_package			(ASPELL		REQUIRED)
    	pkg_check_modules		(GTKMM		gtkmm-2.4)
    	pkg_check_modules		(SNDFILE	sndfile)
    	pkg_check_modules		(PORTAUDIO	portaudio-2.0)
    else (NOT LIBTAG)
        if(MEDIA_TOOLS)
        	pkg_check_modules		(SNDFILE	sndfile)
        	pkg_check_modules		(PORTAUDIO	portaudio-2.0)
        endif(MEDIA_TOOLS)
    endif(NOT LIBTAG)

	# -- Install --
	set(CMAKE_INSTALL_PREFIX	/usr)
endif (UNIX)

# --- Global variables ---
set	(LIBS				${TranscriberAG_SOURCE_DIR}/../libs)
set	(SRC				${TranscriberAG_SOURCE_DIR}/src)

# --- Libraries ---
set	(AGLIB				${LIBS}/aglib-2.0.1)
#set	(GTKSPELL			${LIBS}/gtkspell-2.0.11-patch-BT)
set	(LIVEMEDIA			${LIBS}/live555)
set	(SOUNDTOUCH			${LIBS}/soundtouch-1.4.0)

set (AGLIB_BUILD		${LIBS}/aglib-2.0.1)
#set	(GTKSPELL_BUILD		${LIBS}/build/gtkspell-2.0.11-patch-BT)
set	(LIVEMEDIA_BUILD	${LIBS}/live555)
set	(SOUNDTOUCH_BUILD	${LIBS}/soundtouch-1.4.0)

# --- Recent distributions need this ---
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS -DCMAKE_INSTALL_PREFIX='${CMAKE_INSTALL_PREFIX}' -fPIC")

# -- Global Includes --

# --- Win32 Settings ---
#
# -- TODO here -> manage LIBTAG 
if (WIN32)	
	set	(EXTLIBS	             C:/Dev)		
	set	(WIN32_EXT_GTKMM	     ${EXTLIBS}/gtkmm)		
    set	(WIN32_EXT_LIBSNDFILE    ${EXTLIBS}/libsndfile)
    set	(WIN32_EXT_FFMPEG        ${EXTLIBS}/ffmpeg)
    set	(WIN32_EXT_XERCES        ${EXTLIBS}/xerces-c)
#    set	(WIN32_EXT_ASPELL        ${EXTLIBS}/aspell)
    set	(WIN32_EXT_DL            ${EXTLIBS}/dlfcn)
    
	include_directories(
		# -- GTKmm --
		${WIN32_EXT_GTKMM}/include/giomm-2.4
		${WIN32_EXT_GTKMM}/include/gtkmm-2.4
		${WIN32_EXT_GTKMM}/include/gdkmm-2.4
		${WIN32_EXT_GTKMM}/include/glibmm-2.4
		${WIN32_EXT_GTKMM}/include/atkmm-1.6
		${WIN32_EXT_GTKMM}/include/pangomm-1.4
		${WIN32_EXT_GTKMM}/include/cairomm-1.0
		${WIN32_EXT_GTKMM}/include/sigc++-2.0
		${WIN32_EXT_GTKMM}/include
		${WIN32_EXT_GTKMM}/include/gtk-2.0
		${WIN32_EXT_GTKMM}/include/glib-2.0
		${WIN32_EXT_GTKMM}/include/atk-1.0
		${WIN32_EXT_GTKMM}/include/pango-1.0
		${WIN32_EXT_GTKMM}/include/cairo
		${WIN32_EXT_GTKMM}/include/
	
		${WIN32_EXT_GTKMM}/lib/gtkmm-2.4/include
		${WIN32_EXT_GTKMM}/lib/gdkmm-2.4/include
		${WIN32_EXT_GTKMM}/lib/glibmm-2.4/include
		${WIN32_EXT_GTKMM}/lib/atkmm-1.6/include
		${WIN32_EXT_GTKMM}/lib/pangomm-1.4/include
		${WIN32_EXT_GTKMM}/lib/cairomm-1.0/include
		${WIN32_EXT_GTKMM}/lib/sigc++-2.0/include
		${WIN32_EXT_GTKMM}/lib
		${WIN32_EXT_GTKMM}/lib/gtk-2.0/include
		${WIN32_EXT_GTKMM}/lib/glib-2.0/include
		
		# -- Externals --
		${WIN32_EXT_LIBSNDFILE}
		${LIBS}/portaudio/include
		${WIN32_EXT_FFMPEG}/include
		${WIN32_EXT_XERCES}/include
#		${WIN32_EXT_ASPELL}/include
		${TranscriberAG_SOURCE_DIR}/include
		${WIN32_EXT_DL}/include
)

	link_directories(
		${WIN32_EXT_XERCES}/lib
		${WIN32_EXT_GTKMM}/lib
		${WIN32_EXT_FFMPEG}/lib
		${WIN32_EXT_LIBSNDFILE}
#		${WIN32_EXT_ASPELL}/lib
		${LIBS}/build/portaudio/src
		${WIN32_EXT_DL}/lib
	)

	# -- Libraries --
	set(GTKMM_LINK_LIBRARIES
		giomm-2.4
		glibmm-2.4
		gtkmm-2.4
		gdkmm-2.4
		atkmm-1.6
		pangomm-1.4
		cairomm-1.0
	
		intl
		atk-1.0
		sigc-2.0
		glib-2.0
		gio-2.0
		pango-1.0
		cairo
		gmodule-2.0
		gtk-win32-2.0
		gdk-win32-2.0
		gobject-2.0
		gthread-2.0
)
endif (WIN32)


# --- MacOSX Settings ---
if (APPLE)
	#set (BASE_DIR	/Users/ferry/gtk/inst)
	set (BASE_DIR /opt/local)

	include_directories(
		${BASE_DIR}/include
		${BASE_DIR}/include/gtk-2.0
		${BASE_DIR}/include/glib-2.0
		${BASE_DIR}/include/gtkmm-2.4
		${BASE_DIR}/include/glibmm-2.4
		${BASE_DIR}/include/gdkmm-2.4
		${BASE_DIR}/include/giomm-2.4
		${BASE_DIR}/include/cairomm-1.0
		${BASE_DIR}/include/pangomm-1.4
		${BASE_DIR}/include/atk-1.0
		${BASE_DIR}/include/atkmm-1.6
		${BASE_DIR}/include/cairo
		${BASE_DIR}/include/pango-1.0
		${BASE_DIR}/include/sigc++-2.0

		${BASE_DIR}/lib/gtkmm-2.4/include
		${BASE_DIR}/lib/glibmm-2.4/include
		${BASE_DIR}/lib/gdkmm-2.4/include
		${BASE_DIR}/lib/giomm-2.4/include
		${BASE_DIR}/lib/gtk-2.0/include
		${BASE_DIR}/lib/sigc++-2.0/include
		${BASE_DIR}/lib/glib-2.0/include
		/opt/local/include
		/sw/include
	)

	link_directories(
		${BASE_DIR}/lib
		/opt/local/lib
		/sw/lib
	)	

	# -- Libraries --
	set(GTKMM_LINK_LIBRARIES
		giomm-2.4
		glibmm-2.4
		gtkmm-2.4
		gdkmm-2.4
		atkmm-1.6
		pangomm-1.4
		cairomm-1.0
	
		intl
		atk-1.0
		sigc-2.0
		glib-2.0
		gio-2.0
		pango-1.0
		cairo
		gmodule-2.0
		gobject-2.0
		gthread-2.0
	)

	# -- Install --
	set(CMAKE_INSTALL_PREFIX	/usr/local)

endif (APPLE)


# --- Main Libraries Variables ---
set(LIVEMEDIA_LIBRARIES
	
	MediaComponent
	liveMedia
	groupsock
	BasicUsageEnvironment
	UsageEnvironment
)

set(FFMPEG_LIBRARIES

	avutil
	avdevice
	avcodec
	avformat
	swscale
)


# --- Main subdirectory - src ---
add_subdirectory	(src)

# -- Gettext handling --

# adding some modules, coming from Wesnoth
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

# translation options
if(NOT MSVC)
# Pot updates don't work under Windows so no reason to even try
option(ENABLE_POT_UPDATE_TARGET "Enables the tools to update the pot files and manuals. This target has extra dependencies." ON)
endif(NOT MSVC)
option(ENABLENLS "Enable building of translations" ON)
if (ENABLENLS)
  set(ENABLE_NLS 1)
else (ENABLENLS)
  set(ENABLE_NLS 0)
endif (ENABLENLS)

# get languages
file(READ po/LINGUAS LINGUAS)
string(REPLACE "\n" "" LINGUAS ${LINGUAS})
separate_arguments(LINGUAS)

if(ENABLE_POT_UPDATE_TARGET)
	find_package(TranslationTools REQUIRED)
endif(ENABLE_POT_UPDATE_TARGET)

if (GETTEXT_FOUND AND ENABLENLS)
 	add_subdirectory(po)
endif (GETTEXT_FOUND AND ENABLENLS)

configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

# --- CPack Settings ---

