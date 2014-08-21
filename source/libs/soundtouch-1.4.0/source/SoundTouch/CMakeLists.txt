# ---------------------------
# --- SoundTouch - Source ---
# ---------------------------

# --- Includes ---
include_directories(
	${SRC}/include
)


# --- Links ---
link_directories()


# --- Libraries ---
add_library(
	SoundTouch
	STATIC

	# --- Sources ---
	AAFilter.cpp
	FIRFilter.cpp
	FIFOSampleBuffer.cpp
	mmx_optimized.cpp
	sse_optimized.cpp
	RateTransposer.cpp
	SoundTouch.cpp
	TDStretch.cpp
	cpu_detect_x86_gcc.cpp
	BPMDetect.cpp
	PeakFinder.cpp
)

add_definitions( -msse -fcheck-new )

# --- Linking ---

