#include "ProcessorMap.h"
#include <set>
#include <Windows.h>
#include "RemoteThread.h"

using namespace Processors;

std::wstring ProcessorMap::AffinityToString(GroupIDType GroupID, AffinityType Affinity) {

	std::wstring Result;

	GroupsIter it = _Groups.find(GroupID);
	if (it != _Groups.end()) {
		GroupPtr tGroup = it->second;
		Result = tGroup->AffinityToString(Affinity);
	}
	else {
		Result = L"Invalid";
	}

	return Result;
}

std::wstring ProcessorMap::AffinityToString(GroupIDType GroupID, AffinityType Affinity, ProcessorIDType IdealProcessor) {

	std::wstring Result;

	GroupsIter it = _Groups.find(GroupID);
	if (it != _Groups.end()) {
		GroupPtr tGroup = it->second;
		Result = tGroup->AffinityToString(Affinity, IdealProcessor);
	}
	else {
		Result = L"Invalid";
	}

	return Result;
}

GroupPtr ProcessorMap::AddGroup(GroupIDType GroupID) {

	GroupPtr tGroup = InvalidGroupPtr;

	GroupsIter it = _Groups.find(GroupID);
	if (it == _Groups.end()) {
		tGroup = new Group(GroupID);
		GroupContResult Result = _Groups.emplace(GroupID, tGroup);
	}
	else {
		tGroup = it->second;
	}

	return tGroup;

}

GroupPtr ProcessorMap::GetGroup(GroupIDType GroupID) {

	GroupPtr tGroup = InvalidGroupPtr;

	GroupsIter it = _Groups.find(GroupID);
	if (it != _Groups.end()) {
		tGroup = it->second;
	}

	return tGroup;

}

bool ProcessorMap::RemoveGroup(GroupIDType GroupID) {

	bool Result = false;

	GroupPtr tGroup = InvalidGroupPtr;

	GroupsIter it = _Groups.find(GroupID);
	if (it != _Groups.end()) {
		tGroup = it->second;
	}

	if (tGroup != nullptr) {
		delete tGroup;
		Result = true;
	}

	return Result;
}

void ProcessorMap::RemoveGroups() {

	for (GroupsIter it = _Groups.begin(); it != _Groups.end(); ++it) {
		GroupPtr tGroup = it->second;

		if (tGroup != nullptr) {
			delete tGroup;
		}
	}

	_Groups.clear();

}

AffinityType ProcessorMap::GetMaxAffinity() {

	if (_InvalidateMaxAffinity) {

		AffinityType tAffinity = 0;

		if (_Groups.size() != 0) {
			for (GroupsIter it = _Groups.begin(); it != _Groups.end(); ++it) {
				GroupPtr tGroup = it->second;
				tAffinity |= tGroup->GetMask();
			}
		}

		_MaxAffinity = tAffinity;

		InvalidateMaxAffinity(false);
	}

	return _MaxAffinity;
}

size_t ProcessorMap::GetUsableProcessorCount() const {

	size_t UsableProcessorCount = 0;

	if (_Groups.size() != 0) {
		for (GroupsCIter it = _Groups.begin(); it != _Groups.end(); ++it) {
			GroupPtr tGroup = it->second;

			UsableProcessorCount += tGroup->GetUsableProcessorCount();
		}
	}

	return UsableProcessorCount;

}

size_t ProcessorMap::GetProcessorCount() const {

	size_t ProcessorCount = 0;

	if (_Groups.size() != 0) {
		for (GroupsCIter it = _Groups.begin(); it != _Groups.end(); ++it) {
			GroupPtr tGroup = it->second;

			ProcessorCount += tGroup->GetProcessorCount();
		}
	}

	return ProcessorCount;

}

AffinityType ProcessorMap::GetExcludedAffinity() {

//	if (_InvalidateExclusion) {

		AffinityType tAffinity = 0;

		if (_Groups.size() != 0) {
			for (GroupsIter it = _Groups.begin(); it != _Groups.end(); ++it) {
				GroupPtr tGroup = it->second;

				if (tGroup->IsExcluded()) {
					tAffinity |= tGroup->GetMask();
				}
			}
		}

		_ExclusionAffinity = tAffinity;

	//	InvalidateExclusionAffinity(false);

	//}

	return _ExclusionAffinity;
}

