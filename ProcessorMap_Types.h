#ifndef __HEADER_PROCESSORMAPTYPES
#define __HEADER_PROCESSORMAPTYPES

#include <stdint.h>
#include <map>

class RemoteThread;


namespace Processors {

	class Processor;
	class Group;
	class ProcessorMap;
	
	typedef RemoteThread * ThreadObjectPtr;

	typedef ProcessorMap * ProcessorMapPtr;


	typedef Group * GroupPtr;
	typedef uint16_t GroupIDType;
	constexpr static GroupPtr InvalidGroupPtr = nullptr;

	typedef Processor * ProcessorPtr;
	constexpr static ProcessorPtr InvalidProcessorPtr = nullptr;
	typedef uint16_t ProcessorIDType;
	constexpr static ProcessorIDType ProcessorIDMin = 0;
	constexpr static ProcessorIDType ProcessorIDMax = 64;

	typedef uint64_t AffinityType;
	constexpr static AffinityType ZeroAffinity = 0;

	typedef std::map<GroupIDType, GroupPtr> GroupContainer;
	typedef GroupContainer::iterator GroupsIter;
	typedef GroupContainer::const_iterator GroupsCIter;
	typedef std::pair<GroupsIter, bool> GroupContResult;

	typedef std::map<ProcessorIDType, ProcessorPtr> ProcessorContainer;
	typedef ProcessorContainer::iterator ProcessorsIter;
	typedef ProcessorContainer::const_iterator ProcessorsCIter;
	typedef std::pair<ProcessorsIter, bool> ProcessorContResult;

	typedef std::pair<GroupIDType, GroupPtr> GroupPair;
	typedef std::pair<ProcessorIDType, ProcessorPtr> ProcessorPair;

	typedef uint64_t CounterType;
	constexpr static CounterType ZeroCounter = 0;

	typedef uint8_t FlagIDType;

	namespace Counters {
		enum Counters {
			COUNTER_Default = 0,
			COUNTER_Threads = 1,
			COUNTER_Ideal = 2,
			COUNTER_UserDemand = 3,
			COUNTER_KernelDemand = 4,
			COUNTER_ActiveThreads = 5,
			COUNTER_SleepingThreads = 6,
			COUNTER_DormentThreads = 7,
			COUNTER_TotalThreads = 8,
			COUNTER_Reset = 255
		};

		enum FlagValidState {
			VS_DoesNotExist = 0,
			VS_Valid = 1,
			VS_Invalid = 2
		};
	}

	namespace Flags {
		enum Flags {
			FLAG_Default = 0,
			FLAG_Excluded = 1,
			FLAG_DeadThreadCandidate = 2,
			FLAG_HTCore = 3,
			FLAG_PhysicalCore = 4,
			FLAG_IdealThreadCandidate = 5,
			FLAG_Reserved2 = 6,
			FLAG_Reserved3 = 7,
			FLAG_Reset = 255
		};

		enum FlagValidState {
			VS_DoesNotExist = 0,
			VS_Valid = 1,
			VS_Invalid = 2
		};
	}
};



namespace Algorithms {
	enum TSAlgorithm {
		TSI_Default = 0,
		TSI_ByThreads_Least = 2,
		TSI_ByIdeal_Least = 4,
		TSI_UserTime_Blind = 5,
		TSI_UserTime_Sticky = 6
	};


	enum ViewSort {
		vs_Default = 0,
		vs_ByThreadID = 1,
		vs_ByState = 2,
		vs_ByPriority = 3,
		vs_ByUserWallTime = 4,
		vs_ByKernelWallTime = 5,
		vs_ByAvgUserWallTime = 6,
		vs_ByAvgKernelWallTime = 7,
		vs_ByGroupID = 8,
	};

}

#endif