#include "ProcessFactory.h"
#include <algorithm>
#include <inttypes.h>
#include "NTUndocumented.h"
#include "Configuration.h"

ProcessFactory::ProcessFactory() {

	BufferLength = 0;
	BufferDataSize = 0;
	Buffer = nullptr;

	fNtQuerySystemInformation = (tNtQuerySystemInformation)GetProcAddress(GetModuleHandleW(L"NtDll.dll"), "NtQuerySystemInformation");
	if (fNtQuerySystemInformation == 0)
	{
		fNtQuerySystemInformation = nullptr;
		BufferDataSize = 0;
		wprintf(L"Error while trying to map NtQuerySystemInformation function - Errorcode: %d\n", GetLastError());
	}

}

void ProcessFactory::FreeQueryBuffer() {

	if (Buffer != nullptr) {
		LocalFree(Buffer);
		Buffer = nullptr;
		BufferLength = 0;
		BufferDataSize = 0;
	}

}

bool ProcessFactory::FindMonitoredProcess(size_t ID, MonitoredProcessPtr & RemoteProcess) {
	MProcessCIter it = _Processes.find(ID);
	return (it != _Processes.end()) ? RemoteProcess = it->second : RemoteProcess = nullptr;
}

void ProcessFactory::DeleteObjects(MonitoredProcessPtr MonitoredProcess) {

	if (MonitoredProcess->Process != nullptr) {
		MonitoredProcess->Process->CloseProcess();
		delete MonitoredProcess->Process;
		MonitoredProcess->Process = nullptr;
	}

	if (MonitoredProcess->ProcessorInfo != nullptr) {
		delete MonitoredProcess->ProcessorInfo;
		MonitoredProcess->ProcessorInfo = nullptr;
	}


}

bool ProcessFactory::InitNewProcess(MonitoredProcessPtr tMonitoredProcess) {

	RemoteProcessPtr & tRemoteProcess = tMonitoredProcess->Process;
	Processors::ProcessorMapPtr & tProcessorInfo = tMonitoredProcess->ProcessorInfo;


	if (tRemoteProcess->OpenProcess(PROCESS_ALL_ACCESS)) {
		tProcessorInfo = new Processors::ProcessorMap();
		tProcessorInfo->DetectSystemLayout_2();

		Config::ConfigurationPtr Configuration = Config::Configuration::Instance();
		Config::ProcessConfigPtr ProcessConfig = Config::InvalidProcessConfig;
		if (Configuration->FindConfig(tMonitoredProcess->ResponsibleIndex, ProcessConfig)) {
			tProcessorInfo->LoadConfiguration(ProcessConfig);
		}

	}

	return true;
}

void ProcessFactory::Tick() {

	if ((!QuerySystem()) || (_Processes.size() == 0)) {
		return;
	}

	for (MProcessIter it = _Processes.begin(); it != _Processes.end(); ++it) {
		MonitoredProcessPtr tMonitoredProcess = it->second;

		RemoteProcessPtr & tRemoteProcess = tMonitoredProcess->Process;

		if (tRemoteProcess == nullptr) {
			if (FindProcess(tMonitoredProcess->ExecutableName, tRemoteProcess)) {
				InitNewProcess(tMonitoredProcess);
			}
		}

		if (tRemoteProcess != nullptr) {
			if (tRemoteProcess->IsOpen() && tRemoteProcess->IsValid()) {
				UpdateThreads(tMonitoredProcess);
				AllocateThreads(tMonitoredProcess);
			}
			else
			{
				DeleteObjects(tMonitoredProcess);
			}
		}

	}


}

bool ProcessFactory::DeleteMonitoredProcess(size_t ID) {

	MonitoredProcessPtr MonitoredProcess = nullptr;
	if (FindMonitoredProcess(ID, MonitoredProcess)) {

		DeleteObjects(MonitoredProcess);

		_Processes.erase(ID);
		_FreeKeys.push_back(ID);

		return true;
	}

	return false;
}

