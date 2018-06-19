#include "data_client.h"

DWORD WINAPI LaunchClient(LPVOID tParam) {

	ThreadCtrl	*cThreadRdGame = (ThreadCtrl*)tParam;

	//HANDLE		htReadGame;					//Game thread
	//DWORD		tReadGameID;				//Game thread ID

	createProdConsEvents(cThreadRdGame);

	//Connect to gateway
	if (StartPipeListener(&cThreadRdGame->hPipe, cThreadRdGame) != 0) {
		_tprintf(TEXT("[Error] launching pipe listener...\n"));
		return -1;
	}

	//Connect to Server (through gateway)
	cThreadRdGame->ThreadMustGoOn = 1;
	cThreadRdGame->token = handShakeServer(cThreadRdGame, cThreadRdGame->username);

	//htReadGame = CreateThread(
	//	NULL,										//Thread security attributes
	//	0,											//Stack size (0 for default)
	//	ReadGame,									//Thread function name
	//	cThreadRdGame,								//Thread parameter struct
	//	0,											//Creation flags
	//	&tReadGameID);								//gets thread ID 

	//if (htReadGame == NULL) {
	//	_tprintf(TEXT("[Error] launching ReadGame thread. (%d)\n"), GetLastError());
	//	return -1;
	//}

	//WaitForSingleObject(htReadGame, INFINITE);
	//CloseHandle(cThreadRdGame->hPipe);

	return 0;
}

//void printGame(GameData msg) {
//
//	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
//	int i, j;
//
//	//cls(hStdout);
//
//	for (i = 0; i < MAX_INVADER; i++) {
//		if (msg.invad[i].hp) {
//			gotoxy(msg.invad[i].x, msg.invad[i].y);
//			if (msg.invad[i].rand_path)
//				_tprintf(TEXT("X"));
//			else
//				_tprintf(TEXT("W"));
//		}
//	}
//
//	for (i = 0; i < MAX_INVADER; i++) {
//		for (j = 0; j < MAX_BOMBS; j++) {
//			if (msg.invad[i].bomb[j].fired) {
//				gotoxy(msg.invad[i].bomb[j].x, msg.invad[i].bomb[j].y);
//				_tprintf(TEXT("o"));
//			}
//		}
//	}
//
//	for (i = 0; i < msg.num_players; i++) {
//		for (j = 0; j < MAX_SHOTS; j++) {
//			if (msg.ship[i].shots[j].fired) {
//				gotoxy(msg.ship[i].shots[j].x, msg.ship[i].shots[j].y);
//				_tprintf(TEXT("|"));
//			}
//		}
//	}
//
//	for (i = 0; i < msg.num_players; i++) {
//		if (msg.ship[i].lives >= 0) {
//			gotoxy(msg.ship[i].x, msg.ship[i].y);
//			if (msg.ship[i].shield)
//				_tprintf(TEXT("Â"));
//			else
//				_tprintf(TEXT("A"));
//		}
//	}
//
//	if (msg.pUp.fired == 1) {
//		gotoxy(msg.pUp.x, msg.pUp.y);
//
//		_tprintf(TEXT("%d"), msg.pUp.type);
//	}
//
//}

int readPipeMsg(HANDLE hPipe, HANDLE readReady, GameData * msg) {

	OVERLAPPED	OvrRd = { 0 };
	DWORD		dwBytesRead = 0;
	BOOL		bSuccess = FALSE;

	OvrRd.hEvent = readReady;
	ResetEvent(readReady);

	bSuccess = ReadFile(
		hPipe,
		msg,
		sizeof(GameData),
		&dwBytesRead,
		&OvrRd);

	WaitForSingleObject(readReady, INFINITE);

	GetOverlappedResult(
		hPipe,
		&OvrRd,
		&dwBytesRead,
		FALSE);

	if (dwBytesRead < sizeof(GameData)) {
		if (GetLastError() == ERROR_BROKEN_PIPE) {
			//_tprintf(TEXT("[Error] Connection lost.\n"));
			return 0;
		}
		else {
			//_tprintf(TEXT("[Error] ReadFile failed. Error = %d \n"), GetLastError());
			return 0;
		}
	}

	return 1;
}

