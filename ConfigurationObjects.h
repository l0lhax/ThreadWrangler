#ifndef __HEADER_CONFIGURATIONOBJECTS
#define __HEADER_CONFIGURATIONOBJECTS

#include "Configuration_Types.h"

namespace Config {

	class cGroupConfig {
	public:
		Processors::AffinityType DTC;
		Processors::AffinityType ITC;
		Processors::AffinityType Excluded;
		bool ExcludeGroup;
		bool ExcludeHTCores;
		Processors::GroupIDType GroupID;

		void Defaults() {
			DTC = UINT64_MAX;
			ITC = UINT64_MAX;
			Excluded = 0;
			ExcludeGroup = false;
			ExcludeHTCores = false;
		}

		cGroupConfig(Processors::GroupIDType NewGroupID) {
			GroupID = NewGroupID;
			Defaults();
		}
	};


	class cProcessConfig {

	private:
		GroupConfigCont _GroupConfig;


	public:

		KeyIDType KeyID;
		std::wstring Executable;
		bool IdealThreadMode;
		bool AffinityMode;
		bool MonitorOnly;
		Algorithms::TSAlgorithm Algorithm;
		Algorithms::ViewSort SortMode;

		uint8_t ProcessorsPerThread;

		bool ShowActiveThreads;
		bool ShowDormentThreads;
		bool ShowSleepingThreads;

		bool ChangesPending;
		bool AdvChangesPending;
		bool DeletePending;

		bool Selected;

		size_t GetGroupCount() {
			return _GroupConfig.size();
		}

		inline GroupConfigIter group_begin() noexcept { return _GroupConfig.begin(); }
		inline GroupConfigCIter group_cbegin() const noexcept { return _GroupConfig.cbegin(); }
		inline GroupConfigIter group_end() noexcept { return _GroupConfig.end(); }
		inline GroupConfigCIter group_cend() const noexcept { return _GroupConfig.cend(); }

		GroupConfigPtr GetGroupConfig(Processors::GroupIDType GroupID) {

			GroupConfigPtr tGroupConfig = InvalidGroupConfigPtr;

			GroupConfigIter it = _GroupConfig.find(GroupID);
			if (it == _GroupConfig.end()) {
				tGroupConfig = new cGroupConfig(GroupID);
				tGroupConfig->Defaults();
				_GroupConfig.emplace(GroupID, tGroupConfig);
			}
			else {
				tGroupConfig = it->second;
			}

			return tGroupConfig;
		}

		void Defaults() {
			IdealThreadMode = true;
			AffinityMode = true;
			MonitorOnly = false;
			Algorithm = Algorithms::TSI_UserTime_Sticky;
			SortMode = Algorithms::vs_ByAvgUserWallTime;

			ShowActiveThreads = true;
			ShowDormentThreads = true;
			ShowSleepingThreads = true;

			DeletePending = false;

			Selected = false;

			ProcessorsPerThread = 0;
		}

		cProcessConfig(KeyIDType NewKeyID) {
			KeyID = NewKeyID;
			ChangesPending = false;
			AdvChangesPending = false;
			Defaults();
		}
	};

};

#endif
