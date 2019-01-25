#include "WindowMain.h"
#include <CommCtrl.h>
#include <Uxtheme.h>
#include "resource.h"
#include "ControlHelpers.h"
#include <algorithm>
#include "WindowPConfig.h"
#include "Configuration.h"
#include  <shobjidl_core.h>


extern size_t counter1 = 0;
extern size_t counter2 = 0;

void WindowMain::UpdateMonitoredProcess() {

	if (IsIconic(_HWND)) {
		return;
	}

	bool Update = false;
	bool ProcessActive = false;
	uint64_t TotalThreads = 0;

	wchar_t tBuffer[64];

	if (bCurrentID) {

		ProcessFactory::MonitoredProcessPtr Selected = nullptr;

		if (_ProcessFactory.FindMonitoredProcess(_CurrentID, Selected)) {
			ProcessFactory::RemoteProcessPtr & tRemoteProcess = Selected->Process;
			//ProcessorMapPtr & tProcessor = Selected->Processors;

			if (tRemoteProcess != nullptr) {

				ProcessActive = true;

				wsprintf(tBuffer, L"Process ID = %u     Threads = %u", tRemoteProcess->GetProcessID(),tRemoteProcess->GetTotalThreads());

				RemoteProcess::vThreadContainer _threads;

				size_t vThreads = tRemoteProcess->CopyThreads(_threads, _VSAlgorithm);

				size_t CurrentThread = 0;
				if (vThreads > 0) {

					for (RemoteProcess::vThreadIter it = _threads.begin(); it != _threads.end(); ++it) {
						RemoteProcess::ThreadObjectPtr Thread = *it;

						RemoteThread::ThreadState ThreadState = Thread->GetThreadState();



						if (!_ShowSleeping && ThreadState == RemoteThread::ts_Sleeping)
							continue;

						if (!_ShowDorment && ThreadState == RemoteThread::ts_Dorment)
							continue;

						if (!_ShowActive && ThreadState == RemoteThread::ts_Active)
							continue;

						ListViewCacheItem * tItem = nullptr;

						if (_ListviewItems.size() < (CurrentThread+1)) {
							_ListviewItems.emplace_back(ListViewCacheItem());
							tItem = &_ListviewItems.back();
						}
						else {
							tItem = &_ListviewItems[CurrentThread];
						}

						switch (ThreadState) {
						case RemoteThread::ts_Dorment:
							tItem->ThreadState = L"Dorment";
							break;
						case RemoteThread::ts_Sleeping:
							tItem->ThreadState = L"Sleeping";
							break;
						case RemoteThread::ts_Active:
							tItem->ThreadState = L"Active";
							break;
						default:
							tItem->ThreadState = L"Unknown";
							break;
						}

						std::to_string(5.0f);

						wsprintf(tItem->ThreadID, L"0x%04x", Thread->GetThreadID());
						wsprintf(tItem->BasePriority, L"%I64u", Thread->GetThreadPriority());
						wsprintf(tItem->KernelTime, L"%I64u", Thread->GetKernelTimeInterval());
						wsprintf(tItem->UserTime, L"%I64u", Thread->GetUserTimeInterval());
						wsprintf(tItem->AverageUserTime, L"%I64u", static_cast<uint64_t>(Thread->GetAverageUserTime()));


						std::wstring tAffinityString = Selected->ProcessorInfo->AffinityToString(Thread->GetGroupID(), Thread->GetAffinity(), Thread->GetIdealProcessorID());
						wsprintf(tItem->Bitmap, tAffinityString.c_str());

						tItem->dAverageUserTime = static_cast<uint64_t>(Thread->GetAverageUserTime());

						tItem->GroupID = Thread->GetGroupID();
						wsprintf(tItem->Node, L"Node %u", tItem->GroupID);
						

						CurrentThread++;
						TotalThreads++;

					}

					Update = true;
				}

			}
		}
	}

	bool Changed = false;

	LRESULT Count = SendMessage(HWND_Listview, LVM_GETITEMCOUNT, 0, 0);
	if (static_cast<uint64_t>(Count) != TotalThreads) {
		SendMessage(HWND_Listview, LVM_SETITEMCOUNT, TotalThreads, 0);
		SendMessage(HWND_Listview, LVM_SETCOLUMNWIDTH, 7, LVSCW_AUTOSIZE_USEHEADER);
		Changed = true;
	}

	if (ProcessActive) {

		if (Changed || TotalThreads != 0) {
			PostMessage(HWND_Listview, LVM_REDRAWITEMS, 0, TotalThreads);



		}

		if (!_ThreadViewVisible) {
			EnableThreadView();
		}

	} 
	else
	{

		if (_ThreadViewVisible) {
			DisableThreadView();
		}

		wsprintf(tBuffer, L"Process not found.");
	}


	if (_Status.compare(tBuffer) != 0) {
		_Status.assign(tBuffer);
		SetWindowText(HWND_LblProcess, tBuffer);
	}
}


void WindowMain::Tick() {

	_ProcessFactory.Tick();
	UpdateMonitoredProcess();

}

