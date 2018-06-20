#include "view_client.h"

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

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, 
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,				//Disabled Resize and Maximize
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 1300, 700, 0);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hDC;
	PAINTSTRUCT Ps;

	switch (message)
	{
	case WM_SIZING:
		break;
	case WM_CREATE:
		LoadBitmaps(hWnd);

		CreateThread(
			NULL,													//Thread security attributes
			0,														//Stack size (0 for default)
			UpdateView,												//Thread function name
			(LPVOID)hWnd,											//Thread parameter struct
			0,														//Creation flags
			NULL);													//gets thread ID 

		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_GAME_CONFIGUREKEYS:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, ConfigureKeys);
			break;
		case ID_GAME_HIGHSCORES:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_TOP10), hWnd, HighScores);
			break;
		case ID_FILE_LOGOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGOUT), hWnd, Logout);
			break;
		case ID_FILE_LOGIN:
			if (!PlayerLogged()) {
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG_LOGIN), hWnd, Login);
			}
			else {
				MessageBox(hWnd, TEXT("You are already logged!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
			}
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case ID_GAME_CONNECTGAME:
		{
			if (PlayerLogged()) {
				if (PlayerIsGameBound()) {
					MessageBox(hWnd, TEXT("You are already game bound!"), TEXT("Connect"), MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				else {
					if (MessageBox(hWnd, TEXT("Do you confirm you want to connect to a game?"), TEXT("Connect"), MB_YESNO) == IDYES)
						ConnectGame();
					break;
				}
			}
			else if (!PlayerLogged()) {
				MessageBox(hWnd, TEXT("Please login first!"), TEXT("Connect"), MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			else 
			break;
		}
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
		/*PAINT*/
		hDC = BeginPaint(hWnd, &Ps);
		paintMap( hDC); 
		EndPaint(hWnd, &Ps);
		break;
	}
	break;
	case WM_KEYDOWN:
			/*if default?*/
			switch (wParam) {
			case VK_UP:
				SendKey(3);
				break;
			case VK_RIGHT:
				SendKey(0);
				break;
			case VK_LEFT:
				SendKey(2);
				break;
			case VK_DOWN:
				SendKey(1);
				break;
			case  VK_SPACE:
				SendKey(4);
				break;
			default:
				break;
			}
		break;
	case WM_CHAR:
			SendChar(wParam);
			/* if not default?*/
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
				MessageBox(hDlg, TEXT("Please fill in the username! 1-20 letters!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				break;
			case 2:
				MessageBox(hDlg, TEXT("Please fill in the host login username!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				break;
			case 3:
				MessageBox(hDlg, TEXT("Please fill in the IP/domain!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				break;
			case 0:
				if(LogPlayer(hDlg, (BOOL)SendDlgItemMessage(hDlg, IDC_LOGIN_REMOTE, BM_GETCHECK, 0, 0)))
					MessageBox(hDlg, TEXT("Login successfull"), TEXT("Message"), MB_OK);

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
		switch (wParam) {
		case IDOK:
		{
			BOOL radioActive = (BOOL)SendDlgItemMessage(hDlg, IDC_RADIO_DEFAULT, BM_GETCHECK, 0, 0);
			int result = validateConfigurableKeys(hDlg, radioActive);
			switch (result) {
			case 0:
				//MessageBox(hDlg, TEXT("allright!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				recordKeys(hDlg, radioActive);
				break;
			case 1:
				MessageBox(hDlg, TEXT("Please fill the fields with one character or select default!"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
				break;
			default:
				break;
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
