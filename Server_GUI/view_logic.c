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

	switch (iMsg) { 

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_SETTINGS_CONFIGUREGAME:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hWnd, winGameSettings);
			//openBox?
			break;
		case ID_SETTINGS_CLOSESERVER:
			CloseServerMessageBox(hWnd);
			//DestroyWindow(hWnd);
			break;
		case ID_SETTINGS_ABOUTE:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, winAboutManager);
			break;
		default:
			return DefWindowProc(hWnd, iMsg, wParam, lParam);
		}

	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 100, 100, TEXT("Éló üórledê!"), 13);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
}

/* For cancel button*/
void onCancel(HWND hDlg)
{
	SendMessage(hDlg, WM_CLOSE, 0, 0);
}

int CloseServerMessageBox(HWND hWnd)
{

	int msgboxID = MessageBox(
		NULL,
		L"Do you realy want to shutdown the server?",
		L"Shutdown",
		MB_ICONEXCLAMATION | MB_YESNO
	);

	if (msgboxID == IDYES)
	{
		//DestroyWindow(hWnd);
		PostQuitMessage(0);
		return 0;
	}
	return msgboxID;
}


LRESULT CALLBACK winAboutManager(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	HWND		hwndOwner;
	RECT		rc, rcDlg, rcOwner;

	switch (iMsg) {
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hDlg)) == NULL)
		{
			hwndOwner = GetDesktopWindow();
		}
		/*This gets the data from the original window so that is possible */
		/*to calculate the central coordinate and centrally align the DialogBox*/
		GetWindowRect(hwndOwner, &rcOwner);
		GetWindowRect(hDlg, &rcDlg);
		CopyRect(&rc, &rcOwner);
		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
		OffsetRect(&rc, -rc.left, -rc.top);
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

		SetWindowPos(hDlg,
			HWND_TOP,
			rcOwner.left + (rc.right / 2),
			rcOwner.top + (rc.bottom / 2),
			0, 0,          // Ignores size arguments. 
			SWP_NOSIZE);

		if (GetDlgCtrlID((HWND)wParam) != IDD_DIALOG1)
		{
			SetFocus(GetDlgItem(hDlg, IDD_DIALOG1));
			return FALSE;
		}
		return TRUE;

	case WM_COMMAND:
		switch (wParam)	{
		case IDCLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;
		}
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
	}
	return FALSE;

}

LRESULT CALLBACK winGameSettings(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	HWND		hwndOwner;
	RECT		rc, rcDlg, rcOwner;

	HWND sliderPlayers = GetDlgItem(hDlg, IDC_PLAYERS);
	HWND labelPlayers = GetDlgItem(hDlg, IDPLAYERLABEL);

	switch (iMsg) {
	case WM_INITDIALOG:
	{
		//if ((hwndOwner = GetParent(hDlg)) == NULL)
		//{
		//	hwndOwner = GetDesktopWindow();
		//}
		///*This gets the data from the original window so that is possible */
		///*to calculate the central coordinate and centrally align the DialogBox*/
		//GetWindowRect(hwndOwner, &rcOwner);
		//GetWindowRect(hDlg, &rcDlg);
		//CopyRect(&rc, &rcOwner);
		//OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
		//OffsetRect(&rc, -rc.left, -rc.top);
		//OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

		//SetWindowPos(hDlg,
		//	HWND_TOP,
		//	rcOwner.left + (rc.right / 2),
		//	rcOwner.top + (rc.bottom / 2),
		//	0, 0,          // Ignores size arguments. 
		//	SWP_NOSIZE);

		//if (GetDlgCtrlID((HWND)wParam) != IDD_FORMVIEW)
		//{
		//	SetFocus(GetDlgItem(hDlg, IDD_FORMVIEW));
		//	return FALSE;
		//}
				
		SendMessage(sliderPlayers, TBM_SETRANGE, (WPARAM)1, (LPARAM)MAKELONG(1, 10));
		SendMessage(sliderPlayers, TBM_SETPOS, (WPARAM)1,5);
		SendMessage(sliderPlayers, TBM_SETTICFREQ, 1, 0);
		SendMessage(sliderPlayers, TBM_SETBUDDY, TRUE, (LPARAM)labelPlayers);

		return TRUE;
	}
	case WM_HSCROLL:
	{
		LRESULT pos = SendMessage(sliderPlayers, TBM_GETPOS, 0, 0);
		wchar_t buf[4];
		wsprintfW(buf, L"%ld", pos);

		SetWindowText(hDlg, buf);
		break;
	}
	break;
	case WM_MOUSE:
		switch (wParam) {
		case IDCLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;
		}
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
	}
	return FALSE;
}


