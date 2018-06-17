#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <process.h>
#include "localStructs.h"
#include "debug.h"

#pragma comment(lib,"Winmm.lib")



void printGame(GameData msg) {

	HANDLE		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	int i, j;

	cls(hStdout);

	for (i = 0; i < MAX_INVADER; i++) {
		if (msg.invad[i].hp) {
			gotoxy(msg.invad[i].x, msg.invad[i].y);
			if (msg.invad[i].rand_path)
				_tprintf(TEXT("X"));
			else
				_tprintf(TEXT("W"));
		}
	}

	for (i = 0; i < MAX_INVADER; i++) {
		for (j = 0; j < MAX_BOMBS; j++) {
			if (msg.invad[i].bomb[j].fired) {
				gotoxy(msg.invad[i].bomb[j].x, msg.invad[i].bomb[j].y);
				_tprintf(TEXT("o"));
			}
		}
	}

	for (i = 0; i < msg.num_players; i++) {
		for (j = 0; j < MAX_SHOTS; j++) {
			if (msg.ship[i].shots[j].fired) {
				gotoxy(msg.ship[i].shots[j].x, msg.ship[i].shots[j].y);
				_tprintf(TEXT("|"));
			}
		}
	}

	for (i = 0; i < msg.num_players; i++) {
		if (msg.ship[i].lives >= 0) {
			gotoxy(msg.ship[i].x, msg.ship[i].y);
			if(msg.ship[i].shield)
				_tprintf(TEXT("Â"));
			else
				_tprintf(TEXT("A"));
		}
	}

	if (msg.pUp.fired==1) {
		gotoxy(msg.pUp.x, msg.pUp.y);

		_tprintf(TEXT("%d"), msg.pUp.type);
	}

}

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
			_tprintf(TEXT("[Error] Connection lost.\n"));
			return -1;
		}
		else
			_tprintf(TEXT("[Error] ReadFile failed. Error = %d \n"), GetLastError());
	}

	return 0;
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
	GameData	localGame;

	if (cThreadRdGame->hPipe == NULL) {
		_tprintf(TEXT("[Error] casting pipe. (%d)\n"), GetLastError());
		return -1;
	}

	localGame.gameRunning = 1;				//Needed to enter the loop

	while (cThreadRdGame->ThreadMustGoOn && localGame.gameRunning) {

		readPipeMsg(cThreadRdGame->hPipe, cThreadRdGame->heReadReady,&localGame);
		printGame(localGame);
	}

	return 0;
}

DWORD WINAPI GetKey(LPVOID tParam) {

	ThreadCtrl	*cThread = (ThreadCtrl*)tParam;
	wint_t		k_stroke;

	Packet	localpacket = { 0 };

	int		packetUpd = 0;

	localpacket.owner = cThread->owner;

	while (cThread->ThreadMustGoOn) {
		k_stroke = _gettch();

		switch (k_stroke) {
		case 'w':
			localpacket.instruction = 3;
			packetUpd = 1;
			break;
		case 's':
			localpacket.instruction = 1;
			packetUpd = 1;
			break;
		case 'a':
			localpacket.instruction = 2;
			packetUpd = 1;
			break;
		case 'd':
			localpacket.instruction = 0;
			packetUpd = 1;
			break;
		case 27://esc
			//deauth packet?
			cThread->ThreadMustGoOn = 0;
			packetUpd = 1;
			break;
		case 32://space
			//PlaySound(TEXT("shoot.wav"), NULL, SND_ASYNC | SND_FILENAME);		//needs better aproach
			localpacket.instruction = 4;
			packetUpd = 1;
			break;
		case 224://extended
			_gettch();//ignore extended
			break;
		default:
			break;
		}

		if (packetUpd) {
			writePipeMsg(cThread->hPipe, cThread->heWriteReady, localpacket);
			packetUpd = 0;
		}
	}
	return 0;
}

