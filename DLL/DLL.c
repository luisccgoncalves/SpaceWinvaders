#include <windows.h>
#include "dll.h"

DLL_IMP_API int sharedMemory(SMCtrl *smCtrl)
{
	smCtrl->hSMem= CreateFileMapping(						//Maps a file in memory 
		INVALID_HANDLE_VALUE,								//Handle to file being mapped (INVALID_HANDLE_VALUE to swap)
		NULL,												//Security attributes
		PAGE_READWRITE,										//Maped file permissions
		smCtrl->SMemSize.HighPart,							//MaximumSizeHigh
		smCtrl->SMemSize.LowPart,							//MaximumSizeLow
		SMName);											//File mapping name

	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

DLL_IMP_API int mapServerView(SMCtrl *smCtrl)
{
	smCtrl->pSMemServer = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		smCtrl->hSMem,										//Handle to the whole mapped object
		FILE_MAP_ALL_ACCESS,								//Security attributes
		0,													//OffsetHIgh (0 to map the whole thing)
		0,													//OffsetLow (0 to map the whole thing)
		smCtrl->SMemViewServer.QuadPart);					//Number of bytes to map

	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

DLL_IMP_API int mapGatewayView(SMCtrl *smCtrl)
{
	smCtrl->pSMemGateway = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		smCtrl->hSMem,										//Handle to the whole mapped object
		FILE_MAP_ALL_ACCESS,								//Security attributes
		smCtrl->SMemViewServer.HighPart,					//OffsetHIgh (0 to map the whole thing)
		smCtrl->SMemViewServer.LowPart, 					//OffsetLow (0 to map the whole thing)
		smCtrl->SMemViewGateway.QuadPart);					//Number of bytes to map
	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

DLL_IMP_API int mapMsgView(SMCtrl *smCtrl)
{
	smCtrl->pSMemGateway = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		smCtrl->hSMem,										//Handle to the whole mapped object
		FILE_MAP_ALL_ACCESS,								//Security attributes
		smCtrl->SMemViewServer.HighPart,					//OffsetHIgh (0 to map the whole thing)
		smCtrl->SMemViewServer.LowPart, 					//OffsetLow (0 to map the whole thing)
		smCtrl->SMemViewGateway.QuadPart);					//Number of bytes to map
	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

DLL_IMP_API int mapWriteGameDataView(SMCtrl *smCtrl)
{
	smCtrl->pSMemServer = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		smCtrl->hSMem,										//Handle to the whole mapped object
		FILE_MAP_WRITE,								//Security attributes
		0,													//OffsetHIgh (0 to map the whole thing)
		0,													//OffsetLow (0 to map the whole thing)
		smCtrl->SMemViewServer.QuadPart);					//Number of bytes to map

	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

DLL_IMP_API int mapReadGameDataView(SMCtrl *smCtrl)
{
	smCtrl->pSMemServer = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
		smCtrl->hSMem,										//Handle to the whole mapped object
		FILE_MAP_READ,								//Security attributes
		0,													//OffsetHIgh (0 to map the whole thing)
		0,													//OffsetLow (0 to map the whole thing)
		smCtrl->SMemViewServer.QuadPart);					//Number of bytes to map

	if (smCtrl->hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}



