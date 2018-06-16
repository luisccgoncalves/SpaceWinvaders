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

centerDialogWnd(HWND hDlg) {

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
