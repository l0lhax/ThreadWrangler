#ifndef __HEADER_PROCESSORMAPGROUP
#define __HEADER_PROCESSORMAPGROUP

#include "ProcessorMap_Processor.h"
#include "ProcessorMap_GroupFlag.h"
#include "ProcessorMap_GroupCounter.h"
#include <vector>
#include <set>

namespace Processors {

	class Group : public FlagCounter {
	private:
		GroupIDType _GroupID;
		ProcessorContainer _Processors;
		AffinityType _Mask;

		GroupFlagManager _FlagManager;
		GroupCounterManager _CounterManager;

	public:
		ProcessorPtr AddProcessor(const ProcessorIDType ProcessorID);
		ProcessorPtr GetProcessor(const ProcessorIDType ProcessorID) const;
		bool RemoveProcessor(const ProcessorIDType ProcessorID);
		void RemoveProcessors();

		ProcessorsIter begin() noexcept { return _Processors.begin(); }
		ProcessorsCIter cbegin() const noexcept { return _Processors.cbegin(); }
		ProcessorsIter end() noexcept { return _Processors.end(); }
		ProcessorsCIter cend() const noexcept { return _Processors.cend(); }

		//void AccumulateDemand(const CounterType UserDemand, const CounterType KernelDemand);
		void ResetAccumulatedDemand();

		CounterType GetUserDemand() const;
		CounterType GetKernelDemand() const;

		//void AccumulateThreads(const CounterType ActiveThreads, const CounterType SleepingThreads, const CounterType DormentThreads);
		//void AccumulateActiveThreads(const CounterType ActiveThreads = 1);
		//void AccumulateSleepingThreads(const CounterType SleepingThreads = 1);
		//void AccumulateDormentThreads(const CounterType DormentThreads = 1);

		void AccumulateStatistics(ThreadObjectPtr tThread);

		CounterType GetAccumulateActiveThreads() const;
		CounterType GetAccumulateSleepingThreads() const;
		CounterType GetAccumulateDormentThreads() const;
		CounterType GetAccumulateTotalThreads() const;

		void ResetAcculmulatedThreads();

		ProcessorIDType GetUsableProcessorCount() const;
		ProcessorIDType GetProcessorCount() const;
		AffinityType GetMask() const;
		GroupIDType GetID() const;

		CounterType GetRequestedIdealCount() const;
		void ResetRequestedIdealCount();

		CounterType GetRequestedThreadCount() const;
		void ResetRequestedThreadCount();

		bool GetGroupFlag_Exists(const FlagIDType FlagID) const;
		bool GetGroupFlag_Valid(const FlagIDType FlagID) const;
		bool GetGroupCounter_Exists(const FlagIDType FlagID) const;
		bool GetGroupCounter_Valid(const FlagIDType FlagID) const;

		void InvalidateFlags();
		void InvalidateFlagMask_HTCore();
		void InvalidateFlagMask_Excluded();
		void InvalidateFlagMask_DeadThreadCandidate();
		void InvalidateFlagMask_Default();
		void InvalidateFlagMask_Physical();
		void InvalidateFlagMask_IdealThreadCandidates();

		size_t GetGroupFlagCount(const FlagIDType FlagID) const;

		AffinityType GetGroupFlagMask(const FlagIDType FlagID) const;

		AffinityType GetGroupCounter(const FlagIDType FlagID) const;

		void ResetGroupCounter(const FlagIDType FlagID) const;

		bool MaskPresent(const AffinityType Affinity) const;

		AffinityType GetGroupMask_HTCore() const;
		AffinityType GetGroupMask_Excluded() const;
		AffinityType GetGroupMask_Default() const;
		AffinityType GetGroupMask_DeadThreadCandidate() const;
		AffinityType GetGroupMask_PhysicalCore() const;
		AffinityType GetGroupMask_IdealThreadCandidate() const;

		void InvalidateGroupFlag(FlagIDType FlagID, bool Invalidate = true);
		void InvalidateGroupCounter(FlagIDType FlagID, bool Invalidate = true);

		bool IsExcluded() const;

		std::wstring GetSummary() const;
		std::wstring AffinityToString(const AffinityType Affinity);
		std::wstring AffinityToString(const AffinityType Affinity, const ProcessorIDType IdealProcessor);

		void ResetProcessorCounters(Algorithms::TSAlgorithm Algorithm);

		size_t ExcludeHTCores();
		size_t ExcludeProcessors(const AffinityType ExclusionMask);
		size_t ExcludeAllProcessors(const bool Exclude = true);

		ProcessorPtr GetNextCore(const Algorithms::TSAlgorithm Algorithm, AffinityType AllowedProcessors = 0);
		ProcessorPtr ReallocateIdealCore(const Algorithms::TSAlgorithm Algorithm, Processors::ProcessorIDType Preference, uint8_t MaxIndirection);

		AffinityType GetNextCores(const Algorithms::TSAlgorithm Algorithm, const uint8_t ThreadsRequested, uint8_t & ThreadsSatisfied, AffinityType AllowedProcessors = 0);
		AffinityType ReallocateCores(const Algorithms::TSAlgorithm Algorithm, AffinityType Preference, uint8_t MaxIndirection);

		float GetScale();


		Group(GroupIDType GroupID);
		~Group();

		struct Sorter {

		public:
			bool operator()(GroupPair elem1, GroupPair elem2);

		private:
			Algorithms::TSAlgorithm _Algorithm;

		public:
			Sorter(Algorithms::TSAlgorithm Algorithm) : _Algorithm(Algorithm) {}

		};

	};

};
/**/


#endif
