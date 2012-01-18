/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/
/**
* fast transcription to tag converter
*/

#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <vector>
#include <list>
#include <ag/AGAPI.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "DataModel/DataModel.h"
#include "Common/util/ExpRegul.h"
#include "Common/VersionInfo.h"
#include "Common/FileInfo.h"
#include "Common/util/StringOps.h"
#include "Common/iso639.h"


using namespace tag;


/**
 * print out help message on program usage and exit
 * @param progname current program name
 */

void USAGE(const char* progname)
{
	Log::err() << "USAGE: " << progname << " [-h] [-v] [-d <time>] [-f <time>] [-t <time>] [-l <lang>] [-c <conv>] [-PI] [-s audiofilename] <filename>" << endl;
	Log::err() << "options :" << endl;
	Log::err() << "\t-d <time> : transcripted file duration" << endl;
	Log::err() << "\t-f <time> : transcription start time'"  << endl;
	Log::err() << "\t-t <time> : transcription end time " << endl;
	Log::err() << "\t-c <conv> : applicable conventions for transcription conversion" << endl;
	Log::err() << "\t-l <lang> : iso639-2 language code for transcription (overrides any language information found in file)" << endl;
	Log::err() << "\t-PI       : \"&\" turns converted as [pron=pi] annot" << endl;
	Log::err() << "\t-v : print program version " << endl;
	Log::err() << "\t-h : print this message " << endl;
	Log::err() << "Where <time> can be noted in one of the following forms : " << endl;
	Log::err() << "\twithout millisecond precision : 'hh:mm:ss' 'mm:ss' 'sss'" << endl;
	Log::err() << "\twith millisecond precision : 'hh:mm:ss.ddd' 'mm:ss.ddd' 'sss.ddd'" << endl;
	exit(1);
}

/**
 * parse time string with format "hh:mm:ss.ddd" or "ss.ddd"
 * @param s string to parse
 * @return time in seconds as float value
 *
 * @note if time format is invalid, exit program.
 */
float parseTimeStr(string s)
{
	float t;
	bool okformat = true;
	if ( s.find(':') != string::npos ) {
		int hh,mm,ss,dd;
		int cnt = sscanf(s.c_str(), "%d:%d:%d.%d", &hh, &mm, &ss, &dd);
		if ( cnt < 2 ) {
			okformat = false ;
		} else {
			int div = 10;
			if ( cnt == 2 ) {
				ss=mm; mm=hh; hh=0;
			}
			if ( cnt < 4 ) dd=0;
			else {
				while (dd > div) div *= 10;
			}
			t = (hh*3600) + (mm*60) + ss + dd/div;
		}
	} else {
		char* pt;
		t = strtod(s.c_str(), &pt);
		if ( *pt != 0 ) {
			okformat = false ;
		}
	}

	if ( okformat == false ) {
		Log::err() << _("Invalid time format : should be hh:mm:ss.ddd or ss.ddd") << endl;
		exit(1);
	}
	return t;
}

/**
 * get convention definition file path
 * @param progname current program path
 * @param conventions applicable conventions name
 * @return convention definition file path
 *
 * @note configuration files are located in [program_path]/../etc/TransAG/conventions
 */
string getConfigurationFile(const char* progname, string conventions)
{
	gchar* cfgdir = g_path_get_dirname(progname);
	if ( !g_path_is_absolute (cfgdir) ) {
		gchar* curdir = g_get_current_dir();
		g_chdir(cfgdir);
		g_free(cfgdir);
		cfgdir = g_get_current_dir();
		g_chdir(curdir);
		g_free(curdir);
	}
	gchar* pdir = g_path_get_dirname(cfgdir);
	g_free(cfgdir);
	cfgdir = g_build_filename(pdir, "etc", "TransAG", conventions.c_str(), NULL);
	g_free(pdir);
	string cfgfic=cfgdir;
	g_free(cfgdir);
	return cfgfic;
}

/**
 * check library path includes aglib plugins dir
 */
