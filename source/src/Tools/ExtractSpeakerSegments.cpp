/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

#define MEDIACOMPONENT_STANDALONE 1
#define GUESSER_TRACE 1

#include <string>
#include <iostream>

#include "MediaComponent/tools/AudioTools.h"
#include "DataModel/DataModel.h"
#include "Common/FileInfo.h"
#include "Common/util/Log.h"

using namespace tag;
using namespace std;

const char* suffixes[] = {
		".wav", ".WAV",
		".mp3", ".MP3",
		NULL,
};

string lookForMediaFile(const string& dir, const string& signalFilename)
{
	const char* pt;
	int i=0;
	string path;

	for (i=0; suffixes[i] != NULL; ++i ) {
		path = dir + "/" + signalFilename + suffixes[i];
		if ( FileInfo(path).exists() ) return path;
	}
	return "";
}

string getSignalPath(tag::DataModel& data)
{

	const SignalConfiguration& signalCfg = data.getSignalCfg();
	const std::map<string,string>& signalIds = signalCfg.getIdPaths() ;

	std::map<string,string>::const_iterator it ;
	string signalFilename = "";
	string sigid = "";
	for (it=signalIds.begin(); it!=signalIds.end(); it++)
	{
		sigid = it->first;
		if (signalCfg.isAudio(sigid)) {
			signalFilename = it->second ;
			break;
		}
	}
	if ( signalFilename.empty() ) return "";

	string path = signalFilename ;
	string dir = FileInfo(signalFilename).dirname() ;

	//> if signal file name is relative, let's find that signal
	if (signalFilename == FileInfo(signalFilename).Basename()	)
	{
		// location of signal file not defined in URL -> try to locate it
			dir = data.getSignalProperty(sigid, "path_hint");
			path = "" ;
		// look in default directory
		if ( !dir.empty() )
			path = lookForMediaFile(dir, signalFilename);
		// look for signal file in annot file dir
		if ( path.empty() ) {
			dir = FileInfo(data.getPath()).dirname();
			path = lookForMediaFile(dir, signalFilename);
		}
	}
	return path;
}


/**
 * extract audio segments for given speaker id following segment annotations provided in tag file
 * @param argc 	argument count
 * @param argv	argument array : <progname> <annot_file> <spkid> <outfile>
 * @return 0 if ok, else 1
 */

int main(int argc, const char* argv[])
{
	string progname = FileInfo(argv[0]).Basename();

	if (argc < 4)
	{
		Log::err() << "Usage:" << progname << " <annot_file> <spkid> <outfile> " << endl;
		return 1;
	}


	tag::DataModel::initEnviron("");

	int fileID = -1;
	tag::DataModel data("TransAG");

	string inFile (argv[1]);
	string spkId(argv[2]);
	string outFile(argv[3]);

	// -- load data model from TAG file --
	try
	{
		string format = data.guessFileFormat(inFile);
		if ( format.empty() ) {
			Log::err() << "ERROR: " << progname << " unknown file format, file=" << inFile << endl;
			return 1;
		}
		data.loadFromFile(inFile, format);
	} catch (const char* msg ) {
		Log::err() << "ERROR : " << msg << endl;
		return 1;
	}

	string wavFile = getSignalPath(data);
	if ( wavFile.empty() || ! FileInfo(wavFile).exists() ) {
		Log::err() << "File not found: " << wavFile << endl;
		return 1;
	}

	// -- Opening input device --
	Log::trace() << "Opening audio file " <<  wavFile << endl;
	IODevice*	device	= Guesser::open( wavFile.c_str() );

	if (!device)
	{
		Log::err() << "Failed to open audio file " << wavFile << endl ;
		return 1;
	}


//	// -- set space between turn --
//	if ( argc > 4 )
//		aTools->setSilenceInterval(true);

	// -- add segments from TAG file --
	vector<SignalSegment> v0;
	vector<SignalSegment>::iterator it0;

	tag::SpeakerDictionary::iterator its;
	tag::SpeakerDictionary& dic = data.getSpeakerDictionary();
	for (its = dic.begin(); its != dic.end(); ++its) {
		if ( its->second.getProperty("scope") == spkId ) {
			spkId = its->second.getId();
			break;
		}
	}

	data.getSegments("turn", v0, 0.0, 0.0);

	int notrack = -1;

	if ( data.getNbTracks() > 1 ) {
		// then we have to extract only the track for given speaker
		for(it0 = v0.begin(); it0 != v0.end(); )
		{
			if ( data.getElementProperty(it0->getId(), "speaker") == spkId ) {
				notrack = data.getElementSignalTrack(it0->getId());
				break;
			}
			it0 = v0.erase(it0);
		}
	}

	if ( v0.size() == 0 ) {
		Log::err() << "No segment found for speaker " << spkId << " in file " << inFile << endl;
		return 1;
	}

	// -- New AudioTools instance --
	AudioTools*	aTools	= new AudioTools();

	Log::trace() << "Writing to output file " <<  outFile << endl;

	aTools->setInputDevice(device);

		// -- Creating new output file (for extraction) --
	fileID = aTools->openExtractFile( outFile, notrack);


	for(it0 = v0.begin(); it0 != v0.end(); ++it0)
	{
		if ( data.getElementProperty(it0->getId(), "speaker") == spkId ) {
			// -- Inserting segments --
			aTools->addSegment(fileID, it0->getStartOffset(), it0->getEndOffset());
		}
	}

	// -- Close ouptput file --
	aTools->closeExtractFile(fileID);

	return 0;
}

