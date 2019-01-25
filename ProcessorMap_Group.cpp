#include "ProcessorMap_Group.h"
#include "ProcessorMap_Flag.h"
#include <algorithm>
#include <set>
#include "RemoteThread.h"

#include <windows.h>

#define Hit "*"
#define Miss "-"
#define Sep ""

using namespace Processors;

std::wstring Group::AffinityToString(const AffinityType Affinity) {

	std::wstring Result;

	for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
		ProcessorPtr tProcessor = it->second;
		Result.append(tProcessor->MaskPresent(Affinity) ? L"" Hit Sep : L"" Miss Sep);
	}

	return Result;
}

std::wstring Group::AffinityToString(const AffinityType Affinity, const ProcessorIDType IdealProcessor) {
	std::wstring Result;

	for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
		ProcessorPtr tProcessor = it->second;
		Result.append(tProcessor->MaskPresent(Affinity) ? (tProcessor->GetID() == IdealProcessor) ? L"X" Sep : L"" Hit Sep  : L"" Miss Sep);
	}

	return Result;
}

CounterType Group::GetUserDemand() const {
	return Counter_Get(Counters::COUNTER_UserDemand);
}

CounterType Group::GetKernelDemand() const {
	return Counter_Get(Counters::COUNTER_KernelDemand);
}

void Group::ResetAccumulatedDemand() {
	Counter_Reset(Counters::COUNTER_UserDemand);
	Counter_Reset(Counters::COUNTER_KernelDemand);
}

void Group::AccumulateStatistics(ThreadObjectPtr tThread) {

	switch (tThread->GetThreadState()) {
	case RemoteThread::ts_Dorment:
		Counter_Increment(Counters::COUNTER_DormentThreads);
		break;
	case RemoteThread::ts_Sleeping:
		Counter_Increment(Counters::COUNTER_SleepingThreads);
		break;
	case RemoteThread::ts_Active:
		Counter_Increment(Counters::COUNTER_ActiveThreads);
		break;
	}

	Counter_Increment(Counters::COUNTER_UserDemand, tThread->GetUserTimeInterval());
	Counter_Increment(Counters::COUNTER_KernelDemand, tThread->GetKernelTimeInterval());
}


CounterType Group::GetAccumulateActiveThreads() const {
	return Counter_Get(Counters::COUNTER_ActiveThreads);
};

CounterType Group::GetAccumulateSleepingThreads() const {
	return Counter_Get(Counters::COUNTER_SleepingThreads);
};

CounterType Group::GetAccumulateDormentThreads() const {
	return Counter_Get(Counters::COUNTER_DormentThreads);
};

CounterType Group::GetAccumulateTotalThreads() const {
	return Counter_Get(Counters::COUNTER_ActiveThreads) +
		Counter_Get(Counters::COUNTER_SleepingThreads) +
		Counter_Get(Counters::COUNTER_DormentThreads);
};

void Group::ResetAcculmulatedThreads() {
	Counter_Reset(Counters::COUNTER_ActiveThreads);
	Counter_Reset(Counters::COUNTER_SleepingThreads);
	Counter_Reset(Counters::COUNTER_DormentThreads);
}

ProcessorIDType Group::GetProcessorCount() const {
	return static_cast<ProcessorIDType>(_Processors.size());
}

ProcessorIDType Group::GetUsableProcessorCount() const {

	AffinityType Default = GetGroupMask_Default();
	AffinityType MaxAffinity = Default;

	Default &= ~GetGroupMask_Excluded();

	Processors::ProcessorIDType Count = 0;

	for (Processors::ProcessorIDType i = Processors::ProcessorIDMin; i < Processors::ProcessorIDMax; ++i) {
		AffinityType Mask = (1i64 << i);

		if (Mask > MaxAffinity)
			break;

		if ((Default & Mask) == Mask)
			Count++;
	}

	return Count;

}

AffinityType Group::GetMask() const {
	return _Mask;
}

GroupIDType Group::GetID() const {
	return _GroupID;
}

CounterType Group::GetRequestedIdealCount() const {
	return _CounterManager.GetCounter(Counters::COUNTER_Ideal);
}

void Group::ResetRequestedIdealCount() {
	ResetGroupCounter(Counters::COUNTER_Ideal);
}

CounterType Group::GetRequestedThreadCount() const {
	return _CounterManager.GetCounter(Counters::COUNTER_Threads);
}

