#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "structs.h"
#include "../DLL/dll.h"

typedef struct {
	HANDLE			mhInvader;						//Handle to mutex (TEST)
	SMCtrl			smCtrl;							//Shared memory structure
	int				ThreadMustGoOn;					//Flag for thread shutdown
} SMCtrl_Thread;

typedef struct {
	HANDLE			hTick;							//Handle to event. Warns gateway about updates in shared memory
	int				ThreadMustGoOn;		
}GTickStruct;
/**/

DWORD WINAPI InvaderDeploy(LPVOID tParam) {

	SMCtrl_Thread	*cThread;						//Pointer to thread parameter structure
	cThread = (SMCtrl_Thread *)tParam;				//Points it to the right direction
	int i;
	_tprintf(TEXT("Server deployed an invader."));

	//Let's shorten this path
	invader *invader = &cThread->smCtrl.pSMemServer->pSMem;

	//Populates one invader with initial coords
	invader->x = invader->x_init = 0;
	invader->y = invader->y_init = 0;

	while (cThread->ThreadMustGoOn) {				//Thread main loop

		for (i = 0; i < (YSIZE*4) && cThread->ThreadMustGoOn; i++) {

			invader->y = i/4;						//Invader goes down after 4 iterations

			if ((i % 8) < 4)
				invader->x = i % 8;					//Invader goes right
			else if ((i % 8) > 4)
				invader->x--;						//Invader goes left

			Sleep(500);
		}
	}
}

DWORD WINAPI GameTick(LPVOID tParam) {				//Warns gateway of structure updates
	
	GTickStruct		*sGTick;
	sGTick = (GTickStruct*)tParam;

	while (sGTick->ThreadMustGoOn) {

		Sleep(100);
		_tprintf(TEXT("."));
		SetEvent(sGTick->hTick);
	}
}

DWORD WINAPI ReadGatewayMsg(LPVOID tParam) {		//Warns gateway of structure updates

	SMCtrl_Thread	*cThread;
	cThread = (SMCtrl_Thread*)tParam;

	while (cThread->ThreadMustGoOn) {

		WaitForSingleObject(cThread->smCtrl.hSMGatewayUpdate, INFINITE);
		_tprintf(TEXT(" g "));
	}
}

