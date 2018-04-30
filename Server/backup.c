////#include <windows.h>
////#include <tchar.h>
////#include <stdio.h>
////#include <io.h>
////#include <fcntl.h>
////#include "structs.h"
////#include "../DLL/dll.h"
////
//////####################################################################################################
//////#################################    To include in DLL    ##########################################
//////####################################################################################################
////
////typedef struct {							//Message to use in @ gateway view
////	char			pSMem;					//Object type to use in the memory
////}SMGateway_MSG;
////
//////typedef struct {							////Message to use in @ server view
//////	invader			pSMem;					//Object type to use in the memory
//////}SMServer_MSG;
////
////// Thread to read from memmory
////typedef struct {
////	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
////	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
////
////	HANDLE			mhInvader;				//Handle to mutex (TEST)
////
////	HANDLE			hSMem;					//Handle to shared memory
////	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file
////	LARGE_INTEGER	SMemViewServer;			//Stores the size of the view
////	LARGE_INTEGER	SMemViewGateway;		//Stores the size of the view
////	SMServer_MSG	*pSMemServer;			//Pointer to shared memory's first byte
////	SMGateway_MSG	*pSMGateway;			//Pointer to shared memory's first byte
////
////	int				ThreadMustGoOn;			//Flag for thread shutdown
////} SMCtrl_Thread;
////
//////typedef struct {
//////	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
//////	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
//////
//////	HANDLE			mhInvader;				//Handle to mutex (TEST)
//////
//////	HANDLE			hSMem;					//Handle to shared memory
//////	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file
//////	LARGE_INTEGER	SMemViewServer;			//Stores the size of the view
//////	SMServer_MSG	*pSMemServer;			//Pointer to shared memory's first byte
//////	int				ThreadMustGoOn;			//Flag for thread shutdown
//////} ServerSMCtrl_Thread;
////
//////####################################################################################################
////
////typedef struct {
////	HANDLE			hTick;				//Handle to event. Warns gateway about updates in shared memory
////	int				ThreadMustGoOn;
////}GTickStruct;
/////**/
////
////DWORD WINAPI InvaderDeploy(LPVOID tParam) {
////
////	SMCtrl_Thread	*cThread;			//Pointer to thread parameter structure
////	cThread = (SMCtrl_Thread *)tParam;	//Points it to the right direction
////
////	_tprintf(TEXT("Server deployed an invader."));
////
////	//Populates one invader with initial coords
////	cThread->pSMemServer->pSMem.x = cThread->pSMemServer->pSMem.x_init = 0;
////	cThread->pSMemServer->pSMem.y = cThread->pSMemServer->pSMem.x_init = 0;
////
////	while (cThread->ThreadMustGoOn) {	//Thread main loop
////
////		WaitForSingleObject(cThread->mhInvader, INFINITE);													/**/
////		for (cThread->pSMemServer->pSMem.y = 0;
////			cThread->pSMemServer->pSMem.y <= (YSIZE - 2); cThread->pSMemServer->pSMem.y++) {				/**/
////																											/**/
////																											//Invader goes 4 spaces to the right															/*CRITICAL SECTION*/
////			for (cThread->pSMemServer->pSMem.x = 0;
////				cThread->pSMemServer->pSMem.x < 4; cThread->pSMemServer->pSMem.x++) {						/**/
////				ReleaseMutex(cThread->mhInvader);															/**/
////																											//Sleep(500) should be a variable. Lower number==higher dificulty
////				if (cThread->ThreadMustGoOn) Sleep(500);	//Thread exit condition
////				else return 0;
////			}
////
////			WaitForSingleObject(cThread->mhInvader, INFINITE);												/**/
////																											//Invader goes down 1 space																		/**/
////			cThread->pSMemServer->pSMem.y++;																			/**/
////																														/*CRITICAL SECTION*/
////																														//Invader goes 4 spaces to the left																/**/
////			for (cThread->pSMemServer->pSMem.x = 3;
////				cThread->pSMemServer->pSMem.x > -1; cThread->pSMemServer->pSMem.x--) {						/**/
////				ReleaseMutex(cThread->mhInvader);															/**/
////				if (cThread->ThreadMustGoOn) Sleep(500);	//Thread exit condition
////				else return 0;
////			}
////		}
////	}
////}
////
////DWORD WINAPI GameTick(LPVOID tParam) {		//Warns gateway of structure updates
////
////	GTickStruct		*sGTick;
////	sGTick = (GTickStruct*)tParam;
////
////	while (sGTick->ThreadMustGoOn) {
////
////		Sleep(100);
////		_tprintf(TEXT("TICK "));
////		SetEvent(sGTick->hTick);
////	}
////
////}
////
////int _tmain(int argc, LPTSTR argv[]) {
////
////#ifdef UNICODE				//Sets console to unicode
////	_setmode(_fileno(stdin), _O_WTEXT);
////	_setmode(_fileno(stdout), _O_WTEXT);
////#endif
////
////	SMCtrl_Thread	cThread;			//Thread parameter structure
////	HANDLE			hCanBootNow;		//Handle to event. Warns the gateway the shared memory is mapped
////	DWORD			tInvaderID;			//stores the ID of the Invader thread
////	HANDLE			htInvader;			//Handle to the Invader thread
////
////	GTickStruct		sGTick;
////	HANDLE			htGTick;
////	DWORD			tGTickID;
////
////	SYSTEM_INFO		SysInfo;
////	DWORD			dwSysGran;
////	DWORD			dwViewServerStart;
////	DWORD			dwViewGatewayStart;
////
////	cThread.SMemSize.QuadPart = sizeof(SMServer_MSG) + sizeof(SMGateway_MSG); //This should be in structs.h or dll
////	cThread.SMemViewServer.QuadPart = sizeof(SMServer_MSG);
////	cThread.SMemViewGateway.QuadPart = sizeof(SMGateway_MSG);
////
////	cThread.ThreadMustGoOn = 1;						//Preps thread to run position
////	sGTick.ThreadMustGoOn = 1;
////
////	GetSystemInfo(&SysInfo);									//Used to get system granularity
////	dwSysGran = SysInfo.dwAllocationGranularity;				//Used to get system granularity
////
////																//ViewMap size considering the granularity
////	dwViewServerStart = ((sizeof(cThread.pSMemServer) / dwSysGran)*dwSysGran) + dwSysGran;
////	dwViewGatewayStart = ((sizeof(cThread.pSMGateway) / dwSysGran)*dwSysGran) + dwSysGran;
////
////	cThread.mhInvader = CreateMutex(	//This a test
////		NULL,							//Security attributes
////		FALSE,							//Initial owner
////		NULL);							//Mutex name
////
////	hCanBootNow = CreateEvent(			//Creates the event to warn gateway that the shared memoy is mapped
////		NULL,							//Event attributes
////		FALSE,							//Manual reset (TRUE for auto-reset)
////		FALSE,							//Initial state
////		TEXT("LetsBoot"));				//Event name
////
////	cThread.hSMServerUpdate = CreateEvent(	//Creates the event to warn gateway that the shared memoy is mapped
////		NULL, 							//Event attributes
////		FALSE, 							//Manual reset (TRUE for auto-reset)
////		FALSE, 							//Initial state
////		TEXT("SMServerUpdate"));			//Event name
////
////											//cThread.hSMGatewayUpdate = CreateEvent(		//Creates the event to warn gateway that the shared memoy is mapped
////											//	NULL, 									//Event attributes
////											//	FALSE, 									//Manual reset (TRUE for auto-reset)
////											//	FALSE, 									//Initial state
////											//	TEXT("SMGatewayUpdate"));				//Event name
////
////	sGTick.hTick = cThread.hSMServerUpdate;
////
////	//cThread.hSMem = CreateFileMapping(	//Maps a file in memory 
////	//	INVALID_HANDLE_VALUE,			//Handle to file being mapped (INVALID_HANDLE_VALUE to swap)
////	//	NULL,							//Security attributes
////	//	PAGE_READWRITE,					//Maped file permissions
////	//	cThread.SMemSize.HighPart,		//MaximumSizeHigh
////	//	cThread.SMemSize.LowPart,		//MaximumSizeLow
////	//	SMName);						//File mapping name
////
////	sharedMemory(&cThread.hSMem, SMName, cThread.SMemSize);		//FileMapping
////	if (cThread.hSMem == NULL) {								//Checks for errors
////		_tprintf(TEXT("[Error] Opening file mapping (%d)\n"), GetLastError());
////		return -1;
////	}
////
////	//cThread.pSMemServer = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
////	//	cThread.hSMem,							//Handle to the whole mapped object
////	//	FILE_MAP_WRITE,							//Security attributes
////	//	//cThread.SMemViewServer.HighPart,		//OffsetHIgh (0 to map the whole thing)
////	//	//cThread.SMemViewServer.LowPart, 		//OffsetLow (0 to map the whole thing)
////	//	0,
////	//	0,
////	//	cThread.SMemViewServer.QuadPart);		//Number of bytes to map
////	mapServerView(cThread.pSMemServer, &cThread.hSMem, cThread.SMemViewServer);
////	if (cThread.pSMemServer == NULL) {
////		_tprintf(TEXT("[Error] Mapping server view (%d)\n"), GetLastError());
////		return -1;
////	}
////
////	//####################################################################################################
////	//###################################   Gateway view under construction    ###########################
////	//####################################################################################################
////
////	////Creates a view of the desired part <Gateway>
////	//cThread.pSMGateway = (SMGateway_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
////	//	cThread.hSMem,								//Handle to the whole mapped object
////	//	FILE_MAP_ALL_ACCESS,						//Security attributes
////	//	0,0,
////	//	//cThread.SMemView.HighPart,			//OffsetHIgh (0 to map the whole thing)
////	//	//cThread.SMemViewGateway.LowPart, 			//OffsetLow (0 to map the whole thing)
////	//	cThread.SMemViewGateway.QuadPart);			//Number of bytes to map
////
////	//if (cThread.pSMGateway == NULL) {				//Checks for errors
////	//	_tprintf(TEXT("[Error] Mapping gateway view (%d)\n"), GetLastError());
////	//	return -1;
////	//}
////
////	//####################################################################################################
////
////
////	SetEvent(hCanBootNow);						//Warns gateway that Shared memory is mapped
////
////												//Launches game tick thread
////	_tprintf(TEXT("Launching game tick thread...\n"));
////
////	htGTick = CreateThread(
////		NULL,					//Thread security attributes
////		0,						//Stack size
////		GameTick,				//Thread function name
////		(LPVOID)&sGTick,		//Thread parameter struct
////		0,						//Creation flags
////		&tGTickID);				//gets thread ID to close it afterwards
////
////								//Launches invader thread
////	_tprintf(TEXT("Launching invader thread... ENTER to quit\n"));
////
////	htInvader = CreateThread(
////		NULL,					//Thread security attributes
////		0,						//Stack size
////		InvaderDeploy,			//Thread function name
////		(LPVOID)&cThread,		//Thread parameter struct
////		0,						//Creation flags
////		&tInvaderID);			//gets thread ID to close it afterwards
////
////								//Enter to end thread and exit
////	_gettchar();
////	cThread.ThreadMustGoOn = 0;					//Signals thread to gracefully exit
////	sGTick.ThreadMustGoOn = 0;					//Signals thread to gracefully exit
////	WaitForSingleObject(htInvader, INFINITE);	//Waits for thread to exit
////	WaitForSingleObject(htGTick, INFINITE);		//Waits for thread to exit
////
////	UnmapViewOfFile(cThread.pSMemServer);		//Unmaps view of shared memory
////	CloseHandle(cThread.hSMem);					//Closes shared memory
////
////	return 0;
////}
