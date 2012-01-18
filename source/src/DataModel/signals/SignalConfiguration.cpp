/*
 * SignalConfiguration.cpp
 *
 *  Created on: 3 déc. 2008
 *      Author: montilla
 */

#include "SignalConfiguration.h"
#include "Common/util/Utils.h"
#include "Common/globals.h"

namespace tag {


SignalConfiguration::SignalConfiguration()
{
	singleSignal = false ;
	video_standalone = false ;
	multi_audio = false ;
	stream = false ;
	notrack_cpt = 0 ;
	nbEmptySignalTracks = 0 ;
	id_not_found="";
}

SignalConfiguration::~SignalConfiguration()
{}

void SignalConfiguration::enterAudioSignal(const std::string& sigid, const std::string& path, int notrack, int channels)
{
	enterSignal(sigid, path, notrack, "audio", channels) ;
}

void SignalConfiguration::enterVideoSignal(const std::string& sigid, const std::string& path, int notrack, int channels)
{
	enterSignal(sigid, path, notrack, "video", channels) ;
}

bool SignalConfiguration::deleteSignal(const std::string& sigid)
{
	bool done = false ;
	std::map<string,string>::iterator its ;
	for ( its = id_paths.begin(); its != id_paths.end() && !done ; ++its ) {
		if ( its->first == sigid ) {
			id_paths.erase(its);
			done = true ;
		}
	}
	return done ;
}

void SignalConfiguration::clear()
{
	id_paths.clear() ;
	id_type.clear() ;
	id_channel.clear() ;
	track_id.clear() ;
}

std::map<std::string, std::string> SignalConfiguration::getAudioIdPaths() const
{
	return getIdPathsByType("audio") ;
}

std::map<std::string, std::string> SignalConfiguration::getVideoIdPaths() const
{
	return getIdPathsByType("video") ;
}

std::map<std::string, std::string> SignalConfiguration::getIdPaths() const
{
	return id_paths  ;
}

std::vector<std::string> SignalConfiguration::getIds(const std::string& myType) const
{
	return getOnlyIdOrPath("id", myType) ;
}

std::vector<std::string> SignalConfiguration::getPaths(const std::string& myType) const
{
	return getOnlyIdOrPath("path", myType) ;
}

bool SignalConfiguration::changePath(const std::string& sigid, const std::string& path)
{
	bool done = false ;
	std::map<string,string>::iterator its ;
	for ( its = id_paths.begin(); its != id_paths.end() && !done ; ++its )
	{
		if ( its->first == sigid ) {
			its->second = path ;
			done = true ;
		}
	}
	return done ;
}

bool SignalConfiguration::changeChannel(const std::string& sigid, int channel)
{
	bool done = false ;
	std::map<string,int>::iterator its ;
	for ( its = id_channel.begin(); its != id_channel.end() && !done ; ++its )
	{
		if ( its->first == sigid )
		{
			its->second = channel ;
			done = true ;
		}
	}
	return done ;
}

bool SignalConfiguration::changeFirstPath(const std::string& path)
{
	bool done = false ;
	std::map<string,string>::iterator its ;
	its =  id_paths.begin() ;

	if (its!=id_paths.end()) {
		its->second = path ;
		done = true ;
	}

	return done ;
}

bool SignalConfiguration::isAudio(const std::string& sigid) const
{
	if ( getIdType(sigid)=="audio")
		return true ;
	else
		return false ;
}

bool SignalConfiguration::isVideo(const std::string& sigid) const
{
	if ( getIdType(sigid)=="video")
		return true ;
	else
		return false ;
}

int SignalConfiguration::getNotrack(const std::string& p_sigid) const
{
	std::map<int,string>::const_iterator it ;
	int found = -1 ;
	for (it = track_id.begin(); it!= track_id.end() && found==-1; it++)
	{
		int notrack = it->first ;
		if (it->second==p_sigid)
			found = notrack ;
	}
	return found ;
}

std::string SignalConfiguration::getSigid(int notrack) const
{
	std::map<int,string>::const_iterator it = track_id.find(notrack);
	if ( it!=track_id.end() )
		return it->second ;
	else
		return "" ;
}

int SignalConfiguration::getChannel(const std::string& sigid)
{
	std::map<string,int>::const_iterator it = id_channel.find(sigid);
	if ( it!=id_channel.end() )
		return it->second ;
	else
		return -1 ;
}

std::string SignalConfiguration::getFirstFile(const std::string& mode) const
{
	std::vector<std::string> paths ;
	if (mode == "audio")
		paths = getPaths("audio") ;
	else if (mode == "video")
		paths = getPaths("video") ;
	else
		paths = getPaths("") ;

	if (paths.size()==0)
		return "" ;
	else
		return paths[0] ;
}


void SignalConfiguration::checkTracks()
{
	// TODO ? usefull ?
	if (getNbSignals("")!=1)
		return ;

	std::map<int,string>::iterator it = track_id.begin() ;
	if (it->first!=0)
	{
		track_id[0] = it->second ;
		track_id.erase(it->first) ;
	}
}

int SignalConfiguration::getNbSignals(const std::string& type)
{
	if (nbEmptySignalTracks!=0)
		return nbEmptySignalTracks ;
	else if (isSingleSignal())
		return 1 ;
	else
		return getIds(type).size() ;
}

//******************************************************************************
//*********************************** INTERNAL *********************************
//******************************************************************************

std::vector<std::string> SignalConfiguration::getOnlyIdOrPath(const std::string& mode, const std::string& myType) const
{
	int cpt = 0 ;
	std::vector<string> res ;
	std::map<string,string>::const_iterator it ;
	for (it=id_paths.begin(); it!=id_paths.end(); it++)
	{
		/*** single signal ***/
		// in single signal case, only allow 1 signal
		if (!singleSignal || res.size()==0)
		{
			string id = it->first ;
			string path = it->second ;
			if (mode=="id" && (getIdType(id) == myType || myType.empty()) )
				res.push_back(id) ;
			else if (mode=="path" && (getIdType(id) == myType || myType.empty()) )
				res.push_back(path) ;
		}
	}
	return res ;
}

std::map<std::string, std::string> SignalConfiguration::getIdPathsByType(const string& myType) const
{
	std::map<string,string> res ;
	std::map<string,string>::const_iterator it ;
	std::map<string,string>::const_iterator current ;

	string sigid ;
	string type ;
	string path ;

	for (it=id_paths.begin(); it!=id_paths.end(); it++)
	{
		/*** single signal ***/
		// in single signal case, only allow 1 signal
		if (!singleSignal || res.size()==0)
		{
			sigid = it->first ;
			path = it->second ;
			type = "" ;

			current = id_type.find(sigid) ;
			if (current!=id_type.end()) {
				type = current->second ;
				if (type == myType)
					res[sigid] = path ;
			}
		}
	}
	return res ;
}

void SignalConfiguration::enterSignal(const std::string& sigid, const std::string& path, int notrack, const std::string& type, int channels)
{
	if (notrack!=-1)
	{
		id_channel[sigid] = channels ;
		id_paths[sigid] = path ;
		id_type[sigid] = type ;
		track_id[notrack] = sigid ;
		// set that we're not dealing with track numbering
		notrack_cpt = -1 ;
	}
	else if (notrack==-1 && notrack_cpt!=-1)
	{
		id_channel[sigid] = channels ;
		id_paths[sigid] = path ;
		id_type[sigid] = type ;
		track_id[notrack_cpt] = sigid ;
		// use automatic numerotation
		notrack_cpt ++ ;
	}
	else {
		TRACE_D << "SignalConfiguration::enterSignal:> automatic numbering disabled but asked for adding, abort." << std::endl ;
	}
}

bool SignalConfiguration::hasVideo()
{
	std::vector<std::string> videos = getIds("video") ;
	if (videos.size()>0)
		return true ;
	else
		return false ;
}

const std::string& SignalConfiguration::getIdType(const std::string& id) const
{
	std::map<std::string,std::string>::const_iterator it = id_type.find(id);
	if ( it != id_type.end() ) return it->second;
	return id_not_found;
}


//******************************************************************************
//********************************** MONITORING ********************************
//******************************************************************************

void SignalConfiguration::print() const
{
	TRACE << "SignalConfiguration::print ***********************" <<std::endl ;
	TRACE << "Empty channels : " << nbEmptySignalTracks << std::endl ;
	TRACE << "Single mode : " << singleSignal << std::endl ;
	TRACE << "Multi audio mode : " << multi_audio << std::endl ;
	TRACE << "Video mode : " << video_standalone << std::endl ;
	TRACE << "Stream mode : " << stream << std::endl ;

	std::map<string,string>::const_iterator it ;

	for (it=id_paths.begin(); it!=id_paths.end(); it++)
	{
		const string& sigid = it->first ;
		const string& path = it->second ;
		const string& type = getIdType(sigid) ;
		int notrack = getNotrack(sigid) ;
		TRACE << "id= " << sigid << " - path= " << path << " - type= " << type << " - notrack= " << notrack << std::endl ;
	}
	TRACE << "************************************************* enD" <<std::endl ;
}


} // namespace