void ProcessorMap::DumpProcessorThreads() {

	for (GroupsIter git = _Groups.begin(); git != _Groups.end(); git++) {
		GroupPtr tGroup = git->second;

		wprintf(L"Group: %u - Threads: %I64u\r\n", tGroup->GetID(), tGroup->GetGroupCounter(Counters::COUNTER_Threads));

		for (ProcessorsIter pit = tGroup->begin(); pit != tGroup->end(); ++pit) {
			ProcessorPtr Processor = pit->second;
			wprintf(L"Group: %u - Processor: %u - Threads: %I64u\r\n",tGroup->GetID(), Processor->GetID(), Processor->Counter_Get(Counters::COUNTER_Threads));
		}

	}


}

GroupPtr ProcessorMap::ReallocateGroup(GroupIDType Preference, uint8_t MaxIndirection) {

	AffinityType ExclusionFlag = GetExcludedAffinity();

	GroupPtr retGroup = InvalidGroupPtr;
	Processors::GroupIDType GroupCount = GetGroupCount();

	if (GroupCount > 1) {
		float MinThreads = MAXSIZE_T;
		GroupPtr _Requested = InvalidGroupPtr;

		for (GroupsIter it = _Groups.begin(); it != _Groups.end(); it++) {
			GroupPtr tGroup = it->second;

			if (tGroup->MaskPresent(ExclusionFlag)) {
				continue;
			}

			if (tGroup->GetID() == Preference) {
				_Requested = tGroup;
			}
			else {
				float ThreadCount = tGroup->GetGroupCounter(Counters::COUNTER_Threads) * tGroup->GetScale();

				if (ThreadCount < MinThreads)
					MinThreads = ThreadCount;
			}

		}

		if (_Requested != InvalidGroupPtr) {
			if ((_Requested->GetGroupCounter(Counters::COUNTER_Threads) * _Requested->GetScale()) <= (MinThreads + MaxIndirection)) {
				retGroup = _Requested;
			}
		}
	}
	else if (GroupCount == 1) {
		return _Groups[0];
	}

	return retGroup;
}

GroupPtr ProcessorMap::GetNextGroup(AffinityType AllowedGroups) {

	if (AllowedGroups == 0) {
		AllowedGroups = GetMaxAffinity();
	}

	AffinityType ExclusionFlag = GetExcludedAffinity();

	if ((ExclusionFlag & AllowedGroups) == AllowedGroups) {
		return InvalidGroupPtr;
	}

	GroupPtr tGroup = InvalidGroupPtr;

	Processors::GroupIDType GroupCount = GetGroupCount();

	if (GroupCount > 1) {
		std::set<GroupPair, Group::Sorter> GroupSet(_Groups.begin(), _Groups.end(), Group::Sorter(Algorithms::TSI_ByThreads_Least));

		if (GroupSet.size() > 0) {
			std::set<GroupPair, Group::Sorter>::iterator it;
			for (it = GroupSet.begin(); it != GroupSet.end(); ++it) {
				GroupPtr CurrentGroup = it->second;

				if ((ExclusionFlag != 0) && (CurrentGroup->MaskPresent(ExclusionFlag))) {
					continue;
				}

				if (CurrentGroup->MaskPresent(AllowedGroups)) {
					tGroup = CurrentGroup;
					break;
				}

			}
		}

		GroupSet.clear();
	}
	else if (GroupCount == 1) {
		tGroup = _Groups[0];
	}

	return tGroup;
}


void ProcessorMap::ResetProcessorCounters(Algorithms::TSAlgorithm Algorithm) {

	GroupPtr tGroup = InvalidGroupPtr;

	for (GroupsIter it = _Groups.begin(); it != _Groups.end(); ++it) {
		tGroup = it->second;
		tGroup->ResetProcessorCounters(Algorithm);
	}

}

