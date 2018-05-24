#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>

typedef struct {
	int		x;
	int		y;
}pipeStruct;

DWORD WINAPI tListenSrv(LPVOID tParam) {

	HANDLE hPipe = (HANDLE)tParam;
	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;

	OVERLAPPED OverlRd = { 0 };
	HANDLE ReadReady;

	pipeStruct	msg;

	if (hPipe == NULL) {
		_tprintf(TEXT("\nThread Reader - o handle recebido no param da thread � nulo\n"));
		return -1;
	}

	_tprintf(TEXT("Listening\n"));

	ReadReady = CreateEvent(
		NULL, // default security + non inheritable 
		TRUE, // Reset manual, por requisito do overlapped IO => uso de Reset Event 
		FALSE, // estado inicial = not signaled
		NULL); // nao precisa de nome. Uso interno ao processo 

	if (ReadReady == NULL) {
		_tprintf(TEXT("\nCliente: n�o foi poss�vel criar o Evento Read. Mais vale parar j�"));
		return 1;
	}

	while (1) { // o habitual acerca de condi��o de termina��o

		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(
			hPipe,			// handle para o pipe (recebido no param) 
			&msg,	// buffer para os dados a ler 
			sizeof(msg),			// Tamanho msg a ler 
			&cbBytesRead,	// n�mero de bytes a ler 
			&OverlRd);		// != NULL -> � overlapped I/O

		WaitForSingleObject(ReadReady, INFINITE);
		_tprintf(TEXT("\nRead concluido"));

		// Testar se correu como esperado (servidor pode ter encerrado) 
		// if (!fSuccess || cbBytesRead < Msg_Sz) { 
		// Este teste � muito resumido 
		// o teste agora n�o pode ser feito desta forma

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE); // sem WAIT 
		if (cbBytesRead < sizeof(msg))
			_tprintf(TEXT("\nReadFile falhou. Erro = %d"), GetLastError());

		_tprintf(TEXT("\nServidor disse: [%d]"), msg.x);

	}

	return 0;
}

int _tmain(int argc, TCHAR *argv[]) {

	HANDLE	hPipe;
	BOOL	fSuccess;
	DWORD	dwBWriten, dwMode;
	LPTSTR	lpszPipeName = TEXT("\\\\.\\pipe\\iHatePipes");

	HANDLE	htReadFromSrv;

	while (1) {

		hPipe = CreateFile(
			lpszPipeName,
			GENERIC_READ |
			GENERIC_WRITE,
			0 | FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			0 | FILE_FLAG_OVERLAPPED,
			NULL);


		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		if (GetLastError() != ERROR_PIPE_BUSY) {
			_tprintf(TEXT("\nCreate file deu erro e n�o foi BUSY. Erro = %d\n"), GetLastError());
			pressEnter();
			return -1;
		}

		if (!WaitNamedPipe(lpszPipeName, 30000)) {
			_tprintf(TEXT("Esperei por uma inst�ncia durante 30 segundos. Desisto. Sair"));
			return -1;
		}
	}

	
	dwMode = PIPE_READMODE_MESSAGE;

	fSuccess = SetNamedPipeHandleState(
		hPipe, // handle para o pipe 
		&dwMode, // Novo modo do pipe 
		NULL, // N�o � para mudar max. bytes
		NULL); // N�o � para mudar max. timeout 

	if (!fSuccess) {
		_tprintf(TEXT("SetNamedPipeHandleState falhou. Erro = %d\n"), GetLastError());
		pressEnter();
		return -1;
	}

	htReadFromSrv = CreateThread(
		NULL,					// Sem atributos de seguran�a
		0,						// Tam. de pilha default 
		tListenSrv,	// Fun��o da thread 
		(LPVOID)hPipe,			// Par�metro para a thread = handle ...Dup (nada de Dup) 
		0,						// inicialmente n�o suspensa 
		NULL);			// Ptr p/ onde colocar ID da thread

	if (htReadFromSrv == NULL) { // se a cria��o da thread deu erro
		_tprintf(TEXT("\nErro na cria��o da thread. Erro = %d"), GetLastError());
		return -1;
	}

	WaitForSingleObject(htReadFromSrv, INFINITE);
	return 0;
}