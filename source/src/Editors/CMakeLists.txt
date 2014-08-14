# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

# --- Includes ---
include_directories(
	${SRC}/Editors
	${SRC}/MediaComponent
    ${SRC}/VideoComponent
	${SRC}/DataModel
	${SRC}/AudioWidget
	${SRC}
	${AGLIB}/src/ag
#	${GTKSPELL}
	${LIVEMEDIA}/liveMedia/include
	${LIVEMEDIA}/groupsock/include
	${LIVEMEDIA}/UsageEnvironment/include
	${LIVEMEDIA}/BasicUsageEnvironment/include
	${SOUNDTOUCH_INCLUDE_DIRS}
	${GTKMM_INCLUDE_DIRS}
	${PORTAUDIO_INCLUDE_DIRS}
)

# --- Links ---
link_directories(
	${GTKMM_LIBRARY_DIRS}
	${PORTAUDIO_LIBRARY_DIRS}
)


# --- Library Definition ---
add_library(
	AGEditors
	STATIC
	# -- Sources --
	CommonEditor/ActivityWatcher.cpp
	CommonEditor/AGEditor.cpp
	CommonEditor/AnnotationIndex.cpp
	AnnotationEditor/menus/SpeakerMenu.cpp
	AnnotationEditor/menus/QualifiersMenu.cpp
	AnnotationEditor/menus/SectionMenu.cpp
	AnnotationEditor/menus/AnnotationMenu.cpp
	AnnotationEditor/menus/SignalPopupMenu.cpp
	AnnotationEditor/menus/BackgroundMenu.cpp
    AnnotationEditor/menus/UnitMenu.cpp
	AnnotationEditor/dialogs/CheckerDialog.cpp
	AnnotationEditor/dialogs/DialogSelectConventions.cpp
	AnnotationEditor/dialogs/AnnotationPropertiesDialog.cpp
	AnnotationEditor/dialogs/QualifierPropertiesDialog.cpp
	AnnotationEditor/dialogs/SectionPropertiesDialog.cpp
	AnnotationEditor/dialogs/TurnPropertiesDialog.cpp
	AnnotationEditor/dialogs/AnchoredElementDialog.cpp
	AnnotationEditor/dialogs/DialogFileProperties.cpp
	AnnotationEditor/dialogs/DialogMore.cpp
	AnnotationEditor/dialogs/TopicsDialog.cpp
	AnnotationEditor/dialogs/BackgroundDialog.cpp
	AnnotationEditor/dialogs/GoToDialog.cpp
	AnnotationEditor/dialogs/FileDialogs.cpp
	AnnotationEditor/dialogs/SaveAudioDialog.cpp
	AnnotationEditor/renderers/AnnotationRenderer.cpp
	AnnotationEditor/renderers/SectionRenderer.cpp
	AnnotationEditor/renderers/TurnRenderer.cpp
	AnnotationEditor/renderers/SegmentRenderer.cpp
	AnnotationEditor/renderers/UnitRenderer.cpp
	AnnotationEditor/renderers/EventRenderer.cpp
	AnnotationEditor/renderers/EntityRenderer.cpp
	AnnotationEditor/renderers/BackgroundRenderer.cpp
	AnnotationEditor/renderers/UnknownQualifierRenderer.cpp
	AnnotationEditor/AnnotationBuffer.cpp
	AnnotationEditor/AnnotationEditor.cpp
	AnnotationEditor/AnnotationView.cpp
	AnnotationEditor/Anchors.cpp
	AnnotationEditor/UndoableTextView.cpp
	AnnotationEditor/StatsWidget.cpp
	AnnotationEditor/handlers/SAXAnnotationsHandler.cpp
	AnnotationEditor/AnnotationViewTooltip.cpp
)

# --- Link ---
if (WIN32)
	set(WIN32_LINKS wsock32 ws2_32 dl)
endif (WIN32)

target_link_libraries(
	AGEditors
	
	Common
	CommonWidgets
	AudioWidget
	MediaComponent
	VideoComponent
	TransAG
	TAG
	${PORTAUDIO_LIBRARIES}
	${GTKMM_LINK_LIBRARIES}
	liveMedia
	groupsock
	BasicUsageEnvironment
	UsageEnvironment
	xerces-c
	avcodec
	avformat
	${WIN32_LINKS}
)