void Group::ResetRequestedThreadCount() {
	ResetGroupCounter(Counters::COUNTER_Threads);
}

bool Group::GetGroupFlag_Exists(const FlagIDType FlagID) const {
	return _FlagManager.FlagIsValid(FlagID) != Flags::VS_DoesNotExist;
}

bool Group::GetGroupFlag_Valid(const FlagIDType FlagID) const {
	return _FlagManager.FlagIsValid(FlagID) == Flags::VS_Valid;
}

bool Group::GetGroupCounter_Exists(const FlagIDType FlagID) const {
	return _CounterManager.FlagIsValid(FlagID) != Counters::VS_DoesNotExist;
}

bool Group::GetGroupCounter_Valid(const FlagIDType FlagID) const {
	return _CounterManager.FlagIsValid(FlagID) == Counters::VS_Valid;
}

void Group::InvalidateFlags() {
	_FlagManager.InvalidateAll();
}

void Group::InvalidateFlagMask_HTCore() {
	_FlagManager.Invalidate(Flags::FLAG_HTCore);
}

void Group::InvalidateFlagMask_Excluded() {
	_FlagManager.Invalidate(Flags::FLAG_Excluded);
}

void Group::InvalidateFlagMask_DeadThreadCandidate() {
	_FlagManager.Invalidate(Flags::FLAG_DeadThreadCandidate);
}

void Group::InvalidateFlagMask_Default() {
	_FlagManager.Invalidate(Flags::FLAG_Default);
}

void Group::InvalidateFlagMask_Physical() {
	_FlagManager.Invalidate(Flags::FLAG_PhysicalCore);
}

void Group::InvalidateFlagMask_IdealThreadCandidates() {
	_FlagManager.Invalidate(Flags::FLAG_IdealThreadCandidate);
}

size_t Group::GetGroupFlagCount(const FlagIDType FlagID) const {

	Flags::FlagValidState ValidityState = _FlagManager.FlagIsValid(FlagID);

	size_t Result = 0;


	if (GetProcessorCount() != 0) {
		for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
			ProcessorPtr tProcessor = it->second;

			if (tProcessor->Flag_Get(FlagID)) {
				Result++;
			}
		}
	}

	return Result;
}

AffinityType Group::GetGroupFlagMask(const FlagIDType FlagID) const {

	Flags::FlagValidState ValidityState = _FlagManager.FlagIsValid(FlagID);

	AffinityType Result = ZeroAffinity;


	if (ValidityState == Flags::VS_Valid) {
		Result = _FlagManager.GetAffinity(FlagID);
	}
	else if (ValidityState == Flags::VS_Invalid) {

		if (GetProcessorCount() != 0) {
			for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
				ProcessorPtr tProcessor = it->second;

				if (tProcessor->Flag_Get(FlagID)) {
					Result |= tProcessor->GetMask();
				}
			}
		}

		_FlagManager.SetAffinity(FlagID, Result);

	}

	return Result;
}

AffinityType Group::GetGroupCounter(const FlagIDType FlagID) const {

	Counters::FlagValidState ValidityState = _CounterManager.FlagIsValid(FlagID);

	CounterType Result = ZeroCounter;


	if (ValidityState == Counters::VS_Valid) {
		Result = _CounterManager.GetCounter(FlagID);
	}
	else if (ValidityState == Counters::VS_Invalid) {

		if (GetProcessorCount() != 0) {
			for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
				ProcessorPtr tProcessor = it->second;
				Result += tProcessor->Counter_Get(FlagID);
			}
		}

		_CounterManager.SetCounter(FlagID, Result);

	}

	return Result;
}

void Group::ResetGroupCounter(const FlagIDType FlagID) const {

	if (GetProcessorCount() != 0) {
		for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
			ProcessorPtr tProcessor = it->second;
			tProcessor->Counter_Reset(FlagID, false);
		}
	}

	_CounterManager.Invalidate(FlagID);

}

bool Group::MaskPresent(const AffinityType Affinity) const {
	return (Affinity & _Mask) == _Mask;
}

AffinityType Group::GetGroupMask_HTCore() const {
	return GetGroupFlagMask(Flags::FLAG_HTCore);
}

AffinityType Group::GetGroupMask_Excluded() const {
	return GetGroupFlagMask(Flags::FLAG_Excluded);
}

