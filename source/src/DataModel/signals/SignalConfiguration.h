/********************************************************************************/
/*************** Copyright (C) 2006-2011 Bertin Technologies, SAS  **************/
/*			  				      	TranscriberAG	 							*/
/* 	         																	*/
/* See COPYING for license information										  	*/
/* 	         																	*/
/********************************************************************************/

/**
*  @file 	SignalConfiguration.h
*/


#ifndef SIGNALCONFIGURATION_H_
#define SIGNALCONFIGURATION_H_

//#include <gtkmm.h>
#include <map>
#include <vector>
#include <string>

namespace tag {

/**
 *  @class SignalConfiguration
 *  @ingroup DataModel
 *
 * This class centralizes the signals used for a given transcription.\n
 * It associates signal Id (aglib meaning) to the file path.
 */

class SignalConfiguration
{
	public:

		SignalConfiguration();  /**< Default constructor */
		virtual ~SignalConfiguration(); /**< destructor */

		/**
		 * Accessor to the IdPath map.\n
		 * The IdPath gets the file path corresponding to an signal AG identifier.
		 * @return		Map { agid - file path }
		 */
		std::map<std::string, std::string> getIdPaths() const;

		/**
		 * Accessor to the IdPath map for audio mode.\n
		 * The IdPath gets the file path corresponding to an signal AG identifier,
		 * but only for audio signals
		 * @return		Map { audio agid - audio file path }
		 */
		std::map<std::string, std::string> getAudioIdPaths() const;

		/**
		 * Accessor to the IdPath map for video mode.\n
		 * The IdPath gets the file path corresponding to an signal AG identifier,
		 * but only for audvideo signals
		 * @return		Map { video agid - audio file path }
		 */
		std::map<std::string, std::string> getVideoIdPaths() const;

		/**
		 * Accessor to all signal id.
		 * @param myType	"audio", "video", or empty for all
		 * @return			A vector with all signal id used.
		 * @note			Only audio mode is available for current version.
		 */
		std::vector<std::string> getIds(const std::string& myType) const;

		/**
		 * Accessor to all signal file paths.
		 * @param myType	"audio" or "video
		 * @return			A vector with all signal file paths used.
		 * @note			Only audio mode is available for current version.
		 */
		std::vector<std::string> getPaths(const std::string& myType) const;

		/**
		 * Checks the multi-audio mode.\n
		 * True if the transcription file is associated to more
		 * than one signal file.
		 * @return		True for multi-audio mode, False otherwise.
		 */
		bool isMultiAudio() { return multi_audio ; }

		/**
		 * Modifies multi-audio mode.
		 * @param value		True for multi-audio mode, False otherwise.T
		 */
		void setMultiAudio(bool value) { multi_audio = value ; }

		/**
		 * Adds a new audio signal for current configuration.
		 * @param sigid			Signal id (aglib)
		 * @param path			File path
		 * @param notrack		Track number
		 * @param channels		Channels
		 */
		void enterAudioSignal(const std::string& sigid, const std::string& path, int notrack, int channels=1) ;

		/**
		 * Adds a new video signal for current configuration.
		 * @param sigid			Signal id (aglib)
		 * @param path			File path
		 * @param notrack		Track number
		 * @param channels		Channels
		 */
		void enterVideoSignal(const std::string& sigid, const std::string& path, int notrack, int channels=1) ;

		/**
		 * Accessor to the first listed file corresponding
		 * to the given mode.
		 * @param mode			"audio" or "video"
		 * @return				The file path, or empty if none was found
		 * @note				Only audio mode is available for current version.
		 */
		std::string getFirstFile(const std::string& mode) const;

		/**
		 * Removes a signal from configuration
		 * @param sigid		Signal id (aglib)
		 * @return			True for success, False for failure.
		 */
		bool deleteSignal(const std::string& sigid) ;

		/**
		 * Modifies the path associated to the given signal id.
		 * @param sigid		Signal id (aglib)
		 * @param path		New file path
		 * @return			True for success, False for failure.
		 */
		bool changePath(const std::string& sigid, const std::string& path) ;

		/**
		 * Modifies the bu associated to the given signal id.
		 * @param sigid		Signal id (aglib)
		 * @param channel	Number of channel
		 * @return			True for success, False for failure.
		 */
		bool changeChannel(const std::string& sigid, int channel) ;

		/**
		 * Modifies the path associated to the first listed signal id.
		 * @param path		New file path
		 * @return			True for success, False for failure.
		 */
		bool changeFirstPath(const std::string& path) ;