LRESULT WindowMain::Handle_Paint() {
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(_HWND, &ps);
	EndPaint(_HWND, &ps);
	return 0;
}

void WindowMain::Handle_Closing() {

	if (!IsWindowVisible(_HWND))
	{
		Shell_NotifyIcon(NIM_DELETE, &_NotifyIconData);
	}

	DestroyWindow(_HWND);

}

void WindowMain::Handle_Terminate() {

	Config::ConfigurationPtr Configuration = Config::Configuration::Instance();
	Configuration->SaveConfiguration();

	PendingDestroy = true;
	PostQuitMessage(0);
}

void WindowMain::Handle_NotifyIcon(LPARAM lParam) {

	uint32_t MsgID = LOWORD(lParam);

	if (MsgID == NIN_SELECT) {
		if (IsWindowVisible(_HWND))
		{
			Handle_Minimize();
		}
		else
		{
			Handle_Restore();
		}
	} else if (MsgID == WM_RBUTTONDOWN) {

		POINT curPoint;
		GetCursorPos(&curPoint);

		UINT ClickedID = TrackPopupMenu(_TrayMenu, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, _HWND, NULL);
		SendMessage(_HWND, WM_NULL, 0, 0); 

		switch (ClickedID) {
		case ID_TRAY_SHOW:
			Handle_Restore();
			break;
		case ID_TRAY_CLOSE:
			Handle_Closing();
			break;
		}

		 
	}



}


LRESULT CALLBACK WindowMain::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	WindowMain *pThis = nullptr;

	if (msg == WM_NCCREATE)
	{
		pThis = static_cast<WindowMain*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

		SetLastError(0);
		if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
		{
			if (GetLastError() != 0)
				return FALSE;
		}
	}
	else
	{
		pThis = reinterpret_cast<WindowMain*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}



	switch (msg)
	{
	case WM_CREATE:
		pThis->_HWND = hwnd;
		pThis->InitNotifyIcon();
		return pThis->CreateControls(msg);
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		pThis->Handle_Terminate();
		break;
	case WM_PAINT:
		return pThis->Handle_Paint();
	case WM_TIMER:
		pThis->Tick();
		break;
	case WM_SIZE:
		return pThis->CreateControls(msg);
	case WM_GETMINMAXINFO:
		return pThis->Handle_GetMinManInfo(lParam);
	case WM_SYSCOMMAND:

		switch (wParam) {
		case SC_MINIMIZE:
			pThis->Handle_Minimize();
			return 0;
		case SC_CLOSE:
			pThis->Handle_Closing();
			return 0;
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);

	case WM_NOTIFYICON:
		pThis->Handle_NotifyIcon(lParam);
		break;
	case WM_COMMAND: {

		HWND CallingHWND = (HWND)lParam;
		uint32_t Code = HIWORD(wParam);

		if (Code == BN_CLICKED)
		{

			switch (LOWORD(wParam)) {

			case CID_AddProcess:
				pThis->Handle_AddProcess();
				break;
			case CID_RemoveProcess:
				pThis->Handle_DeleteProcess();
				break;
			case CID_ConfigureProcess:
				pThis->Handle_ConfigureProcess();
				break;
			case CID_ShowSleeping: 
				pThis->Handle_ToggleCKSleeping();
				break;
			case CID_ShowDemand:
				pThis->Handle_ToggleCKActive();
				break;
			case CID_ShowDorment:
				pThis->Handle_ToggleCKDorment();
				break;
			}

		} else if (Code == LBN_SELCHANGE) {
			if (CallingHWND == pThis->HWND_Listbox) {
				pThis->Handle_SelectProcess();
			}
			
		}
		else if (Code == CBN_SELENDOK) {
			if (CallingHWND == pThis->HWND_CM_Sort) {
				pThis->Handle_SortAlgorithmChanged();
			}
		}
		
	}
	break;

	case WM_NOTIFY:

		switch (((LPNMHDR)lParam)->code) {
		case LVN_GETDISPINFO:
			return pThis->Handle_ListviewOwnerData(lParam);
		case NM_CUSTOMDRAW:
			return pThis->Handle_ListviewCustomDraw(lParam);
		}
		break;
	case WM_DPICHANGED: {
		RECT* rect = reinterpret_cast<RECT*>(lParam);
		pThis->Handle_DPIChange(rect);
		break; }
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}


	return 0;
}

void WindowMain::Handle_ConfigureProcess() {

	//get or add config.
	Config::ConfigurationPtr Config = Config::Configuration::Instance();
	Config::ProcessConfigPtr tProcessConfig = Config::InvalidProcessConfig;
	if (!Config->FindConfig(_CurrentID, tProcessConfig)) {
		tProcessConfig = Config->AddConfig(_CurrentID);
	}

	if (tProcessConfig == nullptr) {
		return;
	}

	EnableWindow(_HWND, 0);

	WindowPConfig tWindowPConfig(_HInstance, _HWND);
	tWindowPConfig.SetTargetConfig(tProcessConfig);
	tWindowPConfig.Create();
	tWindowPConfig.Show();
	tWindowPConfig.MessagePump();

	_ProcessFactory.ReInitMonitoredProcess(_CurrentID);

}

