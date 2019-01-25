#include "ProcessorMap_Processor.h"

using namespace Processors;

void Processor::SetID(ProcessorIDType ID) {
	_ProcessorID = ID;
	_Mask = (1i64 << ID);
}

ProcessorIDType Processor::GetID() const {
	return _ProcessorID;
}


void Processor::SetFlag_Excluded(bool Excluded, bool Invalidate) {
	Flag_Set(Flags::FLAG_Excluded, Excluded, Invalidate);
}

bool Processor::GetFlag_Excluded() {
	return Flag_Get(Flags::FLAG_Excluded);
}

void Processor::SetFlag_HTCore(bool HTCore, bool Invalidate) {
	Flag_Set(Flags::FLAG_HTCore, HTCore, Invalidate);
}

bool Processor::GetFlag_HTCore() {
	return Flag_Get(Flags::FLAG_HTCore);
}

void Processor::SetFlag_PhysicalCore(bool HTCore, bool Invalidate) {
	Flag_Set(Flags::FLAG_PhysicalCore, HTCore, Invalidate);
}

bool Processor::GetFlag_PhysicalCore() {
	return Flag_Get(Flags::FLAG_PhysicalCore);
}

void Processor::SetFlag_DeadThreadCandidate(bool DeadThreadCandidate, bool Invalidate) {
	Flag_Set(Flags::FLAG_DeadThreadCandidate, DeadThreadCandidate, Invalidate);
}

bool Processor::GetFlag_DeadThreadCandidate() {
	return Flag_Get(Flags::FLAG_DeadThreadCandidate);
}

void Processor::SetFlag_IdealThreadCandidate(bool IdealThreadCandidate, bool Invalidate) {
	Flag_Set(Flags::FLAG_IdealThreadCandidate, IdealThreadCandidate, Invalidate);
}

bool Processor::GetFlag_IdealThreadCandidate() {
	return Flag_Get(Flags::FLAG_IdealThreadCandidate);
}


void Processor::Invalidate() {
	_Group = InvalidGroupPtr;
}

void Processor::MaskAppend(AffinityType & Affinity) const {
	Affinity |= _Mask;
}

bool Processor::MaskPresent(AffinityType Affinity) const {
	return (Affinity & _Mask) == _Mask;
}

AffinityType Processor::GetMask() const {
	return _Mask;
}

GroupPtr Processor::GetGroup() const {
	return _Group;
}

size_t Processor::GetIdealCount() const {
	return Counter_Get(Counters::COUNTER_Ideal);
}

size_t Processor::GetThreadCount() const {
	return Counter_Get(Counters::COUNTER_Threads);
}

void Processor::AllocatedAsIdeal() {
	Counter_Increment(Counters::COUNTER_Ideal);
}

void Processor::AllocatedThread() {
	Counter_Increment(Counters::COUNTER_Threads);
}

void Processor::ResetIdealCounter() {
	Counter_Reset(Counters::COUNTER_Ideal);
}

void Processor::ResetThreadCounter() {
	Counter_Reset(Counters::COUNTER_Threads);
}

void Processor::ResetDemand() {
	Counter_Reset(Counters::COUNTER_KernelDemand);
	Counter_Reset(Counters::COUNTER_UserDemand);
}

Processor::Processor(ProcessorIDType ProcessorID, GroupPtr Group) {
	_ProcessorID = ProcessorID;
	_Group = Group;
	_Mask = (1i64 << _ProcessorID);

	Flag_Set(Flags::FLAG_Default, true);
	Flag_Set(Flags::FLAG_IdealThreadCandidate, true);
	Flag_Set(Flags::FLAG_DeadThreadCandidate, true);

	Counter_Init(Counters::COUNTER_Default);
	Counter_Init(Counters::COUNTER_Threads);
	Counter_Init(Counters::COUNTER_Ideal);
	Counter_Init(Counters::COUNTER_UserDemand);
    Counter_Init(Counters::COUNTER_KernelDemand);

	ResetDemand();
}


bool Processor::Sorter::operator()(ProcessorPair elem1, ProcessorPair elem2)
{

	ProcessorPtr Processor1 = elem1.second;
	ProcessorPtr Processor2 = elem2.second;

	ProcessorIDType Processor1ID = Processor1->GetID();
	ProcessorIDType Processor2ID = Processor2->GetID();

	if (_Algorithm == Algorithms::TSI_ByIdeal_Least) {

		CounterType Ideal1 = Processor1->Counter_Get(Counters::COUNTER_Ideal);
		CounterType Ideal2 = Processor2->Counter_Get(Counters::COUNTER_Ideal);
		bool IsHTCore1 = Processor1->GetFlag_HTCore();
		bool IsHTCore2 = Processor2->GetFlag_HTCore();

		return (Ideal1 < Ideal2) ||
			((Ideal1 == Ideal2) && (IsHTCore1 < IsHTCore2)) ||
			((Ideal1 == Ideal2) && (IsHTCore1 == IsHTCore2) && (Processor1ID < Processor2ID));
	}
	else if (_Algorithm == Algorithms::TSI_ByThreads_Least) {

		CounterType Threads1 = Processor1->Counter_Get(Counters::COUNTER_Threads);
		CounterType Threads2 = Processor2->Counter_Get(Counters::COUNTER_Threads);

		return (Threads1 < Threads2) ||
			((Threads1 == Threads2) && (Processor1ID < Processor2ID));
	}


	//default.
	return Processor1ID < Processor2ID;

};