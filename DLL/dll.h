#include <windows.h>
#include <tchar.h>

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#define XSIZE 80  //---------------(?)--------------
#define YSIZE 25

#define MAX_INVADER		55
#define RAND_INVADER	2
#define INVADER_SPEED	1000

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

typedef struct {							//Message to use in @ server view
	invader			invad[MAX_INVADER];		//Array of maximum number invaders at one time
}SMServer_MSG;

typedef struct {							//Message to use in @ gateway view
	char			pSMem;					//Object type to use in the memory
}SMGateway_MSG;

typedef struct {
	HANDLE			hSMem;					//Handle to shared memory
	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file

	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
	LARGE_INTEGER	SMemViewServer;			//Stores the size of the view
	SMServer_MSG	*pSMemServer;			//Pointer to shared memory's structure

	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
	LARGE_INTEGER	SMemViewGateway;		//Stores the size of the view
	SMGateway_MSG	*pSMemGateway;			//Pointer to shared memory's first byte
} SMCtrl;

TCHAR SMName[] = TEXT("SWInvadersMem");		//Name of the shared memory, this has to be known between processes

	DLL_IMP_API int sharedMemory(SMCtrl *smCtrl);
	DLL_IMP_API int mapServerView(SMCtrl *smCtrl);
	DLL_IMP_API int mapGatewayView(SMCtrl *smCtrl);

	DLL_IMP_API int mapMsgView(SMCtrl *smCtrl); //Maps Msg area - ALL ACCESS
	DLL_IMP_API int mapWriteGameDateView(SMCtrl *smCtrl); //Maps Msg area - WRITE
	DLL_IMP_API int mapReadGameDataView(SMCtrl *smCtrl); //Maps Msg area - READ

