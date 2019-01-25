#include <ctime>
#include <algorithm>
#include <inttypes.h>
#include "NTUndocumented.h"
#include "RemoteProcess.h"



RemoteProcess::RemoteProcess(DWORD ProcessID, const std::wstring & ProcessExecutable) {
	pHandle = NULL;
	_ProcessID = ProcessID;
	_ExecutableName.assign(ProcessExecutable);

	_TotalThreads = 0;
	_ActiveThreads = 0;
	_DormentThreads = 0;
	_SleepingThreads = 0;

}
const std::wstring & RemoteProcess::GetProcessExecutable() {
	return _ExecutableName;
}

size_t RemoteProcess::CopyThreads(vThreadContainer & tVector, Algorithms::ViewSort SortAlgorithm) {

	tVector.clear();
	tVector.reserve(_threads.size());
	for (ThreadIter it = _threads.begin(); it != _threads.end(); ++it) {
		tVector.push_back(it->second);
	}

	std::sort(tVector.begin(), tVector.end(), Sorter(SortAlgorithm));

	return tVector.size();
}


bool RemoteProcess::OpenProcess(DWORD DesiredAccess) {

	HANDLE Result = ::OpenProcess(DesiredAccess, 0, _ProcessID);
	if (Result != NULL) {
		pHandle = Result;

		wprintf(L"Successfully opened process, ID=%" PRIu32 "\r\n", _ProcessID);

		return true;
	}
	else {
		wprintf(L"Failed to open process, error code = %" PRIu32 "\r\n", ::GetLastError());
	}


	return false;
}


bool RemoteProcess::CloseProcess() {

	if (pHandle != NULL) {

		::CloseHandle(this->pHandle);
		pHandle = NULL;

		InvalidateThreads();
		PurgeInvalidThreads();

		return true;

	}

	return false;
}

bool RemoteProcess::IsOpen() {
	return (pHandle != NULL);
}

bool RemoteProcess::IsValid() {

	if (pHandle == NULL) {
		return false;
	}

	DWORD ExitCode = 0;

	if (GetExitCodeProcess(pHandle, &ExitCode)) {
		if (ExitCode == STILL_ACTIVE) {
			return true;
		}
	}

	return false;
}


	//std::sort(HaveLoad.begin(), HaveLoad.end(), ProcessTools::SortByThreadID());
	//st/d::sort(NoLoad.begin(), NoLoad.end(), ProcessTools::SortByThreadID());


void RemoteProcess::InvalidateThreads() {

	_DormentThreads = 0;
	_SleepingThreads = 0;
	_ActiveThreads = 0;
	_TotalThreads = 0;

	for (ThreadIter it = _threads.begin(); it != _threads.end(); it++) {
		ThreadObjectPtr ThreadInfo = it->second;
		ThreadInfo->Invalidate();
	}

}

void RemoteProcess::PurgeInvalidThreads() {

	for (ThreadIter it = _threads.begin(); it != _threads.end();) {
		ThreadObjectPtr ThreadInfo = it->second;
		if (!ThreadInfo->IsValid()) {
			wprintf(L"Thread ID 0x%04x is being removed.\r\n", ThreadInfo->GetThreadID());
			delete it->second;
			it = _threads.erase(it);
		}
		else {
			++it;
		}

	}

}

bool RemoteProcess::AddThread(ThreadID ThreadToFind, ThreadObjectPtr & Result) {

	ThreadObjectPtr tThreadObjectPtr;
	if (!FindThread(ThreadToFind, tThreadObjectPtr)) {
		tThreadObjectPtr = new RemoteThread(ThreadToFind);
		_threads.emplace(ThreadContPair(ThreadToFind, tThreadObjectPtr));
		Result = tThreadObjectPtr;
	}
	else {
		Result = tThreadObjectPtr;
	}

	return (Result != nullptr);
}

bool RemoteProcess::FindThread(ThreadID ThreadToFind, ThreadObjectPtr & Result) {
	ThreadIter it = _threads.find(ThreadToFind);
	return (it != _threads.end()) ? Result = it->second : Result = nullptr;
}

RemoteProcess::~RemoteProcess() {
	InvalidateThreads();
	PurgeInvalidThreads();
	CloseProcess();
}


