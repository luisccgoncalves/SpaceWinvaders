#include "viewLogic.h"



ATOM regClass(HINSTANCE hInstance, TCHAR * szAppName) {
	
	WNDCLASSEX  wndClass;

	wndClass.cbSize = sizeof(wndClass);
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = szAppName;
	wndClass.lpfnWndProc = winManager; //Thread?
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wndClass.lpszMenuName = NULL;

	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;


	return(RegisterClassEx(&wndClass));
}

HWND winCreation(HINSTANCE hInstance, TCHAR * szAppName) {

	 return CreateWindow(
		 szAppName,
		 TEXT("Space Winvaders Server"),
		 WS_OVERLAPPEDWINDOW,
		 CW_USEDEFAULT, 
		 CW_USEDEFAULT,
		 CW_USEDEFAULT, 
		 CW_USEDEFAULT,
		 NULL, 
		 NULL, 
		 hInstance, 
		 NULL);
}

LRESULT CALLBACK winManager(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
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