# ---------------------------------------
# --- LIVE555 - BasicUsageEnvironment ---
# ---------------------------------------

# --- Includes ---
include_directories(
	${SRC}/groupsock/include
	${SRC}/liveMedia/include
	${SRC}/UsageEnvironment/include
	${SRC}/BasicUsageEnvironment/include
)


# --- Links ---
link_directories()


# --- Libraries ---
add_library(
	BasicUsageEnvironment
	STATIC

	# --- Sources ---
	BasicUsageEnvironment0.cpp
	BasicUsageEnvironment.cpp
	BasicTaskScheduler0.cpp
	BasicTaskScheduler.cpp
	DelayQueue.cpp
	BasicHashTable.cpp
)

# --- MacOSX ---
if (APPLE)
	set(DEFINITIONS

		-DBSD=1
		-DSOCKLEN_T=socklen_t
		-DHAVE_SOCKADDR_LEN=1
	)
else (APPLE)
	set(DEFINITIONS
		-r
		-static
		-DSOCKLEN_T=socklen_t
		-D_LARGEFILE_SOURCE=1
		-D_FILE_OFFSET_BITS=64
	)
endif (APPLE)

add_definitions( ${DEFINITIONS} )