void WindowMain::SetSortAlgorithm(Algorithms::ViewSort SortMode) {

	int32_t SelectedIndex = 0;

	switch (SortMode) {
	case Algorithms::vs_ByThreadID:
		SelectedIndex = 0;
		break;
	case Algorithms::vs_ByState:
		SelectedIndex = 1;
		break;
	case Algorithms::vs_ByPriority:
		SelectedIndex = 2;
		break;
	case Algorithms::vs_ByUserWallTime:
		SelectedIndex = 3;
		break;
	case Algorithms::vs_ByAvgUserWallTime:
		SelectedIndex = 4;
		break;
	case Algorithms::vs_ByKernelWallTime:
		SelectedIndex = 5;
		break;
	case Algorithms::vs_ByAvgKernelWallTime:
		SelectedIndex = 6;
		break;
	case Algorithms::vs_ByGroupID:
		SelectedIndex = 7;
		break;
	}

	ControlHelpers::SetComboboxSelIndex(HWND_CM_Sort, SelectedIndex);

}

void WindowMain::Handle_SortAlgorithmChanged() {

	Algorithms::ViewSort VS = Algorithms::vs_Default;

	int32_t SelectedIndex = 0;
	if (ControlHelpers::GetComboboxSelIndex(HWND_CM_Sort, SelectedIndex)) {
		switch (SelectedIndex) {
		case 0:
			VS = Algorithms::vs_ByThreadID;
			break;
		case 1:
			VS = Algorithms::vs_ByState;
			break;
		case 2:
			VS = Algorithms::vs_ByPriority;
			break;
		case 3:
			VS = Algorithms::vs_ByUserWallTime;
			break;
		case 4:
			VS = Algorithms::vs_ByAvgUserWallTime;
			break;
		case 5:
			VS = Algorithms::vs_ByKernelWallTime;
			break;
		case 6:
			VS = Algorithms::vs_ByAvgKernelWallTime;
			break;
		case 7:
			VS = Algorithms::vs_ByGroupID;
			break;

		}
	}

	_VSAlgorithm = VS;

	UpdateConfiguration();
	UpdateMonitoredProcess();

}

LRESULT WindowMain::Handle_GetMinManInfo(LPARAM lParam) {
	MINMAXINFO* mmi = (MINMAXINFO*)lParam;
	mmi->ptMinTrackSize.x = MINSIZE_X;
	mmi->ptMinTrackSize.y = MINSIZE_Y;
	return 0;
}

void WindowMain::Handle_ToggleCKSleeping() {
	_ShowSleeping = ControlHelpers::ToggleCheckbox(HWND_CKShowSleeping);
	UpdateConfiguration();
	UpdateMonitoredProcess();
}

void WindowMain::Handle_ToggleCKDorment() {
	_ShowDorment = ControlHelpers::ToggleCheckbox(HWND_CKShowDorment);
	UpdateConfiguration();
	UpdateMonitoredProcess();
}

void WindowMain::Handle_ToggleCKActive() {
	_ShowActive = ControlHelpers::ToggleCheckbox(HWND_CKShowDemand);
	UpdateConfiguration();
	UpdateMonitoredProcess();
}

LRESULT WindowMain::Handle_ListviewCustomDraw(LPARAM lParam) {

		LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;
		switch (lplvcd->nmcd.dwDrawStage)
		{
		case CDDS_PREPAINT:
			return CDRF_NOTIFYITEMDRAW;

		case CDDS_ITEMPREPAINT:
		{
			LPNMLVCUSTOMDRAW customDraw = (LPNMLVCUSTOMDRAW)lParam;

			DWORD_PTR index = customDraw->nmcd.dwItemSpec;

			if (_ListviewItems.size() > index) {
				ListViewCacheItem & tItem = _ListviewItems[index];

				uint32_t Value = static_cast<uint32_t>(tItem.dAverageUserTime);

				if (Value > 1000) { Value = 1000; }
				uint32_t Number1 = static_cast<uint8_t>(Value * 0.135f);
				customDraw->clrTextBk = RGB(119 + Number1, 255 - Number1, 119);
				customDraw->clrText = RGB(0, 0, 0); //COLORREF here.
			}

			return CDRF_NEWFONT;
		}
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
			return CDRF_NEWFONT;
		}

		return CDRF_DODEFAULT;
}


