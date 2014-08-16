# -------------------------------
# --- PortAudio Library - src ---
# -------------------------------

# --- Includes ---
include_directories(
	${SRC}/include
	${SRC}/src/common
)


# --- Links ---
link_directories()

# --- Sources (global) ---
set(SOURCES
	common/pa_skeleton.c
	common/pa_stream.c
	common/pa_trace.c
	common/pa_allocation.c
	common/pa_converters.c
	common/pa_cpuload.c
	common/pa_dither.c
	common/pa_front.c
	common/pa_process.c
)


# --- Definitions (global) ---
set(DEFINITIONS
	-O2
	-DPA_LITTLE_ENDIAN
	-DPA_NO_DS
	-DPA_NO_ASIO
	-DPA_NO_WDMKS
	-DPACKAGE_NAME=""
	-DPACKAGE_TARNAME=""
	-DPACKAGE_VERSION=""
	-DPACKAGE_STRING=""
	-DPACKAGE_BUGREPORT=""
	-DSTDC_HEADERS=1
	-DHAVE_SYS_TYPES_H=1
	-DHAVE_SYS_STAT_H=1
	-DHAVE_STDLIB_H=1
	-DHAVE_STRING_H=1
	-DHAVE_MEMORY_H=1
	-DHAVE_STRINGS_H=1
	-DHAVE_INTTYPES_H=1
	-DHAVE_STDINT_H=1
	-DHAVE_UNISTD_H=1
	-DSIZEOF_SHORT=2
	-DSIZEOF_INT=4
	-DSIZEOF_LONG=4
)


# --- Win32 Settings ---
if (WIN32)
	set(DEFINITIONS ${DEFINITIONS}	-mthreads)
	set(SOURCES
		
		${SOURCES}
		os/win/pa_win_hostapis.c
		os/win/pa_win_util.c
		os/win/pa_win_waveformat.c
		hostapi/wmme/pa_win_wmme.c
	)
	set(LINKS
	
		
	)
endif (WIN32)

if (APPLE)
	set(SOURCES
		
		${SOURCES}
		hostapi/coreaudio/pa_mac_core.c
		hostapi/coreaudio/pa_mac_core_blocking.c
		hostapi/coreaudio/pa_mac_core_utilities.c
	)
endif (APPLE)


# --- Definitions ---
add_definitions( ${DEFINITIONS} )

# --- Libraries ---
add_library(
	portaudio
	MODULE

	${SOURCES}
)


if (WIN32)
	target_link_libraries(
		portaudio

		kernel32
		user32
		gdi32
		winspool
		comdlg32
		advapi32
		shell32
		ole32
		oleaut32
		uuid
		m
		stdc++
		odbc32
		odbccp32
		winmm
	)
endif (WIN32)

if (APPLE)
	target_link_libraries(
		portaudio

		"-framework CoreAudio"
		"-framework AudioToolbox"
		"-framework AudioUnit"
		"-framework Carbon"
	)
endif (APPLE)