AffinityType Group::GetGroupMask_Default() const {
	return GetGroupFlagMask(Flags::FLAG_Default);
}

AffinityType Group::GetGroupMask_DeadThreadCandidate() const {
	return GetGroupFlagMask(Flags::FLAG_DeadThreadCandidate);
}

AffinityType Group::GetGroupMask_PhysicalCore() const {
	return GetGroupFlagMask(Flags::FLAG_PhysicalCore);
}

AffinityType Group::GetGroupMask_IdealThreadCandidate() const {
	return GetGroupFlagMask(Flags::FLAG_IdealThreadCandidate);
}

void Group::InvalidateGroupFlag(FlagIDType FlagID, bool Invalidate) {
	_FlagManager.Invalidate(FlagID, Invalidate);
}

void Group::InvalidateGroupCounter(FlagIDType FlagID, bool Invalidate) {
	_CounterManager.Invalidate(FlagID, Invalidate);
}

bool Group::IsExcluded() const {
	return GetGroupFlagMask(Flags::FLAG_Default) == GetGroupFlagMask(Flags::FLAG_Excluded);
}




std::wstring Group::GetSummary() const {

	std::wstring Result1;
	std::wstring Result2;
	std::wstring Result3;
	std::wstring Result4;
	std::wstring Result5;

	AffinityType LogicalCores = GetGroupFlagMask(Flags::FLAG_Default);
	AffinityType HTCores = GetGroupFlagMask(Flags::FLAG_HTCore);
	AffinityType Excluded = GetGroupFlagMask(Flags::FLAG_Excluded);
	AffinityType DeadThreadCandidates = GetGroupFlagMask(Flags::FLAG_DeadThreadCandidate);
	AffinityType IdealThreadCandidates = GetGroupFlagMask(Flags::FLAG_IdealThreadCandidate);

	if (DeadThreadCandidates == 0) {
		DeadThreadCandidates = LogicalCores;
	}

	if (IdealThreadCandidates == 0) {
		IdealThreadCandidates = LogicalCores;
	}

	for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {

		ProcessorPtr tProcessor = it->second;

		Result1.append(tProcessor->MaskPresent(LogicalCores) ? L"" Hit Sep : L"" Miss Sep);
		Result2.append(tProcessor->MaskPresent(HTCores) ? L"" Hit Sep : L"" Miss Sep);
		Result3.append(tProcessor->MaskPresent(Excluded) ? L"" Hit Sep : L"" Miss Sep);
		Result4.append(tProcessor->MaskPresent(DeadThreadCandidates) ? L"" Hit Sep : L"" Miss Sep);
		Result5.append(tProcessor->MaskPresent(IdealThreadCandidates) ? L"" Hit Sep : L"" Miss Sep);

	}

	std::wstring Output;
	Output.append(L"             Logical Cores: " Sep);
	Output.append(Result1);
	Output.append(L"\r\n      Hyperthreading Cores: " Sep);
	Output.append(Result2);
	Output.append(L"\r\n            Excluded Cores: " Sep);
	Output.append(Result3);
	Output.append(L"\r\n    Dead thread candidates: " Sep);
	Output.append(Result4);
	Output.append(L"\r\n   Ideal thread candidates: " Sep);
	Output.append(Result5);
	Output.append(L"\r\n");

	return Output;

}

ProcessorPtr Group::AddProcessor(const ProcessorIDType ProcessorID) {

	ProcessorPtr tProcessor = InvalidProcessorPtr;

	ProcessorsCIter it = _Processors.find(ProcessorID);
	if (it == _Processors.cend()) {
		tProcessor = new Processor(ProcessorID, this);
		tProcessor->SetFlagObserver(std::bind(&Group::InvalidateGroupFlag, this, std::placeholders::_1, std::placeholders::_2));
		tProcessor->SetCounterObserver(std::bind(&Group::InvalidateGroupCounter, this, std::placeholders::_1, std::placeholders::_2));
		ProcessorContResult Result = _Processors.emplace(ProcessorID, tProcessor);
	}
	else {
		tProcessor = it->second;
	}

	InvalidateFlags();

	return tProcessor;

}

ProcessorPtr Group::GetProcessor(const ProcessorIDType ProcessorID) const {

	ProcessorPtr tProcessor = InvalidProcessorPtr;

	ProcessorsCIter it = _Processors.find(ProcessorID);
	if (it != _Processors.cend()) {
		tProcessor = it->second;
	}

	return tProcessor;

}