int StartPipeListener(HANDLE *hPipe) {

	DWORD		dwPipeMode;					//Stores pipe mode

	BOOL		bSuccess;
	BOOL		bRunning;

	bRunning = TRUE;

	HANDLE		hUserToken = NULL;
	BOOL log;

	//LPCTSTR		lpFileName = TEXT("\\\\ENIAC\\pipe\\SpaceWPipe");
	LPCTSTR		lpFileName = TEXT("\\\\.\\pipe\\SpaceWPipe");

	log = LogonUser(
		TEXT("blacksmith"),
		TEXT("."),
		NULL,
		//TEXT("blacksmith"),
		//TEXT("local"),
		//NULL,
		LOGON32_LOGON_NEW_CREDENTIALS,
		LOGON32_PROVIDER_DEFAULT,
		&hUserToken);
	if (log == 0) {
		_tprintf(TEXT("[Error] Logging on user (%d)\n"), GetLastError());
		return -1;
	}

	log = ImpersonateLoggedOnUser(hUserToken);

	if (log == 0) {
		_tprintf(TEXT("[Error] Logging on user (%d)\n"), GetLastError());
		return -1;
	}

	do {

		*hPipe = CreateFile(
			lpFileName,
			GENERIC_READ | 
			GENERIC_WRITE,
			0 |	FILE_SHARE_READ |
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
			bRunning=FALSE;

	} while (bRunning);

	_tprintf(TEXT("[DEBUG] Pipe connected.\nChanging pipe mode...\n"));

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

int markPlayerReady(ThreadCtrl * ps, Packet token) {

	GameData	localGame;

	_tprintf(TEXT("Press ENTER when ready\n"));
	_gettch();

	token.instruction = 7;
	writePipeMsg(ps->hPipe, ps->heWriteReady, token);
	_tprintf(TEXT("READY TO PLAY\n"));

	while (!readPipeMsg(ps->hPipe, ps->heReadReady, &localGame)) {
		if (localGame.gameRunning == 1) {
			for (int i = 0; i < MAX_PLAYERS; i++) {
				if (localGame.ship[i].id == token.Id)
					ps->owner = i;
			}
			break;
		}
	}

	return 0;
}

Packet handShakeServer(ThreadCtrl * ps) {

	Packet		lPacket;
	GameData	localGame;
	BOOL		isLogged=FALSE;

	do {
		_tprintf(TEXT("Username:"));
		_tscanf_s(TEXT("%s"), lPacket.username, (unsigned int)_countof(lPacket.username));

		lPacket.Id = GetCurrentProcessId() + (DWORD)time(NULL);
		lPacket.owner = -1;
		lPacket.instruction = 5;

		_tprintf(TEXT("Hello %s! Trying to login with the ID=%d\n"), lPacket.username, lPacket.Id);

		writePipeMsg(ps->hPipe, ps->heWriteReady, lPacket);

		for (int i = 0; i < 10; i++) {
			readPipeMsg(ps->hPipe, ps->heReadReady, &localGame);
			for (int j = 0; j < MAX_PLAYERS; j++) {
				if (localGame.logged[j].Id == lPacket.Id) {
					isLogged = TRUE;
					break;
				}
			}
		}

		if(isLogged==FALSE)
			_tprintf(TEXT("Login Failed. Try again.\n"));
		else
			_tprintf(TEXT("Logged in!.\n"));

	} while (isLogged == FALSE);


	return lPacket;
}

int _tmain(int argc, LPTSTR argv[]) {

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	HANDLE		htReadGame;					//Game thread
	DWORD		tReadGameID;				//Game thread ID

	HANDLE		htGetKey;
	DWORD		tGetKeyID;

	Packet		token;

	ThreadCtrl	cThreadRdGame;

	cThreadRdGame.ThreadMustGoOn = 1;

	createProdConsEvents(&cThreadRdGame);

	//Connect to gateway
	if (StartPipeListener(&cThreadRdGame.hPipe)!=0) {
		_tprintf(TEXT("[Error] launching pipe listener...\n"));
		return -1;
	}

	//Connect to Server (through gateway)
	token=handShakeServer(&cThreadRdGame);

	do {

		cThreadRdGame.ThreadMustGoOn = 1;

		markPlayerReady(&cThreadRdGame, token);

		htReadGame = CreateThread(
			NULL,										//Thread security attributes
			0,											//Stack size (0 for default)
			ReadGame,									//Thread function name
			(LPVOID)&cThreadRdGame,						//Thread parameter struct
			0,											//Creation flags
			&tReadGameID);								//gets thread ID 

		if (htReadGame == NULL) {
			_tprintf(TEXT("[Error] launching ReadGame thread. (%d)\n"), GetLastError());
			return -1;
		}

		htGetKey = CreateThread(
			NULL,										//Thread security attributes
			0,											//Stack size (0 for default)
			GetKey,										//Thread function name
			(LPVOID)&cThreadRdGame,						//Thread parameter struct
			0,											//Creation flags
			&tGetKeyID);								//gets thread ID 

		if (htGetKey == NULL) {
			_tprintf(TEXT("[Error] launching GetKey thread. (%d)\n"), GetLastError());
			return -1;
		}

		WaitForSingleObject(htReadGame, INFINITE);

		cThreadRdGame.ThreadMustGoOn = 0;
		WaitForSingleObject(htGetKey, INFINITE);
		
		cls(GetStdHandle(STD_OUTPUT_HANDLE));
		gotoxy(0, 0);

	} while (1); //this will be changed with the GUI


	CloseHandle(cThreadRdGame.hPipe);

	return 0;
}