# ----------------------------------
# --- TranscriberAG - CMake File ---
# ----------------------------------

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

# --- Subdirectories ---
add_subdirectory	(Common)
add_subdirectory	(DataModel)
add_subdirectory	(Formats)
add_subdirectory	(Tools)

if (LIBTAG)
    if (MEDIA_TOOLS)  
        add_subdirectory	(MediaComponent)
    endif (MEDIA_TOOLS)  
else(LIBTAG)
    add_subdirectory	(Editors)
    add_subdirectory	(MediaComponent) 
    add_subdirectory	(AudioWidget)
    add_subdirectory    (VideoComponent)
    add_subdirectory	(GUI)
endif(LIBTAG)
