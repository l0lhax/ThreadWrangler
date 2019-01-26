#include "WindowPConfig.h"
#include "resource.h"
#include "ControlHelpers.h"
#include "ProcessorMap.h"
#include "Configuration.h"

LRESULT WindowPConfig::Handle_Paint() {
	PAINTSTRUCT ps;
	HDC hdc;
	hdc = BeginPaint(_HWND, &ps);
	EndPaint(_HWND, &ps);
	return 0;
}

void WindowPConfig::Create() {

	RegClass();

	DWORD sStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;

	CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE | WS_EX_CONTROLPARENT, CLASS_NAME, TITLE_NAME, sStyle, CW_USEDEFAULT, CW_USEDEFAULT, 800, 800, _Parent, NULL, _HInstance, this);

}

void WindowPConfig::Handle_Terminate() {
	PendingDestroy = true;
	EnableWindow(_Parent, 1);
	DestroyWindow(_HWND);
	UnRegClass();
}


LRESULT CALLBACK WindowPConfig::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	WindowPConfig *pThis = nullptr;

	if (msg == WM_NCCREATE)
	{
		pThis = static_cast<WindowPConfig*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

		SetLastError(0);
		if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
		{
			if (GetLastError() != 0)
				return FALSE;
		}
	}
	else
	{
		pThis = reinterpret_cast<WindowPConfig*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}



	switch (msg)
	{
	case WM_CREATE:
		pThis->_HWND = hwnd;
		return pThis->CreateControls(msg);
	case WM_CLOSE:
		pThis->Handle_Terminate();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		return pThis->Handle_Paint();
	case WM_TIMER:
		break;
	case WM_SIZE:
		return pThis->CreateControls(msg);
	case WM_COMMAND: {

		HWND CallingHWND = (HWND)lParam;
		uint32_t Code = HIWORD(wParam);

		if (Code == BN_CLICKED)
		{
			pThis->Handle_Click(CallingHWND);
		}
	}
	break;

	case WM_DPICHANGED:
		pThis->CreateControls(msg);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}


	return 0;
}


void WindowPConfig::ReleaseFonts() {

	if (_DefaultFont != nullptr)
		DeleteObject(_DefaultFont);

	if (_HeaderFont != nullptr)
		DeleteObject(_HeaderFont);

}

void WindowPConfig::CreateFonts(HWND Parent) {

	DPI.GetDPI(Parent);

	int fHeight = DPI.ScaleFontHeight(9); 
	int fBHeight = DPI.ScaleFontHeight(8); 

	ReleaseFonts();

	_DefaultFont = ControlHelpers::CreateDefaultFont(fHeight, FW_NORMAL);
	_HeaderFont = ControlHelpers::CreateDefaultFont(fBHeight, FW_BOLD);

}

