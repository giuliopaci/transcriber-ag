cmake_minimum_required(VERSION 2.6)

# --- Project Root Name ---
project					(TranscriberAG_LIBS)

if (UNIX)
	set (CMAKE_INSTALL_PREFIX /usr)
endif (UNIX)

# --- Core Libraries ---
add_subdirectory(aglib-2.0.1)
#add_subdirectory(gtkspell-2.0.11-patch-BT)
add_subdirectory(live555)
add_subdirectory(soundtouch-1.4.0)

# --- Win32 ---
if(WIN32)
	add_subdirectory(portaudio)
endif(WIN32)

