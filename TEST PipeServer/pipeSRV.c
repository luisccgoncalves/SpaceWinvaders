#pragma warning(disable : 4996) //Disable deprecated function warnings, mainly string manipulation stuff
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h> 
#include <strsafe.h>
#include <io.h> 
#include <fcntl.h>		// _setmode // macro para o _setmode

#define BUFSIZE 2048	// Tamanho de buffer para o pipe // Obs: não o usar o default

#define QUEMSZ 60 
#define MSGTXTSZ 60

typedef struct {

	// Estrutura para a mensagem 
	TCHAR quem[QUEMSZ];
	TCHAR msg[MSGTXTSZ];

} Msg;

#define Msg_Sz sizeof(Msg)

// Função auxiliar para ler texto na consola 
void readTChars(TCHAR * p, int maxchars) {
	int len;
	_fgetts(p, maxchars, stdin);
	len = _tcslen(p);
	if (p[len - 1] == TEXT('\n'))
		p[len - 1] = TEXT('\0');
}

// Função auxiliar para efectuar uma pausa controlada pelo utilizador 
void pressEnter() {

	TCHAR somekeys[25];
	_tprintf(TEXT("\nPress enter > "));
	readTChars(somekeys, 25);
}

DWORD WINAPI InstanceThread(LPVOID lpvParam);

#define MAXCLIENTES 10 
HANDLE clientes[MAXCLIENTES];

void iniciaClientes() {
	int i;
	for (i = 0; i < MAXCLIENTES; i++)
		clientes[i] = NULL;
}

void adicionaCliente(HANDLE cli) {

	int i;

	for (i = 0; i < MAXCLIENTES; i++)
		if (clientes[i] == NULL) {
			clientes[i] = cli;
			return;
		}
}

void removeCliente(HANDLE cli) {
	int i;
	for (i = 0; i < MAXCLIENTES; i++)
		if (clientes[i] == cli) {
			clientes[i] = NULL;
			return;
		}
}

HANDLE WriteReady;

// Esta função envia uma mensagem a um cliente 
int writeClienteASINC(HANDLE hPipe, Msg msg) {
	DWORD	cbWritten = 0;
	BOOL	fSuccess = FALSE;

	OVERLAPPED OverlWr = { 0 };

	ZeroMemory(&OverlWr, sizeof(OverlWr)); // não necessário devido à inicialização com { 0 } 
	ResetEvent(WriteReady); // não assinalado 
	OverlWr.hEvent = WriteReady;

	fSuccess = WriteFile(
		hPipe,			// handle para o pipe
		&msg,			// message (ponteiro)
		Msg_Sz,			// comprimento da messagem 
		&cbWritten,		// ptr p/ guarder num. bytes escritos
		&OverlWr);		// != NULL -> É mesmo overlapped I/O

	WaitForSingleObject(WriteReady, INFINITE);

	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE); // sem WAIT 

	if (cbWritten < Msg_Sz)
		_tprintf(TEXT("\nwriteFile não escreveu toda a informação. Erro = %d"), GetLastError());

	return 1;
}

// Esta função envia uma mensagema todos os clientes registados 
int broadcastclientes(Msg msg) {

	int i, numwrites = 0;

	for (i = 0; i < MAXCLIENTES; i++)
		if (clientes[i] != 0)
			numwrites += writeClienteASINC(clientes[i], msg);

	return numwrites;
}

