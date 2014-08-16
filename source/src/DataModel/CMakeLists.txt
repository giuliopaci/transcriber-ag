# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

#set (CMAKE_CXX_FLAGS -fPIC)

# --- Includes ---
include_directories(
	${SRC}
	${AGLIB_INCLUDE_DIRS}
	${GLIBMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${GLIBMM_LIBRARY_DIRS}
	${AGLIB_LIBRARY_DIRS}
)


# --- Library Definition ---
add_library(
	TransAG
	STATIC
	
	# -- Sources --
	DataModel.cpp
	DataModel_Cfg.cpp
	DataModel_IO.cpp
	DataModel_CPHelper.cpp
	DataModel_StatHelper.cpp
	UndoableDataModel.cpp
	DataModelMerge.cpp
	conventions/Conventions.cpp
	conventions/WordList.cpp
	conventions/Topics.cpp
	conventions/ModelChecker.cpp
	versions/Version.cpp
	versions/VersionList.cpp
	versions/VersionList_XMLHandler.cpp
    signals/SignalSegment.cpp
	signals/SignalConfiguration.cpp
	speakers/Speaker.cpp
	speakers/SpeakerDictionary.cpp
	speakers/SpeakerDictionary_XMLHandler.cpp
	anchorLinks/AnchorLinks.cpp
	anchorLinks/AnchorLinks_XMLHandler.cpp
)



# --- Link ---
if (WIN32)
	SET(WIN32_LINKS	dl)
endif (WIN32)

target_link_libraries(
	TransAG

	${GLIBMM_LIBRARIES}
	TAG
	Common
	xerces-c
	ag

	${WIN32_LINKS}
)