void checkLibraryPath(const char* progname)
{
	if (g_getenv("LD_LIBRARY_PATH") == NULL ) {
		string lib_path("");
		if ( FileInfo("/usr/local/lib/ag").exists() ) {
			lib_path = "/usr/local/lib:/usr/local/lib/ag";
		} else {
			gchar* cfgdir = g_path_get_dirname(progname);
			if ( !g_path_is_absolute (cfgdir) ) {
				gchar* curdir = g_get_current_dir();
				g_chdir(cfgdir);
				g_free(cfgdir);
				cfgdir = g_get_current_dir();
				g_chdir(curdir);
				g_free(curdir);
			}
			gchar* pdir = g_path_get_dirname(cfgdir);
			g_free(cfgdir);
			cfgdir = g_build_filename(pdir, "lib", NULL);
			lib_path = cfgdir; lib_path += ":";
			g_free(pdir);
			pdir = cfgdir;
			cfgdir = g_build_filename(pdir, "ag", NULL);
			lib_path += cfgdir;
			g_free(cfgdir);
			g_free(pdir);
			g_setenv("LD_LIBRARY_PATH", lib_path.c_str(), 0);
		}
	}
}

struct _lcode {
	const char* acro;
	const char* iso;
} LanguageCodes[] = {
		{ "GB", "eng" },
		{ "FR", "fre" },
		{ NULL, NULL },
};

