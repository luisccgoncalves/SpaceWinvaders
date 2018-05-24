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
		_tprintf(TEXT("\nThread Reader - o handle recebido no param da thread é nulo\n"));
		return -1;
	}

	_tprintf(TEXT("Listening\n"));

	ReadReady = CreateEvent(
		NULL, // default security + non inheritable 
		TRUE, // Reset manual, por requisito do overlapped IO => uso de Reset Event 
		FALSE, // estado inicial = not signaled
		NULL); // nao precisa de nome. Uso interno ao processo 

	if (ReadReady == NULL) {
		_tprintf(TEXT("\nCliente: não foi possível criar o Evento Read. Mais vale parar já"));
		return 1;
	}

	while (1) { // o habitual acerca de condição de terminação

		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(
			hPipe,			// handle para o pipe (recebido no param) 
			&msg,	// buffer para os dados a ler 
			sizeof(msg),			// Tamanho msg a ler 
			&cbBytesRead,	// número de bytes a ler 
			&OverlRd);		// != NULL -> é overlapped I/O

		WaitForSingleObject(ReadReady, INFINITE);
		_tprintf(TEXT("\nRead concluido"));

		// Testar se correu como esperado (servidor pode ter encerrado) 
		// if (!fSuccess || cbBytesRead < Msg_Sz) { 
		// Este teste é muito resumido 
		// o teste agora não pode ser feito desta forma

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
			_tprintf(TEXT("\nCreate file deu erro e não foi BUSY. Erro = %d\n"), GetLastError());
			pressEnter();
			return -1;
		}

		if (!WaitNamedPipe(lpszPipeName, 30000)) {
			_tprintf(TEXT("Esperei por uma instância durante 30 segundos. Desisto. Sair"));
			return -1;
		}
	}

	
	dwMode = PIPE_READMODE_MESSAGE;

	fSuccess = SetNamedPipeHandleState(
		hPipe, // handle para o pipe 
		&dwMode, // Novo modo do pipe 
		NULL, // Não é para mudar max. bytes
		NULL); // Não é para mudar max. timeout 

	if (!fSuccess) {
		_tprintf(TEXT("SetNamedPipeHandleState falhou. Erro = %d\n"), GetLastError());
		pressEnter();
		return -1;
	}

	htReadFromSrv = CreateThread(
		NULL,					// Sem atributos de segurança
		0,						// Tam. de pilha default 
		tListenSrv,	// Função da thread 
		(LPVOID)hPipe,			// Parâmetro para a thread = handle ...Dup (nada de Dup) 
		0,						// inicialmente não suspensa 
		NULL);			// Ptr p/ onde colocar ID da thread

	if (htReadFromSrv == NULL) { // se a criação da thread deu erro
		_tprintf(TEXT("\nErro na criação da thread. Erro = %d"), GetLastError());
		return -1;
	}

	WaitForSingleObject(htReadFromSrv, INFINITE);
	return 0;
}