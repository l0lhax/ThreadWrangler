#include "ControlHelpers.h"
#include <Windows.h>

HWND ControlHelpers::CreateSingleControl(DWORD ExStyle, wchar_t * ClassName, wchar_t * WindowName, DWORD dwStyle, RECT & rc, HWND Parent, uint32_t ControlID) {
	return CreateWindowEx(ExStyle, ClassName, WindowName, dwStyle, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, Parent, (HMENU)static_cast<size_t>(ControlID), GetModuleHandle(NULL), NULL);

}

void ControlHelpers::SetFont(HWND Control, HFONT & Font) {
	SendMessage(Control, WM_SETFONT, (WPARAM)Font, TRUE);
}

HFONT ControlHelpers::CreateDefaultFont(int Height, int Weight) {
	return CreateFont(Height, 0, 0, 0, Weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, TEXT("Segoe UI"));
}

bool ControlHelpers::ToggleCheckbox(HWND Control) {
	bool Checkstate = SendMessage(Control, BM_GETCHECK, 0, 0);
	SendMessage(Control, BM_SETCHECK, (Checkstate) ? BST_UNCHECKED : BST_CHECKED, 0);
	return !Checkstate;
}

bool ControlHelpers::GetCheckboxState(HWND Control) {
	bool Checkstate = SendMessage(Control, BM_GETCHECK, 0, 0);
	return Checkstate;
}

void ControlHelpers::SetCheckboxState(HWND Control, bool Checked) {
	SendMessage(Control, BM_SETCHECK, (Checked) ? BST_CHECKED : BST_UNCHECKED, 0);
}


