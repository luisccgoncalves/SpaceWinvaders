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

#define XSIZE 80  //---------------(?)--------------
#define YSIZE 25

typedef struct {
	int		id;
	TCHAR	username[20];
	int		high_score;
}player;

typedef struct {
	int		id;
	int		score;
	int		lives;			//ship is a one shot kill, but has several lives
	int		x;				//ship x,y position
	int		y;
	player	owner;

	//powerups (only player specific)
	int		shield;			//If shield is true, lives won't go down.
	int		drunk;			//If true, controls are inverted.
	int		turbo;			//Player will move faster. -------(?)-------
	int		laser_shots;	//kills all invaders in sight
							//add more
}ship;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		x_init;			//ship x,y initial position
	int		y_init;			//needed for relative coordinates
	int		hp;				//ship hit points
	int		bombrate;		//bomb drop rate
	int		rand_path;		//true for random trajectory, false for zig-zag
}invader;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		hp;
}barriers;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		speed;
}invaderbomb;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		speed;
}ship_shot;

typedef struct {
	int		matrix[XSIZE][YSIZE];
	int		invader_speed;	//invader speed multiplier
	int		ship_shot_spd;	//Ship shot speed multiplier
}map;

typedef struct {							////Message to use in @ server view
	invader			pSMem;					//Object type to use in the memory
}SMServer_MSG;

typedef struct {							//Message to use in @ gateway view
	char			pSMem;					//Object type to use in the memory
}SMGateway_MSG;

typedef struct {
	HANDLE			hSMem;					//Handle to shared memory
	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file

	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
	LARGE_INTEGER	SMemViewServer;			//Stores the size of the view
	SMServer_MSG	*pSMemServer;			//Pointer to shared memory's first byte

	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns gateway about updates in shared memory
	LARGE_INTEGER	SMemViewGateway;		//Stores the size of the view
	SMServer_MSG	*pSMemGateway;			//Pointer to shared memory's first byte
} SMCtrl;


	//Variável global da DLL
	extern DLL_IMP_API int nDLL;

	DLL_IMP_API int sharedMemory(SMCtrl *smCtrl, TCHAR SMName[]);
	DLL_IMP_API int mapServerView(SMCtrl *smCtrl);
	DLL_IMP_API int mapGatewayView(SMCtrl *smCtrl);