void WindowPConfig::ApplyConfiguration() {

	if (_TargetConfig != nullptr) {

		bool Changes = false;

		std::vector<stGroup>::iterator it;
		for (it = _Groups.begin(); it != _Groups.end(); it++) {
			stGroup & tGroup = *it;

			Config::GroupConfigPtr GroupConfig = Config::InvalidGroupConfigPtr;
			GroupConfig = _TargetConfig->GetGroupConfig(tGroup.GroupID);

			bool ExcludeGroup = ControlHelpers::GetCheckboxState(tGroup.CK_ExcludeGroup);
			bool ExcludeHTCores = ControlHelpers::GetCheckboxState(tGroup.CK_ExcludeHTCores);

			Processors::AffinityType ExcludeAffinity = 0;
			Processors::AffinityType DTCAffinity = 0;
			Processors::AffinityType ITCAffinity = 0;

			std::vector<stCPU>::iterator it2;
			for (it2 = tGroup._CPUs.begin(); it2 != tGroup._CPUs.end(); it2++) {

				stCPU & tCPU = *it2;

				Processors::AffinityType Mask = (1i64 << tCPU.CPUID);

				if ((ExcludeGroup || (ExcludeHTCores & tCPU.HTCore))) {
					ExcludeAffinity |= Mask;
				}
				else {

					bool tExclude = ControlHelpers::GetCheckboxState(tCPU.CK_Exclude);
					if (tExclude) {
						ExcludeAffinity |= Mask;
					}
					else 
					{
						bool tITC = ControlHelpers::GetCheckboxState(tCPU.CK_ITC);

						if (tITC)
							ITCAffinity |= Mask;

						bool tDTC = ControlHelpers::GetCheckboxState(tCPU.CK_DTC);

						if (tDTC)
							DTCAffinity |= Mask;
					}

				}

			}

			if (GroupConfig->DTC != DTCAffinity) {
				GroupConfig->DTC = DTCAffinity;
				Changes = true;
			}

			if (GroupConfig->ITC != ITCAffinity) {
				GroupConfig->ITC = ITCAffinity;
				Changes = true;
			}

			if (GroupConfig->Excluded != ExcludeAffinity) {
				GroupConfig->Excluded = ExcludeAffinity;
				Changes = true;
			}

			if (ExcludeGroup) {
				GroupConfig->ExcludeGroup = true;
				GroupConfig->ExcludeHTCores = false;
			}
			else if (ExcludeHTCores) {
				GroupConfig->ExcludeGroup = false;
				GroupConfig->ExcludeHTCores = true;
			}
			else {
				GroupConfig->ExcludeGroup = false;
				GroupConfig->ExcludeHTCores = false;
			}
		}

		_TargetConfig->AdvChangesPending = Changes;

		Changes = false;

		bool IdealThreadMode = ControlHelpers::GetCheckboxState(CK_IdealThreadMode);
		bool AffinityMode = ControlHelpers::GetCheckboxState(CK_AffinityMode);

		if (_TargetConfig->IdealThreadMode != IdealThreadMode) {
			_TargetConfig->IdealThreadMode = IdealThreadMode;
			Changes = true;
		}

		if (_TargetConfig->AffinityMode != AffinityMode) {
			_TargetConfig->AffinityMode = AffinityMode;
			Changes = true;
		}

		Algorithms::TSAlgorithm tAlgorithm = Algorithms::TSI_UserTime_Sticky;

		int SelectedIndex = 0;
		if (ControlHelpers::GetComboboxSelIndex(CB_Algorithm, SelectedIndex)) {

			
			switch (SelectedIndex) {
			case 0:
				tAlgorithm = Algorithms::TSI_UserTime_Blind;
				break;
			case 1:
			default:
				tAlgorithm = Algorithms::TSI_UserTime_Sticky;
				break;
			}
		}
		else {
			tAlgorithm = Algorithms::TSI_UserTime_Sticky;
		}

		if (_TargetConfig->Algorithm != tAlgorithm) {
			_TargetConfig->Algorithm = tAlgorithm;
			Changes = true;
		}

		std::wstring SelectedText;
		if (ControlHelpers::GetComboboxSelText(CB_CoresPerThread, SelectedText)) {

			int32_t tCoresPerThread;

			if (_wcsicmp(SelectedText.c_str(), L"Default") == 0) {
				tCoresPerThread = 0;
			}
			else {
				tCoresPerThread = _wtoi(SelectedText.c_str());
			}

			if (_TargetConfig->ProcessorsPerThread != tCoresPerThread) {
				_TargetConfig->ProcessorsPerThread = tCoresPerThread;
				Changes = true;
			}
		}
		else {
			if (_TargetConfig->ProcessorsPerThread != 0) {
				_TargetConfig->ProcessorsPerThread = 0;
				Changes = true;
			}
		}


		_TargetConfig->ChangesPending = Changes;
	}

}