bool ControlHelpers::GetComboboxSelIndex(HWND Control, int32_t & SelectedIndex) {
	int32_t tSelectedIndex = (int32_t)SendMessage(Control, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (tSelectedIndex != LB_ERR)
	{
		SelectedIndex = tSelectedIndex;
		return true;
	}

	return false;
}

bool ControlHelpers::SetComboboxSelIndex(HWND Control, int32_t SelectedIndex) {

	if ((int32_t)SendMessage(Control, CB_SETCURSEL, (WPARAM)SelectedIndex, (LPARAM)0) == SelectedIndex)
	{
		return true;
	}

	return false;
}

bool ControlHelpers::FindComboboxTextIndex(HWND Control, std::wstring Text, int32_t & TextIndex) {

	int32_t tIndex = (int32_t)SendMessage(Control, CB_FINDSTRINGEXACT, (WPARAM)1, (LPARAM)Text.c_str());
	if (tIndex != CB_ERR) {
		TextIndex = tIndex;
		return true;
	}

	return false;

}

bool ControlHelpers::GetComboboxSelText(HWND Control, std::wstring & Buffer) {

	int32_t CurrentIndex = 0;
	if (GetComboboxSelIndex(Control, CurrentIndex)) {
		int32_t Length = (int32_t)SendMessage(Control, CB_GETLBTEXTLEN, CurrentIndex, (LPARAM)0);
		Buffer.resize(Length + 1);
		wchar_t * tBuffer = &Buffer[0];
		if (SendMessage(Control, CB_GETLBTEXT, CurrentIndex, (LPARAM)tBuffer) != CB_ERR) {
			return true;
		}
	}

	return false;

}

bool ControlHelpers::GetListboxSelIndex(HWND Control, int32_t & SelectedIndex) {
	int32_t tSelectedIndex = (int32_t)SendMessage(Control, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (tSelectedIndex != LB_ERR)
	{
		SelectedIndex = tSelectedIndex;
		return true;
	}

	return false;
}

bool ControlHelpers::AddListboxItem(HWND Control, std::wstring ItemString, size_t AssociatedData, int32_t & Index) {
	LRESULT tIndex = SendMessage(Control, LB_ADDSTRING, 0, (LPARAM)ItemString.c_str());
	if (tIndex != LB_ERR) {
		LRESULT tResult = SendMessage(Control, LB_SETITEMDATA, tIndex, AssociatedData);

		if (tResult != LB_ERR) {
			Index = static_cast<int32_t>(tIndex);
			return true;
			}

	}

	return false;

}

bool ControlHelpers::GetListboxItemData(HWND Control, int32_t Index, size_t & AssociatedData) {
	LRESULT tResult = SendMessage(Control, LB_GETITEMDATA, Index, NULL);
	if (tResult != LB_ERR) {
		AssociatedData = tResult;
		return true;
	}

	return false;

}

bool ControlHelpers::DeleteListboxItem(HWND Control, int32_t Index) {
	LRESULT tResult = SendMessage(Control, LB_DELETESTRING, Index, 0);
	if (tResult == LB_ERR) {
		return false;
	}

	return true;

}

bool ControlHelpers::SetListboxSelIndex(HWND Control, int32_t SelectedIndex) {
	LRESULT tResult = SendMessage(Control, LB_SETCURSEL, (LPARAM)SelectedIndex, NULL);
	if (tResult == LB_ERR) {
		return false;
	}

	return true;
}


void ControlHelpers::ToRect(int32_t X, int32_t Y, int32_t Width, int32_t Height, RECT & rc, ScaleHelper & DPIScaler) {
	rc.left = X;
	rc.top = Y;
	rc.right = X + Width;
	rc.bottom = Y + Height;
	DPIScaler.ScaleRectangle(rc);
}

void ControlHelpers::ToRect(int32_t X, int32_t Y, int32_t Width, int32_t Height, RECT & rc) {
	rc.left = X;
	rc.top = Y;
	rc.right = X + Width;
	rc.bottom = Y + Height;
}

void ControlHelpers::CenterWindowOnHWND(HWND Parent, HWND Child) {
	RECT Parentrc = { 0 };
	GetWindowRect(Parent, &Parentrc);

	RECT Childrc = { 0 };
	GetWindowRect(Child, &Childrc);

	int Center_X = Parentrc.left + ((Parentrc.right - Parentrc.left) / 2);
	int Center_Y = Parentrc.top + ((Parentrc.bottom - Parentrc.top) / 2);


	int ChildWidth = (Childrc.right - Childrc.left);
	int ChildHeight = (Childrc.bottom - Childrc.top);
	int ChildX = Center_X - (ChildWidth / 2);
	int ChildY = Center_Y - (ChildHeight / 2);

	HMONITOR tScreen = MonitorFromWindow(Parent, MONITOR_DEFAULTTONEAREST);
	MONITORINFO tMonitorInfo;
	tMonitorInfo.cbSize = sizeof(MONITORINFO);

	if (GetMonitorInfo(tScreen, &tMonitorInfo) != 0) {
		if (ChildX < tMonitorInfo.rcMonitor.left) { ChildX = tMonitorInfo.rcMonitor.left + 20; }
		if (ChildY < tMonitorInfo.rcMonitor.top) { ChildY = tMonitorInfo.rcMonitor.top + 20; }
		if (ChildX + ChildWidth > tMonitorInfo.rcMonitor.right) { ChildX = tMonitorInfo.rcMonitor.right - ChildWidth - 20; }
		if (ChildY + ChildHeight > tMonitorInfo.rcMonitor.bottom) { ChildY = tMonitorInfo.rcMonitor.bottom - ChildHeight - 20; }
	}

	MoveWindow(Child, ChildX, ChildY, ChildWidth, ChildHeight, 1);

}

void ControlHelpers::CenterWindowOnHWND(HWND Parent, HWND Child, RECT & Childrc) {
	RECT Parentrc = { 0 };
	GetWindowRect(Parent, &Parentrc);

	int Center_X = Parentrc.left + ((Parentrc.right - Parentrc.left) / 2);
	int Center_Y = Parentrc.top + ((Parentrc.bottom - Parentrc.top) / 2);


	int ChildWidth = (Childrc.right - Childrc.left);
	int ChildHeight = (Childrc.bottom - Childrc.top);
	int ChildX = Center_X - (ChildWidth / 2);
	int ChildY = Center_Y - (ChildHeight / 2);

	HMONITOR tScreen = MonitorFromWindow(Parent, MONITOR_DEFAULTTONEAREST);
	MONITORINFO tMonitorInfo;
	tMonitorInfo.cbSize = sizeof(MONITORINFO);

	if (GetMonitorInfo(tScreen, &tMonitorInfo) != 0) {
		if (ChildX < tMonitorInfo.rcMonitor.left) { ChildX = tMonitorInfo.rcMonitor.left + 20; }
		if (ChildY < tMonitorInfo.rcMonitor.top) { ChildY = tMonitorInfo.rcMonitor.top + 20; }
		if (ChildX + ChildWidth > tMonitorInfo.rcMonitor.right) { ChildX = tMonitorInfo.rcMonitor.right - ChildWidth - 20; }
		if (ChildY + ChildHeight > tMonitorInfo.rcMonitor.bottom) { ChildY = tMonitorInfo.rcMonitor.bottom - ChildHeight - 20; }
	}

	MoveWindow(Child, ChildX, ChildY, ChildWidth, ChildHeight, 1);

}
