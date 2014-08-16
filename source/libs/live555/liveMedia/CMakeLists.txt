# ---------------------------
# --- LIVE555 - LiveMedia ---
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
	liveMedia
	STATIC

	# --- Sources ---
	Media.cpp
	MediaSource.cpp
	FramedSource.cpp
	FramedFileSource.cpp
	FramedFilter.cpp
	RTPSource.cpp
	MultiFramedRTPSource.cpp
	SimpleRTPSource.cpp
	H261VideoRTPSource.cpp
	H264VideoRTPSource.cpp
	QCELPAudioRTPSource.cpp
	AMRAudioRTPSource.cpp
	JPEGVideoRTPSource.cpp
	ByteStreamFileSource.cpp
	ByteStreamMultiFileSource.cpp
	BasicUDPSource.cpp
	DeviceSource.cpp
	AudioInputDevice.cpp
	WAVAudioFileSource.cpp
	MPEG1or2Demux.cpp
	StreamParser.hh
	MPEG1or2DemuxedElementaryStream.cpp
	MPEGVideoStreamFramer.cpp
	MPEGVideoStreamParser.hh
	MPEG1or2VideoStreamFramer.cpp
	MPEG1or2VideoStreamDiscreteFramer.cpp
	MPEG4VideoStreamFramer.cpp
	MPEG4VideoStreamDiscreteFramer.cpp
	H264VideoStreamFramer.cpp
	MPEGVideoStreamParser.cpp
	MPEG1or2AudioStreamFramer.cpp
	MPEG1or2AudioRTPSource.cpp
	MPEG4LATMAudioRTPSource.cpp
	MPEG4ESVideoRTPSource.cpp
	MPEG4GenericRTPSource.cpp
	MP3FileSource.cpp
	MP3StreamState.hh
	MP3Internals.hh
	MP3HTTPSource.cpp
	MP3Transcoder.cpp
	MP3ADU.cpp
	MP3ADUdescriptor.cpp
	MP3ADUinterleaving.cpp
	MP3ADUTranscoder.cpp
	MP3StreamState.cpp
	MP3Internals.cpp
	MP3InternalsHuffman.hh
	MP3InternalsHuffman.cpp
	MP3InternalsHuffmanTable.cpp
	MP3ADURTPSource.cpp
	MPEG1or2VideoRTPSource.cpp
	MPEG2TransportStreamMultiplexor.cpp
	MPEG2TransportStreamFromPESSource.cpp
	MPEG2TransportStreamFromESSource.cpp
	MPEG2TransportStreamFramer.cpp
	ADTSAudioFileSource.cpp
	H263plusVideoRTPSource.cpp
	H263plusVideoStreamFramer.cpp
	H263plusVideoStreamParser.hh
	H263plusVideoStreamParser.cpp
	AC3AudioStreamFramer.cpp
	AC3AudioRTPSource.cpp
	JPEGVideoSource.cpp
	AMRAudioSource.cpp
	AMRAudioFileSource.cpp
	InputFile.cpp
	MediaSink.cpp
	FileSink.cpp
	BasicUDPSink.cpp
	AMRAudioFileSink.cpp
	H264VideoFileSink.cpp
	HTTPSink.cpp
	RTPSink.cpp
	MultiFramedRTPSink.cpp
	AudioRTPSink.cpp
	VideoRTPSink.cpp
	RTPInterface.cpp
	MPEG1or2AudioRTPSink.cpp
	MP3ADURTPSink.cpp
	MPEG1or2VideoRTPSink.cpp
	MPEG1or2VideoHTTPSink.cpp
	MPEG4LATMAudioRTPSink.cpp
	MPEG4GenericRTPSink.cpp
	MPEG4ESVideoRTPSink.cpp
	H263plusVideoRTPSink.cpp
	H264VideoRTPSink.cpp
	AC3AudioRTPSink.cpp
	GSMAudioRTPSink.cpp
	JPEGVideoRTPSink.cpp
	SimpleRTPSink.cpp
	AMRAudioRTPSink.cpp
	OutputFile.cpp
	uLawAudioFilter.cpp
	MPEG2IndexFromTransportStream.cpp
	MPEG2TransportStreamIndexFile.cpp
	MPEG2TransportStreamTrickModeFilter.cpp
	RTCP.cpp
	rtcp_from_spec.c
	RTSPServer.cpp
	RTSPOverHTTPServer.cpp
	RTSPClient.cpp
	RTSPCommon.cpp
	SIPClient.cpp
	MediaSession.cpp
	ServerMediaSession.cpp
	PassiveServerMediaSubsession.cpp
	OnDemandServerMediaSubsession.cpp
	FileServerMediaSubsession.cpp
	MPEG4VideoFileServerMediaSubsession.cpp
	H263plusVideoFileServerMediaSubsession.cpp
	WAVAudioFileServerMediaSubsession.cpp
	AMRAudioFileServerMediaSubsession.cpp
	MP3AudioFileServerMediaSubsession.cpp
	MPEG1or2VideoFileServerMediaSubsession.cpp
	MPEG1or2FileServerDemux.cpp
	MPEG1or2DemuxedServerMediaSubsession.cpp
	MPEG2TransportFileServerMediaSubsession.cpp
	ADTSAudioFileServerMediaSubsession.cpp
	QuickTimeFileSink.cpp
	QuickTimeGenericRTPSource.cpp
	DarwinInjector.cpp
	AVIFileSink.cpp
	BitVector.cpp
	StreamParser.cpp
	DigestAuthentication.cpp
	our_md5.c
	our_md5hl.c
	Base64.cpp
	Locale.cpp
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
		-DSOCKLEN_T=socklen_t
		-D_LARGEFILE_SOURCE=1
		-D_FILE_OFFSET_BITS=64
	)
endif (APPLE)

add_definitions( ${DEFINITIONS} )

