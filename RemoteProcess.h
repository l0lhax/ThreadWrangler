#ifndef __HEADER_PROCESSTOOLS
#define __HEADER_PROCESSTOOLS

#include <Windows.h>
#include <tlhelp32.h>
#include <string>
#include <map>
#include "RemoteThread.h"
#include <list>
#include <vector>
#include <thread>
#include "ProcessorMap_Types.h"
#include <queue>

class RemoteProcess {
public:

	typedef DWORD ThreadID;
	typedef RemoteThread * ThreadObjectPtr;
	typedef std::map<ThreadID, ThreadObjectPtr> ThreadContainer;
	typedef ThreadContainer::iterator ThreadIter;
	typedef ThreadContainer::const_iterator ThreadCIter;
	typedef std::pair<ThreadID, ThreadObjectPtr> ThreadContPair;

	typedef std::vector<ThreadObjectPtr> vThreadContainer;
	typedef vThreadContainer::iterator vThreadIter;
	typedef vThreadContainer::const_iterator vThreadCIter;


private:
	ThreadContainer _threads;

	HANDLE pHandle;
	DWORD _ProcessID;

	std::wstring _ExecutableName;

	uint64_t _TotalThreads;
	uint64_t _ActiveThreads;
	uint64_t _DormentThreads;
	uint64_t _SleepingThreads;

	typedef std::function<void(Processors::FlagIDType, bool)> CallbackFunction;

public:
	inline ThreadIter threads_begin() noexcept { return _threads.begin(); }
	inline ThreadCIter threads_cbegin() const noexcept { return _threads.cbegin(); }
	inline ThreadIter threads_end() noexcept { return _threads.end(); }
	inline ThreadCIter threads_cend() const noexcept { return _threads.cend(); }

	bool AddThread(ThreadID ThreadToFind, ThreadObjectPtr & Result);
	bool FindThread(ThreadID ThreadToFind, ThreadObjectPtr & Result);

	size_t CopyThreads(vThreadContainer & tVector, Algorithms::ViewSort SortAlgorithm);

	const std::wstring & GetProcessExecutable();

	bool OpenProcess(DWORD DesiredAccess);
	bool CloseProcess();

	void InvalidateThreads();
	void PurgeInvalidThreads();

	DWORD GetProcessID() {
		return _ProcessID;
	}

	inline uint64_t GetTotalThreads() {
		return _threads.size();
	}

	inline uint64_t GetActiveThreads() {
		return _ActiveThreads;
	}

	inline uint64_t GetDormentThreads() {
		return _DormentThreads;
	}

	inline uint64_t GetSleepingThreads() {
		return _SleepingThreads;
	}

	bool IsOpen();
	bool IsValid();

	RemoteProcess(DWORD ProcessID, const std::wstring & ProcessExecutable);
	~RemoteProcess();

	struct Sorter {
	public:
		bool operator()(ThreadObjectPtr elem1, ThreadObjectPtr elem2);

	private:
		Algorithms::ViewSort _Algorithm;

	public:
		Sorter(Algorithms::ViewSort Algorithm) : _Algorithm(Algorithm) {}
	};


};

#endif