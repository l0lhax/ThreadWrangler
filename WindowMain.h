#ifndef __HEADER_WINDOWMAIN
#define __HEADER_WINDOWMAIN

#include <Windows.h>
#include <stdint.h>
#include "DPI.h"
#include "ProcessFactory.h"


struct ListviewItem {
	DWORD _ThreadID;
};

struct ListViewCacheItem {

	const wchar_t * ThreadState;
	wchar_t ThreadID[8];
	wchar_t BasePriority[6];
	//const wchar_t * BasePriority;

	wchar_t UserTime[6];
	wchar_t KernelTime[6];
	wchar_t AverageUserTime[6];

	wchar_t Bitmap[65];

	wchar_t Node[8];

	Processors::GroupIDType GroupID;

	uint64_t dAverageUserTime;
};


class WindowMain {
private:



	static constexpr uint32_t CID_ProcessTimer = 1000;

	static constexpr uint32_t CID_Listview = 1000;
	static constexpr uint32_t CID_Listbox = 1001;
	
	static constexpr uint32_t CID_AddProcess = 1010;
	static constexpr uint32_t CID_RemoveProcess = 1011;
	static constexpr uint32_t CID_ConfigureProcess = 1012;

	static constexpr uint32_t CID_ShowSleeping = 1020;
	static constexpr uint32_t CID_ShowDemand = 1021;
	static constexpr uint32_t CID_ShowDorment = 1022;

	static constexpr uint32_t CID_SortCombobox = 1040;

	static constexpr uint32_t CID_LabelProcess = 1030;


	constexpr static wchar_t CLASS_NAME[] = L"CLASS_WindowMain";
	constexpr static wchar_t TITLE_NAME[] = L"Thread Wrangler v0.1";

	static constexpr uint32_t MINSIZE_X = 1000;
	static constexpr uint32_t MINSIZE_Y = 600;
	 
	static constexpr uint32_t WM_NOTIFYICON = WM_USER + 100;
	static constexpr uint32_t ID_TRAY_SHOW = 9000;
	static constexpr uint32_t ID_TRAY_CLOSE = 9001;



	ProcessFactory _ProcessFactory;

	bool PendingDestroy;

	HINSTANCE _HInstance;

	HICON ICO_SmallIcon;
	HICON ICO_LargeIcon;

	HWND _HWND;
	HMENU _TrayMenu;

	HWND HWND_Listview;
	HWND HWND_Listbox;
	HWND HWND_CM_Modes;
	HWND HWND_CM_Sort;
	HWND HWND_AddProcess;
	HWND HWND_RemoveProcess;
	HWND HWND_ConfigureProcess;

	HWND HWND_CKShowSleeping;
	HWND HWND_CKShowDemand;
	HWND HWND_CKShowDorment;

	HWND HWND_LblProcess;

	HFONT _DefaultFont;
	HFONT _HeaderFont;

	bool _RedrawRequired;
	bool bCurrentID;
	size_t _CurrentID;

	Algorithms::ViewSort _VSAlgorithm;

	bool _ThreadViewVisible;
	bool _ShowActive;
	bool _ShowSleeping;
	bool _ShowDorment;

	std::wstring _Status;

	NOTIFYICONDATA _NotifyIconData;

	std::vector<ListViewCacheItem> _ListviewItems;

	ScaleHelper DPI;

	void ResizeSingleControl(HWND Control, RECT & rc);

	int32_t GetRectWidth(RECT & rc) {
		return rc.right - rc.left;
	}

	int32_t GetRectHeight(RECT & rc) {
		return rc.bottom - rc.top;
	}


	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void CreateFonts(HWND Parent);
	void ReleaseFonts();

	void Tick();
	void UpdateMonitoredProcess();


	LRESULT CreateControls(UINT Reason);
	void Handle_DPIChange(RECT * rc);

	LRESULT Handle_ListviewCustomDraw(LPARAM lParam);
	LRESULT WindowMain::Handle_ListviewOwnerData(LPARAM lParam);

	void SetSortAlgorithm(Algorithms::ViewSort SortMode);

	void InitNotifyIcon();

	bool Handle_AddProcess();
	bool Handle_SelectProcess();
	bool Handle_DeleteProcess();

	void Handle_ConfigureProcess();

	void Handle_ToggleCKSleeping();
	void Handle_ToggleCKDorment();
	void Handle_ToggleCKActive();

	void Handle_Minimize();
	void Handle_Restore();
	void Handle_Closing();
	void Handle_Terminate();
	void Handle_NotifyIcon(LPARAM lParam);

	void Handle_SortAlgorithmChanged();

	LRESULT Handle_GetMinManInfo(LPARAM lParam);
	LRESULT Handle_Paint();

	void EnableThreadView();
	void DisableThreadView();

	void UpdateSelectedID(size_t ID, bool Delete);

	void ReloadConfiguration();
	void LoadConfiguration();
	void UpdateConfiguration();

	void ClearProcessList();

	bool ShowSelectProcess(std::wstring & Filename, std::wstring & FullPath);


	void ToRect(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, RECT & rc);


	bool RegClass();
	bool UnRegClass();

	Algorithms::ViewSort GetSortingAlgorithm();

public:

	WPARAM MessagePump();

	void Create();

	void Show();
	void Hide();

	WindowMain(HINSTANCE hInstace);
	~WindowMain();

};

#endif