		/**
		 * Clear all signal configuration.
		 */
		void clear() ;

		/**
		 * Checks if the given signal is an audio one.
		 * @param sigid		Signal id
		 * @return			True for audio file, False otherwise.
		 */
		bool isAudio(const std::string& sigid) const;

		/**
		 * Checks if the given signal is an video one.
		 * @param sigid		Signal id
		 * @return			True for video file, False otherwise.
		 */
		bool isVideo(const std::string& sigid) const;

		/**
		 * Accessor to the track number associated to a given signal.
		 * @param sigid		Signal id
		 * @return			The track number corresponding.
		 */
		int getNotrack(const std::string& sigid) const;

		/**
		 * Accessor to the signal id associated to a given track number .
		 * @param notrack 	Track number (indice)
		 * @return			The signal id corresponding.
		 */
		std::string getSigid(int notrack) const ;

		/**
		 * Accessor to the number of channels of the signal of given id .
		 * @param sigid		Signal id
		 * @return			Number of channels, or -1 if signal id not found
		 */
		int getChannel(const std::string& sigid)  ;

		/**
		 * Print configuration to standard output.
		 * @remarks			Debug method
		 */
		void print()  const;

		/**
		 * Checks if video mode is on
		 * @return			True if at least one of signals is a video one
		 */
		bool hasVideo() ;

		/**
		 * Specifies whether a video file is used with associated audio file
		 * @param value		True for video file only, False for video file and audio files separated
		 */
		void setVideoStandAlone(bool value) { video_standalone = value ;}

		/**
		 * Indicates whether a video file is used with associated audio file
		 */
		bool isVideoStandAlone() { return video_standalone ; }

		/**
		 * If set to true, indicates that a multi channel signal will be treated as a mono signal
		 * @param p_singleSignal		True or False
		 */
		void setSingleSignal(bool p_singleSignal) { singleSignal = true ; }

		/**
		 * Indicates whether a multi channel signal configuration is considered as a mono signal
		 * @return		True if we force the mono signal configuration behaviour
		 * @note		If set to true, only one track is visible in the audio component,
		 * 				and the editor (model and view) only displays the track 1 data.
		 */
		bool isSingleSignal() {return singleSignal ;}

		/**
		 * Indicates whether the single signal mode is allowed for this signal configuration
		 * @return		True or False
		 * @see			isSingleSignal() method
		 */
		bool canSingleSignal() {return id_paths.size()>1 ;}

		/**
		 * Sets the number of empty channels used (for no signal openeing mode)
		 * @param nb	Number of channels
		 */
		void setEmptySignalTracks(int nb) { nbEmptySignalTracks = nb ; }

		/**
		 * Indicates the number of empty channels used (for no signal openeing mode)
		 * @return 		Number of empty channels
		 */
		int getEmptySignalTracks() { return nbEmptySignalTracks ; }

		/**
		 * Check and adjust signal track indice and signal id
		 */
		void checkTracks() ;

		/**
		 * Returns the number of tracks
		 * @param type		"audio", "video", "" for all
		 * @return
		 */
		int getNbSignals(const std::string& type) ;

	private:

		//TODO create object signal for id - data associations (too much data now)

		/* map (signal id - signal path) */
		std::map<std::string, std::string> id_paths ;

		/* map (signal id - nbChannel) */
		std::map<std::string, int> id_channel ;

		/* map (signal id - signal type) */
		std::map<std::string, std::string> id_type ;

		/* map (notrack - signal id) */
		std::map<int, std::string> track_id ;

		// internal cpt for track numbering
		int notrack_cpt ;

		/* whether the video file embeds the sound */
		bool video_standalone ;
		/* whether the file is using several audio files */
		bool multi_audio ;
		/* whether the audio is got by streaming */
		bool stream ;
		/* whether we force the application to treat multi channels signal as a mono */
		bool singleSignal ;
		/* whether a file has been opened without signal: empty signal */
		int nbEmptySignalTracks ;

		std::map<std::string, std::string> getIdPathsByType(const std::string& myType) const;
		std::vector<std::string> getOnlyIdOrPath(const std::string& mode, const std::string& myType) const;

		const std::string& getIdType(const std::string& id) const;

		void enterSignal(const std::string& sigid, const std::string& path, int notrack, const std::string& type, int channels) ;

		std::string id_not_found;
} ;

}  //namespace

#endif /* SIGNALCONFIGURATION_H_ */
