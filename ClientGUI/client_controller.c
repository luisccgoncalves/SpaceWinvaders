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