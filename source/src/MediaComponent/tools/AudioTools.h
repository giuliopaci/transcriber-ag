/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/** @file */

#ifndef AUDIOTOOLS_H
#define AUDIOTOOLS_H

#include <map>
#include <string>

#include "MediaComponent/base/Guesser.h"

using namespace std;

/**
 * @class	AudioTools
 * @ingroup	MediaComponent
 * 
 * Class providing a collection of useful methods dedicated to audio processing
 */
class AudioTools
{
public:
	AudioTools();
	~AudioTools();

	/**
	 * Opens a new output file, which will receive samples from another medium
	 * @param filePath	Medium URL
	 * @param noTrack	Track ID (relevant for stereo files) : -1 -> both tracks
	 * @return A unique file identifier
	 */
	int		openExtractFile(string filePath, int noTrack=-1);

	/**
	 * Closes a previously-opened output file
	 * @param fileID	Unique file identifier
	 * @return True on success, false otherwise.
	 */
	bool	closeExtractFile(int fileID);

	/**
	 * Appends an audio segment to a previously-opened output file
	 * @param fileID	Unique file identifier
	 * @param tsStart	Audio segment start offset
	 * @param tsEnd		Audio segment end offset
	 * @return True on success, false otherwise.
	 */
	bool	addSegment(int fileID, float tsStart, float tsEnd);

	/**
	 * Enables / disables silence intervals between audio segments
	 * @param inMode	Control variable
	 */
	void	setSilenceInterval(bool inMode)		{ silenceInterval = inMode; }

	/**
	 * Stores an external input device reference.\n
	 * Audio samples will be extracted from this device 
	 * @param inDevice	Device reference
	 */
	void	setInputDevice(IODevice* inDevice)	{ audioIn = inDevice; }

protected:
	/**
	 * Unique identifier provider
	 * @return A new unique identifier (Signed integer)
	 */
	int		getUniqueID();

private:
	map<int, SndFileHandler*> handlers;

	IODevice*		audioIn;
	int				trackID;
	bool			silenceInterval;
	MediumFrame*	silenceBuf;
	MediumFrame*	monoFrame;
};

#endif

