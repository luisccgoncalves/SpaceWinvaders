#pragma warning(disable : 4996) //Disable deprecated function warnings, mainly string manipulation stuff
#include <windows.h> 
#include <stdio.h> 
#include <conio.h> 
#include <tchar.h> 
#include <io.h> // _setmode 
#include <fcntl.h> // macro para o _setmode

#define QUEMSZ 60
#define MSGTXTSZ 60

typedef struct { // estutura de mensagem (para ambos os sentidos da comunicação)
	TCHAR quem[QUEMSZ];
	TCHAR msg[MSGTXTSZ];
} Msg;

#define Msg_Sz sizeof(Msg)

// função auxiliary para ler caracteres em consola e de pouca relevância 
void readTChars(TCHAR * p, int maxchars) {
	int len; _fgetts(p, maxchars, stdin);
	len = _tcslen(p);
	if (p[len - 1] == TEXT('\n'))
		p[len - 1] = TEXT('\0');
}

// função auxiliar para fazer uma pausa de pouca relevância para o exemplo 
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

	// tenta abrir uma instância. Poderá ter que esperar
	while (1) {				// Tenta abrir o pipe repetidamente em ciclo

		hPipe = CreateFile(
			lpszPipename,							// Nome do pipe 
			GENERIC_READ |							// acesso read e write 
			GENERIC_WRITE,
			0 | FILE_SHARE_READ | FILE_SHARE_WRITE, // sem->com partilha 
			NULL,									// atributos de segurança = default 
			OPEN_EXISTING,							// É para abrir um pipe já existente 
			0 | FILE_FLAG_OVERLAPPED,				// atributos default 
			NULL);									// sem ficheiro template

		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		if (GetLastError() != ERROR_PIPE_BUSY) {
			_tprintf(TEXT("\nCreate file deu erro e não foi BUSY. Erro = %d\n"), GetLastError());
			pressEnter();
			return -1;
		}

		// Se chegou aqui é porque todas as instâncias 
		// do pipe estão ocupadas. Remédio: aguardar que uma 
		// fique livre com um timeout

		// Aguarda por instância no máximo de 30 segs (podia ser outro intervalo de tempo. 
		if (!WaitNamedPipe(lpszPipename, 30000)) {
			_tprintf(TEXT("Esperei por uma instância durante 30 segundos. Desisto. Sair"));
			pressEnter();
			return -1;
		}

	} // Fim do ciclo em que tenta abrir a instância do pipe

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

	hThread = CreateThread(
		NULL,					// Sem atributos de segurança
		0,						// Tam. de pilha default 
		ThreadClienteReader,	// Função da thread 
		(LPVOID)hPipe,			// Parâmetro para a thread = handle ...Dup (nada de Dup) 
		0,						// inicialmente não suspensa 
		&dwThreadId);			// Ptr p/ onde colocar ID da thread

	if (hThread == NULL) { // se a criação da thread deu erro
		_tprintf(TEXT("\nErro na criação da thread. Erro = %d"), GetLastError());
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
		_tprintf(TEXT("\nCliente: não foi possível criar o Evento. Mais vale parar já"));
		return 1;
	}

	_tprintf(TEXT("\nligação estabelecida. \"exit\" para sair"));

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
			&OverlWr);		// não é NULL -> não é overlapped I/O

		WaitForSingleObject(WriteReady, INFINITE);
		_tprintf(TEXT("\nwrite concluido"));

		GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE); // sem WAIT 
		if (cbWritten < Msg_Sz)
			_tprintf(TEXT("\nWriteFile TALVEZ falhou. Erro = %d"), GetLastError());

		// --- Este teste deixa de fazer sentido porque WriteFile retorna // antes da operação concluir
		// if (!fSuccess) { 
		// _tprintf(TEXT("\nwriteFile TALVEZ falhou. Erro = %d"),


		_tprintf(TEXT("\nMessagem enviada"));
		// Esta parte do programa não lê nada 
		// quem faz isso é a (outra) thread

	}

	_tprintf(TEXT("\nEncerrar a thread ouvinte"));

	// Encerrar a outra thread (a thread "reader" - que lê o servidor) 
	DeveContinuar = 0;
	if (ReaderAlive) {
		WaitForSingleObject(hThread, 3000); // aguarda que a thread realmente encerre 
		_tprintf(TEXT("\nThread reder encerrada ou timeout"));
	}

	_tprintf(TEXT("\nCliente vai terminar ligação e sair"));
	CloseHandle(WriteReady);
	CloseHandle(hPipe);
	pressEnter();
	return 0;
}

DWORD WINAPI ThreadClienteReader(LPVOID lpvParam) {
	Msg FromServer;

	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;
	HANDLE hPipe = (HANDLE)lpvParam; // a informação enviada é o handle

	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };

	if (hPipe == NULL) {
		_tprintf(TEXT("\nThread Reader - o handle recebido no param da thread é nulo\n"));
		return -1;
	}

	ReadReady = CreateEvent(
		NULL, // default security + non inheritable 
		TRUE, // Reset manual, por requisito do overlapped IO => uso de Reset Event 
		FALSE, // estado inicial = not signaled
		NULL); // nao precisa de nome. Uso interno ao processo 

	if (ReadReady == NULL) {
		_tprintf(TEXT("\nCliente: não foi possível criar o Evento Read. Mais vale parar já"));
		return 1;
	}

	// informa "resto do" cliente que está tudo a andar 
	ReaderAlive = 1;
	_tprintf(TEXT("Thread Reader - a receber mensagens\n"));

	// Ciclo de diálogo com o cliente
	while (DeveContinuar) { // o habitual acerca de condição de terminação

		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(
			hPipe,			// handle para o pipe (recebido no param) 
			&FromServer,	// buffer para os dados a ler 
			Msg_Sz,			// Tamanho msg a ler 
			&cbBytesRead,	// número de bytes a ler 
			&OverlRd);		// != NULL -> é overlapped I/O

		WaitForSingleObject(ReadReady, INFINITE);
		_tprintf(TEXT("\nRead concluido"));

		// Testar se correu como esperado (servidor pode ter encerrado) 
		// if (!fSuccess || cbBytesRead < Msg_Sz) { 
		// Este teste é muito resumido 
		// o teste agora não pode ser feito desta forma

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE); // sem WAIT 
		if (cbBytesRead < Msg_Sz)
			_tprintf(TEXT("\nReadFile falhou. Erro = %d"), GetLastError());

		_tprintf(TEXT("\nServidor disse: [%s]"), FromServer.msg);

		// --> Isto só lê servidor + processa mensagem. Não escreve no pipe 
		// Esse envio é feito na thread "principal"
	}

	ReaderAlive = 0;

	// Esta thread nao fecha o pipe. 
	// o "resto do cliente" faz isso 
	_tprintf(TEXT("Thread Reader a terminar. \n"));
	return 1;
}
