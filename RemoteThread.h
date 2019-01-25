#ifndef __HEADER_REMOTETHREAD
#define __HEADER_REMOTETHREAD

#include <Windows.h>
#include <stdint.h>
#include <vector>
#include "ProcessorMap_Types.h"
#include "NTUndocumented.h"


struct ThreadSetStruct {
public:
	bool Affinity_Set;
	Processors::AffinityType Affinity_Requested;
	Processors::AffinityType Affinity_Current;
	
	bool IdealCPU_Set;
	Processors::ProcessorIDType IdealCPU_RequestedID;
	Processors::ProcessorIDType IdealCPU_Current;

	Processors::GroupIDType GroupID_Requested;
	Processors::GroupIDType GroupID_Current;

	Processors::AffinityType Affinity_New;
	Processors::ProcessorPtr  IdealCPU_NewPtr;
	Processors::GroupPtr  GroupID_NewPtr;



	bool AnySet;

	bool Success() {

		if (Affinity_Set & IdealCPU_Set) {
			return ((Affinity_New != 0) & (IdealCPU_NewPtr != 0));
		}
		else if (Affinity_Set) {
			return (Affinity_New != 0);
		}
		else if (IdealCPU_Set) {
			return (IdealCPU_NewPtr != 0);
		}

		return false;
	}
};

struct ThreadStats {

};


class RemoteThread {
public:

	enum ThreadState {
		ts_Unknown = 0,
		ts_Dorment = 1,
		ts_Sleeping = 2,
		ts_Active = 3
	};

private:
	DWORD _ThreadID;

	uint64_t PollingTicks;

	time_t _CreationTime;
	time_t _ExitTime;
	uint64_t _PrevKernelTime;
	uint64_t _PrevUserTime;
	uint64_t _KernelTime;
	uint64_t _UserTime;
	uint64_t _KernelTimeInterval;
	uint64_t _UserTimeInterval;

	const static uint8_t AvgSamples = 2;
	double _AvgUserTime;
	double _AvgKernelTime;

	HANDLE _ThreadHandle;
	int _ThreadPriority;

	Processors::ProcessorIDType _IdealProcessorID;
	Processors::GroupIDType _IdealGroupID;

	Processors::GroupIDType _GroupID;
	Processors::AffinityType _Affinity;



	Processors::ProcessorIDType _RequestedIdealProcessorID;
	Processors::GroupIDType _RequestedGroupID;
	Processors::AffinityType _RequestedAffinity;

	bool _Valid;
	bool _ThreadOpen;


	bool _KernelDemand;
	bool _UserDemand;
	bool _ShowThread;

	bool _IdealProcessorSet;
	bool _AffinitySet;

	bool _Allocated;

	static time_t GetTimeFromFileTime(const FILETIME & filetime);
	static uint64_t GetDurationFromFileTimeMs(const FILETIME & filetime);

public:

	bool OpenThread();
	bool CloseThread();

	inline DWORD GetThreadID() const {
		return _ThreadID;
	}

	inline int GetThreadPriority() const {
		return _ThreadPriority;
	}

	inline Processors::GroupIDType GetGroupID() const {
		return _GroupID;
	}

	inline Processors::AffinityType GetAffinity() const {
		return _Affinity;
	}

	inline Processors::ProcessorIDType GetIdealProcessorID() const {
		return _IdealProcessorID;
	}

	inline Processors::GroupIDType GetIdealGroupID() {
		return _IdealGroupID;
	}

	inline Processors::GroupIDType GetRequestedGroupID() const {
		return _RequestedGroupID;
	}

	inline Processors::AffinityType GetRequestedAffinity() const {
		return _RequestedAffinity;
	}

	inline Processors::ProcessorIDType GetRequestedIdealProcessorID() const {
		return _RequestedIdealProcessorID;
	}

	inline bool IsValid() const {
		return _Valid;
	}

	inline bool IsAffinitySet() const {
		return _AffinitySet;
	}

	inline bool IsIdealProcessorSet() const {
		return _IdealProcessorSet;
	}

	inline bool IsUnassigned() const {
		return (!_AffinitySet && !_IdealProcessorSet);
	}

	inline void ResetAllocation() {
		_AffinitySet = false;
		_IdealProcessorSet = false;
	}
	
