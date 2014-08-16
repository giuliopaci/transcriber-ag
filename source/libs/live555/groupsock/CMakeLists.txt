# ---------------------------
# --- LIVE555 - Groupsock ---
# ---------------------------

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
	groupsock
	STATIC

	# --- Sources ---
	GroupsockHelper.cpp
	GroupEId.cpp
	inet.c
	Groupsock.cpp
	NetInterface.cpp
	NetAddress.cpp
	IOHandlers.cpp
)

# --- MacOSX ---
if (APPLE)
	set(DEFINITIONS

		-DBSD=1
		-O2
		-DSOCKLEN_T=socklen_t
		-DHAVE_SOCKADDR_LEN=1
	)
else (APPLE)
	set(DEFINITIONS
		-O2
		-r
		-static
		-DSOCKLEN_T=socklen_t
		-D_LARGEFILE_SOURCE=1
		-D_FILE_OFFSET_BITS=64
	)
endif (APPLE)

add_definitions( ${DEFINITIONS} )

if (WIN32)

	target_link_libraries(
		groupsock

		winsock
		wsock32
		ws3_32
	)
endif (WIN32)

