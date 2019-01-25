#include "RemoteThread.h"
#include <string>
#include <ctime>
#include <inttypes.h>

time_t RemoteThread::GetTimeFromFileTime(const FILETIME & filetime) {

	if (filetime.dwLowDateTime == 0 && filetime.dwHighDateTime == 0) {
		return 0;
	}

	ULARGE_INTEGER ull;

	ull.LowPart = filetime.dwLowDateTime;
	ull.HighPart = filetime.dwHighDateTime;

	return static_cast<time_t>(ull.QuadPart / 10000000ULL - 11644473600ULL);

}

uint64_t RemoteThread::GetDurationFromFileTimeMs(const FILETIME & filetime) {

	if (filetime.dwLowDateTime == 0 && filetime.dwHighDateTime == 0) {
		return 0;
	}

	ULARGE_INTEGER ull;
	ull.LowPart = filetime.dwLowDateTime;
	ull.HighPart = filetime.dwHighDateTime;
	return static_cast<uint64_t>(ull.QuadPart / 10000ULL);
}

bool RemoteThread::OpenThread() {

	if (_ThreadOpen == false) {
		_ThreadHandle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _ThreadID);
		_ThreadOpen = (_ThreadHandle != nullptr);
	}
	
	_Valid = _ThreadOpen;

	return _ThreadOpen;
}

bool RemoteThread::CloseThread() {

	bool Result = false;

	if (_ThreadHandle != nullptr) {
		Result = static_cast<bool>(::CloseHandle(_ThreadHandle));
		_ThreadHandle = nullptr;
	}

	_ThreadOpen = false;

	return Result;
}

bool RemoteThread::PollThread() {

	bool bResult = false;

	if (_ThreadOpen == false) {
		OpenThread();
	}

	if (_Valid) {

		DWORD Result = 0;

		GROUP_AFFINITY tGroupAffinity = { 0 };
		Result = GetThreadGroupAffinity(_ThreadHandle, &tGroupAffinity);

		if (Result != 0) {
			_GroupID = tGroupAffinity.Group;
			_Affinity = tGroupAffinity.Mask;
		}
		else {
			wprintf(L"Failed to get thread group affinity - GetThreadedGroupAffinity(), Errorcode = %" PRIu32 "\r\n", ::GetLastError());
		}

		//_ThreadPriority = ::GetThreadPriority(_ThreadHandle);

		PROCESSOR_NUMBER tProcessor = { 0 };
		Result = GetThreadIdealProcessorEx(_ThreadHandle, &tProcessor);

		if (Result != 0) {
			_IdealGroupID = tProcessor.Group;
			_IdealProcessorID = tProcessor.Number;
		}
		else {
			wprintf(L"Failed to get thread ideal processor - GetThreadIdealProcessorEx(), Errorcode = %" PRIu32 "\r\n", ::GetLastError());
		}

		bResult = true;
	}

	return bResult;

}


bool RemoteThread::SetThreadAffinity(Processors::AffinityType AffinityMask) {

	bool ThreadWasOpen = _ThreadOpen;

	bool bResult = false;

	if (_ThreadOpen == false) {
		OpenThread();
	}

	if (_Valid) {

		DWORD_PTR Result = SetThreadAffinityMask(_ThreadHandle, AffinityMask);

		if (Result == 0) {
			wprintf(L"Failed to set thread affinity (ID: 0x%04x -> AFX:%I64u), Errorcode = %" PRIu32 "\r\n", _ThreadID, AffinityMask, ::GetLastError());
		}
		else {
			_Affinity = AffinityMask;
			Result = true;
		}

	
	}

	_AffinitySet = bResult;
	_RequestedAffinity = AffinityMask;

	if ((ThreadWasOpen == false) && (_ThreadOpen == true)) {
		CloseThread();
	}

	return bResult;
}

bool RemoteThread::SetThreadGroupAffinity(Processors::GroupIDType Group, Processors::AffinityType AffinityMask) {

	bool ThreadWasOpen = _ThreadOpen;
	
	bool bResult = false;

	if (_ThreadOpen == false) {
		OpenThread();
	}

	if (_Valid) {

		GROUP_AFFINITY tGroupAffinity = { 0 };
		tGroupAffinity.Mask = AffinityMask;
		tGroupAffinity.Group = Group;

		GROUP_AFFINITY tPGroupAffinity = { 0 };

		DWORD Result = ::SetThreadGroupAffinity(_ThreadHandle, &tGroupAffinity, &tPGroupAffinity);

		if (Result == 0) {
			wprintf(L"Failed to set thread group affinity (ID: 0x%04x -> G:%u AFX:%I64u), Errorcode = %" PRIu32 "\r\n", _ThreadID, Group, AffinityMask, ::GetLastError());
		}
		else {
			_GroupID = Group;
			_Affinity = AffinityMask;
			bResult = true;
		}

	}

	_RequestedGroupID = Group;
	_RequestedAffinity = AffinityMask;
	_AffinitySet = bResult;

	if ((ThreadWasOpen == false) && (_ThreadOpen == true)) {
		CloseThread();
	}

	return bResult;

}

bool RemoteThread::SetIdealProcessor(Processors::GroupIDType Group, Processors::ProcessorIDType Processor) {

	bool ThreadWasOpen = _ThreadOpen;

	bool bResult = false;

	if (_ThreadOpen == false) {
		OpenThread();
	}

	if (_Valid) {

		PROCESSOR_NUMBER Previous = { 0 };

		PROCESSOR_NUMBER Requested = { 0 };
		Requested.Group = Group;
		Requested.Number = static_cast<BYTE>(Processor);

		DWORD Result = SetThreadIdealProcessorEx(_ThreadHandle, &Requested, &Previous);

		if (Result == 0) {
			wprintf(L"Failed to set thread ideal processor (0x%04x -> G:%u P:%u), Errorcode = %" PRIu32 "\r\n", _ThreadID, Group, Processor, ::GetLastError());
		}
		else {
			_IdealGroupID = Group;
			_IdealProcessorID = Processor;
			bResult = true;
		}

	}

	_RequestedIdealProcessorID = Processor;
	_RequestedGroupID = Group;
	_IdealProcessorSet = bResult;

	if ((ThreadWasOpen == false) && (_ThreadOpen == true)) {
		CloseThread();
	}

	return bResult;

}

RemoteThread::RemoteThread(DWORD ThreadID) {

	_CreationTime = 0;
	_ExitTime = 0;
	_PrevKernelTime = 0;
	_PrevUserTime = 0;
	_KernelTime = 0;
	_UserTime = 0;
	_KernelTimeInterval = 0;
	_UserTimeInterval = 0;
	_AvgUserTime = 0;
	_AvgKernelTime = 0;

	_RequestedIdealProcessorID = 0;
	_IdealProcessorSet = false;
	_RequestedAffinity = 0;
	_RequestedGroupID = 0;
	_AffinitySet = false;

	_IdealProcessorID = 0;
	_IdealGroupID = 0;
	_Affinity = 0;
	_GroupID = 0;

	_KernelDemand = false;
	_UserDemand = false;
	_ShowThread = false;

	_Valid = false;
	_ThreadHandle = 0;
	_ThreadPriority = 0;

	_ThreadID = ThreadID;
	_ThreadOpen = false;
	_Allocated = false;

}

RemoteThread::~RemoteThread() {

	CloseThread();

}
