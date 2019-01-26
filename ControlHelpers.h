#ifndef __HEADER_CONTROLHELPERS
#define __HEADER_CONTROLHELPERS

#include <Windows.h>
#include <stdint.h>
#include <string>
#include "DPI.h"

class ControlHelpers {
public:

	static void SetFont(HWND Control, HFONT & Font);
	static HWND CreateSingleControl(DWORD ExStyle, wchar_t * ClassName, wchar_t * WindowName, DWORD dwStyle, RECT & rc, HWND Parent, uint32_t ControlID);

	static HFONT CreateDefaultFont(int Height, int Weight);

	static bool ToggleCheckbox(HWND Control);
	static void SetCheckboxState(HWND Control, bool Checked);
	static bool GetCheckboxState(HWND Control);

	static bool GetComboboxSelIndex(HWND Control, int32_t & SelectedIndex);
	static bool SetComboboxSelIndex(HWND Control, int32_t SelectedIndex);
	static bool GetComboboxSelText(HWND Control, std::wstring & Buffer);
	static bool FindComboboxTextIndex(HWND Control, std::wstring Text, int32_t & TextIndex);

	static bool SetListboxSelIndex(HWND Control, int32_t SelectedIndex);
	static bool GetListboxSelIndex(HWND Control, int32_t & SelectedIndex);
	static bool AddListboxItem(HWND Control, std::wstring ItemString, size_t AssociatedData, int32_t & Index);
	static bool DeleteListboxItem(HWND Control, int32_t Index);
	static bool GetListboxItemData(HWND Control, int32_t Index, size_t & AssociatedData);

	static void CenterWindowOnHWND(HWND Parent, HWND Child);
	static void CenterWindowOnHWND(HWND Parent, HWND Child, RECT & rc);

	static void ToRect(int32_t X, int32_t Y, int32_t Width, int32_t Height, RECT & rc, ScaleHelper & DPIScaler);
	static void ToRect(int32_t X, int32_t Y, int32_t Width, int32_t Height, RECT & rc);


};


#endif