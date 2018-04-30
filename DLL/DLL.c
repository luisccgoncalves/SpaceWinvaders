#include <windows.h>
#include "dll.h"



DLL_IMP_API int sharedMemory(SMCtrl *smCtrl, TCHAR SMName[])
{
	smCtrl->hSMem= CreateFileMapping(		//Maps a file in memory 
		INVALID_HANDLE_VALUE,		//Handle to file being mapped (INVALID_HANDLE_VALUE to swap)
		NULL,						//Security attributes
		PAGE_READWRITE,				//Maped file permissions
		smCtrl->SMemSize.HighPart,			//MaximumSizeHigh
		smCtrl->SMemSize.LowPart,			//MaximumSizeLow
		SMName);						//File mapping name

	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

DLL_IMP_API int mapServerView(SMCtrl *smCtrl)
{
	smCtrl->pSMemServer = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		smCtrl->hSMem,										//Handle to the whole mapped object
		FILE_MAP_WRITE,								//Security attributes
													//cThread.SMemViewServer.HighPart,		//OffsetHIgh (0 to map the whole thing)
													//cThread.SMemViewServer.LowPart, 		//OffsetLow (0 to map the whole thing)
		0,
		0,
		smCtrl->SMemViewServer.QuadPart);			//Number of bytes to map
	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

DLL_IMP_API int mapGatewayView(SMCtrl *smCtrl)
{
	smCtrl->pSMemServer = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		smCtrl->hSMem,										//Handle to the whole mapped object
		FILE_MAP_WRITE,								//Security attributes
													//cThread.SMemViewServer.HighPart,		//OffsetHIgh (0 to map the whole thing)
													//cThread.SMemViewServer.LowPart, 		//OffsetLow (0 to map the whole thing)
		0,
		0,
		smCtrl->SMemViewServer.QuadPart);			//Number of bytes to map
	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}