bool Group::RemoveProcessor(const ProcessorIDType ProcessorID) {

	bool Result = false;

	ProcessorPtr tProcessor = InvalidProcessorPtr;

	ProcessorsCIter it = _Processors.find(ProcessorID);
	if (it != _Processors.cend()) {
		tProcessor = it->second;
	}

	if (tProcessor != nullptr) {
		delete tProcessor;
		InvalidateFlags();
		Result = true;
	}

	return Result;

}

void Group::RemoveProcessors() {

	if (_Processors.size() != 0) {
		for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
			ProcessorPtr tProcessor = it->second;
			tProcessor->Invalidate();
			delete tProcessor;
		}
	}

	_Processors.clear();

	InvalidateFlags();
}

size_t Group::ExcludeHTCores() {

	size_t Count = 0;

	if (_Processors.size() != 0) {
		
		for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
			ProcessorPtr tCurrentProcessor = it->second;
			tCurrentProcessor->SetFlag_Excluded(tCurrentProcessor->GetFlag_HTCore(), false);
			Count++;
		}

		InvalidateGroupFlag(Flags::FLAG_Excluded);
	}

	return Count;
}

size_t Group::ExcludeProcessors(AffinityType ExclusionMask) {

	if (ExclusionMask == GetGroupFlagMask(Flags::FLAG_Default)) {
		ExcludeAllProcessors(true);
	}
	else if (ExclusionMask == 0) {
		ExcludeAllProcessors(false);
	}
	
	size_t Count = 0;
	
	if (_Processors.size() != 0) {
		for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
			ProcessorPtr tProcessor = it->second;
			tProcessor->SetFlag_Excluded(tProcessor->MaskPresent(ExclusionMask), false);
			Count++;
		}
	}
	
	InvalidateGroupFlag(Flags::FLAG_Excluded);
	
	return Count;
}

size_t Group::ExcludeAllProcessors(bool Exclude) {

	size_t Count = 0;

	if (_Processors.size() != 0) {
		for (ProcessorsCIter it = _Processors.cbegin(); it != _Processors.cend(); ++it) {
			ProcessorPtr tProcessor = it->second;
			tProcessor->SetFlag_Excluded(Exclude, false);
			Count++;
		}
	}

	InvalidateGroupFlag(Flags::FLAG_Excluded);

	return Count;
}



void Group::ResetProcessorCounters(Algorithms::TSAlgorithm Algorithm) {

	


	switch (Algorithm) {
	case Algorithms::TSI_ByIdeal_Least:
		ResetGroupCounter(Counters::COUNTER_Ideal);
		break;
	case Algorithms::TSI_ByThreads_Least:
		ResetGroupCounter(Counters::COUNTER_Threads);
		break;
	default:
		ResetGroupCounter(Counters::COUNTER_Threads);
		ResetGroupCounter(Counters::COUNTER_Ideal);
	}

}

Group::Group(GroupIDType GroupID) {
	InvalidateFlags();
	_GroupID = GroupID;
	_Mask = (1i64 << GroupID);

	_FlagManager.InitFlag(Flags::FLAG_Default);
	_FlagManager.InitFlag(Flags::FLAG_Excluded);
	_FlagManager.InitFlag(Flags::FLAG_DeadThreadCandidate);
	_FlagManager.InitFlag(Flags::FLAG_HTCore);

	_CounterManager.Init_Counter(Counters::COUNTER_Default);
	_CounterManager.Init_Counter(Counters::COUNTER_Threads);
	_CounterManager.Init_Counter(Counters::COUNTER_Ideal);
	_CounterManager.Init_Counter(Counters::COUNTER_UserDemand);
	_CounterManager.Init_Counter(Counters::COUNTER_KernelDemand);

	Counter_Init(Counters::COUNTER_Default);
	Counter_Init(Counters::COUNTER_Threads);
	Counter_Init(Counters::COUNTER_Ideal);
	Counter_Init(Counters::COUNTER_UserDemand);
	Counter_Init(Counters::COUNTER_KernelDemand);
	Counter_Init(Counters::COUNTER_ActiveThreads);
	Counter_Init(Counters::COUNTER_SleepingThreads);
	Counter_Init(Counters::COUNTER_DormentThreads);
}

Group::~Group() {
	RemoveProcessors();
}