LRESULT WindowMain::Handle_ListviewOwnerData(LPARAM lParam) {

	
	LPNMHDR  lpnmh = (LPNMHDR)lParam;

	switch (lpnmh->code)
	{
		case LVN_GETDISPINFO:
		{
			LV_DISPINFO *lpdi = (LV_DISPINFO *)lParam;

			LVITEM & item = lpdi->item;

			if (item.mask & LVIF_TEXT)
			{

				if (item.iItem >= _ListviewItems.size()) {
					wcscpy_s(item.pszText, item.cchTextMax, L"INVALID");
					return 0;
				}

				ListViewCacheItem & tItem = _ListviewItems[item.iItem];

				const wchar_t * ptr = nullptr;

				switch (item.iSubItem) {
				case 0:
					ptr = tItem.ThreadID;
					break;
				case 1:
					ptr = tItem.ThreadState;
					break;
				case 2:
					ptr = tItem.BasePriority;
					break;
				case 3:
					ptr = tItem.KernelTime;
					break;
				case 4:
					ptr = tItem.UserTime;
					break;
				case 5:
					ptr = tItem.AverageUserTime;
					break;
				case 6:
					ptr = tItem.Node;
					break;
				case 7:
					ptr = tItem.Bitmap;
					break;
				default:
					ptr = L"Undefined";
				}

				wcscpy_s(item.pszText, item.cchTextMax, ptr);

			}
	

		}
	}

	return 0;
}

bool WindowMain::ShowSelectProcess(std::wstring & Filename, std::wstring & FullPath) {

	bool Result = false;

	wchar_t fileName[MAX_PATH] = L"";
	wchar_t fileTitle[MAX_PATH] = L"";

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = _HWND;
	ofn.lpstrFilter = L"Executable Files (*.exe, *.com)\0*.exe;*.com\0";
	ofn.lpstrFile = fileName;
	ofn.lpstrFileTitle = fileTitle;
	ofn.lpstrTitle = L"Select an executable or type the executable name";
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.Flags =  OFN_EXPLORER | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L".exe";
	std::wstring fileNameStr;
	if (GetOpenFileName(&ofn)) {
		Filename.assign(fileTitle);
		FullPath.assign(fileName);
		Result = true;
	}

	return Result;

}

void WindowMain::ClearProcessList() {
	SendMessage(HWND_Listbox, LB_RESETCONTENT, 0, 0);
}

void WindowMain::ReloadConfiguration() {

	_CurrentID = 0;
	bCurrentID = false;

	ClearProcessList();

	_ProcessFactory.DeleteMonitoredProcesses();

	int32_t SelectChange = -1;

	Config::ConfigurationPtr Config = Config::Configuration::Instance();
	if (Config->LoadConfiguration()) {

		Config::CProcessIter it;
		for (it = Config->begin(); it != Config->end(); ++it) {
			Config::ProcessConfigPtr tConfig = it->second;

			if (_ProcessFactory.AddMonitoredProcess(tConfig->Executable, tConfig->KeyID)) {
				int32_t Index = -1;
				ControlHelpers::AddListboxItem(HWND_Listbox, tConfig->Executable, tConfig->KeyID, Index);

				if (tConfig->Selected) {
					SelectChange = Index;
				}

			}

		}
	
	}

	if (SelectChange != -1) {
		ControlHelpers::SetListboxSelIndex(HWND_Listbox, SelectChange);
		Handle_SelectProcess();
	}
}

void WindowMain::LoadConfiguration() {

	//get or add config.
	Config::ConfigurationPtr Config = Config::Configuration::Instance();
	Config::ProcessConfigPtr tProcessConfig = Config::InvalidProcessConfig;
	if (Config->FindConfig(_CurrentID, tProcessConfig)) {

		bool TriggerUpdate = false;

		if (_ShowSleeping != tProcessConfig->ShowSleepingThreads) {
			_ShowSleeping = tProcessConfig->ShowSleepingThreads;
			ControlHelpers::SetCheckboxState(HWND_CKShowSleeping, _ShowSleeping);
			TriggerUpdate = true;
		}

		if (_ShowDorment != tProcessConfig->ShowDormentThreads) {
			_ShowDorment = tProcessConfig->ShowDormentThreads;
			ControlHelpers::SetCheckboxState(HWND_CKShowDorment, _ShowDorment);
			TriggerUpdate = true;
		}

		if (_ShowActive != tProcessConfig->ShowActiveThreads) {
			_ShowActive = tProcessConfig->ShowActiveThreads;
			ControlHelpers::SetCheckboxState(HWND_CKShowDemand, _ShowActive);
			TriggerUpdate = true;
		}

		if (_VSAlgorithm != tProcessConfig->SortMode) {
			_VSAlgorithm = tProcessConfig->SortMode;
			SetSortAlgorithm(_VSAlgorithm);
			TriggerUpdate = true;
		}

		if (TriggerUpdate) {
			UpdateMonitoredProcess();
		}
		
	}

}

void WindowMain::UpdateConfiguration() {

	//get or add config.
	Config::ConfigurationPtr Config = Config::Configuration::Instance();
	Config::ProcessConfigPtr tProcessConfig = Config::InvalidProcessConfig;
	if (Config->FindConfig(_CurrentID, tProcessConfig)) {
	
		bool TriggerUpdate = false;

		if (_ShowSleeping != tProcessConfig->ShowSleepingThreads) {
			tProcessConfig->ShowSleepingThreads = _ShowSleeping;
			TriggerUpdate = true;
		}

		if (_ShowDorment != tProcessConfig->ShowDormentThreads) {
			tProcessConfig->ShowDormentThreads = _ShowDorment;
			TriggerUpdate = true;
		}

		if (_ShowActive != tProcessConfig->ShowActiveThreads) {
			tProcessConfig->ShowActiveThreads = _ShowActive;
			TriggerUpdate = true;
		}

		if (_VSAlgorithm != tProcessConfig->SortMode) {
			tProcessConfig->SortMode = _VSAlgorithm;
			TriggerUpdate = true;
		}

		if (TriggerUpdate) {
			tProcessConfig->ChangesPending = true;
		}

	}

}