bool ProcessFactory::AddMonitoredProcess(const std::wstring & ProcessExecutable, size_t & ID) {

	size_t NextID = GetNextFreeKey();

	MonitoredProcessPtr MonitoredProcess = nullptr;
	if (!FindMonitoredProcess(NextID, MonitoredProcess)) {

		MonitoredProcess = new stMonitoredProcess();
		MonitoredProcess->ExecutableName = ProcessExecutable;
		MonitoredProcess->Selected = false;
		MonitoredProcess->Configuring = false;
		MonitoredProcess->Process = nullptr;
		MonitoredProcess->ResponsibleIndex = NextID;
		MonitoredProcess->ProcessorInfo = nullptr;

		_Processes.emplace(MProcessPair(NextID, MonitoredProcess));

		ID = NextID;
	}


	return (MonitoredProcess != nullptr);

}

bool ProcessFactory::ReInitMonitoredProcess(size_t ID) {

	MonitoredProcessPtr MonitoredProcess = nullptr;
	if (FindMonitoredProcess(ID, MonitoredProcess)) {
		
		RemoteProcessPtr & tRemoteProcess = MonitoredProcess->Process;
		Processors::ProcessorMapPtr & tProcessorInfo = MonitoredProcess->ProcessorInfo;

		if (tRemoteProcess != nullptr) {

			if (tRemoteProcess->IsOpen() && tRemoteProcess->IsValid()) {
				tRemoteProcess->CloseProcess();
			}

			if (tProcessorInfo != nullptr) {
				Config::ConfigurationPtr Configuration = Config::Configuration::Instance();
				Config::ProcessConfigPtr ProcessConfig = Config::InvalidProcessConfig;
				if (Configuration->FindConfig(ID, ProcessConfig)) {
					tProcessorInfo->LoadConfiguration(ProcessConfig);
				}
			}

			if (tRemoteProcess->OpenProcess(PROCESS_ALL_ACCESS)) {
				return true;
			}


		}


	}


	return false;
}

size_t ProcessFactory::GetNextFreeKey() {

	size_t tKey = 0;

	if (_FreeKeys.size() > 0) {
		tKey = _FreeKeys.back();
		_FreeKeys.pop_back();
	}
	else {
		if (_Processes.size() > 0) {
			tKey = (_Processes.rbegin()->first + 1);
		}
	}

	return tKey;
}


ProcessFactory::~ProcessFactory() {

	FreeQueryBuffer();

	for (MProcessIter it = _Processes.begin(); it != _Processes.end(); ++it) {
		MonitoredProcessPtr tMonitoredProcess = it->second;


		DeleteObjects(tMonitoredProcess);

		tMonitoredProcess->Selected = false;
	}

	_Processes.clear();
}

void ProcessFactory::DeleteMonitoredProcesses() {

	for (MProcessIter it = _Processes.begin(); it != _Processes.end(); ++it) {
		MonitoredProcessPtr tMonitoredProcess = it->second;
		DeleteObjects(tMonitoredProcess);
		tMonitoredProcess->Selected = false;
	}

	_Processes.clear();
	_FreeKeys.clear();

}

bool ProcessFactory::AllocateThread_ByAvgUserTime_Blind(MonitoredProcessPtr MonitoredProcess, Config::ProcessConfigPtr ProcessConfig) {

	RemoteProcessPtr & tRemoteProcess = MonitoredProcess->Process;
	Processors::ProcessorMapPtr & tProcessorInfo = MonitoredProcess->ProcessorInfo;

	RemoteProcess::vThreadContainer _threads;
	size_t vThreads = tRemoteProcess->CopyThreads(_threads, Algorithms::vs_ByAvgUserWallTime);

	if (vThreads > 0) {

		for (RemoteProcess::vThreadIter it = _threads.begin(); it != _threads.end(); ++it) {
			RemoteProcess::ThreadObjectPtr tThread = *it;
			tProcessorInfo->AllocateThread(tThread, ProcessConfig);
		}

	}

	tProcessorInfo->DumpProcessorThreads();

	return true;

}

