/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/


// -> Standalone file that provides executable 'GenPeaks'
// -> Computes peaks for the input (local) file
// -> Store each channel into a separate file (.pks)

#include "MediaComponent/base/Types.h"
#include "MediaComponent/base/Guesser.h"
#include "Common/util/Log.h"
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>
#include <getopt.h>

using namespace std;

#define MAX_PEAK			32767
#define BUFFER_SIZE			512
#define AUDIO_ZOOM_MAX		0.001
#define ABSOLUTE_PEAKS_NORM	true


void	normalizePeaks(float*&, int);
float*	computeStreamPeaks(IODevice*, int);


// ----------------------
// --- NormalizePeaks ---
// ----------------------
void normalizePeaks(float*& in_tab, int ws)
{
	float max = MAX_PEAK;

	if ( ! ABSOLUTE_PEAKS_NORM )
	{
		max = 0 ;
		for (int j = 0; j < ws; j++)
		{
			if (in_tab[j] > max)
				max = in_tab[j];
		}
	}
	for (int j = 0; j < ws; j++)
		in_tab[j] = in_tab[j]/1.10/max;
}


// --------------------------
// --- ComputeStreamPeaks ---
// --------------------------
float* computeStreamPeaks(IODevice *iodev, int chan_ID)
{
	// -- Streaming Case : no device yet --
	if (iodev == NULL)
		return NULL;

	// -- Gathering Stream Info --
	MediumFrame*	frame;
	MediumInfo*		s_info = iodev->m_info();

	int sample_rate		= s_info->audio_sample_rate;
	int channels		= s_info->audio_channels;
	int frame_size		= s_info->audio_frame_size;
	float duration		= s_info->audio_duration;

	// -- Inits --
	int ws = (int)(duration / AUDIO_ZOOM_MAX);
	int ns = (int)(roundf(AUDIO_ZOOM_MAX * sample_rate));
	int is = 0;

	float* maxPeaks = (float*)calloc(ws+1, sizeof(float));
	float* buffer;


	// -- Buffering Process --
	int indice		= 0;
	int stop		= 0;
	bool finTroncon = false;

	TRACE << "Computing Waveform for Track no. " << chan_ID << std::endl ;

	// -- First frame grab => Init --
	iodev->m_seek(0.0);
	frame = iodev->m_read();

	buffer = new float[frame_size];

	// -- Computing Loop --
	while (frame != NULL)
	{
		// -- Frame Buffer --
		for (int idx=0; idx<frame_size; idx++)
			buffer[idx] = (float)frame->samples[channels * idx + chan_ID];

		int n = frame_size;
		int nbEchPerPeak = ns;
		int debut = 0;
		int fin = nbEchPerPeak - (indice % nbEchPerPeak) - 1;

		if ( n < (frame_size * channels) )
			stop = 1;

		if (fin > n-1)
			fin = n-1;
		else
			finTroncon = true;

		while (true)
		{
			for (int j = debut; j < fin; j++)
			{
				float sample = fabs(buffer[j]);

				if (is > ws)
					break;

				if (sample > maxPeaks[is])
				{
					maxPeaks[is] = sample;
				}
			}

			if (finTroncon)
			{
				is++;
				finTroncon = false;
			}

			if (fin == n-1)
				break;

			debut = fin+1;

			fin = debut + nbEchPerPeak - 1;

			if (fin > n-1)
				fin = n-1;
			else
				finTroncon = true;
		}

		indice += n;
		frame = iodev->m_read();
	}

	//normalizePeaks(maxPeaks, ws);

	return maxPeaks;
}

void USAGE()
{
	Log::err() << "usage : GenPeaks [-d outdir] <audiofile>" << endl;
	exit(1);
}

// ---------------------
// --- GenPeaks Main ---
// ---------------------
int main(int argc, char**argv)
{
	int c;
	string outdir("");
	//
	// parse program options
	while ((c = getopt(argc , argv, "d:")) != -1) {
		switch (c) {
		case 'd': // service configuration file
			outdir= optarg;
			break;
		case '?':
			USAGE();
		}
	}

	// -- Args Check --
	if (optind >= argc )
	{
		USAGE();
	}


	// -- File Open --
	string		in_path	= argv[optind];
	IODevice*	device	= Guesser::open( in_path.c_str() );

	if (device == NULL)
	{
		Log::err() << "Error --> Unable to open file " << in_path << endl;
		return 1;
	}

	// -- Settings --
	MediumInfo*	info = device->m_info();

	string			peak_file;
	if ( outdir.empty() ) {
		unsigned long pos = in_path.rfind(".");
		if ( pos > 0 && pos != string::npos )
			peak_file = in_path.substr( 0, pos );
		else peak_file = in_path;
	} else {
		unsigned long pos1 = in_path.rfind("/");
		unsigned long pos2 = in_path.rfind(".");
		if ( pos1 > pos2 ) pos2 = string::npos;
		peak_file = outdir + in_path.substr(pos1, pos2 - pos1);
	}
	// -- Peaks processing (per channel) --
	for(int i=0; i<info->audio_channels; i++)
	{
		ostringstream	oss;

		oss << peak_file << "_" << i << ".pks";

		cout << "Processing & saving peaks : " << oss.str() <<  "  SZ = " << sizeof(float) << endl;

		float* tab	= computeStreamPeaks(device, i);
		int size	= (int)(info->audio_duration  / AUDIO_ZOOM_MAX);

		FILE* fd = fopen(oss.str().c_str(), "wb");

		if (fd != NULL)
		{
			int cur = 0;

			while (cur < size)
			{
				int toRead = BUFFER_SIZE;

				if (size-cur < BUFFER_SIZE)
					toRead = size-cur;

				int n = fwrite(tab + cur, sizeof(float), toRead, fd);
				cur += n;
			}
			fflush(fd);
		}
	}

	return 0;
}

