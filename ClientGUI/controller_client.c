#include "controller_client.h"

ThreadCtrl		cThread;
GameKeys		keys;
HBITMAP			bmpSprites[BMP_NUM];

//HDC MemDCExercising;
//HBITMAP bmpExercising;

//Packet			token;
//int		packetUpd = 0;

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
	_tcscpy_s(cThread.username, SMALL_BUFF, username);

	if (remote == TRUE) {
		cThread.remoteLogin = 1;
		GetDlgItemText(hDlg, IDC_LOGIN_ULOGIN, userlogin, SMALL_BUFF);
		_tcscpy_s(cThread.userlogin, SMALL_BUFF, userlogin);

		GetDlgItemText(hDlg, IDC_LOGIN_IP, domain, SMALL_BUFF);
		_tcscpy_s(cThread.domain, SMALL_BUFF, domain);

		if (GetDlgItemText(hDlg, IDC_LOGIN_PASS, password, SMALL_BUFF) == 0)
			_tcscpy_s(password, SMALL_BUFF, TEXT("\0"));
		else
			_tcscpy_s(cThread.password, SMALL_BUFF, password);
	}

	startClient();
	
	return 1;

}


int loadTop10(HWND hDlg) {

	//cThread.localGame.top10[0].timestamp
	//Fills in timestamps
	int j = 0;
	TCHAR	tScores[10][6];

	for(int i=0;i<10;i++)
		_stprintf_s(tScores[i], _countof(tScores[i]), TEXT("%05d"), cThread.localGame.top10[i].score);

	for (int i = IDC_TIME1; i < (IDC_TIME1+10); i++)
		SetDlgItemText(hDlg, i, cThread.localGame.top10[j++].timestamp);

	j = 0;

	//Fills in scores
	for (int i = IDC_SCORE1; i < (IDC_SCORE1 + 10); i++)
		SetDlgItemText(hDlg, i, tScores[j++]);

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
		if(GetDlgItemText(hDlg, IDC_LOGIN_PASS, password, SMALL_BUFF)==0)
			_tcscpy_s(password, SMALL_BUFF, TEXT("\0"));
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

int ConnectGame() {
	markPlayerReady(&cThread);
	CreateThread(
		NULL,													//Thread security attributes
		0,														//Stack size (0 for default)
		SendKeyPacket,												//Thread function name
		(LPVOID)&cThread,										//Thread parameter struct
		0,														//Creation flags
		NULL);													//gets thread ID 
	return 1;
}

int PlayerIsGameBound() {
	if (cThread.token.owner >= 0 && cThread.token.owner <= MAX_PLAYERS && cThread.localGame.gameRunning) {
		return 1;
	}
	return 0;
}

int PlayerLogged() {
	if (cThread.logged) {
		return 1;
	}
	return 0;
}

DWORD WINAPI UpdateView(LPVOID tParam) {

	rectEvent = CreateEvent(				//Creates the event to warn client about window refresh
		NULL,										//Event attributes
		FALSE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		NULL);										//Event name
	if (rectEvent == NULL) {
		_tprintf(TEXT("[Error] Event 1st pipe instance (%d)\n"), GetLastError());
		return -1;
	}

	cThread.ThreadMustGoOn = 1;

	while (cThread.ThreadMustGoOn) {

		InvalidateRect(tParam, NULL, TRUE);

		WaitForSingleObject(rectEvent, INFINITE);
	}
	return 1;
}
int LoadBitmaps(hWnd) {
	//bmpExercising[0] = LoadBitmap(GetModuleHandle(hWnd), MAKEINTRESOURCE(IDB_BITMAP1));
	return 1;
}

int paintMap(HDC hDC) {
	HDC DCSprites;
	TCHAR	tLives[3];// = { TEXT("0") };
	TCHAR	tScore[SMALL_BUFF];
	//Draw bounding boxes
	Rectangle(hDC, 0, 0, XSIZE, YSIZE+cThread.localGame.ship[0].height);
	Rectangle(hDC, 0, YSIZE+cThread.localGame.ship[0].height, XSIZE, YSIZE+100+ cThread.localGame.ship[0].height);

	bmpSprites[0] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP1)); //invader
	bmpSprites[1] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP2));	//ice Power Up
	bmpSprites[2] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP3)); // laser
	bmpSprites[3] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP4)); // turbo Power Up
	bmpSprites[4] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP5)); // drunk
	bmpSprites[5] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP6)); // shield
	bmpSprites[6] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP7)); // other invader
	bmpSprites[7] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP8)); // bombs
	bmpSprites[8] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP9)); // shots
	bmpSprites[9] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP10)); // defenders
	bmpSprites[10] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP11)); // +speed
	bmpSprites[11] = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP12)); // battery

	DCSprites = CreateCompatibleDC(hDC);

	int i, j;

	if (cThread.localGame.gameRunning==1) {

		for (i = 0; i < cThread.localGame.max_invaders; i++) {
			if (cThread.localGame.invad[i].hp>0) {
				if (cThread.localGame.invad[i].rand_path) {
					SelectObject(DCSprites, bmpSprites[0]);
					BitBlt(hDC,
						cThread.localGame.invad[i].x,
						cThread.localGame.invad[i].y,
						cThread.localGame.invad[i].width,
						cThread.localGame.invad[i].height,
						DCSprites,
						0,
						0,
						SRCCOPY);
					//Rectangle(hDC,							//rand invader bitmap
					//	cThread.localGame.invad[i].x,
					//	cThread.localGame.invad[i].y,
					//	cThread.localGame.invad[i].x + cThread.localGame.invad[i].width,
					//	cThread.localGame.invad[i].y + cThread.localGame.invad[i].height);
				}
				else
					SelectObject(DCSprites, bmpSprites[6]);
					BitBlt(hDC,
					cThread.localGame.invad[i].x,
					cThread.localGame.invad[i].y,
					cThread.localGame.invad[i].width,
					cThread.localGame.invad[i].height,
					DCSprites,
					0,
					0,
					SRCCOPY);

			}
			for (j = 0; j < MAX_BOMBS; j++) {
				if (cThread.localGame.invad[i].bomb[j].fired == 1)
				SelectObject(DCSprites, bmpSprites[7]);
				BitBlt(hDC,
					cThread.localGame.invad[i].bomb[j].x,
					cThread.localGame.invad[i].bomb[j].y,
					cThread.localGame.invad[i].bomb[j].width,
					cThread.localGame.invad[i].bomb[j].height,
					DCSprites,
					0,
					0,
					SRCCOPY);
			}
		}
		for (i = 0; i < cThread.localGame.num_players; i++) {
			if (cThread.localGame.ship[i].lives >= 0) {
				SelectObject(DCSprites, bmpSprites[9]);
				BitBlt(hDC,
					cThread.localGame.ship[i].x,
					cThread.localGame.ship[i].y,
					cThread.localGame.ship[i].width,
					cThread.localGame.ship[i].height,
					DCSprites,
					0,
					0,
					SRCCOPY);
						for (j = 0; j < MAX_SHOTS; j++) {
							if (cThread.localGame.ship[i].shots[j].fired == 1)
							SelectObject(DCSprites, bmpSprites[8]);
							BitBlt(hDC,
								cThread.localGame.ship[i].shots[j].x,
								cThread.localGame.ship[i].shots[j].y,
								cThread.localGame.ship[i].shots[j].width,
								cThread.localGame.ship[i].shots[j].height,
								DCSprites,
								0,
								0,
								SRCCOPY);
						}
			}
		}

		TextOut(hDC, 1, 616, TEXT("LIVES: "), 8);
		_stprintf_s(tLives, _countof(tLives), TEXT("%02d"), cThread.localGame.ship[cThread.token.owner].lives);
		TextOut(hDC, 55, 616, tLives, _tcslen(tLives));

		TextOut(hDC, 1, 637, TEXT("SCORE: "), 8);
		_stprintf_s(tScore, _countof(tScore), TEXT("%05d"), cThread.localGame.score);
		TextOut(hDC, 55, 637, tScore, _tcslen(tScore));

		if (cThread.localGame.pUp.fired == 1) {
				if (cThread.localGame.pUp.type == 4) { //ice
					SelectObject(DCSprites, bmpSprites[1]);
					BitBlt(hDC,
						cThread.localGame.pUp.x,
						cThread.localGame.pUp.y,
						cThread.localGame.pUp.width,
						cThread.localGame.pUp.height,
						DCSprites,
						0,
						0,
						SRCCOPY);
				}
				else if (cThread.localGame.pUp.type == 2) { //turbo
					SelectObject(DCSprites, bmpSprites[3]);
					BitBlt(hDC,
						cThread.localGame.pUp.x,
						cThread.localGame.pUp.y,
						cThread.localGame.pUp.width,
						cThread.localGame.pUp.height,
						DCSprites,
						0,
						0,
						SRCCOPY);
				}
				else if (cThread.localGame.pUp.type == 1) { //drunk
					SelectObject(DCSprites, bmpSprites[4]);
					BitBlt(hDC,
						cThread.localGame.pUp.x,
						cThread.localGame.pUp.y,
						cThread.localGame.pUp.width,
						cThread.localGame.pUp.height,
						DCSprites,
						0,
						0,
						SRCCOPY);
				}
				else if (cThread.localGame.pUp.type == 0) {//shield
					SelectObject(DCSprites, bmpSprites[5]);
					BitBlt(hDC,
						cThread.localGame.pUp.x,
						cThread.localGame.pUp.y,
						cThread.localGame.pUp.width,
						cThread.localGame.pUp.height,
						DCSprites,
						0,
						0,
						SRCCOPY);
				}
				else if (cThread.localGame.pUp.type == 3) {//laser
					SelectObject(DCSprites, bmpSprites[2]);
					BitBlt(hDC,
						cThread.localGame.pUp.x,
						cThread.localGame.pUp.y,
						cThread.localGame.pUp.width,
						cThread.localGame.pUp.height,
						DCSprites,
						0,
						0,
						SRCCOPY);
				}
				else if (cThread.localGame.pUp.type == 5) {//+speed
					SelectObject(DCSprites, bmpSprites[10]);
					BitBlt(hDC,
						cThread.localGame.pUp.x,
						cThread.localGame.pUp.y,
						cThread.localGame.pUp.width,
						cThread.localGame.pUp.height,
						DCSprites,
						0,
						0,
						SRCCOPY);
				}
				else if (cThread.localGame.pUp.type == 6) {//battery
					SelectObject(DCSprites, bmpSprites[11]);
					BitBlt(hDC,
						cThread.localGame.pUp.x,
						cThread.localGame.pUp.y,
						cThread.localGame.pUp.width,
						cThread.localGame.pUp.height,
						DCSprites,
						0,
						0,
						SRCCOPY);
				}
		}
		DeleteDC(DCSprites);
		for (i = 0; i < BMP_NUM; i++) {
			DeleteObject(bmpSprites[i]);
			/*maybe put this below eevery usage? */
		}
		return 1;

	}/*
	 
	 
	 if (cThread.localGame.gameRunning) {

		for (i = 0; i < cThread.localGame.max_invaders; i++) {
			if (cThread.localGame.invad[i].hp>0) {
				if (cThread.localGame.invad[i].rand_path) {
					SelectObject(MemDCExercising, bmpExercising[0]);
					BitBlt(hDC,
						cThread.localGame.invad[i].x,
						cThread.localGame.invad[i].y,
						cThread.localGame.invad[i].width,
						cThread.localGame.invad[i].height,
						MemDCExercising,
						0,
						0,
						SRCCOPY);
					//Rectangle(hDC,							//rand invader bitmap
					//	cThread.localGame.invad[i].x,
					//	cThread.localGame.invad[i].y,
					//	cThread.localGame.invad[i].x + cThread.localGame.invad[i].width,
					//	cThread.localGame.invad[i].y + cThread.localGame.invad[i].height);
				}
				else
					SelectObject(MemDCExercising, bmpExercising[6]);
					BitBlt(hDC,
					cThread.localGame.invad[i].x,
					cThread.localGame.invad[i].y,
					cThread.localGame.invad[i].width,
					cThread.localGame.invad[i].height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
					//Rectangle(hDC,							 //Regular invader bitmap
					//	cThread.localGame.invad[i].x,
					//	cThread.localGame.invad[i].y,
					//	cThread.localGame.invad[i].x + cThread.localGame.invad[i].width,
					//	cThread.localGame.invad[i].y + cThread.localGame.invad[i].height);

			}
			for (j = 0; j < MAX_SHOTS; j++) {
				if (cThread.localGame.invad[i].bomb[j].fired == 1)
					SelectObject(MemDCExercising, bmpExercising[7]);
					BitBlt(hDC,
					cThread.localGame.invad[i].bomb[j].x,
					cThread.localGame.invad[i].bomb[j].y,
					cThread.localGame.invad[i].bomb[j].width,
					cThread.localGame.invad[i].bomb[j].height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
					//Rectangle(hDC,
					//	cThread.localGame.invad[i].bomb[j].x,
					//	cThread.localGame.invad[i].bomb[j].y,
					//	cThread.localGame.invad[i].bomb[j].x + cThread.localGame.invad[i].bomb[j].width,
					//	cThread.localGame.invad[i].bomb[j].y + cThread.localGame.invad[i].bomb[j].height);
			}
		}
		for (i = 0; i < cThread.localGame.num_players; i++) {
			if (cThread.localGame.ship[i].lives >= 0) {
				SelectObject(MemDCExercising, bmpExercising[9]);
				BitBlt(hDC,
					cThread.localGame.ship[i].x,
					cThread.localGame.ship[i].y,
					cThread.localGame.ship[i].width,
					cThread.localGame.ship[i].height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
					//Rectangle(hDC,							//ship bitmap
					//	cThread.localGame.ship[i].x,
					//	cThread.localGame.ship[i].y,
					//	cThread.localGame.ship[i].x + cThread.localGame.ship[i].width,
					//	cThread.localGame.ship[i].y + cThread.localGame.ship[i].height);
					if(cThread.localGame.ship[i].shots[j].fired == 1)
						for (j = 0; j < MAX_BOMBS; j++) {
							SelectObject(MemDCExercising, bmpExercising[8]);
							BitBlt(hDC,
								cThread.localGame.ship[i].shots[j].x,
								cThread.localGame.ship[i].shots[j].y,
								cThread.localGame.ship[i].shots[j].width,
								cThread.localGame.ship[i].shots[j].height,
								MemDCExercising,
								0,
								0,
								SRCCOPY);
							//Rectangle(hDC,
							//	cThread.localGame.ship[i].shots[j].x,
							//	cThread.localGame.ship[i].shots[j].y,
							//	cThread.localGame.ship[i].shots[j].x + cThread.localGame.ship[i].shots[j].width,
							//	cThread.localGame.ship[i].shots[j].y + cThread.localGame.ship[i].shots[j].height);
						}
					TextOut(hDC, 1 + (70 * i), 620, TEXT("LIVES: "), 8);
					TextOut(hDC, 45 + (70 * i), 620, TEXT("2"), 1);
			}
		}
		if (cThread.localGame.pUp.fired == 1) {
			if (cThread.localGame.pUp.type == 4) { //ice
				SelectObject(MemDCExercising, bmpExercising[1]);
				BitBlt(hDC,
					cThread.localGame.pUp.x,
					cThread.localGame.pUp.y,
					cThread.localGame.pUp.width,
					cThread.localGame.pUp.height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
			}
			else if(cThread.localGame.pUp.type == 2) { //turbo
				SelectObject(MemDCExercising, bmpExercising[3]);
				BitBlt(hDC,
					cThread.localGame.pUp.x,
					cThread.localGame.pUp.y,
					cThread.localGame.pUp.width,
					cThread.localGame.pUp.height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
			}
			else if(cThread.localGame.pUp.type == 1){ //drunk
				SelectObject(MemDCExercising, bmpExercising[4]);
				BitBlt(hDC,
					cThread.localGame.pUp.x,
					cThread.localGame.pUp.y,
					cThread.localGame.pUp.width,
					cThread.localGame.pUp.height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
			}
			else if(cThread.localGame.pUp.type == 0) {//shield
				SelectObject(MemDCExercising, bmpExercising[5]);
				BitBlt(hDC,
					cThread.localGame.pUp.x,
					cThread.localGame.pUp.y,
					cThread.localGame.pUp.width,
					cThread.localGame.pUp.height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
			}			
			else if(cThread.localGame.pUp.type == 3){//laser
				SelectObject(MemDCExercising, bmpExercising[2]);
				BitBlt(hDC,
					cThread.localGame.pUp.x,
					cThread.localGame.pUp.y,
					cThread.localGame.pUp.width,
					cThread.localGame.pUp.height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
			}			
			else if(cThread.localGame.pUp.type == 5){//+speed
				SelectObject(MemDCExercising, bmpExercising[10]);
				BitBlt(hDC,
					cThread.localGame.pUp.x,
					cThread.localGame.pUp.y,
					cThread.localGame.pUp.width,
					cThread.localGame.pUp.height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
			}
			else if(cThread.localGame.pUp.type == 6){//battery
				SelectObject(MemDCExercising, bmpExercising[11]);
				BitBlt(hDC,
					cThread.localGame.pUp.x,
					cThread.localGame.pUp.y,
					cThread.localGame.pUp.width,
					cThread.localGame.pUp.height,
					MemDCExercising,
					0,
					0,
					SRCCOPY);
			}

		}
		DeleteDC(MemDCExercising);
		for (i = 0; i < BMP_NUM; i++) {
			DeleteObject(bmpExercising[i]);
		}
		return 1;

	}
	 
	 
	 */


	return 0;
}