size_t ProcessorMap::ExcludeHTCores() const {

	size_t Count = 0;

	if (_Groups.size() != 0) {
		for (GroupsCIter it = _Groups.cbegin(); it != _Groups.cend(); ++it) {
			GroupPtr tGroup = it->second;
			Count += tGroup->ExcludeHTCores();
		}
	}
	
	return Count;
}

bool ProcessorMap::DetectSystemLayout_2(bool Silent) {

	char* buffer = nullptr;
	DWORD len = 0;
	if (GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &len) == 0) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			buffer = (char*)malloc(len);
			if (GetLogicalProcessorInformationEx(RelationAll, (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer, &len)) {

				PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)buffer;
				for (size_t i = 0; i < len; i += ptr->Size)
				{
					ptr = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)(buffer + i);

					switch (ptr->Relationship)
					{
					case RelationProcessorCore: {

						for (int ii = 0; ii < ptr->Processor.GroupCount; ii++) {

							GROUP_AFFINITY & GroupAffinity = ptr->Processor.GroupMask[ii];

							GroupIDType GroupID = GroupAffinity.Group;
							AffinityType Mask = GroupAffinity.Mask;

							GroupPtr Group = AddGroup(GroupID);
							if (Group != InvalidGroupPtr) {

								ProcessorIDType Counts = 0;

								for (ProcessorIDType ProcessorID = 0; ProcessorID < ProcessorIDMax; ProcessorID++) {
									AffinityType flag = (1i64 << ProcessorID);
									if ((Mask & flag) == flag) {
										Counts++;

										ProcessorPtr tProcessor = Group->AddProcessor(ProcessorID);

										bool HTCore = false;

										if (ptr->Processor.Flags == LTP_PC_SMT && Counts == 2 && (ProcessorID % 2)) {
											HTCore = true;
										}

										tProcessor->SetFlag_HTCore(HTCore, false);

										if (!Silent)
										wprintf(L"Processor %u added to Group %u %s\n", ProcessorID, GroupID, HTCore ? L"(HT/SMT)" : L"");

									}
								}

								//							wprintf(L"Processor %u has %u logical cores.\n", ProcessorID, Counts);

							}
						}

						break; }


					default:
						//printf(TEXT("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n"));
						break;
					}
				}
			}

			if (buffer != nullptr) {
				free(buffer);
			}
		}
	}

	return true;

}

bool ProcessorMap::DetectSystemLayout() {

	wprintf(L"Attempting to detect system layout.\n");


	SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		uint32_t NumProcessors = static_cast<uint32_t>(sysinfo.dwNumberOfProcessors);

		ULONG HighestNodeNumber;
		if (!GetNumaHighestNodeNumber(&HighestNodeNumber))
		{
			wprintf(L"NUMA detection error - GetNumaHighestNodeNumber returned %d\n", GetLastError());
			return false;
		}
		else {

			_NUMADetected = false;

			if (HighestNodeNumber > 0)
			{
				wprintf(L"NUMA layout: Detected\n");
				_NUMADetected = true;
			}
			else {
				wprintf(L"NUMA layout: Not Detected\n");

				wprintf(L"\n");

				GroupPtr Group = AddGroup(0);
				if (Group != InvalidGroupPtr) {
					for (ProcessorIDType i = 0; i < NumProcessors; i++) {
						Group->AddProcessor(i);
						wprintf(L"Processor %u added to socket %u\n", i, 0);
					}
				}

				return true;
			}

		}

		//continue to detect NUMA.

		for (uint8_t Nodes = 0; Nodes < HighestNodeNumber; ++Nodes) {
			GROUP_AFFINITY ProcessorMask;
			memset(&ProcessorMask, 0, sizeof(GROUP_AFFINITY));
			GetNumaNodeProcessorMaskEx(Nodes, &ProcessorMask);

			if (ProcessorMask.Mask != 0) {

				GroupPtr Group = AddGroup(Nodes);
				if (Group != InvalidGroupPtr) {
					wprintf(L"\nDiscovered NUMA node %u\n", Nodes);

					for (ProcessorIDType Processor = 0; Processor < ProcessorIDMax; ++Processor)
					{

						if ((ProcessorMask.Mask & (1i64 << Processor)) != 0) {
							if (Group->AddProcessor(Processor) != InvalidProcessorPtr) {
								wprintf(L"Processor %u added to NUMA node %u\n", Processor, Nodes);
							}
							else {
								wprintf(L"Failed to add Processor %u added to NUMA node %u\n", Processor, Nodes);
							}
						}

					}
				}
				else {
					wprintf(L"Failure adding NUMA node to list - exiting.\n");
				}

			}

		}

		wprintf(L"\n");

		return true;
}

