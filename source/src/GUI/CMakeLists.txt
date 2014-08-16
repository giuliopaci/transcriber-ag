# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${LIVEMEDIA_INCLUDE_DIRS}
#	${GTKSPELL}
	${SOUNDTOUCH_INCLUDE_DIRS}
	${GTKMM_INCLUDE_DIRS}
	${AGLIB_INCLUDE_DIRS}
	${SRC}/Editors
	${SRC}/MediaComponent
	${SRC}
	./Common
	./Dialogs
	./CheckLicense
	./Preferences
	./Search
	./Speaker
	./Tools
	./Tree
)


# --- Links ---
link_directories(
	${LIVEMEDIA_LIBRARY_DIRS}
	${AGLIB_LIBRARY_DIRS}
#	${GTKSPELL_BUILD}/gtkspell
	${SOUNDTOUCH_LIBRARY_DIRS}
	${GTKMM_LIBRARY_DIRS}
	${SRC}/Common
	${SRC}/CommonWidgets
	${SRC}/Editors
	${SRC}/DataModel
	${SRC}/GUI/CheckLicense
	${SRC}/GUI/Common
	${SRC}/GUI/Dialogs
	${SRC}/GUI/Preferences
	${SRC}/GUI/Search
	${SRC}/GUI/Speaker
	${SRC}/GUI/Tools
	${SRC}/GUI/Tree
)


# --- Subdirectories ---
add_subdirectory(Common)
add_subdirectory(Dialogs)
add_subdirectory(Preferences)
add_subdirectory(Search)
add_subdirectory(Speaker)
add_subdirectory(Tools)
add_subdirectory(Tree)

# ------------------------------------------------
#                    TranscriberAG
# ------------------------------------------------

set(APP_NAME	"TranscriberAG")


# --- Win32 Settings ---
SET(APP_ICON_OBJ	"")
SET(APP_BUILD_MODE	"")

if (WIN32 AND NOT MINGW)
    find_program(WINDRES_EXECUTABLE NAMES windres)

    SET(RC_FILE         transag_icon.rc)
    SET(APP_ICON_OBJ    Transcriber.o)
	SET(APP_BUILD_MODE	WIN32)

    add_custom_command(
        OUTPUT  ${APP_ICON_OBJ}
        COMMAND ${WINDRES_EXECUTABLE}
        ARGS -i ${RC_FILE} -o ${PROJECT_BINARY_DIR}/src/GUI/${APP_ICON_OBJ}
        WORKING_DIRECTORY ${SRC}/GUI
    )

	set(WIN32_LINKS	wsock32 ws2_32 dl)
endif(WIN32 AND NOT MINGW)

if(APPLE)
	set(APPLE_LINKS "-framework CoreServices")
endif(APPLE)


# --- Executables ---
add_executable(
	${APP_NAME}
	${APP_BUILD_MODE}

	# --- Sources ---
	GuiWidget.cpp
	main.cpp
	${APP_ICON_OBJ}
)


# --- Linking ---
target_link_libraries(
	${APP_NAME}

	# --- TranscriberAG ---
	Common
	CommonWidgets
	AGEditors
	AudioWidget
	CommonModule
	DialogsModule
#	FTPModule
	PreferencesModule
	SearchModule
	ToolsModule
	TransAG
	TreeModule
	SpeakerModule
	VideoComponent
	MediaComponent
	
	# --- Libraries ---
	SoundTouch
	${GTKMM_LIBRARIES}
	${SNDFILE_LIBRARIES}
	xerces-c
	gthread-2.0
	ag
	portaudio
	sndfile
#	gtkspell
	${GTKMM_LINK_LIBRARIES}
	${APPLE_LINKS}
	${WIN32_LINKS}
	X11
)

# ------------------------------------------------
#                    MsgERR
# ------------------------------------------------

set(ERR_APP	        "msgErr")

# --- Executables ---
add_executable(
	${ERR_APP}

	# --- Sources ---
	msgErr.cpp
)

# --- Linking ---
target_link_libraries(
	${ERR_APP}

	# --- TranscriberAG ---
	Common
	gthread-2.0
	${GTKMM_LIBRARIES}
	${GTKMM_LINK_LIBRARIES}
	${APPLE_LINKS}
	${WIN32_LINKS}
)

install(
	TARGETS ${ERR_APP} ${APP_NAME}

	RUNTIME	DESTINATION bin
	LIBRARY	DESTINATION lib
)
