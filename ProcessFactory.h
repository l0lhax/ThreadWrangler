#ifndef __HEADER_PROCESSFACTORY
#define __HEADER_PROCESSFACTORY

#include <Windows.h>
#include "RemoteProcess.h"
#include <string>
#include <mutex>
#include <thread>
#include <atomic>
#include "ProcessorMap.h"


class ProcessFactory {
public:

	typedef RemoteProcess * RemoteProcessPtr;
	typedef RemoteProcess & RemoteProcessRef;

	struct stMonitoredProcess {
		size_t ResponsibleIndex;
		std::wstring ExecutableName;
		Processors::ProcessorMapPtr ProcessorInfo;
		RemoteProcessPtr Process;
		bool Selected;
		bool Configuring;
	};

	typedef stMonitoredProcess * MonitoredProcessPtr;

	typedef RemoteProcess * RemoteProcessPtr;
	std::vector<size_t> _FreeKeys;

	typedef std::map<size_t, MonitoredProcessPtr> MProcessCont;
	typedef MProcessCont::iterator MProcessIter;
	typedef MProcessCont::iterator MProcessCIter;
	typedef std::pair<size_t, MonitoredProcessPtr> MProcessPair;


	MProcessCont _Processes;


private:
	typedef NTSTATUS(WINAPI* tNtQuerySystemInformation)(int, PVOID, ULONG, PULONG);
	tNtQuerySystemInformation fNtQuerySystemInformation;

	ULONG BufferLength;
	ULONG BufferDataSize;
	BYTE* Buffer;

	void FreeQueryBuffer();


	bool QuerySystem();
	size_t GetNextFreeKey();

	bool FindProcess(const std::wstring & ExecutableName, RemoteProcessPtr & RemoteProcess);
	bool UpdateThreads(MonitoredProcessPtr MonitoredProcess);
	bool AllocateThreads(MonitoredProcessPtr MonitoredProcess);

	bool AllocateThread_ByAvgUserTime_Sticky(MonitoredProcessPtr MonitoredProcess, Config::ProcessConfigPtr ProcessConfig);
	bool AllocateThread_ByAvgUserTime_Blind(MonitoredProcessPtr MonitoredProcess, Config::ProcessConfigPtr ProcessConfig);

	void DeleteObjects(MonitoredProcessPtr MonitoredProcess);
	bool InitNewProcess(MonitoredProcessPtr tMonitoredProcess);

public:
	void Tick();


	bool AddMonitoredProcess(const std::wstring & ProcessExecutable, size_t & ID);
	bool ReInitMonitoredProcess(size_t ID);
	bool FindMonitoredProcess(size_t ID, MonitoredProcessPtr & RemoteProcess);
	bool DeleteMonitoredProcess(size_t ID);
	void DeleteMonitoredProcesses();


	ProcessFactory();
	~ProcessFactory();

};

#endif