bool RemoteProcess::Sorter::operator()(ThreadObjectPtr elem1, ThreadObjectPtr elem2)
{

	ThreadObjectPtr Thread1 = elem1;
	ThreadObjectPtr Thread2 = elem2;

	if (_Algorithm == Algorithms::vs_ByThreadID) {
		return (Thread1->GetThreadID() < Thread2->GetThreadID());
	}
	else if (_Algorithm == Algorithms::vs_ByGroupID) {

		Processors::GroupIDType GroupID1 = Thread1->GetGroupID();
		Processors::GroupIDType GroupID2 = Thread2->GetGroupID();

		double AvgUserTime1 = Thread1->GetAverageUserTime();
		double AvgUserTime2 = Thread2->GetAverageUserTime();

		RemoteThread::ThreadState State1 = Thread1->GetThreadState();
		RemoteThread::ThreadState State2 = Thread2->GetThreadState();

		return (GroupID1 < GroupID2) || ((GroupID1 == GroupID2) && (AvgUserTime1 > AvgUserTime2)) ||
			((GroupID1 == GroupID2) && (AvgUserTime1 == AvgUserTime2) && (State1 > State2));
	}
	else if (_Algorithm == Algorithms::vs_ByUserWallTime) {

		uint64_t UserTime1 = Thread1->GetUserTimeInterval();
		uint64_t UserTime2 = Thread2->GetUserTimeInterval();

		RemoteThread::ThreadState State1 = Thread1->GetThreadState();
		RemoteThread::ThreadState State2 = Thread2->GetThreadState();

		return (UserTime1 > UserTime2) || ((UserTime1 == UserTime2) && (State1 > State2));
	}

	else if (_Algorithm == Algorithms::vs_ByKernelWallTime) {

		uint64_t KernelTime1 = Thread1->GetKernelTimeInterval();
		uint64_t KernelTime2 = Thread2->GetKernelTimeInterval();

		RemoteThread::ThreadState State1 = Thread1->GetThreadState();
		RemoteThread::ThreadState State2 = Thread2->GetThreadState();

		return (KernelTime1 > KernelTime2) || ((KernelTime1 == KernelTime2) && (State1 > State2));
	}
	else if (_Algorithm == Algorithms::vs_ByAvgUserWallTime) {

		double UserTimeAvg1 = Thread1->GetAverageUserTime();
		double UserTimeAvg2 = Thread2->GetAverageUserTime();

		uint64_t UserTime1 = Thread1->GetUserTimeInterval();
		uint64_t UserTime2 = Thread2->GetUserTimeInterval();

		RemoteThread::ThreadState State1 = Thread1->GetThreadState();
		RemoteThread::ThreadState State2 = Thread2->GetThreadState();

		return (UserTimeAvg1 > UserTimeAvg2) || ((UserTimeAvg1 == UserTimeAvg2) && (UserTime1 > UserTime2))  ||
			((UserTimeAvg1 == UserTimeAvg2) && (UserTime1 == UserTime2) && (State1 > State2));
	}
	else if (_Algorithm == Algorithms::vs_ByAvgKernelWallTime) {

		double KernelTime1 = Thread1->GetAverageKernelTime();
		double KernelTime2 = Thread2->GetAverageKernelTime();

		RemoteThread::ThreadState State1 = Thread1->GetThreadState();
		RemoteThread::ThreadState State2 = Thread2->GetThreadState();

		return (KernelTime1 > KernelTime2) || ((KernelTime1 == KernelTime2) && (State1 > State2));
	}
	else if (_Algorithm == Algorithms::vs_ByState) {

		RemoteThread::ThreadState TState1 = Thread1->GetThreadState();
		RemoteThread::ThreadState TState2 = Thread2->GetThreadState();

		return (TState1 > TState2);
	}
	else if (_Algorithm == Algorithms::vs_ByPriority) {

		int TPriority1 = Thread1->GetThreadPriority();
		int TPriority2 = Thread2->GetThreadPriority();

		return (TPriority1 > TPriority2);
	}

	//default.
	return (Thread1->GetThreadID() < Thread2->GetThreadID());

};