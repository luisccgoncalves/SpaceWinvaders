#pragma warning(disable : 4996) //Disable deprecated function warnings, mainly string manipulation stuff
#include <windows.h> 
#include <stdio.h> 
#include <conio.h> 
#include <tchar.h> 
#include <io.h> // _setmode 
#include <fcntl.h> // macro para o _setmode

#define QUEMSZ 60
#define MSGTXTSZ 60

typedef struct { // estutura de mensagem (para ambos os sentidos da comunica��o)
	TCHAR quem[QUEMSZ];
	TCHAR msg[MSGTXTSZ];
} Msg;

#define Msg_Sz sizeof(Msg)

// fun��o auxiliary para ler caracteres em consola e de pouca relev�ncia 
void readTChars(TCHAR * p, int maxchars) {
	int len; _fgetts(p, maxchars, stdin);
	len = _tcslen(p);
	if (p[len - 1] == TEXT('\n'))
		p[len - 1] = TEXT('\0');
}

// fun��o auxiliar para fazer uma pausa de pouca relev�ncia para o exemplo 
void pressEnter() {
	TCHAR somekeys[25];
	_tprintf(TEXT("\nPress enter > "));
	readTChars(somekeys, 25);
}

DWORD WINAPI ThreadClienteReader(LPVOID lpvParam);

int DeveContinuar = 1; 
int ReaderAlive = 0;

int _tmain(int argc, TCHAR *argv[]) {

	HANDLE hPipe /*, hPipeDup*/;
	BOOL	fSuccess = FALSE;
	DWORD cbWritten, dwMode;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeexemplo");

	Msg MsgToSend;
	HANDLE hThread;

	DWORD dwThreadId = 0;

	_setmode(_fileno(stdout), _O_WTEXT);// proteger com #ifdef UNICODE .... #endif

	_tprintf(TEXT("Escreve nome > "));
	readTChars(MsgToSend.quem, QUEMSZ);

	// tenta abrir uma inst�ncia. Poder� ter que esperar
	while (1) {				// Tenta abrir o pipe repetidamente em ciclo

		hPipe = CreateFile(
			lpszPipename,							// Nome do pipe 
			GENERIC_READ |							// acesso read e write 
			GENERIC_WRITE,
			0 | FILE_SHARE_READ | FILE_SHARE_WRITE, // sem->com partilha 
			NULL,									// atributos de seguran�a = default 
			OPEN_EXISTING,							// � para abrir um pipe j� existente 
			0 | FILE_FLAG_OVERLAPPED,				// atributos default 
			NULL);									// sem ficheiro template

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		if (GetLastError() != ERROR_PIPE_BUSY) {
			_tprintf(TEXT("\nCreate file deu erro e n�o foi BUSY. Erro = %d\n"), GetLastError());
			pressEnter();
			return -1;
		}

		// Se chegou aqui � porque todas as inst�ncias 
		// do pipe est�o ocupadas. Rem�dio: aguardar que uma 
		// fique livre com um timeout

		// Aguarda por inst�ncia no m�ximo de 30 segs (podia ser outro intervalo de tempo. 
		if (!WaitNamedPipe(lpszPipename, 30000)) {
			_tprintf(TEXT("Esperei por uma inst�ncia durante 30 segundos. Desisto. Sair"));
			pressEnter();
			return -1;
		}

	} // Fim do ciclo em que tenta abrir a inst�ncia do pipe

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

	hThread = CreateThread(
		NULL,					// Sem atributos de seguran�a
		0,						// Tam. de pilha default 
		ThreadClienteReader,	// Fun��o da thread 
		(LPVOID)hPipe,			// Par�metro para a thread = handle ...Dup (nada de Dup) 
		0,						// inicialmente n�o suspensa 
		&dwThreadId);			// Ptr p/ onde colocar ID da thread

	if (hThread == NULL) { // se a cria��o da thread deu erro
		_tprintf(TEXT("\nErro na cria��o da thread. Erro = %d"), GetLastError());
		return -1;
	}

	HANDLE WriteReady; // Handle para o evento da leitura (cada thread tem um) 
	OVERLAPPED OverlWr = { 0 };

	WriteReady = CreateEvent(
		NULL,					// default security 
		TRUE,					// Reset manual, por requisito do overlapped IO => uso de Reset Event 
		FALSE,					// estado inicial = not signaled 
		NULL);					// nao precisa de nome. Uso interno ao processo

	if (WriteReady == NULL) {
		_tprintf(TEXT("\nCliente: n�o foi poss�vel criar o Evento. Mais vale parar j�"));
		return 1;
	}

	_tprintf(TEXT("\nliga��o estabelecida. \"exit\" para sair"));

	while (1) {

		_tprintf(TEXT("\n%s > "), MsgToSend.quem);
		readTChars(MsgToSend.msg, MSGTXTSZ);

		if (_tcscmp(TEXT("exit"), MsgToSend.msg) == 0)
			break;

		_tprintf(TEXT("\nA enviar %d bytes: \"%s\""), Msg_Sz, MsgToSend.msg);

		ZeroMemory(&OverlWr, sizeof(OverlWr));
		ResetEvent(WriteReady);
		OverlWr.hEvent = WriteReady;

		fSuccess = WriteFile(
			hPipe,			// handle para o pipe 
			&MsgToSend,		// message (ponteiro) 
			Msg_Sz,			// comprimento da mensagem 
			&cbWritten,		// ptr p/ guarder num. bytes escritos
			&OverlWr);		// n�o � NULL -> n�o � overlapped I/O

		WaitForSingleObject(WriteReady, INFINITE);
		_tprintf(TEXT("\nwrite concluido"));

		GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE); // sem WAIT 
		if (cbWritten < Msg_Sz)
			_tprintf(TEXT("\nWriteFile TALVEZ falhou. Erro = %d"), GetLastError());

		// --- Este teste deixa de fazer sentido porque WriteFile retorna // antes da opera��o concluir
		// if (!fSuccess) { 
		// _tprintf(TEXT("\nwriteFile TALVEZ falhou. Erro = %d"),


		_tprintf(TEXT("\nMessagem enviada"));
		// Esta parte do programa n�o l� nada 
		// quem faz isso � a (outra) thread

	}

	_tprintf(TEXT("\nEncerrar a thread ouvinte"));

	// Encerrar a outra thread (a thread "reader" - que l� o servidor) 
	DeveContinuar = 0;
	if (ReaderAlive) {
		WaitForSingleObject(hThread, 3000); // aguarda que a thread realmente encerre 
		_tprintf(TEXT("\nThread reder encerrada ou timeout"));
	}

	_tprintf(TEXT("\nCliente vai terminar liga��o e sair"));
	CloseHandle(WriteReady);
	CloseHandle(hPipe);
	pressEnter();
	return 0;
}