void WindowPConfig::RestoreConfiguration() {

	bool ExcludeGroup = false;
	bool ExcludeHTCores = false;

	if (_TargetConfig != nullptr) {

		std::vector<stGroup>::iterator it;
		for (it = _Groups.begin(); it != _Groups.end(); it++) {
			stGroup & tGroup = *it;

			Config::GroupConfigPtr GroupConfig = Config::InvalidGroupConfigPtr;
			GroupConfig = _TargetConfig->GetGroupConfig(tGroup.GroupID);

			ExcludeGroup = GroupConfig->ExcludeGroup;
			ExcludeHTCores = GroupConfig->ExcludeHTCores;

			if (ExcludeGroup) {
				ControlHelpers::SetCheckboxState(tGroup.CK_ExcludeGroup, true);
				EnableWindow(tGroup.CK_ExcludeGroup, TRUE);

				ControlHelpers::SetCheckboxState(tGroup.CK_ExcludeHTCores, false);
				EnableWindow(tGroup.CK_ExcludeHTCores, FALSE);
			}
			else if (ExcludeHTCores) {
				ControlHelpers::SetCheckboxState(tGroup.CK_ExcludeGroup, false);
				EnableWindow(tGroup.CK_ExcludeGroup, FALSE);

				ControlHelpers::SetCheckboxState(tGroup.CK_ExcludeHTCores, true);
				EnableWindow(tGroup.CK_ExcludeHTCores, TRUE);
			}
			else {
				ControlHelpers::SetCheckboxState(tGroup.CK_ExcludeGroup, false);
				EnableWindow(tGroup.CK_ExcludeGroup, TRUE);

				ControlHelpers::SetCheckboxState(tGroup.CK_ExcludeHTCores, false);
				EnableWindow(tGroup.CK_ExcludeHTCores, TRUE);
			}

			std::vector<stCPU>::iterator it2;
			for (it2 = tGroup._CPUs.begin(); it2 != tGroup._CPUs.end(); it2++) {

				stCPU & tCPU = *it2;

				if ((ExcludeGroup || (ExcludeHTCores & tCPU.HTCore))) {

					tCPU.ExcludePState = false;
					ControlHelpers::SetCheckboxState(tCPU.CK_Exclude, true);
					EnableWindow(tCPU.CK_Exclude, FALSE);

					tCPU.DTCPState = true;
					ControlHelpers::SetCheckboxState(tCPU.CK_DTC, false);
					EnableWindow(tCPU.CK_DTC, FALSE);

					tCPU.ITCPState = true;
					ControlHelpers::SetCheckboxState(tCPU.CK_ITC, false);
					EnableWindow(tCPU.CK_ITC, FALSE);

				}
				else {

					Processors::AffinityType Mask = (1i64 << tCPU.CPUID);

					bool tExclude = ((GroupConfig->Excluded & Mask) == Mask);
					ControlHelpers::SetCheckboxState(tCPU.CK_Exclude, tExclude);
					EnableWindow(tCPU.CK_Exclude, TRUE);

					bool tPresent = ((GroupConfig->ITC & Mask) == Mask);
					ControlHelpers::SetCheckboxState(tCPU.CK_ITC, tExclude ? false : tPresent);
					EnableWindow(tCPU.CK_ITC, !tExclude);

					tPresent = ((GroupConfig->DTC & Mask) == Mask);
					ControlHelpers::SetCheckboxState(tCPU.CK_DTC, tExclude ? false : tPresent);
					EnableWindow(tCPU.CK_DTC, !tExclude);

				}

			}


		}

		ControlHelpers::SetCheckboxState(CK_IdealThreadMode, _TargetConfig->IdealThreadMode);
		ControlHelpers::SetCheckboxState(CK_AffinityMode, _TargetConfig->AffinityMode);
		


		SetSortAlgorithm(_TargetConfig->Algorithm);
		SetCoresPerThread(_TargetConfig->ProcessorsPerThread);
	}



}