void ProcessorMap::LoadConfiguration(Config::ProcessConfigPtr ProcessConfig) {

	for (GroupsIter git = _Groups.begin(); git != _Groups.end(); git++) {
		GroupPtr tGroup = git->second;

		Config::GroupConfigPtr GroupConfig = ProcessConfig->GetGroupConfig(tGroup->GetID());
		if (GroupConfig != Config::InvalidGroupConfigPtr) {
			if (GroupConfig->ExcludeGroup) {
				tGroup->ExcludeAllProcessors();
			}

			if (GroupConfig->ExcludeHTCores) {
				tGroup->ExcludeHTCores();
			}

			for (ProcessorsIter pit = tGroup->begin(); pit != tGroup->end(); ++pit) {
				ProcessorPtr Processor = pit->second;
				Processor->SetFlag_DeadThreadCandidate(Processor->MaskPresent(GroupConfig->DTC));
				Processor->SetFlag_IdealThreadCandidate(Processor->MaskPresent(GroupConfig->ITC));
				Processor->SetFlag_Excluded(Processor->MaskPresent(GroupConfig->Excluded));
			}

		}


	}

}

void ProcessorMap::ResetAccumulatedDemand() {

	if (_Groups.size() != 0) {
		for (GroupsIter it = _Groups.begin(); it != _Groups.end(); ++it) {
			GroupPtr tGroup = it->second;
			tGroup->ResetAccumulatedDemand();
		}
	}
}

void ProcessorMap::ResetAccumulatedThreads() {

	if (_Groups.size() != 0) {
		for (GroupsIter it = _Groups.begin(); it != _Groups.end(); ++it) {
			GroupPtr tGroup = it->second;
			tGroup->ResetAcculmulatedThreads();
		}
	}
}

void ProcessorMap::AccumulateStatistics(ThreadObjectPtr tThread) {

	if (_Groups.size() != 0) {
		GroupsIter it = _Groups.find(tThread->GetGroupID());
		if (it != _Groups.end()) {
			GroupPtr tGroup = it->second;
			tGroup->AccumulateStatistics(tThread);
		}
	}

}

bool ProcessorMap::AllocateThread(ThreadObjectPtr tThread, Config::ProcessConfigPtr ProcessConfig) {

	bool bResult = false;

	Processors::GroupPtr tGroup = Processors::InvalidGroupPtr;

	if (ProcessConfig->AffinityMode) {

		if (tGroup == Processors::InvalidGroupPtr) {
			tGroup = GetNextGroup(0);
		}

		uint8_t ThreadsSatisfied = 0;

		Processors::AffinityType AllowedProcessors = 0;

		if (tThread->GetThreadState() == RemoteThread::ts_Dorment) {
			AllowedProcessors = tGroup->GetGroupMask_DeadThreadCandidate();
		}


		Processors::AffinityType Affinity = tGroup->GetNextCores(Algorithms::TSI_ByThreads_Least, ProcessConfig->ProcessorsPerThread, ThreadsSatisfied, AllowedProcessors);
		tThread->SetThreadGroupAffinity(tGroup->GetID(), Affinity);
		bResult = true;

	}

	if (ProcessConfig->IdealThreadMode) {

		if (tGroup == Processors::InvalidGroupPtr) {
			tGroup = GetNextGroup(0);
		}

		Processors::AffinityType AllowedProcessors = 0;

		if (tThread->GetThreadState() == RemoteThread::ts_Dorment) {
			AllowedProcessors = tGroup->GetGroupMask_DeadThreadCandidate();
		}
		else {
			AllowedProcessors = tGroup->GetGroupMask_IdealThreadCandidate();
		}

		Processors::ProcessorPtr tIdealProcessor = tGroup->GetNextCore(Algorithms::TSI_ByIdeal_Least, AllowedProcessors);
		tThread->SetIdealProcessor(tGroup->GetID(), tIdealProcessor->GetID());
		bResult = true;

	}

	if (bResult) {
		tThread->Allocate();
	}

	return bResult;
}

