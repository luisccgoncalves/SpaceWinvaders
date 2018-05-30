#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>

typedef struct {
	int		x;
	int		y;
}pipeStruct;

HANDLE WriteReady;

DWORD WINAPI tListenClients(LPVOID tParam) {

	_tprintf(TEXT("Client connected!\n"));

	HANDLE hPipe = (HANDLE)tParam;

	DWORD	cbWritten = 0;
	BOOL	fSuccess = FALSE;

	OVERLAPPED OverlWr = { 0 };

	pipeStruct	msg;
	msg.x = 1234;

	ZeroMemory(&OverlWr, sizeof(OverlWr)); // não necessário devido à inicialização com { 0 } 
	ResetEvent(WriteReady); // não assinalado 
	OverlWr.hEvent = WriteReady;

	fSuccess = WriteFile(
		hPipe,			// handle para o pipe
		&msg,			// message (ponteiro)
		sizeof(msg),			// comprimento da messagem 
		&cbWritten,		// ptr p/ guarder num. bytes escritos
		&OverlWr);		// != NULL -> É mesmo overlapped I/O

	_tprintf(TEXT("wAITFSO\n"));
	WaitForSingleObject(WriteReady, INFINITE);
	_tprintf(TEXT("DONE\n"));

	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE); // sem WAIT 

	if (cbWritten < sizeof(msg))
		_tprintf(TEXT("\nwriteFile não escreveu toda a informação. Erro = %d"), GetLastError());
	

	return 0;
}

int _tmain(int argc, TCHAR *argv[]) {

	HANDLE	hPipe;
	BOOL	fSuccess;
	DWORD	dwBWriten, dwMode;
	LPTSTR	lpszPipeName=TEXT("\\\\.\\pipe\\iHatePipes");

	HANDLE	hListenCLT;

	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (WriteReady == NULL) {
		_tprintf(TEXT("\nServidor: não foi possível criar o evento Write. Mais vale parar já"));
		return 1;
	}

	while (1) {

		hPipe = CreateNamedPipe(
			lpszPipeName,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE |
			PIPE_READMODE_MESSAGE |
			PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			2048,
			2048,
			5000,
			NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("\nCreateNamedPipe falhou, erro = %d"), GetLastError());
			return -1;// Neste caso, nada a fazer 
		} // Tratamento de erro pode ser melhorado

		_tprintf(TEXT("\nServidor a aguardar que um cliente se ligue"));

		fSuccess = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fSuccess) {
			hListenCLT = CreateThread(
				NULL,				// Sem atributos de segurança 
				0,					// Tamaho de pilha default 
				tListenClients,		// Função da thread 
				(LPVOID)hPipe,		// Parâmetro para a thread = handle
				0,					// inicialmente não suspensa 
				NULL);		// Ptr p/ onde colocar ID da thread

			if (hListenCLT == NULL) { // se a criação da thread deu erro
				_tprintf(TEXT("\nErro na criação da thread. Erro = %d"), GetLastError());
				return -1;
			}
			else
				CloseHandle(hListenCLT); // Não precisa do handle p/ a thread
		}
		else
			// O cliente não conseguiu ligar - fecha esta instância do pipe 
			CloseHandle(hPipe);
	}
	
	return 0;
}