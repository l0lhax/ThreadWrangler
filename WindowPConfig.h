#ifndef __HEADER_CONFIGWINDOW
#define __HEADER_CONFIGWINDOW

#include <stdint.h>
#include <Windows.h>
#include "DPI.h"
#include <vector>
#include "Configuration.h"

struct stProcessConfig;

class WindowPConfig {
public:

	struct stCPU {
		Processors::ProcessorIDType CPUID;
		HWND lbl_CPUID;
		HWND CK_Exclude;
		bool ExcludePState;
		HWND CK_ITC;
		bool ITCPState;
		HWND CK_DTC;
		bool DTCPState;
		bool HTCore;

		stCPU() {
			CPUID = 0;
			lbl_CPUID = 0;
			CK_Exclude = 0;
			CK_ITC = 0;
			CK_DTC = 0;
			HTCore = false;
			ExcludePState = false;
			ITCPState = false;
			DTCPState = false;
		}
	};

	struct stGroup {
		Processors::GroupIDType GroupID;
		HWND CK_ExcludeGroup;
		HWND CK_ExcludeHTCores;
		HWND GB_Processors;
		HWND lbl_CPUCol1;
		HWND lbl_CPUCol2;
		HWND lbl_CPUCol3;
		HWND lbl_CPUCol4;
		std::vector<stCPU> _CPUs;

		stGroup() {
			GroupID = 0;
			CK_ExcludeGroup = 0;
			CK_ExcludeHTCores = 0;
			GB_Processors = 0;
			lbl_CPUCol1 = 0;
			lbl_CPUCol2 = 0;
			lbl_CPUCol3 = 0;
			lbl_CPUCol4 = 0;
		}
	};



private:

	constexpr static wchar_t CLASS_NAME[] = L"CLASS_ConfigWindow";
	constexpr static wchar_t TITLE_NAME[] = L"Thread Wrangler - Process Configuration";

	HWND _HWND;
	HWND _Parent;
	HINSTANCE _HInstance;

	HWND CK_IdealThreadMode;
	HWND CK_AffinityMode;
	HWND BTN_OK;
	HWND BTN_Cancel;
	HWND BTN_Revert;

	ScaleHelper DPI;
	
	HFONT _DefaultFont;
	HFONT _HeaderFont;

	HWND CB_Algorithm;

	Config::ProcessConfigPtr _TargetConfig;


	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void SetSortAlgorithm(Algorithms::TSAlgorithm ThreadSortAlg);

	LRESULT CreateControls(UINT Reason);
	void CreateFonts(HWND Parent);
	void ReleaseFonts();

	LRESULT Handle_GetMinManInfo(LPARAM lParam);
	LRESULT Handle_Paint();
	void Handle_Terminate();

	void ApplyConfiguration();
	void RestoreConfiguration();

	bool RegClass();
	bool UnRegClass();
	
	bool PendingDestroy;

	std::vector<stGroup> _Groups;

public:

	void SetTargetConfig(Config::ProcessConfigPtr ProcessConfig);

	void Handle_Click(HWND Control);

	WPARAM MessagePump();

	void Create();

	void Show();
	void Hide();

	WindowPConfig(HINSTANCE hInstace, HWND Parent);
	~WindowPConfig();
};




#endif