void WindowPConfig::Handle_Click(HWND Control) {


	if (Control == BTN_OK) {
		ApplyConfiguration();		
		Handle_Terminate();
	}
	else if (Control == BTN_Cancel) {
		Handle_Terminate();
	}
	else if (Control == BTN_Revert) {
		RestoreConfiguration();
	}
	else if ((Control == CK_AffinityMode) || (Control == CK_IdealThreadMode)) {
		ControlHelpers::ToggleCheckbox(Control);
	}
	else {
		std::vector<stGroup>::iterator it;

		for (it = _Groups.begin(); it != _Groups.end(); it++) {

			stGroup & tGroup = *it;
			if ((Control == tGroup.CK_ExcludeGroup) || (Control == tGroup.CK_ExcludeHTCores)) {
				bool State = ControlHelpers::ToggleCheckbox(Control);

				std::vector<stCPU>::iterator it2;

				bool HTOnly = (Control == tGroup.CK_ExcludeHTCores);

				for (it2 = tGroup._CPUs.begin(); it2 != tGroup._CPUs.end(); it2++) {
					stCPU & tCPU = *it2;

					if (HTOnly && !tCPU.HTCore)
						continue;

					if (State)
						tCPU.ExcludePState = ControlHelpers::GetCheckboxState(tCPU.CK_Exclude);
					ControlHelpers::SetCheckboxState(tCPU.CK_Exclude, State ? true : tCPU.ExcludePState);
					EnableWindow(tCPU.CK_Exclude, !State);

					if (State)
						tCPU.DTCPState = ControlHelpers::GetCheckboxState(tCPU.CK_DTC);
					ControlHelpers::SetCheckboxState(tCPU.CK_DTC, State ? false : tCPU.DTCPState);
					EnableWindow(tCPU.CK_DTC, !(State | tCPU.ExcludePState));

					if (State)
						tCPU.ITCPState = ControlHelpers::GetCheckboxState(tCPU.CK_ITC);
					ControlHelpers::SetCheckboxState(tCPU.CK_ITC, State ? false : tCPU.ITCPState);
					EnableWindow(tCPU.CK_ITC, !(State | tCPU.ExcludePState));
				}

				if (HTOnly) {
					EnableWindow(tGroup.CK_ExcludeGroup, !State);
				}
				else {
					EnableWindow(tGroup.CK_ExcludeHTCores, !State);
				}


			}
			else {
				std::vector<stCPU>::iterator it2;

				for (it2 = tGroup._CPUs.begin(); it2 != tGroup._CPUs.end(); it2++) {

					stCPU & tCPU = *it2;
					if (Control == tCPU.CK_DTC) {
						ControlHelpers::ToggleCheckbox(Control);
					}
					else if (Control == tCPU.CK_Exclude) {
						bool State = ControlHelpers::ToggleCheckbox(Control);

						if (State)
							tCPU.DTCPState = ControlHelpers::GetCheckboxState(tCPU.CK_DTC);
						ControlHelpers::SetCheckboxState(tCPU.CK_DTC, State ? false : tCPU.DTCPState);
						EnableWindow(tCPU.CK_DTC, !State);

						if (State)
							tCPU.ITCPState = ControlHelpers::GetCheckboxState(tCPU.CK_ITC);
						ControlHelpers::SetCheckboxState(tCPU.CK_ITC, State ? false : tCPU.ITCPState);
						EnableWindow(tCPU.CK_ITC, !State);

					}
					else if (Control == tCPU.CK_ITC) {
						ControlHelpers::ToggleCheckbox(Control);
					}

				}
			}
		}
	}




}