bool WindowMain::Handle_AddProcess() {

	std::wstring Filename;
	std::wstring FullPath;

	if (!ShowSelectProcess(Filename, FullPath)) {
		return false;
	}

	Config::ConfigurationPtr Config = Config::Configuration::Instance();
	Config::ProcessConfigPtr tProcessConfig = Config::InvalidProcessConfig;

	if (Config->FindConfig(Filename, tProcessConfig)) {
		if (!tProcessConfig->DeletePending) {
			return false;
		}
		else {
			Config->DeleteConfig(tProcessConfig->KeyID);
		}
			
	}

	size_t NextID = 0;
	if (_ProcessFactory.AddMonitoredProcess(Filename, NextID)) {

		if (!Config->FindConfig(NextID, tProcessConfig)) {
			tProcessConfig = Config->AddConfig(NextID);
			tProcessConfig->Executable = Filename;
		}

		int32_t Index = -1;
		if (ControlHelpers::AddListboxItem(HWND_Listbox, Filename, NextID, Index)) {
			if (ControlHelpers::SetListboxSelIndex(HWND_Listbox, Index)) {
				return Handle_SelectProcess();
			}
		}
	}

	return false;

}

void WindowMain::UpdateSelectedID(size_t ID, bool Delete) {

	Config::ConfigurationPtr Config = Config::Configuration::Instance();
	Config::ProcessConfigPtr tProcessConfig = Config::InvalidProcessConfig;

	if (bCurrentID & Delete) {

		if (Config->FindConfig(_CurrentID, tProcessConfig)) {
			tProcessConfig->Selected = false;
			tProcessConfig->DeletePending = true;
		}

		_CurrentID = 0;
		bCurrentID = false;

		EnableWindow(HWND_ConfigureProcess, FALSE);


	} else if ((bCurrentID) & (ID != _CurrentID)) {

		if (Config->FindConfig(_CurrentID, tProcessConfig)) {
			tProcessConfig->Selected = false;
		}

		_CurrentID = ID;

		if (Config->FindConfig(ID, tProcessConfig)) {
			tProcessConfig->Selected = true;
		}

		EnableWindow(HWND_ConfigureProcess, TRUE);

		LoadConfiguration();

	}
	else {

		if (Config->FindConfig(ID, tProcessConfig)) {
			tProcessConfig->Selected = true;
		}


		bCurrentID = true;
		_CurrentID = ID;

		EnableWindow(HWND_ConfigureProcess, TRUE);

		LoadConfiguration();

	}

	UpdateMonitoredProcess();
	
}

bool WindowMain::Handle_SelectProcess() {

	int32_t ItemIndex = 0;
	if (ControlHelpers::GetListboxSelIndex(HWND_Listbox, ItemIndex)) {

		size_t AssociatedData = 0;
		if (ControlHelpers::GetListboxItemData(HWND_Listbox, ItemIndex, AssociatedData)) {

			ProcessFactory::MonitoredProcessPtr MonitoredProcess = nullptr;
			if (_ProcessFactory.FindMonitoredProcess(AssociatedData, MonitoredProcess)) {
				UpdateSelectedID(AssociatedData, false);
				return true;
			}

		}
	}

	return false;
}

bool WindowMain::Handle_DeleteProcess() {

	int32_t ItemIndex = 0;
	if (ControlHelpers::GetListboxSelIndex(HWND_Listbox, ItemIndex)) {
		size_t AssociatedData = 0;
		if (ControlHelpers::GetListboxItemData(HWND_Listbox, ItemIndex, AssociatedData)) {

			ControlHelpers::DeleteListboxItem(HWND_Listbox, ItemIndex);
			_ProcessFactory.DeleteMonitoredProcess(AssociatedData);
			UpdateSelectedID(AssociatedData, true);
			return true;

		}

	}

	return false;
}

void WindowMain::ReleaseFonts() {

	if (_DefaultFont != nullptr) {
		DeleteObject(_DefaultFont);
		_DefaultFont = 0;
	}

	if (_HeaderFont != nullptr) {
		DeleteObject(_HeaderFont);
		_HeaderFont = 0;
	}

}

void WindowMain::ToRect(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, RECT & rc) {
	rc.left = X;
	rc.top = Y;
	rc.right = X + Width;
	rc.bottom = Y + Height;
}


void WindowMain::ResizeSingleControl(HWND Control, RECT & rc) {
	MoveWindow(Control, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 1);
}

