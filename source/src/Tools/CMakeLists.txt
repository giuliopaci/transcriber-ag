# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

if (LIBTAG)
    set(MM_INCLUDES  ${GLIBMM_INCLUDE_DIRS})
    set(MM_LIBS  ${GLIBMM_LIBRARY_DIRS})
else(LIBTAG)
    set(MM_INCLUDES ${GTKMM_INCLUDE_DIRS})
    set(MM_LIBS ${GTKMM_LIBRARY_DIRS})
endif(LIBTAG)

# --- Includes ---
include_directories(
    ${LIVEMEDIA_INCLUDE_DIRS}
    ${AGLIB_INCLUDE_DIRS}
    ${SRC}
#	${SRC}/MediaComponent
	${MM_INCLUDES}
	${SOUNDTOUCH_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${SRC}/DataModel
	${SRC}/Formats/TransAG
	${AGLIB_LIBRARY_DIRS}
	${MM_LIBS}
	${LIVEMEDIA_LIBRARY_DIRS}
	${SOUNDTOUCH_LIBRARY_DIRS}
)


# --- Executables Definitions ---
add_executable(add_section	add_section.cpp)
add_executable(printtag		printtag.cpp)
add_executable(tagstats		tagstats.cpp)
add_executable(testag		testag.cpp)
add_executable(merge_tag	merge_tag.cpp)
add_executable(print_spk	print_spk.cpp)
if (NOT LIBTAG)
    add_executable(clean_tag	clean_tag.cpp)
endif(NOT LIBTAG)

add_executable(convert_tag	convert_tag.cpp)

# --- Executables ---
add_executable(
    extract_speaker_segments
    ExtractSpeakerSegments.cpp
)


# --- Linking ---
set(LINKS	TransAG TAG TXT Common ag xerces-c ${GLIBMM_LIBRARIES})

if (WIN32 AND NOT MINGW)
	set(LINKS ${LINKS} dl)
endif(WIN32 AND NOT MINGW)

target_link_libraries(
	add_section
	
	${LINKS}
)
if (NOT LIBTAG)
    target_link_libraries(
    	clean_tag
    	
    	${LINKS}
    )
endif(NOT LIBTAG)

target_link_libraries(
	printtag
	
	${LINKS}
	
)

target_link_libraries(
	tagstats
	
	${LINKS}
)

target_link_libraries(
	testag
	
	${LINKS}
)

target_link_libraries(
	merge_tag
	
	${LINKS}
)

target_link_libraries(
	convert_tag
	
	${LINKS}
)

target_link_libraries(
	print_spk
	
	${LINKS}
)

# --- Link ---
target_link_libraries(
    extract_speaker_segments
    
    TransAG
    Common
    MediaComponent
    liveMedia
    BasicUsageEnvironment
    UsageEnvironment
    groupsock
    ${FFMPEG_LIBRARIES}
    ${SNDFILE_LIBRARIES}
    glibmm-2.4
    portaudio
    sndfile
    ${APPLE_LINKS}
    ${WIN32_LINKS}
)
install(
	TARGETS	add_section clean_tag convert_tag extract_speaker_segments merge_tag print_spk printtag tagstats testag
	
	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION lib
)
