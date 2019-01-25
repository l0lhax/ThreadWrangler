#ifndef __HEADER_CONFIGURATIONTYPES
#define __HEADER_CONFIGURATIONTYPES

#include <stdint.h>
#include <map>
#include "ProcessorMap_Types.h"

namespace Config {

	class Configuration;
	class cProcessConfig;
	class cGroupConfig;

	typedef size_t KeyIDType;

	typedef Configuration * ConfigurationPtr;

	typedef cProcessConfig * ProcessConfigPtr;
	constexpr static ProcessConfigPtr InvalidProcessConfig = nullptr;


	typedef std::map<KeyIDType, ProcessConfigPtr> CProcessCont;
	typedef CProcessCont::iterator CProcessIter;
	typedef CProcessCont::const_iterator CProcessCIter;
	typedef std::pair<KeyIDType, ProcessConfigPtr> CProcessPair;
	typedef std::pair<CProcessIter, bool> CProcessContResult;

	typedef cGroupConfig * GroupConfigPtr;
	constexpr static GroupConfigPtr InvalidGroupConfigPtr = nullptr;

	typedef std::map<Processors::GroupIDType, GroupConfigPtr> GroupConfigCont;
	typedef GroupConfigCont::iterator GroupConfigIter;
	typedef GroupConfigCont::const_iterator GroupConfigCIter;

};

#endif