int writePipeMsg(HANDLE hPipe, HANDLE writeReady, Packet msg) {

	OVERLAPPED	OvrWr = { 0 };
	DWORD		dwBytesWritten = 0;
	BOOL		bSuccess = FALSE;

	ResetEvent(writeReady);
	OvrWr.hEvent = writeReady;

	bSuccess = WriteFile(
		hPipe,
		&msg,
		sizeof(Packet),
		&dwBytesWritten,
		&OvrWr);

	WaitForSingleObject(writeReady, INFINITE);
	GetOverlappedResult(hPipe, &OvrWr, &dwBytesWritten, FALSE);
	if (dwBytesWritten < sizeof(Packet)) {
		_tprintf(TEXT("[Error] WriteFile failed. Error = %d \n"), GetLastError());
		return -1;
	}

	return 0;
}

DWORD WINAPI ReadGame(LPVOID tParam) {

	ThreadCtrl	*cThreadRdGame = (ThreadCtrl*)tParam;
	//GameData	localGame;

	rectEvent = CreateEvent(				//Creates the event to warn client about window refresh
		NULL,										//Event attributes
		TRUE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		NULL);										//Event name
	if (rectEvent == NULL) {
		_tprintf(TEXT("[Error] Event 1st pipe instance (%d)\n"), GetLastError());
		return -1;
	}

	if (cThreadRdGame->hPipe == NULL) {
		//_tprintf(TEXT("[Error] casting pipe. (%d)\n"), GetLastError());
		return -1;
	}

	cThreadRdGame->token.instruction = 7;
	writePipeMsg(cThreadRdGame->hPipe, cThreadRdGame->heWriteReady, cThreadRdGame->token);

	int waiting=1;
	while (waiting && cThreadRdGame->ThreadMustGoOn) {
		readPipeMsg(cThreadRdGame->hPipe, cThreadRdGame->heReadReady, &cThreadRdGame->localGame);

		if (cThreadRdGame->localGame.gameRunning == 1) {
			for (int i = 0; i < MAX_PLAYERS && waiting; i++) {
				if (cThreadRdGame->localGame.ship[i].id == cThreadRdGame->token.Id) {
					cThreadRdGame->owner = i;
					waiting = 0;
				}
			}
		}
	}

	while (cThreadRdGame->ThreadMustGoOn){

		readPipeMsg(&cThreadRdGame->hPipe, cThreadRdGame->heReadReady, &cThreadRdGame->localGame);
		SetEvent(rectEvent);

	} 

	return 0;
}

//DWORD WINAPI GetKey(LPVOID tParam) {
//
//	ThreadCtrl	*cThread = (ThreadCtrl*)tParam;
//	wint_t		k_stroke;
//
//	Packet	localpacket = { 0 };
//
//	int		packetUpd = 0;
//
//	localpacket.owner = cThread->owner;
//
//	while (cThread->ThreadMustGoOn) {
//		k_stroke = _gettch();
//
//		switch (k_stroke) {
//		case 'w':
//			localpacket.instruction = 3;
//			packetUpd = 1;
//			break;
//		case 's':
//			localpacket.instruction = 1;
//			packetUpd = 1;
//			break;
//		case 'a':
//			localpacket.instruction = 2;
//			packetUpd = 1;
//			break;
//		case 'd':
//			localpacket.instruction = 0;
//			packetUpd = 1;
//			break;
//		case 27://esc
//				//deauth packet?
//			cThread->ThreadMustGoOn = 0;
//			packetUpd = 1;
//			break;
//		case 32://space
//				//PlaySound(TEXT("shoot.wav"), NULL, SND_ASYNC | SND_FILENAME);		//needs better aproach
//			localpacket.instruction = 4;
//			packetUpd = 1;
//			break;
//		case 224://extended
//			_gettch();//ignore extended
//			break;
//		default:
//			break;
//		}
//
//		if (packetUpd) {
//			writePipeMsg(cThread->hPipe, cThread->heWriteReady, localpacket);
//			packetUpd = 0;
//		}
//	}
//	return 0;
//}

