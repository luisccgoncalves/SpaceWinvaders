#include "view_logic.h"
#include "resource.h"

HINSTANCE hInst;

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
	wndClass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);;

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

LRESULT CALLBACK winManager(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC         hdc;

	switch (iMsg) { // for exploring yet
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 100, 100, TEXT("Éló üórledê!"), 13);
		EndPaint(hWnd, &ps);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_SETTINGS_CONFIGUREGAME:
			//openBox?
		case ID_SETTINGS_CLOSESERVER:
			PostQuitMessage(0);
			return 0;
		case ID_SETTINGS_ABOUTE:
			//opne Window
			//dialog(hInst);
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, NULL);

		default:
			break;
		}

	case WM_DESTROY:

		PostQuitMessage(0);
		return 0;
	}


	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}


//int dialog(HINSTANCE hInstance) {
//
//	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, NULL);
//	return 1;
//}