int main(int argc, char* const argv[])
{
	float startTime = 0.0;
	float endTime = 0.0;
	float duration = 0.0;
	string lang("");
	string conventions="mono_h4_detailed";
	float minsz = 0.5;  // min seg size = 1 second.
	bool ok_duration = false;
	bool do_pi = false;
	string signalFormat;
	string signalEncoding;
	int signalNbTracks;
	int retcode=0;
	string signalFilename ("");
	string corpusName="TransAG";

	ExpRegul expInfo("# *[iI]nformations *[ =:]? *([^ ]+) +([^ ]+) +([^ ]+) +([^ ]+) +([^-]+)-([^ ]+)");
	ExpRegul expStartTime("# *[sS]tart *[ =:]? *([^ \n]+)");
	ExpRegul expEndTime("# *[eE]nd *[ =:]? *([^ \n]+)");
	ExpRegul expDuration("# *[dD]uration *[ =:]? *([^ \n]+)");
	ExpRegul expTurn("[sS]([0-9]+)(\\(.*\\))?/ *(.*) *$");
	ExpRegul expSpeaker("\\( *([MF]) *([ -] *(.*) *\\))?");

	const char* progname = argv[0];

	checkLibraryPath(progname);

	int c;

	while ((c = getopt(argc , argv, "c:d:f:l:t:vhP:s:")) != -1) {
		switch (c) {
		case 'd':
			duration=parseTimeStr(optarg);
			ok_duration=true;
			break;
		case 'f':
			startTime=parseTimeStr(optarg);
			break;
		case 't':
			endTime=parseTimeStr(optarg);
			break;
		case 'l':
			lang=optarg;
			break;
		case 'c':
			conventions=optarg;
			break;
		case 's' :
			signalFilename = optarg;
			break;
		case 'P':
			if ( strcmp(optarg, "I") == 0 ) do_pi=true;
			else  USAGE(progname);
			break;
		case 'v':
			cout << progname << " version " << TRANSAG_VERSION_NO << endl;
			return 0;
		default: USAGE(progname);
		}
	}


	if ( optind == argc ) USAGE(progname);
	const char* filename = argv[optind];

	string cfgfic = conventions;
	if ( ! FileInfo(cfgfic).exists() )
		cfgfic = getConfigurationFile(progname, conventions);

	AudioFileInputStream* signal = NULL;


	ifstream fi (filename);
	if ( !fi.good() ) {
		Log::err() << _("Can't open file for reading: ") << filename << endl;
		return 1;
	}

	signalFormat="WAV";
	signalEncoding="";
	signalNbTracks= 1;

	if ( !ok_duration ) {
		// guessing time from audio file
		if ( signalFilename.empty() ) {
			FileInfo info(filename);
			info.setTail("wav");
			if ( !info.exists() ) 	info.setTail("WAV");
			if ( !info.exists() ) {
//				Log::err() << _("Signal file not found, please indicate signal duration") << endl;
//				exit(1);
			} else
				signalFilename = info.path();
		} else {
			if ( ! FileInfo(signalFilename).exists() ) {
				Log::err() << "Audio file not found : " << signalFilename << endl;
				exit(1);
			}
		}
		if ( ! signalFilename.empty() ) {
			AudioFileInputStream* signal = new AudioFileInputStream((char*)(signalFilename.c_str()));
			duration = (signal->getTotalSamplesCount() / signal->getSamplingRate());

			signalEncoding=signal->getEncodingString();
			signalNbTracks= signal->getChannelsCount();
			delete signal;
		}
	}

	if ( signalFilename.empty() ) {
		FileInfo info(filename);
		info.setTail("wav");
		signalFilename = info.path();
	}

	DataModel data("TransAG");

	string buf;
	int i;
	const char* pt, *pt2;
	unsigned long off=0;

	float sigLength = (endTime - startTime);
	struct stat bufstat;
	stat(filename, &bufstat);
	float filesz = bufstat.st_size;
	map <string, string> spkids;
	bool config_done=false;
	string segmentId("");
	string turnId("");
	bool add_nospeech_sect=false;
	float elapsed;
	bool prev_is_pi = true;
	string prev_spkid = "";

	try {

		while ( fi.good() ) {
			getline(fi, buf);

			for ( pt=buf.c_str(); *pt && isspace(*pt);  ++pt);
			if ( !*pt  ) continue;
			if ( *pt == '#' ) {
				filesz -= buf.length();
				off=0;
				if ( expInfo.match(buf, off) ) {
					if ( lang == "" ) {
						// not forced on command line
						string item = expInfo.getSubmatch(buf, 1);

						for (i=0; LanguageCodes[i].acro != NULL && strcasecmp(LanguageCodes[i].acro, item.c_str()) != 0; ++i);
						if ( LanguageCodes[i].acro != NULL ) lang = LanguageCodes[i].iso;
						else {
							if ( item.length() == 2 ) {
								const char* pl = ISO639::get3LetterCode(item.c_str());
								if ( pl != NULL ) lang = pl;
							} else if (item.length() == 3) {
								// check validity
								const char* pl = ISO639::get2LetterCode(item.c_str());
								if ( pl != NULL ) lang=item;
							}
						}
					}
				} else {
					if ( expStartTime.match(buf, off) ) {
						startTime = parseTimeStr(expStartTime.getSubmatch(buf, 1));
					} else
					if ( expEndTime.match(buf, off) ) {
						endTime = parseTimeStr(expEndTime.getSubmatch(buf, 1));
					}
					if ( expDuration.match(buf, off) ) {
						duration = parseTimeStr(expDuration.getSubmatch(buf, 1));
					}
				}
			} else {
				if ( ! config_done ) {
					// initialize data model for applicable conventions
					if (lang == "" ) {
						Log::err() << _("Unknown transcription language, using 'eng' as default");
						lang = "eng";
					}
					try {
						data.configure(cfgfic, lang);
					} catch ( const char* msg ) {
						Log::err() << msg << endl;
						data.configure("");
					}


					if ( endTime > duration ) {
						Log::err() << _("Transcription end time set to signal duration ") << duration << " secs" << endl;
						endTime = duration;
					} else if ( endTime == 0 ) endTime = duration;
					sigLength = (endTime - startTime);

					data.initAGSet(corpusName);

					data.addSignal(signalFilename, "audio", signalFormat, signalEncoding, signalNbTracks);

					data.setSignalDuration(duration);

					// initialize transcription graph
					data.initAnnotationGraphs("", lang, "fft2tag");
					config_done = true;
				}

				off=0;
				if (expTurn.match(buf, off) ) {
					string spkid = expTurn.getSubmatch(buf, 1);
					string spkinfo = expTurn.getSubmatch(buf, 2);
					string text = expTurn.getSubmatch(buf, 3);

					if ( spkids.find(spkid) == spkids.end() ) {
						string dicoId = "spk"+spkid;
						Speaker spk ;
						if ( data.getSpeakerDictionary().existsSpeaker(String(dicoId)) ) {
							spk = data.getSpeakerDictionary().getSpeaker(dicoId);
						} else {
							spk = data.getSpeakerDictionary().defaultSpeaker();
							data.getSpeakerDictionary().addSpeaker(spk);
						}
						// parse speaker info
						if ( ! spkinfo.empty() ) {
							off=0;
							if ( expSpeaker.match(spkinfo, off) ) {
								string gender = expSpeaker.getSubmatch(spkinfo, 1);
								spk.setGender(gender.c_str());
								string name = expSpeaker.getSubmatch(spkinfo, 3);
								if ( !name.empty() ) {
									unsigned long pos;
									if ( (pos=name.find(' ')) != string::npos ) {
										spk.setFirstName(name.substr(0,pos));
										spk.setLastName(name.substr(pos+1));
									} else
										spk.setLastName(name);
								}
							}
						}
						data.getSpeakerDictionary().updateSpeaker(spk, false);
						spkids[spkid] = spk.getId();
					}

					// insert speech segment
					// create turn
					if ( segmentId == "" && startTime > 0.0 ) {
						// let no speaker turn at file start
						segmentId = data.getByOffset(data.mainstreamBaseType(), 0.0);
						turnId =  data.getParentElement(segmentId);
						data.deleteElementProperty(turnId, "speaker", false);
						if ( startTime > 30.) { // add section
							add_nospeech_sect=true;
						}
					}

					if ( segmentId == "" ) {
						// set segment text
						segmentId = data.getByOffset(data.mainstreamBaseType(), 0.0);
						turnId =  data.getParentElement(segmentId);
						data.setElementProperty(turnId, "speaker", spkids[spkid], false);
					} else {
						if ( ! prev_is_pi || prev_spkid != spkid ) {
							segmentId = data.addSegment(segmentId, startTime);
							turnId = data.addTurn(segmentId, 0, spkids[spkid], false);
						}
						if ( do_pi && text == "&" ) {
							// add [pron_pi] to current segment start
							string qid = data.addQualifier("pronounce", segmentId, 0, false, "", 0,
											"unintelligible", false, false);
							text="";
							prev_is_pi = true;
	//						segmentId = data.getBaseTypeEndId(qid);

						} else prev_is_pi = false;
					}

					prev_spkid = spkid;

					if ( !text.empty() ) {
						data.setElementProperty(segmentId, "value", text);
						// compute avg turn length
						elapsed =  sigLength * ((float)(buf.length())/filesz);
					} else elapsed = 0.0;

					if ( add_nospeech_sect )  {
						string secId =  data.getParentElement(turnId);
						data.setElementProperty(secId, "type", "nontrans");
						data.addSection(turnId, "report" ,false);
						add_nospeech_sect = false;
					}

				} else {
					// plain text to be added to previous segment
					string curtext = data.getElementProperty(segmentId, "value");
					if ( ! isspace(buf[0]) ) curtext += " ";
					curtext += StringOps(buf).trim();
					data.setElementProperty(segmentId, "value", curtext);
					elapsed =  sigLength * ((float)(buf.length())/filesz);
				}
				if ( elapsed < minsz ) elapsed = minsz;
				startTime += elapsed;
			}
		}
		if ( endTime < duration ) {
			// add empty segment at end
			data.addSegment(segmentId, endTime);
			turnId = data.addTurn(segmentId, 0, Speaker::NO_SPEECH, false);
			data.addSection(turnId, "nontrans" , false);
		}

		data.updateVersionInfo("txt2tag", "1");

		FileInfo tagFile(filename);
		tagFile.setTail("tag");
		data.saveToFile(tagFile.path(), "TransAG", true);

	} catch (const char* msg) {
		Log::err() << "Error : " << msg << endl;
		fi.close();
		return 1;
	}

	fi.close();
	return 0;
}


