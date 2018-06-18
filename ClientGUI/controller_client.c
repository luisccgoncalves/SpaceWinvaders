#include "controller_client.h"

ThreadCtrl		cThread;
Packet			token;

int startClient(){

	//Start Client thread HANDLE/ID (uses cThread as parameter)
	HANDLE			htStartClient;								//Handle to the Start Client thread
	DWORD			tStartClientID;								//Stores the ID of the Start server thread;

	htStartClient = CreateThread(
		NULL,													//Thread security attributes
		0,														//Stack size (0 for default)
		LaunchClient,											//Thread function name
		(LPVOID)&cThread,										//Thread parameter struct
		0,														//Creation flags
		&tStartClientID);										//gets thread ID 
	if (htStartClient == NULL) {
		_tprintf(TEXT("[Error] Creating thread Server (%d) at Server\n"), GetLastError());
		return 0;
	}
	return 1;
}

int LogPlayer(HWND hDlg, BOOL remote) {
	TCHAR username[SMALL_BUFF];

	TCHAR userlogin[SMALL_BUFF];
	TCHAR password[SMALL_BUFF];
	TCHAR domain[SMALL_BUFF];

	UINT result = 0;

	GetDlgItemText(hDlg, IDC_LOGIN_USER, username, SMALL_BUFF);
	token = handShakeServer(&cThread,username);
	_tcscpy(&cThread.username, username);

	if (remote == TRUE) {
		cThread.remoteLogin = 1;
		GetDlgItemText(hDlg, IDC_LOGIN_ULOGIN, userlogin, SMALL_BUFF);
		_tcscpy(&cThread.userlogin, userlogin);

		GetDlgItemText(hDlg, IDC_LOGIN_IP, domain, SMALL_BUFF);
		_tcscpy(&cThread.domain, domain);

		GetDlgItemText(hDlg, IDC_LOGIN_PASS, password, SMALL_BUFF);
		_tcscpy(&cThread.password, password);
	}

	startClient();

	return 0;

}


int loadTop10(HWND hDlg) {
	//Fills in timestamps
	for (int i = IDC_TIME1; i < (IDC_TIME1+10); i++)
		SetDlgItemText(hDlg, i, TEXT("placeholder"));

	//Fills in scores
	for (int i = IDC_SCORE1; i < (IDC_SCORE1 + 10); i++)
		SetDlgItemText(hDlg, i, TEXT("placeholder"));

	return 0;
}

int validateLoginValues(HWND hDlg, BOOL remote) {

	TCHAR username[SMALL_BUFF];

	TCHAR userlogin[SMALL_BUFF];
	TCHAR password[SMALL_BUFF];
	TCHAR domain[SMALL_BUFF];

	UINT result = 0;

	if(GetDlgItemText(hDlg, IDC_LOGIN_USER, username, SMALL_BUFF)==0)
		return 1;	
	else if (remote == TRUE) {
		if(GetDlgItemText(hDlg, IDC_LOGIN_ULOGIN, userlogin, SMALL_BUFF)==0)
			return 2;
		if (GetDlgItemText(hDlg, IDC_LOGIN_IP, domain, SMALL_BUFF) == 0)
			return 3;
		GetDlgItemText(hDlg, IDC_LOGIN_PASS, password, SMALL_BUFF);
	}

	return 0;
}

int centerDialogWnd(HWND hDlg) {

	HWND		hwndOwner;
	RECT		rc, rcDlg, rcOwner;

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
	return 1;
}