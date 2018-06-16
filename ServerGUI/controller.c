#include "controller.h"

setCreateGameDlgValues(HWND hDlg) {
	//get define values and set them here
	SetDlgItemText(hDlg, IDC_EDIT1, TEXT("1"));
	SetDlgItemText(hDlg, IDC_EDIT2, TEXT("2"));
	SetDlgItemText(hDlg, IDC_EDIT3, TEXT("3"));
	SetDlgItemText(hDlg, IDC_EDIT4, TEXT("4"));
	SetDlgItemText(hDlg, IDC_EDIT5, TEXT("5"));
	SetDlgItemText(hDlg, IDC_EDIT6, TEXT("6"));
	SetDlgItemText(hDlg, IDC_EDIT7, TEXT("7"));
	SetDlgItemText(hDlg, IDC_EDIT8, TEXT("8"));
	SetDlgItemText(hDlg, IDC_EDIT9, TEXT("9"));
	return 1;
}

