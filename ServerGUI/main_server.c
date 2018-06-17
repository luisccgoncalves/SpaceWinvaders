#include "view_logic.h"

#ifdef _UNICODE									//Sets windows to unicode
#define _tWinMain wWinMain
#else
#define _tWinMain WinMain
#endif

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) { //some warning here about  LPSTR lpCmdLine

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	TCHAR		*szAppName = TEXT("SpaceWinvaders");
	HINSTANCE	hInste = hInstance;


	HWND        hWnd;
	MSG         lpMsg;

	startServer();

	if (!regClass(hInstance, szAppName)) {
		return 0;
	}
	hWnd = winCreation(hInstance, szAppName);
	if (hWnd == NULL) {
		return 0;
	}

	SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 1300, 700, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}

	return (int)lpMsg.wParam;
}