bool ProcessorMap::ReAllocateThread(ThreadObjectPtr tThread) {

	bool bResult = false;

	ThreadSetStruct Info = tThread->GetThreadSetStruct();

	if (Info.AnySet) {

		Info.GroupID_NewPtr = ReallocateGroup(Info.GroupID_Requested, 0);

		if (Info.GroupID_NewPtr != Processors::InvalidGroupPtr) {

			bool Failure = false;
			
			if (Info.Affinity_Set & (Failure == false)) {
				Info.Affinity_New = Info.GroupID_NewPtr->ReallocateCores(Algorithms::TSI_ByThreads_Least, Info.Affinity_Requested, 0);
				if (Info.Affinity_New == 0) {
					Failure = true;
					wprintf(L"Failed to allocate affinity\r\n");

				}
			}

			//if (Info.IdealCPU_Set & (Failure == false)) {
				//Info.IdealCPU_NewPtr = Info.GroupID_NewPtr->ReallocateIdealCore(Algorithms::TSI_ByIdeal_Least, Info.IdealCPU_RequestedID, 0);
				//if (Info.IdealCPU_NewPtr == nullptr) {
				//	Failure = true;
			//		wprintf(L"Failed to allocate Ideal Core\r\n");
			//		Info.IdealCPU_NewPtr = Info.GroupID_NewPtr->GetNextCore(Algorithms::TSI_ByIdeal_Least);
			//	}
		//	}
			Info.IdealCPU_NewPtr = (Processors::ProcessorPtr)1;

			if (Info.Success()) {
				bResult = true;
				tThread->Allocate();

				if (Info.Affinity_Set) {
					if ((Info.GroupID_Current != Info.GroupID_Requested) || (Info.Affinity_Current != Info.Affinity_New)) {
						tThread->SetThreadGroupAffinity(Info.GroupID_Requested, Info.Affinity_New);
					}
				}

				if (Info.IdealCPU_Set) {
					if ((Info.GroupID_Current != Info.GroupID_Requested) || Info.IdealCPU_Current != Info.IdealCPU_RequestedID) {
						tThread->SetIdealProcessor(Info.GroupID_Requested, Info.IdealCPU_RequestedID);
					}
				}

			}

		}

	}

	return bResult;


}

ProcessorMap::ProcessorMap() {
	_MaxAffinity = 0;
	_NUMADetected = false;
	_ExclusionAffinity = 0;
	InvalidateAffinity();
}

ProcessorMap::~ProcessorMap() {
	RemoveGroups();
}

ProcessorMap::Instance ProcessorMap::getInstance()
{
	static ProcessorMap instance;

	return &instance;
}

void ProcessorMap::ShowSummary() {

	GroupsIter it;
	for (it = _Groups.begin(); it != _Groups.end(); ++it) {
		GroupPtr tGroup = it->second;
		wprintf(L"\r\nThread Group=%u - Threads=%u - Affinity -> Max:0x%llX   Exclusion:0x%llX   Dormant Threads:0x%llX   Ideal Threads:0x%llX\n", tGroup->GetID(), tGroup->GetProcessorCount(), tGroup->GetGroupMask_Default(), tGroup->GetGroupMask_Excluded(), tGroup->GetGroupMask_DeadThreadCandidate(), tGroup->GetGroupMask_IdealThreadCandidate());
		wprintf(L"\r\n");
		std::wstring Summary = tGroup->GetSummary();
		wprintf(Summary.c_str());
	}

	wprintf(L"Domains Detected: %u\n", GetGroupCount());
}