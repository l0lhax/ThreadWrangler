#ifndef __HEADER_DPIHELPER
#define __HEADER_DPIHELPER

#include <Windows.h>
#include <ShellScalingApi.h>

class ScaleHelper
{
public:
	ScaleHelper()
	{
		m_nScaleFactor = 0;
	}

	UINT GetScaleFactor()
	{
		return m_nScaleFactor;
	}

	int32_t ScaleValue(int32_t value)
	{
		return MulDiv(value, m_nScaleFactor, 100);
	}

	int32_t UnScaleValue(int32_t value)
	{
		return MulDiv(value, 100, m_nScaleFactor);
	}

	// Scale rectangle from raw pixels to relative pixels.
	void ScaleRectangle(RECT & pRectangle)
	{
		pRectangle.left = ScaleValue(pRectangle.left);
		pRectangle.right = ScaleValue(pRectangle.right);
		pRectangle.top = ScaleValue(pRectangle.top);
		pRectangle.bottom = ScaleValue(pRectangle.bottom);
	}

	void UnScaleRectangle(RECT & pRectangle)
	{
		pRectangle.left = UnScaleValue(pRectangle.left);
		pRectangle.right = UnScaleValue(pRectangle.right);
		pRectangle.top = UnScaleValue(pRectangle.top);
		pRectangle.bottom = UnScaleValue(pRectangle.bottom);
	}

	// Scale Point from raw pixels to relative pixels.
	void ScalePoint(POINT & pPoint)
	{
		pPoint.x = ScaleValue(pPoint.x);
		pPoint.y = ScaleValue(pPoint.y);
	}

	void GetDPI(HWND Window) {
		UINT x = 0;
		UINT y = 0;

		HMONITOR monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTONEAREST);
		HRESULT Result = GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y);

		if (Result != S_OK) {
			MessageBox(Window, L"Failed to determine DPI", L"Error", MB_OK);
			return;
		}

		m_nScaleFactor = MulDiv(x, 100, 96);

	}


private:
	UINT m_nScaleFactor;
};

#endif