ProcessorPtr Group::ReallocateIdealCore(const Algorithms::TSAlgorithm Algorithm, Processors::ProcessorIDType Preference, uint8_t MaxIndirection) {

	UNREFERENCED_PARAMETER(Algorithm);


	AffinityType ExclusionFlag = GetGroupFlagMask(Flags::FLAG_Excluded);

	if (IsExcluded()) {
		return InvalidProcessorPtr;
	}

	ProcessorPtr retProcessor = InvalidProcessorPtr;

	size_t MinThreads = MAXSIZE_T;
	ProcessorPtr _Requested = InvalidProcessorPtr;

	for (ProcessorsIter it = _Processors.begin(); it != _Processors.end(); it++) {
		ProcessorPtr tProcessor = it->second;

		if (tProcessor->MaskPresent(ExclusionFlag)) {
			continue;
		}

		if (tProcessor->GetID() == Preference) {
			_Requested = tProcessor;
		}
		else {
			size_t ThreadCount = tProcessor->GetIdealCount();

			if (ThreadCount < MinThreads)
				MinThreads = ThreadCount;
		}

	}

	if (_Requested != InvalidProcessorPtr) {
		if (_Requested->GetIdealCount() <= (MinThreads + MaxIndirection)) {
			retProcessor = _Requested;
			retProcessor->AllocatedAsIdeal();
		}
	}

	return retProcessor;
}

ProcessorPtr Group::GetNextCore(const Algorithms::TSAlgorithm Algorithm, AffinityType AllowedProcessors) {

	AffinityType ExclusionFlag = GetGroupFlagMask(Flags::FLAG_Excluded);

	if (AllowedProcessors == 0) {
		AllowedProcessors = GetGroupFlagMask(Flags::FLAG_Default);
	}

	if (IsExcluded()) {
		return InvalidProcessorPtr;
	}

	ProcessorPtr retProcessor = InvalidProcessorPtr;

	std::set<ProcessorPair, Processors::Processor::Sorter> ProcessorSet(_Processors.begin(), _Processors.end(), Processors::Processor::Sorter(Algorithm));

	if (ProcessorSet.size() > 0) {
		std::set<ProcessorPair, Processor::Sorter>::const_iterator it;
		for (it = ProcessorSet.cbegin(); it != ProcessorSet.cend(); ++it) {
			ProcessorPtr CurrentProcessor = it->second;

			if ((ExclusionFlag != 0) && (CurrentProcessor->MaskPresent(ExclusionFlag))) {
				continue;
			}

			if (CurrentProcessor->MaskPresent(AllowedProcessors)) {

				switch (Algorithm) {
				case Algorithms::TSI_ByIdeal_Least:
					CurrentProcessor->AllocatedAsIdeal();
					break;
				case Algorithms::TSI_ByThreads_Least:
					CurrentProcessor->AllocatedThread();
					break;
				}

				retProcessor = CurrentProcessor;
				break;

			}

		}
	}

	ProcessorSet.clear();

	return retProcessor;

}

AffinityType Group::ReallocateCores(const Algorithms::TSAlgorithm Algorithm, AffinityType Preference, uint8_t MaxIndirection) {

	AffinityType ExclusionFlag = GetGroupFlagMask(Flags::FLAG_Excluded);

	if (IsExcluded()) {
		return ZeroAffinity;
	}

	size_t MinThreads = MAXSIZE_T;
	size_t ThreadsRequested = 0;

	std::vector<ProcessorPtr> _Requested;

	for (ProcessorsIter it = _Processors.begin(); it != _Processors.end(); it++) {
		ProcessorPtr tProcessor = it->second;

		if (tProcessor->MaskPresent(ExclusionFlag)) {
			continue;
		}

		if (tProcessor->MaskPresent(Preference)) {
			_Requested.push_back(tProcessor);
			ThreadsRequested++;
		}
		else {
			size_t ThreadCount = tProcessor->GetThreadCount();

			if (ThreadCount < MinThreads)
				MinThreads = ThreadCount;
		}

	}

	bool Acceptable = true;

	for (std::vector<ProcessorPtr>::iterator it = _Requested.begin(); it != _Requested.end(); it++) {
		ProcessorPtr tProcessor = *it;

		if (tProcessor->GetThreadCount() > (MinThreads + MaxIndirection)) {
			Acceptable = false;
			break;
		}
		

	}

	if (Acceptable) {
		while (_Requested.size()) {
			ProcessorPtr tProcessor = _Requested.back();
			tProcessor->AllocatedThread();
			_Requested.pop_back();
		}

		return Preference;
	}
	
	return ZeroAffinity;

}

