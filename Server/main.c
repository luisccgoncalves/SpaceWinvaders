#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "structs.h"
#include "../DLL/dll.h"

int _tmain(int argc, LPTSTR argv[]) {

	#ifdef UNICODE
		_setmode(_fileno(stdin), _O_WTEXT);
		_setmode(_fileno(stdout), _O_WTEXT);
	#endif

	HANDLE hSMem; //handle para mem�ria partilhada
	char * pSMem; //Ponteiro para o primeiro byte da mem�ria
	char * RHead; //Ponteiro que vai percorrendo a mem�ria, para fazer a leitura
	LARGE_INTEGER SMemSize;

	SMemSize.QuadPart = sizeof(char);
	//Criar um objecto para o mapeamento para cada ficheiro aberto
	hSMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, SMemSize.HighPart, SMemSize.LowPart, SMName);

	if (hSMem == NULL) {
		_tprintf(TEXT("[Erro]Criar objectos mapeamentos(%d)\n"), GetLastError());
		return -1;
	}

	//Mapear efectivamente o ficheiro em mem�ria
	pSMem = (char *)MapViewOfFile(hSMem, FILE_MAP_ALL_ACCESS, 0, 0, SMemSize.QuadPart);
	if (pSMem == NULL) {
		_tprintf(TEXT("[Erro]Mapear para mem�ria(%d)\n"), GetLastError());
		return -1;
	}

	RHead = pSMem;

	*pSMem = 'T';

	_tprintf(TEXT("This was put in shared memory -> %c\n"), *pSMem);
	_gettchar();
	UnmapViewOfFile(pSMem);
	CloseHandle(hSMem);

	return 0;
}
