#include "Configuration.h"
#include "WindowMain.h"
#include <commctrl.h>
#include <ShellScalingApi.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' ""version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//int wmain(int argc, wchar_t * argv[]) {
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR     lpCmdLine, _In_ int       nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	//AllocConsole();
	//AttachConsole(GetCurrentProcessId());
	//freopen("CON", "w", stdout);

	INITCOMMONCONTROLSEX icex = { 0 };          
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	if (!InitCommonControlsEx(&icex)) {
		MessageBox(NULL, L"Initialisation of common controls failed", L"Fail", MB_OK);
		exit(1);
	}

	Processors::ProcessorMapPtr ProcessorInfo = Processors::ProcessorMap::getInstance();
	ProcessorInfo->DetectSystemLayout_2();

	WindowMain tWindow(hInstance);
	tWindow.Create();
	tWindow.Show();

	tWindow.MessagePump();


	return 0;
}

