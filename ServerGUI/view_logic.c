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
	wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
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
		case ID_SETTINGS_CREATEGAME:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hWnd, winGameCreateDlg);
			break;
		case ID_SETTINGS_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, winAboutDlg);
			break;
		case ID_SETTINGS_CLOSESERVER:
			SendMessage(hWnd, WM_CLOSE,wParam,lParam);
			break;
		default:
			return DefWindowProc(hWnd, iMsg, wParam, lParam);
		}

	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc, 100, 100, TEXT("Luís & Simão!"), 13);
		//Here get logged clients
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("Do you realy want to shutdown the server?"), TEXT("Server Shutdown"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, iMsg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK winAboutDlg(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	switch (iMsg) {
	case WM_INITDIALOG:
		centerDialogWnd(hDlg);
		
		//set focus
		if (GetDlgCtrlID((HWND)wParam) != IDD_DIALOG1)
		{
			SetFocus(GetDlgItem(hDlg, IDD_DIALOG1));
			return FALSE;
		}
		return TRUE;

	case WM_COMMAND:
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

LRESULT CALLBACK winGameCreateDlg(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	switch (iMsg) {
	case WM_INITDIALOG:
	{
		setCreateGameDlgValues(hDlg);

		centerDialogWnd(hDlg);
		//set focus
		if (GetDlgCtrlID((HWND)wParam) != IDD_DIALOG2)
		{
			SetFocus(GetDlgItem(hDlg, IDD_DIALOG2));
			return FALSE;
		}


		return TRUE;
	}
	break;

	case WM_COMMAND:
		switch (wParam) {
		case IDOK:
		{
			BOOL fError;
			int numPlayers;

			numPlayers = GetDlgItemInt(hDlg, IDC_EDIT1, &fError, TRUE);
			//chamar o controller com numPlayers
			//get result, and:
			//switch (result) {
			//case 1:
			if(numPlayers==1)
				MessageBox(hDlg, TEXT("Invalid number of players!"), TEXT("Start Game"), MB_OK);
			break;		//closes warning!
		}
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return TRUE;
	}
	return FALSE;
}



