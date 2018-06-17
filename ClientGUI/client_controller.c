#include "client_controller.h"

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