void WindowMain::CreateFonts(HWND Parent) {
	DPI.GetDPI(Parent);

	int fHeight = -MulDiv(9, GetDpiForWindow(_HWND), 72);

	ReleaseFonts();

	_DefaultFont = ControlHelpers::CreateDefaultFont(fHeight, FW_NORMAL);
	_HeaderFont = ControlHelpers::CreateDefaultFont(fHeight, FW_BOLD);

}

void WindowMain::EnableThreadView() {
	_ThreadViewVisible = true;
	EnableWindow(HWND_Listview, 1);
	EnableWindow(HWND_CKShowDemand, 1);
	EnableWindow(HWND_CKShowSleeping, 1);
	EnableWindow(HWND_CKShowDorment, 1);
	EnableWindow(HWND_CM_Sort, 1);

}
void WindowMain::DisableThreadView() {
	_ThreadViewVisible = false;
	EnableWindow(HWND_Listview, 0);
	EnableWindow(HWND_CKShowDemand, 0);
	EnableWindow(HWND_CKShowSleeping, 0);
	EnableWindow(HWND_CKShowDorment, 0);
	EnableWindow(HWND_CM_Sort, 0);
}

void WindowMain::InitNotifyIcon() {

	_NotifyIconData = { 0 };
	_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	_NotifyIconData.hWnd = _HWND;
	_NotifyIconData.uID = 7331;
	_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	_NotifyIconData.uCallbackMessage = WM_NOTIFYICON; 
	_NotifyIconData.hIcon = ICO_SmallIcon;
	wcscpy_s(_NotifyIconData.szTip, 120, TITLE_NAME);

	_NotifyIconData.uVersion = NOTIFYICON_VERSION_4;

	_TrayMenu = CreatePopupMenu();
	AppendMenu(_TrayMenu, MF_STRING, ID_TRAY_SHOW, L"Show");
	AppendMenu(_TrayMenu, MF_STRING, ID_TRAY_CLOSE, L"Exit");

}

void WindowMain::Handle_Minimize() {
	Shell_NotifyIcon(NIM_ADD, &_NotifyIconData);
	Shell_NotifyIcon(NIM_SETVERSION, &_NotifyIconData);

	ShowWindow(_HWND, SW_HIDE);
}

void WindowMain::Handle_Restore() {
	Shell_NotifyIcon(NIM_DELETE, &_NotifyIconData);

	UpdateMonitoredProcess();

	ShowWindow(_HWND, SW_SHOW);

	SetForegroundWindow(_HWND);
	SetFocus(_HWND);
}

void WindowMain::Create() {

	RegClass();

	DWORD sStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE , CLASS_NAME, TITLE_NAME, sStyle , CW_USEDEFAULT, CW_USEDEFAULT, MINSIZE_X, MINSIZE_Y, NULL, NULL, _HInstance, this);


}

