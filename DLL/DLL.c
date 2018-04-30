#include <windows.h>
#include "dll.h"



DLL_IMP_API int sharedMemory(HANDLE *hSMem, TCHAR SMName[], LARGE_INTEGER SMemSize)
{
	*hSMem = CreateFileMapping(		//Maps a file in memory 
		INVALID_HANDLE_VALUE,		//Handle to file being mapped (INVALID_HANDLE_VALUE to swap)
		NULL,						//Security attributes
		PAGE_READWRITE,				//Maped file permissions
		SMemSize.HighPart,			//MaximumSizeHigh
		SMemSize.LowPart,			//MaximumSizeLow
		SMName);						//File mapping name

	if (*hSMem == INVALID_HANDLE_VALUE) {
		return 1;
	}
	return 0;
}

//DLL_IMP_API int mapServerView(SMServer_MSG *pSMemServer, HANDLE *hSMem, LARGE_INTEGER SMemViewServer)
//{
//	pSMemServer = (SMServer_MSG *)MapViewOfFile(	//Casts view of shared memory to a known struct type
//		hSMem,										//Handle to the whole mapped object
//		FILE_MAP_WRITE,								//Security attributes
//													//cThread.SMemViewServer.HighPart,		//OffsetHIgh (0 to map the whole thing)
//													//cThread.SMemViewServer.LowPart, 		//OffsetLow (0 to map the whole thing)
//		0,
//		0,
//		SMemViewServer.QuadPart);			//Number of bytes to map
//	if (*hSMem == INVALID_HANDLE_VALUE) {
//		return 1;
//	}
//	return 0;
//}