	inline void Invalidate() {
		_Valid = false;
		_Allocated = false;
	}

	inline void Validate() {
		_Valid = true;
	}

	inline void Allocate() {
		_Allocated = true;
	}

	inline bool IsAllocated() {
		return _Allocated;
	}

	bool PollThread();

	inline uint64_t GetUserTime() const {
		return _UserTime;
	}

	inline uint64_t GetKernelTime() const {
		return _KernelTime;
	}

	inline double RemoteThread::GetAverageUserTime() const {
		return _AvgUserTime;
	}

	inline double RemoteThread::GetAverageKernelTime() const {
		return _AvgKernelTime;
	}

	inline void RemoteThread::FromSystemThreadInfo(SYSTEM_THREAD_INFORMATION & STI) {

		_PrevUserTime = _UserTime;
		_PrevKernelTime = _KernelTime;

		_UserTime = GetDurationFromFileTimeMs(STI.UserTime);
		_KernelTime = GetDurationFromFileTimeMs(STI.KernelTime);
		_CreationTime = GetTimeFromFileTime(STI.CreateTime);
		_ThreadPriority = STI.BasePriority;

		if (_PrevUserTime != 0 && _UserTime != 0) {
			_UserTimeInterval = _UserTime - _PrevUserTime;
			_AvgUserTime -= _AvgUserTime / AvgSamples;
			_AvgUserTime += _UserTimeInterval / AvgSamples;
			//if (_AvgUserTime < 0.1f) { _AvgUserTime = 0.0f; }
		}

		if (_PrevKernelTime != 0 && _KernelTime != 0) {
			_KernelTimeInterval = _KernelTime - _PrevKernelTime;
			_AvgKernelTime -= _AvgKernelTime / AvgSamples;
			_AvgKernelTime += (_KernelTimeInterval) / AvgSamples;
			//if (_AvgKernelTime < 0.1f) { _AvgKernelTime = 0.0f; }
		}

		_UserDemand = (_UserTime != 0);
		_KernelDemand = (_KernelTime != 0);
	}


	uint64_t GetKernelTimeInterval() {
		return _KernelTimeInterval;
	}

	uint64_t GetUserTimeInterval() {
		return _UserTimeInterval;
	}

	inline bool HasDemand() {
		return (_UserDemand != 0) || (_KernelDemand != 0);
	}

	inline bool HasIntervalDemand() {
		return ((_KernelTimeInterval != 0) || (_UserTimeInterval != 0)) || ((_AvgKernelTime >1.0f) || (_AvgUserTime > 1.0f));
	}

	inline ThreadState GetThreadState()
	{

		if (HasDemand() == false) {
			return ts_Dorment;
		}
		else if (HasIntervalDemand() == false) {
			return ts_Sleeping;
		}

		return ts_Active;
	}

	inline void SetShowThread(bool ShowThread = true) {
		_ShowThread = ShowThread;
	}

	inline bool GetShowThread() const {
		return _ShowThread;
	}

	bool SetThreadAffinity(Processors::AffinityType AffinityMask);
	bool SetThreadGroupAffinity(Processors::GroupIDType Group, Processors::AffinityType AffinityMask);
	bool SetIdealProcessor(Processors::GroupIDType Group, Processors::ProcessorIDType Processor);

	ThreadSetStruct GetThreadSetStruct() {

		ThreadSetStruct SS;
		
		SS.Affinity_Set = _AffinitySet;

		if (_AffinitySet)
		{
			SS.Affinity_Requested = _RequestedAffinity;
			SS.Affinity_Current = _Affinity;
		}

		SS.IdealCPU_Set = _IdealProcessorSet;
		
		if (_IdealProcessorSet) {
			SS.IdealCPU_RequestedID = _RequestedIdealProcessorID;
			SS.IdealCPU_Current = _IdealProcessorID;
		}

		SS.AnySet = (_AffinitySet || _IdealProcessorSet);

		if (SS.AnySet) {
			SS.GroupID_Requested = _RequestedGroupID;
			SS.GroupID_Current = _GroupID;
		}

		return SS;
	}



	RemoteThread(DWORD ThreadID);
	~RemoteThread();



};




#endif