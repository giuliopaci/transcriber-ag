# -------------------------------------
# --- PutPixel API : sample project ---
# -------------------------------------

# --- Includes ---
include_directories(
	${SRC}
	${GNOMECANVASMM_INCLUDE_DIRS}
)


# --- Links ---
link_directories(
	${GNOMECANVASMM_LIBRARY_DIRS}
)


# --- Library Definition ---
add_executable(
	PutPixel	

	# -- Sources --
	GraphicsCanvas.cpp
	main.cpp
)

target_link_libraries(
	PutPixel

    ${GNOMECANVASMM_LIBRARIES}
)

