#ifndef __HEADER_DPIHELPER
#define __HEADER_DPIHELPER

#include <Windows.h>
#include <ShellScalingApi.h>


class ScaleHelper
{
private:

	
	typedef HRESULT(WINAPI* GetDPIForMonitor_t)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY);
	HMODULE Shcore;
	GetDPIForMonitor_t getDPIForMonitor;

public:
	ScaleHelper()
	{
		m_nScaleFactorX = 0;
		m_nScaleFactorY = 0;
		Shcore = nullptr;
		getDPIForMonitor = nullptr;
	}

	UINT GetScaleFactor()
	{
		return m_nScaleFactorX;
	}

	int32_t ScaleValue(int32_t value)
	{
		return MulDiv(value, m_nScaleFactorX, 100);
	}

	int32_t UnScaleValue(int32_t value)
	{
		return MulDiv(value, 100, m_nScaleFactorX);
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

	int ScaleFontHeight(int FontHeight) {
		return -MulDiv(FontHeight, m_nScaleFactorY, 100);
	}

	void GetDPI(HWND Window) {
			   
		if (Shcore == nullptr)
		{
			Shcore = LoadLibrary(L"Shcore.dll");
			getDPIForMonitor = Shcore ? (GetDPIForMonitor_t)GetProcAddress(Shcore, "GetDpiForMonitor") : nullptr;
		}

		UINT x = 0;
		UINT y = 0;

		if ((Shcore == nullptr) || (getDPIForMonitor == nullptr))
		{
			HDC tDC = GetDC(0);
			x = GetDeviceCaps(tDC, LOGPIXELSX);
			y = GetDeviceCaps(tDC, LOGPIXELSY);
			ReleaseDC(0, tDC);
		}
		else {
			HMONITOR monitor = MonitorFromWindow(Window, MONITOR_DEFAULTTONEAREST);
			HRESULT Result = getDPIForMonitor(monitor, MDT_EFFECTIVE_DPI, &x, &y);

			if (Result != S_OK) {
				MessageBox(Window, L"Failed to determine DPI", L"Error", MB_OK);
				return;
			}
		}

		m_nScaleFactorX = MulDiv(x, 100, 96);
		m_nScaleFactorY = MulDiv(y, 100, 72);




	}


private:


	UINT m_nScaleFactorX;
	UINT m_nScaleFactorY;
};

#endif