int _oldmain(VOID) {	// aplicação genérica quanto a char/Unicode

	BOOL	fConnected = FALSE;
	DWORD	dwThreadId = 0;
	HANDLE	hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	LPTSTR	lpszPipename = TEXT("\\\\.\\pipe\\pipeexemplo");

	_setmode(_fileno(stdout), _O_WTEXT);	// Proteger com #ifdef UNICODE .... #endif

	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (WriteReady == NULL) {
		_tprintf(TEXT("\nServidor: não foi possível criar o evento Write. Mais vale parar já"));
		return 1;
	}

	iniciaClientes();

	while (1) {

		hPipe = CreateNamedPipe(
			lpszPipename,								// nome do pipe 
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_MESSAGE |							// tipo de pipe = message
			PIPE_READMODE_MESSAGE |						// com modo message-read e 
			PIPE_WAIT,									// bloqueante (não usar PIPE_NOWAIT nem mesmo em Asyncr) 
			PIPE_UNLIMITED_INSTANCES,					// max. instancias (255)
			BUFSIZE,									// tam buffer output 
			BUFSIZE,									// tam buffer input 
			5000,										// time-out p/ cliente 5k milisegundos (0->default=50) 
			NULL);										// atributos segurança default

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("\nCreateNamedPipe falhou, erro = %d"), GetLastError());
			return -1;// Neste caso, nada a fazer 
		} // Tratamento de erro pode ser melhorado

		_tprintf(TEXT("\nServidor a aguardar que um cliente se ligue"));
		// aguarda que um cliente se ligue 
		// != 0 significa sucesso 
		// = 0 e LastError = ERROR_PIPE_CONNECT significa sucesso também.

		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected) {
			hThread = CreateThread(
				NULL,				// Sem atributos de segurança 
				0,					// Tamaho de pilha default 
				InstanceThread,		// Função da thread 
				(LPVOID)hPipe,		// Parâmetro para a thread = handle
				0,					// inicialmente não suspensa 
				&dwThreadId);		// Ptr p/ onde colocar ID da thread

			if (hThread == NULL) { // se a criação da thread deu erro
				_tprintf(TEXT("\nErro na criação da thread. Erro = %d"), GetLastError());
				return -1;
			}
			else
				CloseHandle(hThread); // Não precisa do handle p/ a thread
		}
		else
			// O cliente não conseguiu ligar - fecha esta instância do pipe 
			CloseHandle(hPipe);
	}

	return 0; // encerra servidor
}

DWORD WINAPI InstanceThread(LPVOID lpvParam) {

	Msg Pedido, Resposta;
	DWORD cbBytesRead = 0, cbReplyBytes = 0;
	int numresp = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = (HANDLE)lpvParam; // a informação enviada à thread é o handle do pipe

	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };

	_tcscpy(Resposta.quem, TEXT("SRV"));

	if (hPipe == NULL) {
		_tprintf(TEXT("\nErro - o handle enviado no param da thread é nulo"));
		return -1;
	}

	ReadReady = CreateEvent(
		NULL, // default security + non inheritable 
		TRUE, // Reset manual, por requisito do overlapped IO => uso de ResetEvent 
		FALSE, // estado inicial = not signaled 
		NULL); // não precisa de nome: uso interno ao processo

	if (ReadReady == NULL) {
		_tprintf(TEXT("\nServidor: não foi possível criar o evento Read. Mais vale parar já"));
		return -1;
	}

	adicionaCliente(hPipe);// Regista cliente. Se já for conhecido não faz nada

						   // Ciclo de diálogo com o cliente 
	while (1) {					 // termina mediante uma condição qualquer

								 // Obtém mensagem do cliente
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		ResetEvent(ReadReady);
		OverlRd.hEvent = ReadReady;

		fSuccess = ReadFile(
			hPipe,				// handle para o pipe(recebido no param) 
			&Pedido,			// buffer para os dados a ler 
			Msg_Sz,				// Tamanho msg a ler 
			&cbBytesRead,		// número de bytes lidos 
			&OverlRd);			//!= NULL->é overlapped I / O

		WaitForSingleObject(ReadReady, INFINITE);

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE); // sem WAIT 
		if (cbBytesRead < Msg_Sz)
			_tprintf(TEXT("\nReadFile não leu os dados todos. Erro = %d"), GetLastError()); // acrescentar lógica de encerrar a thread cliente

			// if (!fSuccess || cbBytesRead < Msg_Sz) { 
			// Este teste deixa de fazer sentido porque 
			// ReadFIle retornou antes da conclusão da operação

		// Processa a mensagem recebida. 
		_tprintf(TEXT("\nServidor: Recebi (?) de: [%s] msg: [%s]"), Pedido.quem, Pedido.msg);
		_tcscpy(Resposta.msg, Pedido.quem);
		_tcscat(Resposta.msg, TEXT(": "));
		_tcscat(Resposta.msg, Pedido.msg);

		numresp = broadcastclientes(Resposta);
		_tprintf(TEXT("\nServidor: %d respostas enviadas"), numresp);
	}

	// Desligar a ligação ao cliente.
	removeCliente(hPipe);

	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe); // Desliga servidor da instância 
	CloseHandle(hPipe); // Fecha este lado desta instância

	_tprintf(TEXT("\nThread dedicada Cliente a terminar"));
	return 1;
}