int SendKey(int num) {
	switch (num) {
	case 3:
		cThread.token.instruction = 3;
		cThread.packetUpd = 1;
		break;
	case 1:
		cThread.token.instruction = 1;
		cThread.packetUpd = 1;
		break;
	case 2:
		cThread.token.instruction = 2;
		cThread.packetUpd = 1;
		break;
	case 0:
		cThread.token.instruction = 0;
		cThread.packetUpd = 1;
		break;
	case 4:
		cThread.token.instruction = 4;
		cThread.packetUpd = 1;
		break;
	//case 27://esc
	//		//deauth packet?
	//	cThread.token.instruction = 0;
	//	//packetUpd = 1;
	//	break;
	//case 32://space
	//		//PlaySound(TEXT("shoot.wav"), NULL, SND_ASYNC | SND_FILENAME);		//needs better aproach
	//	localpacket.instruction = 4;
	//	packetUpd = 1;
	//	break;
	//case 224://extended
	//	_gettch();//ignore extended
	//	break;
	default:
		break;
	}
	return 1;
}

int SendChar(WPARAM wParam) {
	//TCHAR up, down, right, left, space;

	/*delete after recordKeys is working */
	//up = 'w';
	//down = 's';
	//right = 'd';
	//left = 'a';
	//space = ' ';

	if (wParam == keys.up) {
		cThread.token.instruction = 3;
		cThread.packetUpd = 1;
	}
	else if (wParam == keys.down) {
		cThread.token.instruction = 1;
		cThread.packetUpd = 1;
	}
	else if (wParam == keys.left) {
		cThread.token.instruction = 2;
		cThread.packetUpd = 1;
	}
	else if (wParam == keys.right) {
		cThread.token.instruction = 0;
		cThread.packetUpd = 1;
	}
	else if(wParam == keys.fire){
		cThread.token.instruction = 4;
		cThread.packetUpd = 1;
	}
	return 1;
}