bool ProcessFactory::AllocateThread_ByAvgUserTime_Sticky(MonitoredProcessPtr MonitoredProcess, Config::ProcessConfigPtr ProcessConfig) {


	RemoteProcessPtr & tRemoteProcess = MonitoredProcess->Process;
	Processors::ProcessorMapPtr & tProcessorInfo = MonitoredProcess->ProcessorInfo;

	RemoteProcess::vThreadContainer _threads;
	size_t vThreads = tRemoteProcess->CopyThreads(_threads, Algorithms::vs_ByAvgUserWallTime);

	size_t IterLimit = tProcessorInfo->GetProcessorCount();
	size_t Iterations = 0;
	uint8_t Mode = 0;
	size_t Allocations = 0;


	if (vThreads > 0) {

		RemoteProcess::vThreadIter it = _threads.begin();
		RemoteProcess::vThreadIter last;
		RemoteProcess::vThreadIter it2;

		bool Bounded = false;
		if (IterLimit > _threads.size()) {
			last = std::prev(_threads.end());
		}
		else {
			Bounded = true;
			last = it + (IterLimit - 1);
		}

		bool bSuccess = false;

		while (it != _threads.end()) {
			Iterations++;
			switch (Mode) {
			case 0:
				if ((*it)->IsAllocated()) { ++it; break; }
				bSuccess = tProcessorInfo->ReAllocateThread(*it);
				if (bSuccess) { Allocations++; ++it; }
				else { Mode = 1; }
				break;

			case 1:
				it2 = last;

				while ((it2 != it) & (bSuccess == false)) {
					Iterations++;

					if (!(*it2)->IsAllocated()) {
						bSuccess = tProcessorInfo->ReAllocateThread(*it2);
					}
					else {
						if (it2 == last) { --last; }
					}

					--it2;
				}

				if (bSuccess) { Allocations++; Mode = 0; break; }

				Mode = 3;

				break;

			case 3:
				tProcessorInfo->AllocateThread(*it, ProcessConfig);
				Allocations++;
				++it;
				Mode = 0;
				break;
			}

			if (Bounded && it > last) {
				Bounded = false;
				last = std::prev(_threads.end());
			}
		}
	}

	//tProcessorInfo->DumpProcessorThreads();

	//wprintf(L"Complete, Allocations: %u  Iterations: %u\r\n", Allocations, Iterations);


	return true;

}

bool ProcessFactory::AllocateThreads(MonitoredProcessPtr MonitoredProcess) {

	Processors::ProcessorMapPtr & tProcessorInfo = MonitoredProcess->ProcessorInfo;

	Config::ConfigurationPtr Configuration = Config::Configuration::Instance();
	Config::ProcessConfigPtr ProcessConfig = Config::InvalidProcessConfig;
	Configuration->FindConfig(MonitoredProcess->ResponsibleIndex, ProcessConfig);

	bool AnySet = false;

	if (ProcessConfig->AffinityMode) {
		tProcessorInfo->ResetProcessorCounters(Algorithms::TSI_ByThreads_Least);
		AnySet = true;
	}
	
	if (ProcessConfig->IdealThreadMode) {
		tProcessorInfo->ResetProcessorCounters(Algorithms::TSI_ByIdeal_Least);
		AnySet = true;
	}

	if (AnySet == false) { return false; }


	switch (ProcessConfig->Algorithm) {
	case Algorithms::TSI_UserTime_Sticky:
		return AllocateThread_ByAvgUserTime_Sticky(MonitoredProcess, ProcessConfig);
	case Algorithms::TSI_UserTime_Blind:
		return AllocateThread_ByAvgUserTime_Blind(MonitoredProcess, ProcessConfig);
	}

	return true;

}