void WindowMain::Handle_DPIChange(RECT * rc) {
	DPI.GetDPI(_HWND);

	if (rc != nullptr) {
		SetWindowPos(_HWND, nullptr, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	}

	CreateControls(WM_DPICHANGED);
}

LRESULT WindowMain::CreateControls(UINT Reason) {



	if (Reason == WM_CREATE) {
		DPI.GetDPI(_HWND);

		 SetLayeredWindowAttributes(_HWND, RGB(0, 0, 0), 255, LWA_ALPHA);


		SetWindowPos(_HWND, nullptr, 1, 1, DPI.ScaleValue(MINSIZE_X), DPI.ScaleValue(MINSIZE_Y), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		
	}

	RECT ParentRect;
	GetClientRect(_HWND, &ParentRect);
	DPI.UnScaleRectangle(ParentRect);

	int32_t ControlPadding = 6;
	int32_t ControlPadding_2 = ControlPadding * 2;
	int32_t ControlPadding_3 = ControlPadding * 3;
	int32_t ControlPadding_4 = ControlPadding * 4;
	int32_t ButtonHeight = 28;
	int32_t ButtonWidth = 70;
	int32_t CheckboxWidth = 72;
	int32_t CheckboxHeight = 20;
	int32_t ListboxWidth = 205;

	int32_t ParentWidth = ParentRect.right - ParentRect.left;
	int32_t ParentHeight = ParentRect.bottom;

	RECT rc;
	ControlHelpers::ToRect(ControlPadding, ControlPadding, ListboxWidth, ParentRect.bottom - ControlPadding_3 - ButtonHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_Listbox = ControlHelpers::CreateSingleControl(WS_EX_CLIENTEDGE , L"Listbox", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT, rc, _HWND, CID_Listbox);
	}
	else {
		ResizeSingleControl(HWND_Listbox, rc);
	}

	ControlHelpers::ToRect(ControlPadding_2 + ListboxWidth, ControlPadding, ParentWidth - ListboxWidth - ControlPadding_3, ParentHeight - ControlPadding_3 - ButtonHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_Listview = ControlHelpers::CreateSingleControl(WS_EX_CLIENTEDGE , L"SysListview32", L"", WS_DISABLED | WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_OWNERDATA , rc, _HWND, CID_Listview); // | LVS_OWNERDATA
		ListView_SetExtendedListViewStyle(HWND_Listview, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT );
	}
	else {
		ResizeSingleControl(HWND_Listview, rc);
	}


	uint32_t ButtonSetY = ParentRect.bottom - ControlPadding - ButtonHeight;

	ControlHelpers::ToRect(ControlPadding_2, ButtonSetY, ButtonWidth, ButtonHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_AddProcess = ControlHelpers::CreateSingleControl(NULL, L"button", L"Add", WS_VISIBLE | WS_CHILD , rc, _HWND, CID_AddProcess);
	}
	else {
		ResizeSingleControl(HWND_AddProcess, rc);
	}

	ControlHelpers::ToRect(ControlPadding_3 + ButtonWidth, ButtonSetY, ButtonWidth, ButtonHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_RemoveProcess = ControlHelpers::CreateSingleControl(NULL, L"button", L"Remove", WS_VISIBLE | WS_CHILD , rc, _HWND, CID_RemoveProcess);
	}
	else {
		ResizeSingleControl(HWND_RemoveProcess, rc);
	}

	ControlHelpers::ToRect(ControlPadding_4 + ButtonWidth * 2, ButtonSetY, 38, ButtonHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_ConfigureProcess = ControlHelpers::CreateSingleControl(NULL, L"button", L"C", BS_ICON | WS_VISIBLE | WS_CHILD | WS_DISABLED, rc, _HWND, CID_ConfigureProcess);

		HICON ConfigureIcon = (HICON)LoadImage(_HInstance, MAKEINTRESOURCE(ICON_CGEAR), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);
		SendMessage(HWND_ConfigureProcess, BM_SETIMAGE, IMAGE_ICON, (LPARAM)ConfigureIcon);
	}
	else {
		ResizeSingleControl(HWND_ConfigureProcess, rc);
	}


	ControlHelpers::ToRect(ParentRect.right - CheckboxWidth - ControlPadding, ButtonSetY+2, CheckboxWidth, CheckboxHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_CKShowDorment = ControlHelpers::CreateSingleControl(NULL, L"button", L"Dorment", WS_VISIBLE | WS_CHILD | BS_CHECKBOX , rc, _HWND, CID_ShowDorment);
		ControlHelpers::SetCheckboxState(HWND_CKShowDorment, true);
	}
	else {
		ResizeSingleControl(HWND_CKShowDorment, rc);
	}


	ControlHelpers::ToRect(ParentRect.right - CheckboxWidth * 2 - ControlPadding_2, ButtonSetY+2, CheckboxWidth, CheckboxHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_CKShowSleeping = ControlHelpers::CreateSingleControl(NULL, L"button", L"Sleeping", WS_VISIBLE | WS_CHILD | BS_CHECKBOX , rc, _HWND, CID_ShowSleeping);
		ControlHelpers::SetCheckboxState(HWND_CKShowSleeping, true);

	}
	else {
		ResizeSingleControl(HWND_CKShowSleeping, rc);
	}


	ControlHelpers::ToRect(ParentRect.right - CheckboxWidth * 3 - ControlPadding, ButtonSetY+2, CheckboxWidth - 12, CheckboxHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_CKShowDemand = ControlHelpers::CreateSingleControl(NULL, L"button", L"Active", WS_VISIBLE | WS_CHILD | BS_CHECKBOX , rc, _HWND, CID_ShowDemand);
		ControlHelpers::SetCheckboxState(HWND_CKShowDemand, true);

	}
	else {
		ResizeSingleControl(HWND_CKShowDemand, rc);
	}

	ControlHelpers::ToRect(ControlPadding_3 + ListboxWidth + 110, ButtonSetY - 2, ParentWidth - 560, ButtonHeight+4, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_LblProcess = ControlHelpers::CreateSingleControl(NULL, L"static", L"Test", WS_VISIBLE | WS_CHILD | SS_CENTER , rc, _HWND, CID_LabelProcess);
	}
	else {
		ResizeSingleControl(HWND_LblProcess, rc);
	}


	ControlHelpers::ToRect(ControlPadding_2 + ListboxWidth, ButtonSetY+2, 110, ButtonHeight, rc, DPI);
	if (Reason == WM_CREATE) {
		HWND_CM_Sort = ControlHelpers::CreateSingleControl(NULL, WC_COMBOBOX, L"Select Sort Order...", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, rc, _HWND, CID_SortCombobox);
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"Thread ID");
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"State");
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"Priority");
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"User Time");
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"User Time (Avg)");
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"Kernel Time");
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"Kernel Time (Avg)");
		SendMessage(HWND_CM_Sort, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"Group ID");
		SendMessage(HWND_CM_Sort, CB_SETCURSEL, (WPARAM)4, (LPARAM)0);
	}
	else {
		ResizeSingleControl(HWND_CM_Sort, rc);
	}





	if (Reason == WM_CREATE || Reason == WM_DPICHANGED) {
		CreateFonts(_HWND);

		ControlHelpers::SetFont(HWND_Listview, _DefaultFont);
		ControlHelpers::SetFont(HWND_Listbox, _DefaultFont);

		HWND ListviewHEADER = ListView_GetHeader(HWND_Listview);
		ControlHelpers::SetFont(ListviewHEADER, _HeaderFont);

		ControlHelpers::SetFont(_HWND, _DefaultFont);
		ControlHelpers::SetFont(HWND_AddProcess, _DefaultFont);
		ControlHelpers::SetFont(HWND_RemoveProcess, _DefaultFont);
		ControlHelpers::SetFont(HWND_ConfigureProcess, _DefaultFont);
		ControlHelpers::SetFont(HWND_CKShowDorment, _DefaultFont);
		ControlHelpers::SetFont(HWND_CKShowSleeping, _DefaultFont);
		ControlHelpers::SetFont(HWND_CKShowDemand, _DefaultFont);

		ControlHelpers::SetFont(HWND_CM_Sort, _DefaultFont);

		ControlHelpers::SetFont(HWND_LblProcess, _DefaultFont);

		if (Reason == WM_CREATE) {

			LV_COLUMN tColumn = { 0 };
			tColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
			tColumn.fmt = LVCFMT_CENTER;

			const static uint16_t ColumnWidths[8] = { 70, 82, 76, 72, 72, 86, 72, 192 };
			static const wchar_t * ColumnHeaders[8] = { L"ID",L"State", L"Priority", L"Kernel", L"User", L"User Avg", L"Node", L"Bitmap" };

			size_t Elements = sizeof(ColumnHeaders) / sizeof(ColumnHeaders[0]);

			for (int Column = 0; Column < Elements; Column++) {
				tColumn.cx = ColumnWidths[Column];
				tColumn.pszText = (LPWSTR)ColumnHeaders[Column];
				SendMessage(HWND_Listview, LVM_INSERTCOLUMN, Column, (LPARAM)&tColumn);
			}

			DisableThreadView();

			SetTimer(_HWND, 4753, 999, NULL);

			UpdateMonitoredProcess();
		}

		ControlHelpers::CenterWindowOnHWND(GetDesktopWindow(), _HWND);

	}

	if (Reason == WM_SIZE) {
		SendMessage(HWND_Listview, LVM_SETCOLUMNWIDTH, 7, LVSCW_AUTOSIZE_USEHEADER);
	}

	if (Reason == WM_CREATE) {
		ReloadConfiguration();
	}


	return 0;
}