int StartPipeListener(HANDLE *hPipe, ThreadCtrl *cThread) {

	DWORD		dwPipeMode;					//Stores pipe mode

	BOOL		bSuccess;
	BOOL		bRunning;

	bRunning = TRUE;

	HANDLE		hUserToken = NULL;
	BOOL log;

	TCHAR		lpFileName[_MAX_PATH];

	if (cThread->remoteLogin) {

		_tcscpy_s(lpFileName, _MAX_PATH, TEXT("\\\\"));
		_tcscat_s(lpFileName, _MAX_PATH, cThread->domain);
		_tcscat_s(lpFileName, _MAX_PATH, TEXT("\\pipe\\SpaceWPipe"));

		if (_tcscmp(cThread->password, TEXT("\0"))) {
			log = LogonUser(
				cThread->userlogin,
				cThread->domain,
				NULL,
				LOGON32_LOGON_NEW_CREDENTIALS,
				LOGON32_PROVIDER_DEFAULT,
				&hUserToken);
		}
		else {
			log = LogonUser(
				cThread->userlogin,
				cThread->domain,
				cThread->password,
				LOGON32_LOGON_NEW_CREDENTIALS,
				LOGON32_PROVIDER_DEFAULT,
				&hUserToken);
		}
	
		if (log == 0) {
			_tprintf(TEXT("[Error] Logging on user (%d)\n"), GetLastError());
			return -1;
		}

		log = ImpersonateLoggedOnUser(hUserToken);

		if (log == 0) {
			_tprintf(TEXT("[Error] Logging on user (%d)\n"), GetLastError());
			return -1;
		}
	}
	else {
		_tcscpy_s(lpFileName, _MAX_PATH, PIPE_NAME);
	}


	do {

		*hPipe = CreateFile(
			lpFileName,
			GENERIC_READ |
			GENERIC_WRITE,
			0 | FILE_SHARE_READ |
			FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0 | FILE_FLAG_OVERLAPPED,
			NULL);

		if (GetLastError() == ERROR_PIPE_BUSY) {
			_tprintf(TEXT("[DEBUG] Server full. Waiting for 30 seconds\n"));

			bSuccess = WaitNamedPipe(PIPE_NAME, 30000);

			if (bSuccess)
				continue;
			else
				return -1;

		}
		else if (*hPipe == INVALID_HANDLE_VALUE) {

			_tprintf(TEXT("[Error] opening pipe. (%d)\n"), GetLastError());
			return -1;
		}
		else
			bRunning = FALSE;

	} while (bRunning);

	//_tprintf(TEXT("[DEBUG] Pipe connected.\nChanging pipe mode...\n"));

	dwPipeMode = PIPE_READMODE_MESSAGE;
	bSuccess = SetNamedPipeHandleState(
		*hPipe,
		&dwPipeMode,
		NULL,
		NULL);

	if (!bSuccess) {
		_tprintf(TEXT("[Error] setting pipe mode. (%d)\n"), GetLastError());
		return -1;
	}

	return 0;
}

int createProdConsEvents(ThreadCtrl * ps) {

	ps->heWriteReady = CreateEvent(
		NULL,										//Event attributes
		TRUE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		NULL);										//Event name
	if (ps->heWriteReady == NULL) {
		_tprintf(TEXT("[Error] Event WriteReady(%d)\n"), GetLastError());
		return -1;
	}

	ps->heReadReady = CreateEvent(
		NULL,										//Event attributes
		TRUE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		NULL);										//Event name
	if (ps->heReadReady == NULL) {
		_tprintf(TEXT("[Error] Event ReadReady(%d)\n"), GetLastError());
		return -1;
	}

	return 0;
}

int markPlayerReady(ThreadCtrl * ps) {

	//GameData	localGame;
	HANDLE		htReadGame;					//Game thread
	DWORD		tReadGameID;				//Game thread ID

	//_tprintf(TEXT("Press ENTER when ready\n"));
	//_gettch();

	//ps->token.instruction = 7;
	//writePipeMsg(ps->hPipe, ps->heWriteReady, ps->token);
	//_tprintf(TEXT("READY TO PLAY\n"));


	htReadGame = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size (0 for default)
		ReadGame,									//Thread function name
		ps,											//Thread parameter struct
		0,											//Creation flags
		&tReadGameID);								//gets thread ID 

	if (htReadGame == NULL) {
		_tprintf(TEXT("[Error] launching ReadGame thread. (%d)\n"), GetLastError());
		return -1;
	}

	//WaitForSingleObject(htReadGame, INFINITE);

	return 0;
}

Packet handShakeServer(ThreadCtrl * ps, TCHAR *username) {

	Packet		lPacket;
	GameData	localGame;

	_tcscpy_s(lPacket.username, SMALL_BUFF, username);
		lPacket.Id = GetCurrentProcessId() + (DWORD)time(NULL);
		lPacket.owner = -1;
		lPacket.instruction = 5;

		//_tprintf(TEXT("Hello %s! Trying to login with the ID=%d\n"), lPacket.username, lPacket.Id);

		writePipeMsg(ps->hPipe, ps->heWriteReady, lPacket);

		for (int i = 0; i < 10; i++) {
			readPipeMsg(ps->hPipe, ps->heReadReady, &localGame);
			for (int j = 0; j < MAX_PLAYERS; j++) {
				if (localGame.logged[j].Id == lPacket.Id) {
					ps->logged = 1;
					return lPacket;
				}
			}
		}

	return lPacket;
}