int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE									//Sets console to unicode
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif
	
	SMCtrl_Thread	cThread;						//Thread parameter structure
	HANDLE			hCanBootNow;					//Handle to event. Warns the gateway the shared memory is mapped
	DWORD			tInvaderID;						//stores the ID of the Invader thread
	HANDLE			htInvader;						//Handle to the Invader thread

	GTickStruct		sGTick;
	HANDLE			htGTick;
	HANDLE			htGReadMsg;
	DWORD			tGTickID;
	DWORD			tRGMsgID;

	SYSTEM_INFO		SysInfo;
	DWORD			dwSysGran;

	GetSystemInfo(&SysInfo);						//Used to get system granularity
	dwSysGran = SysInfo.dwAllocationGranularity;	//Used to get system granularity

	cThread.smCtrl.SMemViewServer.QuadPart = ((sizeof(SMServer_MSG) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.smCtrl.SMemViewGateway.QuadPart = ((sizeof(SMGateway_MSG) / dwSysGran)*dwSysGran) + dwSysGran;
	cThread.smCtrl.SMemSize.QuadPart = cThread.smCtrl.SMemViewServer.QuadPart + cThread.smCtrl.SMemViewGateway.QuadPart;

	//#######################################################################################################################
	//#####################################GRANULARITY TESTS//DELETE THIS####################################################
	//#######################################################################################################################
	_tprintf(TEXT("Sysgran: %d bytes\nSize of servstruct: %d\nSize of gateway: %d\n"), dwSysGran, sizeof(SMServer_MSG), sizeof(SMGateway_MSG));
	_tprintf(TEXT("ServerView:\t((%d/%d)*%d)+%d=%d\n"), sizeof(SMServer_MSG), dwSysGran, dwSysGran, dwSysGran, ((sizeof(SMServer_MSG) / dwSysGran)*dwSysGran) + dwSysGran);
	_tprintf(TEXT("GatewayView:\t((%d/%d)*%d)+%d=%d\n"), sizeof(SMGateway_MSG),dwSysGran,dwSysGran, dwSysGran, ((sizeof(SMGateway_MSG) / dwSysGran)*dwSysGran) + dwSysGran);
	_tprintf(TEXT("TestBigView:\t((%d/%d)*%d)+%d=%d\n"), 66000, dwSysGran, dwSysGran, dwSysGran, ((66000 / dwSysGran)*dwSysGran) + dwSysGran);
	//#######################################################################################################################
	//#######################################################################################################################
	//#######################################################################################################################

	cThread.ThreadMustGoOn = 1;						//Preps thread to run position
	sGTick.ThreadMustGoOn = 1;

	cThread.mhInvader = CreateMutex(				//This a test
		NULL,										//Security attributes
		FALSE,										//Initial owner
		NULL);										//Mutex name

	hCanBootNow = CreateEvent(						//Creates the event to warn gateway that the shared memoy is mapped
		NULL,										//Event attributes
		FALSE,										//Manual reset (TRUE for auto-reset)
		FALSE,										//Initial state
		TEXT("LetsBoot"));							//Event name

	cThread.smCtrl.hSMServerUpdate = CreateEvent(	//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMServerUpdate"));					//Event name

	cThread.smCtrl.hSMGatewayUpdate = CreateEvent(	//Creates the event to warn gateway that the shared memoy is mapped
		NULL, 										//Event attributes
		FALSE, 										//Manual reset (TRUE for auto-reset)
		FALSE, 										//Initial state
		TEXT("SMGatewayUpdate"));					//Event name

	sGTick.hTick = cThread.smCtrl.hSMServerUpdate;

	sharedMemory(&cThread.smCtrl);
	if (cThread.smCtrl.hSMem== NULL) {				//Checks for errors
		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Server>
	mapServerView(&cThread.smCtrl);
	if (cThread.smCtrl.pSMemServer == NULL) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping server view (%d)\n"), GetLastError());
		return -1;
	}

	//Creates a view of the desired part <Gateway>
	mapGatewayView(&cThread.smCtrl);
	if (cThread.smCtrl.pSMemGateway== NULL) {		//Checks for errors
		_tprintf(TEXT("[Error] Mapping gateway view (%d)\n"), GetLastError());
		return -1;
	}

	SetEvent(hCanBootNow);							//Warns gateway that Shared memory is mapped

	//Launches game tick thread
	_tprintf(TEXT("Launching game tick thread...\n"));

	htGTick = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		GameTick,									//Thread function name
		(LPVOID)&sGTick,							//Thread parameter struct
		0,											//Creation flags
		&tGTickID);									//gets thread ID to close it afterwards

	//Launches gateway message receiver thread
	_tprintf(TEXT("Launching gateway message receiver thread...\n"));
	htGReadMsg = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		ReadGatewayMsg,								//Thread function name
		(LPVOID)&cThread,							//Thread parameter struct
		0,											//Creation flags
		&tRGMsgID);									//gets thread ID to close it afterwards

	//Launches invader thread
	_tprintf(TEXT("Launching invader thread... ENTER to quit\n"));

	htInvader = CreateThread(
		NULL,										//Thread security attributes
		0,											//Stack size
		InvaderDeploy,								//Thread function name
		(LPVOID)&cThread,							//Thread parameter struct
		0,											//Creation flags
		&tInvaderID);								//gets thread ID to close it afterwards

	//Enter to end thread and exit
	_gettchar();
	cThread.ThreadMustGoOn = 0;						//Signals thread to gracefully exit
	sGTick.ThreadMustGoOn = 0;						//Signals thread to gracefully exit

	WaitForSingleObject(htInvader, INFINITE);		//Waits for thread to exit
	WaitForSingleObject(htGTick, INFINITE);			//Waits for thread to exit

	SetEvent(cThread.smCtrl.hSMGatewayUpdate);
	WaitForSingleObject(htGReadMsg, INFINITE);
	
	UnmapViewOfFile(cThread.smCtrl.pSMemServer);	//Unmaps view of shared memory
	UnmapViewOfFile(cThread.smCtrl.pSMemGateway);	//Unmaps view of shared memory
	CloseHandle(cThread.smCtrl.hSMem);				//Closes shared memory

	return 0;
}