WPARAM WindowMain::MessagePump() {

	MSG Msg = { 0 };


	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

void WindowMain::Hide() {
	ShowWindow(_HWND, SW_HIDE);
}

void WindowMain::Show() {
	ShowWindow(_HWND, SW_SHOW);
}

WindowMain::WindowMain(HINSTANCE hInstance) {

	PendingDestroy = false;

	_HInstance = hInstance;

	ICO_SmallIcon = 0;
	ICO_LargeIcon = 0;

	_HWND = 0;
	_TrayMenu = 0;

	HWND_Listview = 0;
	HWND_Listbox = 0;
	HWND_CM_Modes = 0;
	HWND_CM_Sort = 0;
	HWND_AddProcess = 0;
	HWND_RemoveProcess = 0;
	HWND_ConfigureProcess = 0;

	HWND_CKShowSleeping = 0;
	HWND_CKShowDemand = 0;
	HWND_CKShowDorment = 0;

	HWND_LblProcess = 0;

	_DefaultFont = 0;
	_HeaderFont = 0;

	_RedrawRequired = true;
	bCurrentID = false;
	_CurrentID = 0;

	_VSAlgorithm = Algorithms::vs_ByAvgUserWallTime;

	_ThreadViewVisible = false;
	_ShowActive = true;
	_ShowSleeping = true;
	_ShowDorment = true;

}

bool WindowMain::RegClass() {

	WNDCLASSEX _wc = { 0 };
	_wc.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(_HInstance, CLASS_NAME, &_wc) == 0) {

		_wc.lpfnWndProc = WindowMain::WndProc;
		_wc.hInstance = _HInstance;
		_wc.lpszClassName = CLASS_NAME;
		_wc.style = CS_HREDRAW | CS_VREDRAW;

		ICO_LargeIcon = (HICON)LoadImage(_HInstance, MAKEINTRESOURCE(MAINICON), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), 0);
		ICO_SmallIcon = (HICON)LoadImage(_HInstance, MAKEINTRESOURCE(MAINICON), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);

		_wc.hIcon = ICO_LargeIcon;
		_wc.hIconSm = ICO_SmallIcon;
		_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		_wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);

		if (RegisterClassEx(&_wc) != 0) {
			return true;
		}
	}

	return false;

}

bool WindowMain::UnRegClass() {

	WNDCLASSEX _wc = { 0 };
	_wc.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(_HInstance, CLASS_NAME, &_wc) != 0) {
		if (UnregisterClass(CLASS_NAME, _HInstance) != 0) {
			return true;
		}
	}

	return false;
}

WindowMain::~WindowMain() {
	
	ReleaseFonts();

	UnRegClass();

}