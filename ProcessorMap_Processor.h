#ifndef __HEADER_PROCESSORMAPPROCESSOR
#define __HEADER_PROCESSORMAPPROCESSOR

#include <vector>
#include "ProcessorMap_Flag.h"
#include "ProcessorMap_FlagCounter.h"

namespace Processors {

	class Processor : public tFlag, public FlagCounter {
	private:
		ProcessorIDType _ProcessorID;
		GroupPtr _Group;
		AffinityType _Mask;

	public:
		void SetID(ProcessorIDType ID);
		ProcessorIDType GetID() const;

		void SetFlag_Excluded(bool Excluded = true, bool Invalidate = true);
		bool GetFlag_Excluded();

		void SetFlag_HTCore(bool HTCore = true, bool Invalidate = true);
		bool GetFlag_HTCore();

		void SetFlag_PhysicalCore(bool HTCore = true, bool Invalidate = true);
		bool GetFlag_PhysicalCore();

		void SetFlag_DeadThreadCandidate(bool DeadThreadCandidate = true, bool Invalidate = true);
		bool GetFlag_DeadThreadCandidate();

		void SetFlag_IdealThreadCandidate(bool IdealThreadCandidate = true, bool Invalidate = true);
		bool GetFlag_IdealThreadCandidate();

		void Invalidate();

		void MaskAppend(AffinityType & Affinity) const;
		bool MaskPresent(AffinityType Affinity) const;

		AffinityType GetMask() const;
		GroupPtr GetGroup() const;
		size_t GetIdealCount() const;
		size_t GetThreadCount() const;

		void AllocatedAsIdeal();
		void AllocatedThread();
		void ResetIdealCounter();
		void ResetThreadCounter();

	//	void AccumulateDemand(uint64_t UserDemand, uint64_t KernelDemand);
	//	uint64_t GetUserDemand();
	//	uint64_t GetKernelDemand();

		void ResetDemand();

		Processor(ProcessorIDType ProcessorID, GroupPtr Group);

		struct Sorter {
		public:
			bool operator()(ProcessorPair elem1, ProcessorPair elem2);

		private:
			Algorithms::TSAlgorithm _Algorithm;

		public:
			Sorter(Algorithms::TSAlgorithm Algorithm) : _Algorithm(Algorithm) {}
		};
	};
};


#endif