AffinityType Group::GetNextCores(const Algorithms::TSAlgorithm Algorithm, const uint8_t ThreadsRequested, uint8_t & ThreadsSatisfied, AffinityType AllowedProcessors) {

	AffinityType ExclusionFlag = GetGroupFlagMask(Flags::FLAG_Excluded);

	if (IsExcluded()) {
		ThreadsSatisfied = 0;
		return ZeroAffinity;
	}

	if (AllowedProcessors == 0) {
		AllowedProcessors = GetGroupFlagMask(Flags::FLAG_Default);
	}

	uint8_t tThreadsRequested = ThreadsRequested;

	if (ThreadsRequested == 0) {
		tThreadsRequested = static_cast<ProcessorIDType>(GetProcessorCount());
	}

	std::set<ProcessorPair, Processors::Processor::Sorter> ProcessorSet(_Processors.begin(), _Processors.end(), Processors::Processor::Sorter(Algorithm));

	AffinityType AffinityResult = ZeroAffinity;

	if (ProcessorSet.size() > 0) {
		std::set<ProcessorPair, Processor::Sorter>::const_iterator it;
		for (it = ProcessorSet.cbegin(); it != ProcessorSet.cend(); ++it) {
			ProcessorPtr CurrentProcessor = it->second;

			if (CurrentProcessor->MaskPresent(ExclusionFlag)) {
				continue;
			}

			if (CurrentProcessor->MaskPresent(AllowedProcessors)) {

				switch (Algorithm) {
				case Algorithms::TSI_ByIdeal_Least:
					CurrentProcessor->AllocatedAsIdeal();
					CurrentProcessor->MaskAppend(AffinityResult);
					break;
				case Algorithms::TSI_ByThreads_Least:
					CurrentProcessor->AllocatedThread();
					CurrentProcessor->MaskAppend(AffinityResult);
					break;
				}

				ThreadsSatisfied++;
				if (ThreadsSatisfied == tThreadsRequested) {
					break;
				}
			}
			

		}
	}

	ProcessorSet.clear();

	return AffinityResult;

}

float Group::GetScale() {

	float Scale = 1.0f;

	if (GetUsableProcessorCount()) {
		Scale = (GetProcessorCount() / GetUsableProcessorCount());
		Scale =  (Scale / GetUsableProcessorCount());  
	}

	return Scale;
}

bool Group::Sorter::operator()(GroupPair elem1, GroupPair elem2)
{

	GroupPtr Group1 = elem1.second;
	GroupPtr Group2 = elem2.second;

	GroupIDType Group1ID = Group1->GetID();
	GroupIDType Group2ID = Group2->GetID();

	if (_Algorithm == Algorithms::TSI_ByIdeal_Least) {
		CounterType Ideal1 = Group1->GetGroupCounter(Counters::COUNTER_Ideal);
		CounterType Ideal2 = Group2->GetGroupCounter(Counters::COUNTER_Ideal);

		bool Excluded1 = Group1->IsExcluded();
		bool Excluded2 = Group2->IsExcluded();


		return (Excluded1 < Excluded2) || (Excluded1 == Excluded2) && (Ideal1 < Ideal2) ||
			((Excluded1 == Excluded2) && (Ideal1 == Ideal2) && (Group1ID < Group2ID));
	}
	else if (_Algorithm == Algorithms::TSI_ByThreads_Least) {
		CounterType RThreads1 = Group1->GetGroupCounter(Counters::COUNTER_Threads);
		CounterType RThreads2 = Group2->GetGroupCounter(Counters::COUNTER_Threads);

		bool Excluded1 = Group1->IsExcluded();
		bool Excluded2 = Group2->IsExcluded();

		float Threads1 = RThreads1 * Group1->GetScale();
		float Threads2 = RThreads2 * Group2->GetScale();

		return (Excluded1 < Excluded2) || (Excluded1 == Excluded2) && (Threads1 < Threads2) ||
			((Excluded1 == Excluded2) && (Threads1 == Threads2) && (Group1ID < Group2ID));
	}


	//default.
	return Group1ID < Group2ID;

};