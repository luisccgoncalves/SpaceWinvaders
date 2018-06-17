#include "controller.h"


SMCtrl		cThread;

int startServer() {

	LoadGDataDefaults(&cThread.localGameData);

	//Start server thread HANDLE/ID (uses cThread as parameter)
	HANDLE			htStartServer;								//Handle to the Start server thread
	DWORD			tStartServerID;								//Stores the ID of the Start server thread;

	htStartServer = CreateThread(
		NULL,													//Thread security attributes
		0,														//Stack size (0 for default)
		LaunchServer,											//Thread function name
		(LPVOID)&cThread,										//Thread parameter struct
		0,														//Creation flags
		&tStartServerID);										//gets thread ID 
	if (htStartServer == NULL) {
		_tprintf(TEXT("[Error] Creating thread Server (%d) at Server\n"), GetLastError());
		return 0;
	}
	return 1;
}

int startGameProcedure() {

	StartLobby(&cThread);

	return 0;
}

int setCreateGameDlgValues(HWND hDlg) {
	GameData game = cThread.localGameData;

	SetDlgItemInt(hDlg, IDC_EDIT1, game.num_players, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT2, game.max_invaders, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT3, game.max_rand_invaders, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT4, game.invaders_speed, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT5, game.projectiles_speed, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT6, game.bombRate, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT7, game.shotRate, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT8, game.moveRate, TRUE);
	SetDlgItemInt(hDlg, IDC_EDIT9, game.pup_duration/1000, TRUE);
	return 1;
}

int validateCreateGameDlgValues(HWND hDlg) {
	BOOL fError;
	int players, maxInvaders, hardInvaders;
	int invadersSpeed, projectileSpeed, pUpDuration;
	int bombRate, shotRate, moveRate;

	players = GetDlgItemInt(hDlg, IDC_EDIT1, &fError, TRUE);
	maxInvaders = GetDlgItemInt(hDlg, IDC_EDIT2, &fError, TRUE);
	hardInvaders = GetDlgItemInt(hDlg, IDC_EDIT3, &fError, TRUE);
	invadersSpeed = GetDlgItemInt(hDlg, IDC_EDIT4, &fError, TRUE);
	projectileSpeed = GetDlgItemInt(hDlg, IDC_EDIT5, &fError, TRUE);
	bombRate = GetDlgItemInt(hDlg, IDC_EDIT6, &fError, TRUE);
	shotRate = GetDlgItemInt(hDlg, IDC_EDIT7, &fError, TRUE);
	moveRate = GetDlgItemInt(hDlg, IDC_EDIT8, &fError, TRUE);
	pUpDuration = GetDlgItemInt(hDlg, IDC_EDIT9, &fError, TRUE);

	if (players < 1 && players >10)
		return 1;
	else if (maxInvaders < 5 || maxInvaders >20)
		return 2;
	else if (hardInvaders < 0 || hardInvaders >10)
		return 3;
	else if (hardInvaders > maxInvaders)
		return 10;
	else if (invadersSpeed < 200 || invadersSpeed >2000)
		return 4;
	else if (projectileSpeed < 200 || projectileSpeed >2000)
		return 11;
	else if (bombRate < 10 || bombRate >50)
		return 6;
	else if (shotRate < 200 || shotRate >1000)
		return 7;
	else if (moveRate < 50 || moveRate >200)
		return 8;
	else if (pUpDuration < 5 || pUpDuration >20)
		return 9;
	else {
		return 0;
	}
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

int sendCreateGameValuesToServer(HWND hDlg) {
	BOOL				fError;
	ConfigurableVars	vars;

	vars.players = GetDlgItemInt(hDlg, IDC_EDIT1, &fError, TRUE);
	vars.maxInvaders = GetDlgItemInt(hDlg, IDC_EDIT2, &fError, TRUE);
	vars.hardInvaders = GetDlgItemInt(hDlg, IDC_EDIT3, &fError, TRUE);
	vars.invadersSpeed = GetDlgItemInt(hDlg, IDC_EDIT4, &fError, TRUE);
	vars.projectileSpeed = GetDlgItemInt(hDlg, IDC_EDIT5, &fError, TRUE);
	vars.bombRate = GetDlgItemInt(hDlg, IDC_EDIT6, &fError, TRUE);
	vars.shotRate = GetDlgItemInt(hDlg, IDC_EDIT7, &fError, TRUE);
	vars.moveRate = GetDlgItemInt(hDlg, IDC_EDIT8, &fError, TRUE);
	vars.pUpDuration = 1000*GetDlgItemInt(hDlg, IDC_EDIT9, &fError, TRUE);

	if(updateConfigGameValues(vars, &cThread.localGameData, cThread.mhStructSync))
		return 1;
	return 0;
}

int getPlayersReady() {

	int i;

	for (i = 0; i < cThread.localGameData.num_players; i++) {
		if (cThread.localGameData.logged[i].isReady==1)
			return 1;
	}
	
	return 0;
}