LRESULT WindowPConfig::CreateControls(UINT Reason) {

	if (Reason == WM_CREATE) {
		DPI.GetDPI(_HWND);
	}

	//WM_SIZE WM_DPICHANGED
	if (Reason == WM_CREATE) {
		DPI.GetDPI(_HWND);
		CreateFonts(_HWND);

		ControlHelpers::SetFont(_HWND, _DefaultFont);
	
		Processors::ProcessorMapPtr ProcessorInfo = Processors::ProcessorMap::getInstance();

		/*uint16_t testprocs = 16;



		Processors::GroupPtr bsGroup0 = ProcessorInfo->GetGroup(0);
		bsGroup0->RemoveProcessors();
		for (int i = 0; i < testprocs; i++) {
			Processors::ProcessorPtr tProcessor = bsGroup0->AddProcessor(i);
			if (i % 2 != 0) { tProcessor->SetFlag_HTCore(true, true); }
		}

		Processors::GroupPtr bsGroup1 = ProcessorInfo->AddGroup(1);
		bsGroup1->RemoveProcessors();
		for (int i = 0; i < testprocs; i++) {
			Processors::ProcessorPtr tProcessor = bsGroup1->AddProcessor(i);
			if (i % 2 != 0) { tProcessor->SetFlag_HTCore(true, true); }
		}

		Processors::GroupPtr bsGroup2 = ProcessorInfo->AddGroup(2);
		bsGroup2->RemoveProcessors();
		for (int i = 0; i < testprocs; i++) {
			Processors::ProcessorPtr tProcessor = bsGroup2->AddProcessor(i);
			if (i % 2 != 0) { tProcessor->SetFlag_HTCore(true, true); }
		}
		
		Processors::GroupPtr bsGroup3 = ProcessorInfo->AddGroup(3);
		bsGroup3->RemoveProcessors();
		for (int i = 0; i < testprocs; i++) {
			Processors::ProcessorPtr tProcessor = bsGroup3->AddProcessor(i);
			if (i % 2 != 0) { tProcessor->SetFlag_HTCore(true, true); }
		}*/
		
		uint32_t ELeftOffset = 0;
		uint32_t ETopOffset = 0;
		uint32_t Colomn = 0;
		uint32_t Row = 0;
		uint32_t TopOffset = 0;
		uint32_t LeftOffset = 0;
		RECT rc = { 0 };

		uint32_t MaxCores = 0;


		for (Processors::GroupsIter git = ProcessorInfo->begin(); git != ProcessorInfo->end(); git++) {
			Processors::GroupPtr Group = git->second;

			stGroup tGroup;
			tGroup.GroupID = Group->GetID();

			uint32_t TotalCores = Group->GetProcessorCount();
			if ((TotalCores > MaxCores) || (MaxCores == 0)) {
				MaxCores = TotalCores;
			}

			uint32_t CoresPerColumn = TotalCores / 4;

			if (TotalCores % 4 == 0) {
				CoresPerColumn = TotalCores / 4;
			}
			else if (TotalCores % 3 == 0) {
				CoresPerColumn = TotalCores / 3;
			}
			else if (TotalCores % 2 == 0) {
				CoresPerColumn = TotalCores / 2;
			}

			Colomn = 0;
			Row = 0;


			for (Processors::ProcessorsIter pit = Group->begin(); pit != Group->end(); ++pit) {
				Processors::ProcessorPtr tProcessor = pit->second;

				stCPU CurrentCPU;
				CurrentCPU.HTCore = tProcessor->GetFlag_HTCore();

				TopOffset = (Row * 16) + 46;
				LeftOffset = (Colomn * 140);

				CurrentCPU.CPUID = tProcessor->GetID();

				wchar_t CPUID[10];
				wsprintf(CPUID, L"CPU%u", CurrentCPU.CPUID);

				ControlHelpers::ToRect(ELeftOffset + LeftOffset + 26, ETopOffset + TopOffset, 35, 13, rc, DPI);
				CurrentCPU.lbl_CPUID = ControlHelpers::CreateSingleControl(NULL, L"static", CPUID, WS_CHILD | WS_VISIBLE | SS_RIGHT, rc, _HWND, 0);

				ControlHelpers::ToRect(ELeftOffset + LeftOffset + 70, ETopOffset + TopOffset, 15, 16, rc, DPI);
				CurrentCPU.CK_Exclude = ControlHelpers::CreateSingleControl(NULL, L"button", L"", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);

				ControlHelpers::ToRect(ELeftOffset + LeftOffset + 95, ETopOffset + TopOffset, 15, 16, rc, DPI);
				CurrentCPU.CK_ITC = ControlHelpers::CreateSingleControl(NULL, L"button", L"", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);

				ControlHelpers::ToRect(ELeftOffset + LeftOffset + 120, ETopOffset + TopOffset, 15, 16, rc, DPI);
				CurrentCPU.CK_DTC = ControlHelpers::CreateSingleControl(NULL, L"button", L"", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);
				
				ControlHelpers::SetFont(CurrentCPU.lbl_CPUID, _DefaultFont);
				ControlHelpers::SetFont(CurrentCPU.CK_Exclude, _DefaultFont);
				ControlHelpers::SetFont(CurrentCPU.CK_ITC, _DefaultFont);
				ControlHelpers::SetFont(CurrentCPU.CK_DTC, _DefaultFont);


				if ((Row+1) == CoresPerColumn) {

					ControlHelpers::ToRect(LeftOffset + 67, ETopOffset + 30, 22, 13, rc, DPI);
					HWND tLabel = ControlHelpers::CreateSingleControl(NULL, L"static", L"Excl", WS_CHILD | WS_VISIBLE | SS_CENTER, rc, _HWND, 0);
					ControlHelpers::SetFont(tLabel, _HeaderFont);

					ControlHelpers::ToRect(LeftOffset + 91, ETopOffset + 30, 22, 13, rc, DPI);
					tLabel = ControlHelpers::CreateSingleControl(NULL, L"static", L"ITC", WS_CHILD | WS_VISIBLE | SS_CENTER, rc, _HWND, 0);
					ControlHelpers::SetFont(tLabel, _HeaderFont);

					ControlHelpers::ToRect(LeftOffset + 117, ETopOffset + 30, 22, 13, rc, DPI);
					tLabel = ControlHelpers::CreateSingleControl(NULL, L"static", L"DTC", WS_CHILD | WS_VISIBLE | SS_CENTER, rc, _HWND, 0);
					ControlHelpers::SetFont(tLabel, _HeaderFont);


					if (Colomn == 1) { tGroup.lbl_CPUCol1 = tLabel; }
					else if (Colomn == 2) { tGroup.lbl_CPUCol2 = tLabel; }
					else if (Colomn == 3) { tGroup.lbl_CPUCol3 = tLabel; }
					else if (Colomn == 4) { tGroup.lbl_CPUCol4 = tLabel; }

					Colomn++;
					Row = 0;
				}
				else {
					Row++;
				}

				tGroup._CPUs.emplace_back(CurrentCPU);
			}

			ControlHelpers::ToRect(ELeftOffset + 24, ETopOffset + TopOffset + 32, 100, 16, rc, DPI); //MaxX - 210
			tGroup.CK_ExcludeGroup = ControlHelpers::CreateSingleControl(NULL, L"button", L"Exclude Group", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);

			ControlHelpers::ToRect(ELeftOffset + 130, ETopOffset + TopOffset + 32, 110, 16, rc, DPI); //MaxX -110
		    tGroup.CK_ExcludeHTCores = ControlHelpers::CreateSingleControl(NULL, L"button", L"Exclude HT Cores", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);

			ControlHelpers::SetFont(tGroup.CK_ExcludeGroup, _DefaultFont);
			ControlHelpers::SetFont(tGroup.CK_ExcludeHTCores, _DefaultFont);

			wchar_t GroupHeader[20];
			wsprintf(GroupHeader, L"Group %u", tGroup.GroupID);
			ControlHelpers::ToRect(ELeftOffset + 10, ETopOffset + 10, LeftOffset + 144, TopOffset + 50, rc, DPI);
			tGroup.GB_Processors = ControlHelpers::CreateSingleControl(NULL, L"button", GroupHeader, WS_CHILD | WS_VISIBLE | BS_GROUPBOX, rc, _HWND, 0);
			ControlHelpers::SetFont(tGroup.GB_Processors, _HeaderFont);

			_Groups.emplace_back(tGroup);

			ETopOffset += TopOffset + 60;

		}

		uint32_t FormWidth = LeftOffset + 180;

		ControlHelpers::ToRect(ELeftOffset + 10, ETopOffset + 4, LeftOffset + 144, 78, rc, DPI);
		HWND AlgGroup = ControlHelpers::CreateSingleControl(NULL, L"button", L"Thread Allocation Algorithm", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, rc, _HWND, 0);
		ControlHelpers::SetFont(AlgGroup, _HeaderFont);


		if (FormWidth < 400) {
			ControlHelpers::ToRect(ELeftOffset + 170, ETopOffset + 26, 119, 16, rc, DPI);
			CK_IdealThreadMode = ControlHelpers::CreateSingleControl(NULL, L"button", L"Ideal Thread Mode", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);

			ControlHelpers::ToRect(ELeftOffset + 170, ETopOffset + 48, 95, 16, rc, DPI);
			CK_AffinityMode = ControlHelpers::CreateSingleControl(NULL, L"button", L"Affinity Mode", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);
		}
		else {
			ControlHelpers::ToRect(ELeftOffset + 170, ETopOffset + 24, 119, 16, rc, DPI);
			CK_IdealThreadMode = ControlHelpers::CreateSingleControl(NULL, L"button", L"Ideal Thread Mode", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);

			ControlHelpers::ToRect(ELeftOffset + 300, ETopOffset + 24, 90, 16, rc, DPI);
			CK_AffinityMode = ControlHelpers::CreateSingleControl(NULL, L"button", L"Affinity Mode", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, rc, _HWND, 0);
		}


		ControlHelpers::SetCheckboxState(CK_IdealThreadMode, true);
		ControlHelpers::SetCheckboxState(CK_AffinityMode, true);

		ControlHelpers::SetFont(CK_IdealThreadMode, _DefaultFont);
		ControlHelpers::SetFont(CK_AffinityMode, _DefaultFont);



		ControlHelpers::ToRect(ELeftOffset + 18, ETopOffset + 22, 120, 16, rc, DPI); //MaxX - 210
		CB_Algorithm = ControlHelpers::CreateSingleControl(NULL, L"combobox", L"",  CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, rc, _HWND, 0);
		ControlHelpers::SetFont(CB_Algorithm, _DefaultFont);

		SendMessage(CB_Algorithm, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"User Time (Blind)");
		SendMessage(CB_Algorithm, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"User Time (Sticky)");
		ControlHelpers::SetComboboxSelIndex(CB_Algorithm, 0);

		ControlHelpers::ToRect(ELeftOffset + 18, ETopOffset + 50, 120, 16, rc, DPI); //MaxX - 210
		CB_CoresPerThread = ControlHelpers::CreateSingleControl(NULL, L"combobox", L"", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, rc, _HWND, 0);
		ControlHelpers::SetFont(CB_CoresPerThread, _DefaultFont);

		SendMessage(CB_CoresPerThread, CB_ADDSTRING, (WPARAM)0, (LPARAM)L"Default");


		for (uint32_t mc = 1; mc <= MaxCores; mc++) {
			if (mc % 2 == 0) {
				wchar_t tBuffer[16];
				wsprintf(tBuffer, L"%u", mc);
				SendMessage(CB_CoresPerThread, CB_ADDSTRING, (WPARAM)0, (LPARAM)tBuffer);
			}
		}

		int cbcount = SendMessage(CB_CoresPerThread, CB_GETCOUNT, (WPARAM)0, (LPARAM)0);

		ControlHelpers::SetComboboxSelIndex(CB_CoresPerThread, cbcount-1);


		
		ControlHelpers::ToRect((FormWidth / 2) - 40 - 80 - 5, ETopOffset + 93, 80, 20, rc, DPI); //MaxX - 210
		BTN_OK = ControlHelpers::CreateSingleControl(NULL, L"button", L"OK",  WS_CHILD | WS_VISIBLE, rc, _HWND, 0);
		ControlHelpers::SetFont(BTN_OK, _DefaultFont);

		ControlHelpers::ToRect((FormWidth / 2) - 40, ETopOffset + 93, 80, 20, rc, DPI); //MaxX - 210
		BTN_Cancel = ControlHelpers::CreateSingleControl(NULL, L"button", L"Cancel", WS_CHILD | WS_VISIBLE, rc, _HWND, 0);
		ControlHelpers::SetFont(BTN_Cancel, _DefaultFont);

		ControlHelpers::ToRect((FormWidth / 2) + 40 + 5, ETopOffset + 93, 80, 20, rc, DPI); //MaxX - 210
		BTN_Revert = ControlHelpers::CreateSingleControl(NULL, L"button", L"Revert", WS_CHILD | WS_VISIBLE, rc, _HWND, 0);
		ControlHelpers::SetFont(BTN_Revert, _DefaultFont);

		if (_Parent != nullptr) {

			int titleheight = (GetSystemMetrics(SM_CXSIZEFRAME) * 2) + GetSystemMetrics(SM_CYCAPTION);
			titleheight = DPI.UnScaleValue(titleheight);

			ControlHelpers::ToRect(1, 1, LeftOffset + 180, ETopOffset + 132 + titleheight, rc, DPI);
			ControlHelpers::CenterWindowOnHWND(_Parent, _HWND, rc);
		}

		RestoreConfiguration();
		BringWindowToTop(_HWND);


	}

	return 0;
}

