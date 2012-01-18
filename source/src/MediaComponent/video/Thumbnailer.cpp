/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#include "Thumbnailer.h"

// --- Thread Caller ---
gpointer Thumbnailer_process(gpointer data)
{
	printf("Thumbnailer --> Thread Enter\n");

	Thumbnailer* handler = (Thumbnailer*)data;
	handler->initThumbs();

	printf("Thumbnailer --> Thread Leave\n");

	g_thread_exit(0);

	return NULL;
}


// -- Thumbnailer --
Thumbnailer::Thumbnailer(string inPath, int inWidth, int inHeight, int inStep)
	: path(inPath), width(inWidth), height(inHeight), step(inStep)
{
	if (step <= 0)
		step = 1;
}


// -- ~Thumbnailer --
Thumbnailer::~Thumbnailer()
{}


// -- GetThumbs --
void
Thumbnailer::initThumbs()
{
	// -- Opening --
	FILE* fd = fopen(thumbsFile.c_str(), "rb");

	if (fd == NULL)
	{
		printf("Thumbnailer --> Thumbs file not found!\n");
		saveThumbs();
	}
	else
	{
		printf("Thumbnailer --> Thumbs file found! Loading...\n");
		getThumbs();
	}
}


// -- GetThumbs --
void
Thumbnailer::getThumbs()
{
	printf("Thumbnailer --> Retrieving thumbnails\n");

	int nbFrames = device->m_info()->video_duration / step;
	FILE* fd = fopen(thumbsFile.c_str(), "rb");

	// --  Looping through frames --
	if (fd != NULL)
	{
		int* frameLen = new int[1];
		int bytesRead;
		double target_ts;

		for(int i=0; i<nbFrames + 1; i++)
		{
			target_ts = (double)i * (double)step;

			MediumFrame* frame = new MediumFrame;

			frame->v_samples	= new uint8_t*[1];
			frame->v_linesize	= new int[1];

			fread(frameLen, sizeof(int), 1, fd);
	
			// -- Null frame case --
			if (frameLen == 0)
			{
				s_FrameReady.emit(NULL, frame->ts, 1);
				continue;
			}
		
			frame->v_samples[0] = new uint8_t[ frameLen[0] ];

			bytesRead = fread(frame->v_samples[0], sizeof(uint8_t), frameLen[0], fd);

			// -- Read error from thumbs file --
			if (bytesRead != frameLen[0])
			{
				printf("Thumbnailer --> Error while reading frame #%i\n", i);
				
				s_FrameReady.emit(NULL, target_ts, 1);
				continue;
			}

			// -- Filling frame info --
			frame->vlen				= frameLen[0];
			frame->v_linesize[0]	= rowStride;
			frame->v_width			= width;
			frame->v_height			= height;
			frame->ts				= target_ts;

			s_FrameReady.emit(frame, frame->ts, 0);
		}

		printf("Thumbnailer --> %i frames successfully recovered!\n", nbFrames);

		// -- Thread complete --
		s_FrameReady.emit(NULL, -1.0, 0);
	}
	else
	{
		// -- Sending error code --
		s_FrameReady.emit(NULL, -1.0, 2);
	}
}


// -- SaveThumbs --
void
Thumbnailer::saveThumbs()
{
	printf("Thumbnailer --> Thumbnails file generation : %s\n", thumbsFile.c_str());

	// -- Writing to file --
	int nbFrames = device->m_info()->video_duration / step;

	FILE* fd = fopen(thumbsFile.c_str(), "wb");
	MediumFrame* frame;

	if (fd != NULL)
	{
		int* frameLen = new int[1];
		double target_ts;

		for(int i=0; i<nbFrames + 1; i++)
		{
 			target_ts	= (double)i * (double)step;
			frame		= device->m_get_rgb_frame(target_ts);

			// -- Null frame case --
			if (frame == NULL)
			{
				frameLen[0] = 0;
				fwrite(frameLen, sizeof(int), 1, fd);

				s_FrameReady.emit(NULL, -1.0, 1);
				continue;
			}

			// -- Output frame --
			MediumFrame* oFrame = new MediumFrame;

			oFrame->v_samples		= new uint8_t*[1];
			oFrame->v_samples[0]	= new uint8_t[frame->vlen];
			oFrame->v_linesize		= new int[1];
			oFrame->v_linesize[0]	= rowStride;
			oFrame->v_width			= width;
			oFrame->v_height		= height;
			oFrame->ts				= frame->ts;

			memcpy(oFrame->v_samples[0], frame->v_samples[0], frame->vlen);

			frameLen[0] = frame->vlen;
			fwrite(frameLen, sizeof(int), 1, fd);
			fwrite(frame->v_samples[0], sizeof(uint8_t), frame->vlen, fd);

			s_FrameReady.emit(oFrame, oFrame->ts, 0);
		}
	
		fflush(fd);

		printf("Thumbnailer --> %i frames stored successfully!\n", nbFrames);

		s_FrameReady.emit(NULL, -1.0, 0);
	}
	else
	{
		printf("Thumbnailer --> Error : Unable to open thumbnails file!\n");
		s_FrameReady.emit(NULL, -1.0, 2);
	}
}


// -- Process (Thread entry point) --
VideoResult*
Thumbnailer::process()
{
	// -- Opening source file --
	device = Guesser::open(path.c_str(), VideoCtx);

	// -- Processing --
	if (device != NULL)
	{
		double base_ratio	= (double)device->m_info()->video_width / (double)device->m_info()->video_height;
		double aspect_ratio	= device->m_info()->video_aspect_ratio;

		if (aspect_ratio == 0.0)
			aspect_ratio = 1.0;

		// -- New algorithm (assuming video ratio > 1) --
		height = (int)(width / base_ratio);

		device->m_set_dimensions(width, height);

		// -- Retrieving row stride --
		MediumFrame* f = NULL;

		while (f == NULL)
			f = device->m_get_rgb_frame(0.0);

		rowStride = f->v_linesize[0];

		// -- Thumbnails file --
		thumbsFile = path.substr(0, path.find_last_of(".")) + ".thumbs";	

		vResult.filePath		= thumbsFile;
		vResult.processState	= 0;
		vResult.videoWidth		= width;
		vResult.videoHeight		= height;
		vResult.fps				= device->m_info()->video_fps;

		processThread = g_thread_create(Thumbnailer_process, this, true, NULL);
	}
	else
	{
		vResult.processState = 1;
	}

	return &vResult;
}


// -- JoinThread --
void
Thumbnailer::joinThread()
{
	if (processThread)
		g_thread_join(processThread);
}