int validateConfigurableKeys(HWND hDlg, BOOL defaultKey) {
	TCHAR up[3], down[3], right[3], left[3], fire[3];
	int lUp, lDown, lRight, lLeft, lFire;

	lUp = GetWindowTextLength(GetDlgItem(hDlg, IDC_UP));
	lDown = GetWindowTextLength(GetDlgItem(hDlg, IDC_DOWN));
	lRight = GetWindowTextLength(GetDlgItem(hDlg, IDC_RIGHT));
	lLeft = GetWindowTextLength(GetDlgItem(hDlg, IDC_LEFT));
	lFire = GetWindowTextLength(GetDlgItem(hDlg, IDC_FIRE));

	if (defaultKey == TRUE) {
		return 0;
	}
	else{
		if (GetDlgItemText(hDlg, IDC_UP, up, SMALL_BUFF) == 0)
			return 1;
		else if (GetDlgItemText(hDlg, IDC_DOWN, down, SMALL_BUFF) == 0)
			return 1;
		else if (GetDlgItemText(hDlg, IDC_RIGHT, right, SMALL_BUFF) == 0)
			return 1;
		else if (GetDlgItemText(hDlg, IDC_LEFT, left, SMALL_BUFF) == 0)
			return 1;
		else if (GetDlgItemText(hDlg, IDC_FIRE, fire, SMALL_BUFF) == 0)
			return 1;
		else if ( lUp > 1 || lDown > 1 || lRight > 1 || lLeft > 1 || lFire> 1)
			return 1;
	}

	return 0;
}
int recordKeys(HWND hDlg, BOOL defaultKeys) {
	TCHAR up[3], down[3], right[3], left[3], fire[3];
	if (!defaultKeys) {
		GetDlgItemText(hDlg, IDC_UP, up, SMALL_BUFF);
		GetDlgItemText(hDlg, IDC_DOWN, down, SMALL_BUFF);
		GetDlgItemText(hDlg, IDC_RIGHT, right, SMALL_BUFF);
		GetDlgItemText(hDlg, IDC_LEFT, left, SMALL_BUFF);
		GetDlgItemText(hDlg, IDC_FIRE, fire, SMALL_BUFF);

		keys.up = up[0];
		keys.down = down[0];
		keys.right = right[0];
		keys.left = left[0];
		keys.fire = fire[0];
	}
	else {
		//?
	}

	return 0;
}
