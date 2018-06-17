#include "view_client.h"
#include "resource.h"

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_INVADER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENTGUI);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON_INVADER));

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_GAME_CONFIGUREKEYS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_KCONFIG), hWnd, ConfigureKeys);
			break;
		case ID_GAME_HIGHSCORES:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_TOP10), hWnd, HighScores);
			break;
		case ID_FILE_LOGOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGOUT), hWnd, Logout);
			break;
		case ID_FILE_LOGIN:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hWnd, Login);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			//Replicates WM_CLOSE
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CLOSE:
		if (MessageBox(hWnd, TEXT("Are you sure?"), TEXT("Quit"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		centerDialogWnd(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for login window.
INT_PTR CALLBACK Login(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		centerDialogWnd(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			switch (validateLoginValues(hDlg, (BOOL)SendDlgItemMessage(hDlg, IDC_LOGIN_REMOTE, BM_GETCHECK, 0, 0))) {
			case 1:
				MessageBox(hDlg, TEXT("Please fill in the username!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				break;
			case 2:
				MessageBox(hDlg, TEXT("Please fill in the host login username!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				break;
			case 3:
				MessageBox(hDlg, TEXT("Please fill in the IP/domain!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				break;
			case 0:
				MessageBox(hDlg, TEXT("Login successfull"), TEXT("Message"), MB_OK);
				//login procedure()
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
				break;
			}
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Message handler for logout window.
INT_PTR CALLBACK Logout(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		centerDialogWnd(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK HighScores(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
	case WM_INITDIALOG:
		centerDialogWnd(hDlg);
		loadTop10(hDlg);
		
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCLOSE) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
	}

	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ConfigureKeys(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		centerDialogWnd(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