void WindowPConfig::SetCoresPerThread(uint32_t CoresPerThread) {

	if (CoresPerThread == 0) {
		ControlHelpers::SetComboboxSelIndex(CB_CoresPerThread, 0);
	}
	else {
		wchar_t Buffer[16];
		_itow_s(CoresPerThread, Buffer, 16, 10);

		int32_t tIndex = 0;
		if (ControlHelpers::FindComboboxTextIndex(CB_CoresPerThread, Buffer, tIndex)) {
			ControlHelpers::SetComboboxSelIndex(CB_CoresPerThread, tIndex);
		}
	}

}

void WindowPConfig::SetSortAlgorithm(Algorithms::TSAlgorithm ThreadSortAlg) {

	int32_t SelectedIndex = 0;

	switch (ThreadSortAlg) {
	case Algorithms::TSI_UserTime_Blind:
		SelectedIndex = 0;
		break;
	case Algorithms::TSI_UserTime_Sticky:
		SelectedIndex = 1;
		break;
	}

	ControlHelpers::SetComboboxSelIndex(CB_Algorithm, SelectedIndex);

}

WPARAM WindowPConfig::MessagePump() {

	MSG Msg = { 0 };

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

void WindowPConfig::Hide() {
	ShowWindow(_HWND, SW_HIDE);
}

void WindowPConfig::Show() {
	ShowWindow(_HWND, SW_SHOW);
}


bool WindowPConfig::RegClass() {

	WNDCLASSEX _wc = { 0 };
	_wc.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(_HInstance, CLASS_NAME, &_wc) == 0) {

		_wc.lpfnWndProc = WindowPConfig::WndProc;
		_wc.hInstance = _HInstance;
		_wc.lpszClassName = CLASS_NAME;
		_wc.style = CS_HREDRAW | CS_VREDRAW;
		_wc.hIcon = (HICON)LoadImage(_HInstance, MAKEINTRESOURCE(MAINICON), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), 0);
		_wc.hIconSm = (HICON)LoadImage(_HInstance, MAKEINTRESOURCE(MAINICON), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
		_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		_wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);


		if (RegisterClassEx(&_wc) != 0) {
			return true;
		}
	}

	return false;

}

void WindowPConfig::SetTargetConfig(Config::ProcessConfigPtr ProcessConfig) {
	_TargetConfig = ProcessConfig;
}

bool WindowPConfig::UnRegClass() {

	WNDCLASSEX _wc = { 0 };
	_wc.cbSize = sizeof(WNDCLASSEX);

	if (GetClassInfoEx(_HInstance, CLASS_NAME, &_wc) != 0) {
		if (UnregisterClass(CLASS_NAME, _HInstance) != 0) {
			return true;
		}
	}

	return false;
}

WindowPConfig::~WindowPConfig() {

	ReleaseFonts();

	UnRegClass();

}

WindowPConfig::WindowPConfig(HINSTANCE hInstance, HWND Parent) {
	_HInstance = hInstance;
	_DefaultFont = nullptr;
	_Parent = Parent;
	PendingDestroy = false;
	CB_CoresPerThread = nullptr;
	CB_Algorithm = nullptr;
	CK_IdealThreadMode = nullptr;
	CK_AffinityMode = nullptr;
	BTN_OK = nullptr;
	BTN_Cancel = nullptr;
	BTN_Revert = nullptr;
}