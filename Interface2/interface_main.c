#include <windows.h>

#ifdef _UNICODE									//Sets windows to unicode
#define _tWinMain wWinMain
#else
#define _tWinMain WinMain
#endif
//HWND createWindow(HINSTANCE hInstance, TCHAR *szWinName) {
//	return CreateWindow(
//	);
//
//}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR LpCmdLine, int nCmdShow) {



	//HWND hWindow;

	//hWindow = createWindow(hInstance, TEXT("Space Winvaders"));
	
	//RegisterClassEx();

	//CreateWindowEx(
	//	0,
	//	0,
	//	TEXT("Space Winvaders"),
	//	WS_OVERLAPPEDWINDOW,
	//	CW_USEDEFAULT,
	//	CW_USEDEFAULT,
	//	CW_USEDEFAULT,
	//	CW_USEDEFAULT,
	//	(HWND)HWND_DESKTOP,
	//	(HMENU)NULL,
	//	(HINSTANCE)hInstance,
	//	NULL);

	//MessageBoxEx(NULL, TEXT("o_O'"), TEXT("Space Winvaders"), MB_OK , 0);

	return 0;
}