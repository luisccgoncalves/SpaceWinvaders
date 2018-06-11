#include <windows.h>
#include <tchar.h>

#ifdef _UNICODE									//Sets windows to unicode
#define _tWinMain wWinMain
#else
#define _tWinMain WinMain
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR LpCmdLine, int nCmdShow) {

	static TCHAR *szAppName = TEXT("batatas");
	HWND        hwnd;
	MSG         msg;
	WNDCLASSEX  wndclass;

	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;

	RegisterClassEx(&wndclass);

	hwnd = CreateWindow(szAppName, TEXT("Éló üórledê!"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);   
		DispatchMessage(&msg); 
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC         hdc;

	switch (iMsg) {
	case WM_PAINT:

		hdc = BeginPaint(hwnd, &ps);
		TextOut(hdc, 100, 100, TEXT("Éló üórledê!"), 13);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:

		PostQuitMessage(0);
		return 0;
	}


	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}