bool ProcessFactory::UpdateThreads(MonitoredProcessPtr MonitoredProcess) {

	if (Buffer == nullptr) {
		return false;
	}

	RemoteProcessPtr & tRemoteProcess = MonitoredProcess->Process;
	Processors::ProcessorMapPtr & tProcessorInfo = MonitoredProcess->ProcessorInfo;

	DWORD ProcessID = tRemoteProcess->GetProcessID();

	SYSTEM_PROCESS_INFORMATION * Info = nullptr;

	size_t i = 0;
	do {
		Info = (SYSTEM_PROCESS_INFORMATION *)&Buffer[i];

		if (Info->ProcessId != ProcessID) {
			continue;
		}

		tRemoteProcess->InvalidateThreads();
		tProcessorInfo->ResetAccumulatedDemand();
		tProcessorInfo->ResetAccumulatedThreads();

		if (Info->ThreadCount == 0) {
			break;
		}

		for (unsigned int ii = 0; ii < Info->ThreadCount; ii++)
		{

			SYSTEM_THREAD_INFORMATION & ThreadInfo = Info->ThreadInfos[ii];
			DWORD & ThreadID = ThreadInfo.Client_Id.UniqueThread;
			
			RemoteProcess::ThreadObjectPtr ThreadObject = nullptr;
			if (!tRemoteProcess->FindThread(ThreadID, ThreadObject)) {
				tRemoteProcess->AddThread(ThreadID, ThreadObject);
			}

			if (ThreadObject == nullptr) {
				//error
				continue;
			}

			ThreadObject->Validate();
			ThreadObject->FromSystemThreadInfo(ThreadInfo);
				
			if (ThreadObject->PollThread()) {
				tProcessorInfo->AccumulateStatistics(ThreadObject);
			}
		}


		break;

	} while ((Info->NextOffset > 0) && ((i += Info->NextOffset) < BufferDataSize));

	tRemoteProcess->PurgeInvalidThreads();


	return true;

}

bool ProcessFactory::FindProcess(const std::wstring & ExecutableName, RemoteProcessPtr & pRemoteProcess) {

	if (Buffer == nullptr) {
		pRemoteProcess = nullptr;
		return false;
	}

	pRemoteProcess = nullptr;
	size_t Processes = 0;

	SYSTEM_PROCESS_INFORMATION * Info = nullptr;

	size_t StrLength = ExecutableName.length() * sizeof(wchar_t);

	size_t i = 0;
	do {
		Info = (SYSTEM_PROCESS_INFORMATION *)&Buffer[i];

		Processes++;

		if ((Info->ImageName.Buffer == nullptr) || (StrLength != Info->ImageName.Length))
			continue;

		if (_wcsicmp(ExecutableName.c_str(), Info->ImageName.Buffer) == 0) {
			std::wstring tImageName(Info->ImageName.Buffer);
			pRemoteProcess = new RemoteProcess(Info->ProcessId, tImageName);
			break;
		}

	} while ((Info->NextOffset > 0) && ((i += Info->NextOffset) < BufferDataSize));

	return (pRemoteProcess != nullptr);

}


bool ProcessFactory::QuerySystem() {

	ULONG RequiredBuffer = 0;

	if (fNtQuerySystemInformation != nullptr) {

		// first call just to retrieve the needed buffer size for the information
		NTSTATUS lResult = fNtQuerySystemInformation(SYSTEMPROCESSINFORMATION, Buffer, RequiredBuffer, &RequiredBuffer);
		if (lResult == STATUS_INFO_LENGTH_MISMATCH)
		{

			if (BufferLength < RequiredBuffer) {
				FreeQueryBuffer();
				BufferLength = RequiredBuffer + (1024 * 64);
				Buffer = (BYTE*)LocalAlloc(LMEM_FIXED, BufferLength);
			}

			if (Buffer != nullptr) {
				BufferDataSize = RequiredBuffer;
				memset(Buffer, 0, BufferLength);
			}

		}
		else if (lResult != 0)
		{
			BufferDataSize = 0;
			wprintf(L"Error calling NtQuerySystemInformation to get required buffer size - Errorcode: %d\n", GetLastError());
			return false;
		}

		// buffer is generated, now retireve the information
		if (fNtQuerySystemInformation(SYSTEMPROCESSINFORMATION, Buffer, RequiredBuffer, &RequiredBuffer))
		{
			BufferDataSize = 0;
			wprintf(L"Error calling NtQuerySystemInformation to retrieve system information - Errorcode: %d\n", GetLastError());
			return false;
		}

	}

	return true;

}