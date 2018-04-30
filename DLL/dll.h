#include <windows.h>
#include <tchar.h>
//Definir uma constante para facilitar a leitura do protótipo da função
//Este .h deve ser incluído no projeto que o vai usar (modo implícito)
#define TAM 256

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		x_init;			//ship x,y initial position
	int		y_init;			//needed for relative coordinates
	int		hp;				//ship hit points
	int		bombrate;		//bomb drop rate
	int		rand_path;		//true for random trajectory, false for zig-zag
}invader;

typedef struct {							////Message to use in @ server view
	invader			pSMem;					//Object type to use in the memory
}SMServer_MSG;


	//Variável global da DLL
	extern DLL_IMP_API int nDLL;

	DLL_IMP_API int sharedMemory(HANDLE *hSMem, TCHAR SMName[], LARGE_INTEGER SMemSize);
	DLL_IMP_API int mapServerView(SMServer_MSG *pSMemServer, HANDLE *hSMem, LARGE_INTEGER SMemViewServer);
