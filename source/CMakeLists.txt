# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- CMake Internals ---
cmake_minimum_required	(VERSION 2.8)
include (CPack)

# --- Project Root Name ---
project					(TranscriberAG)


# --- Global Package Dependencies ---
find_package			(PkgConfig	REQUIRED)
# modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
find_package			(Gettext	REQUIRED)

# -----------------------
# --- Project Options ---
# -----------------------
option(VIDEO_BUILD	"Adds Video Player Module to Build Rules"	ON)
option(LIBTAG		"Sets libtag only mode"					OFF)
option(MEDIA_TOOLS	"(use with LIBTAG=ON) Adds (audio) Media tools to build rules"	OFF)

# Default config, share and doc directory
if (APPLE)
   set(DEFAULT_ETC_DIR ${CMAKE_INSTALL_PREFIX}/etc/TranscriberAG)
   option(ETC_DIR      "Directory containing the configuration"	${DEFAULT_ETC_DIR})
   set(DEFAULT_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/TranscriberAG)
   option(ETC_DIR      "Directory containing the "	${DEFAULT_SHARE_DIR})
   set(DEFAULT_DOC_DIR ${CMAKE_INSTALL_PREFIX}/share/doc/TranscriberAG)
   option(ETC_DIR      "Directory containing the "	${DEFAULT_DOC_DIR})
endif (APPLE)
if (WIN32)
   # code for win32 won't rely on this, but a default is used somewhere...
   set(DEFAULT_ETC_DIR "C:\\\\Program Files\\TranscriberAG\\etc")
   set(DEFAULT_SHARE_DIR "C:\\\\Program Files\\TranscriberAG\\share")
   set(DEFAULT_DOC_DIR "C:\\\\Program Files\\TranscriberAG\\doc")
endif(WIN32)
if (UNIX)
   set(DEFAULT_ETC_DIR "/etc/TranscriberAG")
   option(ETC_DIR      "Directory containing the documentation"	${DEFAULT_ETC_DIR})
   set(DEFAULT_SHARE_DIR ${CMAKE_INSTALL_PREFIX}/share/TranscriberAG)
   set(DEFAULT_DOC_DIR ${CMAKE_INSTALL_PREFIX}/share/doc/TranscriberAG)
endif(UNIX)



if(LIBTAG)
    set(VIDEO_BUILD OFF)
endif(LIBTAG)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUGMODE")

if(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -ggdb3 -pedantic")
  set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_RELEASE} -s -Os")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
endif(CMAKE_COMPILER_IS_GNUCC)

# --- Unix Settings ---
if (UNIX OR MINGW)
	pkg_check_modules		(X11		x11)
	pkg_check_modules		(GLIBMM		glibmm-2.4)
	pkg_check_modules		(XERCES		xerces-c)
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
endif (UNIX OR MINGW)

# --- Global variables ---
set	(SRC				${PROJECT_SOURCE_DIR}/src)

# --- Libraries ---
#set	(GTKSPELL			${LIBS}/gtkspell-2.0.11-patch-BT)
set	(LIVEMEDIA			${LIBS}/live555)

set (SOUNDTOUCH_LIBRARY_DIRS ${PROJECT_BINARY_DIR}/libs/soundtouch-1.4.0/source/SoundTouch)
set (SOUNDTOUCH_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/libs/soundtouch-1.4.0/include)

set (AGLIB_LIBRARY_DIRS		${PROJECT_BINARY_DIR}/libs/aglib-2.0.1/src/ag)
set (AGLIB_INCLUDE_DIRS		${PROJECT_SOURCE_DIR}/libs/aglib-2.0.1/src/ag)
#set	(GTKSPELL_BUILD		${LIBS}/gtkspell-2.0.11-patch-BT)
set (LIVEMEDIA_LIBRARY_DIRS		${PROJECT_BINARY_DIR}/libs/live555/liveMedia ${PROJECT_BINARY_DIR}/libs/live555/groupsock ${PROJECT_BINARY_DIR}/libs/live555/UsageEnvironment ${PROJECT_BINARY_DIR}/libs/live555/BasicUsageEnvironment)
set (LIVEMEDIA_INCLUDE_DIRS		${PROJECT_SOURCE_DIR}/libs/live555/liveMedia/include ${PROJECT_SOURCE_DIR}/libs/live555/groupsock/include ${PROJECT_SOURCE_DIR}/libs/live555/UsageEnvironment/include ${PROJECT_SOURCE_DIR}/libs/live555/BasicUsageEnvironment/include)

# --- Recent distributions need this ---
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

find_package(FFMPEG	REQUIRED)

# -- Global Includes --

include_directories(${PROJECT_BINARY_DIR}/include/ ${PROJECT_SOURCE_DIR}/include/)

# --- Win32 Settings ---
#
# -- TODO here -> manage LIBTAG 
if (WIN32 AND NOT MINGW)
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
		${LIBS}/portaudio/src
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
endif (WIN32 AND NOT MINGW)


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

if (UNIX OR APPLE OR MINGW)
    set(LOCALEDIR 	  "${CMAKE_INSTALL_PREFIX}/share/locale" CACHE STRING "change the name of the locale data directory to a non-default name")
    set(AG_PLUGINDIR "${CMAKE_INSTALL_PREFIX}/lib/ag/"      CACHE STRING "change the name of the AG plugin directory to a non-default name")
endif (UNIX OR APPLE OR MINGW)

# --- Main Libraries Variables ---
set(LIVEMEDIA_LIBRARIES
	liveMedia
	groupsock
	BasicUsageEnvironment
	UsageEnvironment
)

add_subdirectory	(libs)
add_subdirectory	(src)

# -- Gettext handling --

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

configure_file (
  "${PROJECT_SOURCE_DIR}/include/config.h.in"
  "${PROJECT_BINARY_DIR}/include/TranscriberAG-config.h"
  )

if(ENABLE_POT_UPDATE_TARGET)
	find_package(TranslationTools REQUIRED)
endif(ENABLE_POT_UPDATE_TARGET)

add_subdirectory(po)