DWORD WINAPI ThreadClienteReader(LPVOID lpvParam) {
	Msg FromServer;

	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = (HANDLE)lpvParam; // a informa��o enviada � o handle

	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };

	if (hPipe == NULL) {
		_tprintf(TEXT("\nThread Reader - o handle recebido no param da thread � nulo\n"));
		return -1;
	}

	ReadReady = CreateEvent(
		NULL, // default security + non inheritable 
		TRUE, // Reset manual, por requisito do overlapped IO => uso de Reset Event 
		FALSE, // estado inicial = not signaled
		NULL); // nao precisa de nome. Uso interno ao processo 

	if (ReadReady == NULL) {
		_tprintf(TEXT("\nCliente: n�o foi poss�vel criar o Evento Read. Mais vale parar j�"));
		return 1;
	}

	// informa "resto do" cliente que est� tudo a andar 
	ReaderAlive = 1;
	_tprintf(TEXT("Thread Reader - a receber mensagens\n"));

	// Ciclo de di�logo com o cliente
	while (DeveContinuar) { // o habitual acerca de condi��o de termina��o

		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(
			hPipe,			// handle para o pipe (recebido no param) 
			&FromServer,	// buffer para os dados a ler 
			Msg_Sz,			// Tamanho msg a ler 
			&cbBytesRead,	// n�mero de bytes a ler 
			&OverlRd);		// != NULL -> � overlapped I/O

		WaitForSingleObject(ReadReady, INFINITE);
		_tprintf(TEXT("\nRead concluido"));

		// Testar se correu como esperado (servidor pode ter encerrado) 
		// if (!fSuccess || cbBytesRead < Msg_Sz) { 
		// Este teste � muito resumido 
		// o teste agora n�o pode ser feito desta forma

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE); // sem WAIT 
		if (cbBytesRead < Msg_Sz)
			_tprintf(TEXT("\nReadFile falhou. Erro = %d"), GetLastError());

		_tprintf(TEXT("\nServidor disse: [%s]"), FromServer.msg);

		// --> Isto s� l� servidor + processa mensagem. N�o escreve no pipe 
		// Esse envio � feito na thread "principal"
	}

	ReaderAlive = 0;

	// Esta thread nao fecha o pipe. 
	// o "resto do cliente" faz isso 
	_tprintf(TEXT("Thread Reader a terminar. \n"));
	return 1;
}
