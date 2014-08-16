# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

if(APPLE)
	set(APPLE_INCLUDES
	
		/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/AE.framework/Versions/A/Headers
		/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
	)
endif(APPLE) 

if (LIBTAG)
    set(MM_INCLUDES  ${GLIBMM_INCLUDE_DIRS})
    set(MM_LIBS  ${GLIBMM_LIBRARY_DIRS})
else(LIBTAG)
    set(MM_INCLUDES ${GTKMM_INCLUDE_DIRS})
    set(MM_LIBS ${GTKMM_LIBRARY_DIRS})
endif(LIBTAG)

# --- Includes ---
include_directories(
	${MM_INCLUDES}
	${GETTEXT_INCLUDE_DIR}
	${SRC}
	${SRC}/Common
	${APPLE_INCLUDES}
)

# --- Links ---
link_directories(
	${MM_LIBS}
)

# --- Library Definition ---
ADD_LIBRARY(
	Common
	STATIC
	
	# --- Sources ---
	CommonXMLReader.cpp
	FileInfo.cpp
	Formats.cpp
	Parameters.cpp
	Encoding.cpp
	iso639.cpp
	Languages.cpp
	VersionInfo.cpp
	util/StringOps.cpp
	util/FormatTime.cpp
	util/ExpRegul.cpp
	util/str.cpp
	util/FormatToUTF8.cpp
	util/Log.cpp
)

if (NOT LIBTAG)
    ADD_LIBRARY(
    	CommonWidgets
    	STATIC
    	
    	# --- Sources ---
    	Dialogs.cpp
    	InputLanguageHandler.cpp
    	InputLanguage.cpp
    	InputLanguageArabic.cpp
    	Explorer_filter.cpp
    	Explorer_utils.cpp
        ResultSet.cpp
    	externals/sendpraat.cpp
    	util/Utils.cpp
    	util/FileHelper.cpp
    	widgets/VScale_mod.cpp
    	widgets/HScale_mod.cpp
    	widgets/TooltipTT.cpp
    	widgets/ComboEntry_mod.cpp
    	widgets/FieldEntry.cpp
    	widgets/GtUtil.cpp
    	widgets/DynamicTable.cpp
    	widgets/Settings.cpp
    	widgets/GeoWindow.cpp
    	widgets/ToolLauncher.cpp
    	widgets/ProgressionWatcher.cpp
    	icons/IcoPackButton.cpp
    	icons/IcoPackToggleButton.cpp
    	icons/IcoPackToolButton.cpp
    	icons/IcoPackMenuToolButton.cpp
    	icons/IcoPackImage.cpp
    	icons/Icons.cpp
    	icons/IcoPackElement.cpp
    	icons/IcoPackImageButton.cpp
    )
    set(COMMON_WIDGETS CommonWidgets)
    target_link_libraries(
    	CommonWidgets
	    ${WIN32_LINKS}
	    ${GTKMM_LIBRARIES}
    )
endif(NOT LIBTAG)

# --- Link ---
if (WIN32 AND NOT MINGW)
	set(WIN32_LINKS wsock32 ws2_32 imm32 dl)
endif (WIN32 AND NOT MINGW)

target_link_libraries(
	Common
	${COMMON_WIDGETS}
    ${WIN32_LINKS}
	${GTKMM_LIBRARIES}
)
