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
			break;
		case ID_SETTINGS_CLOSESERVER:
			PostQuitMessage(0);
			return 0;
		case ID_SETTINGS_ABOUTE:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, winAboutManager);
			break;
		default:
			break;
		}

	case WM_DESTROY:

		PostQuitMessage(0);
		return 0;
	}


	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}


LRESULT CALLBACK winAboutManager(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	HWND		hwndOwner;
	RECT		rc, rcDlg, rcOwner;

	switch (iMsg) {
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hWnd)) == NULL)
		{
			hwndOwner = GetDesktopWindow();
		}
		/*This gets the data from the original window so that is possible */
		/*to calculate the central coordinate and centrally align the DialogBox*/
		GetWindowRect(hwndOwner, &rcOwner);
		GetWindowRect(hWnd, &rcDlg);
		CopyRect(&rc, &rcOwner);
		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
		OffsetRect(&rc, -rc.left, -rc.top);
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

		SetWindowPos(hWnd,
			HWND_TOP,
			rcOwner.left + (rc.right / 2),
			rcOwner.top + (rc.bottom / 2),
			0, 0,          // Ignores size arguments. 
			SWP_NOSIZE);

		if (GetDlgCtrlID((HWND)wParam) != IDD_DIALOG1)
		{
			SetFocus(GetDlgItem(hWnd, IDD_DIALOG1));
			return FALSE;
		}
		return TRUE;

	case WM_COMMAND:
		switch (wParam)	{
		case IDCLOSE:
			//DestroyWindow(hWnd);
			EndDialog(hWnd, 0);
			return TRUE;
		}
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	}
	return FALSE;

}

LRESULT CALLBACK winCloseManager(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	HWND		hwndOwner;
	RECT		rc, rcDlg, rcOwner;

	switch (iMsg) {
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hWnd)) == NULL)
		{
			hwndOwner = GetDesktopWindow();
		}
		/*This gets the data from the original window so that is possible */
		/*to calculate the central coordinate and centrally align the DialogBox*/
		GetWindowRect(hwndOwner, &rcOwner);
		GetWindowRect(hWnd, &rcDlg);
		CopyRect(&rc, &rcOwner);
		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
		OffsetRect(&rc, -rc.left, -rc.top);
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

		SetWindowPos(hWnd,
			HWND_TOP,
			rcOwner.left + (rc.right / 2),
			rcOwner.top + (rc.bottom / 2),
			0, 0,          // Ignores size arguments. 
			SWP_NOSIZE);

		if (GetDlgCtrlID((HWND)wParam) != IDD_DIALOG1)
		{
			SetFocus(GetDlgItem(hWnd, IDD_DIALOG1));
			return FALSE;
		}
		return TRUE;

	case WM_COMMAND:
		switch (wParam) {
		case IDCLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
		}
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	}
	return FALSE;

}
