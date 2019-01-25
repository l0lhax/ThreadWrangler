#ifndef __HEADER_PROCESSORMAP
#define __HEADER_PROCESSORMAP

#include "ProcessorMap_Group.h"
#include "Configuration.h"

namespace Processors {

	class ProcessorMap {
	public:

		AffinityType _MaxAffinity;
		AffinityType _ExclusionAffinity;
		bool _InvalidateExclusion;
		bool _InvalidateMaxAffinity;
		bool _NUMADetected;


	private:
		GroupContainer _Groups;

	public:
		typedef ProcessorMap * Instance;

		static Instance getInstance();

		GroupPtr AddGroup(GroupIDType GroupID);
		GroupPtr GetGroup(GroupIDType GroupID);
		bool RemoveGroup(GroupIDType GroupID);
		void RemoveGroups();

		inline GroupsIter begin() noexcept { return _Groups.begin(); }
		inline GroupsCIter cbegin() const noexcept { return _Groups.cbegin(); }
		inline GroupsIter end() noexcept { return _Groups.end(); }
		inline GroupsCIter cend() const noexcept { return _Groups.cend(); }

		inline GroupIDType GetGroupCount() const {
			return static_cast<GroupIDType>(_Groups.size());
		}

		AffinityType GetMaxAffinity();
		AffinityType GetExcludedAffinity();

		inline void InvalidateExclusionAffinity(bool Invalidate = true) { _InvalidateExclusion = Invalidate; }
		inline void InvalidateMaxAffinity(bool Invalidate = true) { _InvalidateMaxAffinity = Invalidate; }

		inline void InvalidateAffinity(bool Invalidate = true) {
			_InvalidateMaxAffinity = Invalidate;
			_InvalidateExclusion = Invalidate;
		}


		void ResetProcessorCounters(Algorithms::TSAlgorithm Algorithm);

		size_t ExcludeHTCores() const;

		size_t GetUsableProcessorCount() const;
		size_t GetProcessorCount() const;

		GroupPtr ReallocateGroup(GroupIDType Preference, uint8_t MaxIndirection);
		GroupPtr GetNextGroup(AffinityType AllowedGroups);

		bool DetectSystemLayout();
		bool DetectSystemLayout_2(bool Silent = true);

		void AccumulateStatistics(ThreadObjectPtr tThread);

		bool AllocateThread(ThreadObjectPtr tThread, Config::ProcessConfigPtr ProcessConfig);
		bool ReAllocateThread(ThreadObjectPtr tThread);

		void DumpProcessorThreads();


		void ResetAccumulatedDemand();
		void ResetAccumulatedThreads();

		size_t GetTotalThreadCount() {
			
		}

		void LoadConfiguration(Config::ProcessConfigPtr ProcessConfig);

		std::wstring AffinityToString(GroupIDType GroupID, AffinityType Affinity);
		std::wstring AffinityToString(GroupIDType GroupID, AffinityType Affinity, ProcessorIDType IdealProcessor);

		void ShowSummary();

		ProcessorMap();
		~ProcessorMap();


	};
};

#endif
