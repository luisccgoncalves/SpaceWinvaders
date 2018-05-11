#include <windows.h>
#include <tchar.h>

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#define SMEM_NAME		TEXT("SWInvadersMem")		//Name of the shared memory
#define SMALL_BUFF		20							//Used for small strings (Ex:username)
#define XSIZE			80							//Terminal max collumn number
#define YSIZE			25							//Terminal max row number

#define MAX_PLAYERS		1							//Maximum number of concurrent players
#define MAX_INVADER		57							//Maximum invaders by level
#define RAND_INVADER	2							//Number of random path invaders
#define INVADER_SPEED	1000						//Regular path invader speed in miliseconds

#define INVADER_BY_ROW	11							//Number of maximum invaders by row

typedef struct {
	int		id;
	TCHAR	username[SMALL_BUFF];
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
}Ship;

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
	int		matrix[XSIZE][YSIZE];			//collision detection
	int		invader_speed;					//invader speed multiplier
	int		ship_shot_spd;					//Ship shot speed multiplier
}map;

typedef struct {							//Message to use in the game data view
	invader			invad[MAX_INVADER];		//Array of maximum number invaders at one time
	invaderbomb		bomb;
	Ship			ship[MAX_PLAYERS];
}SMGameData;

typedef struct {							//Message to use in the message view
	int		msgID;							
	int		owner;							//player1, player2, server, gateway, etc..
	int		instruction;					//movement, powerUp, shutdown, updateHighscore, etc...
	int		details;						//up, down, right, left, highscore, etc...

	//if this aproach is to follow we need to consider constants
}SMMessage;

typedef struct {
	HANDLE			hSMem;					//Handle to shared memory
	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file

	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
	LARGE_INTEGER	SMemViewServer;			//Stores the size of the view

	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
	LARGE_INTEGER	SMemViewGateway;		//Stores the size of the view
	
	SMGameData		*pSMemGameData;			//Pointer to shared memory's structure
	SMMessage		*pSMemMessage;			//Pointer to shared memory's first byte

	HANDLE			mhStructSync;				//Handle to mutex (TEST)
	int				ThreadMustGoOn;			//Flag for thread shutdown
} SMCtrl;

	DLL_IMP_API int sharedMemory(HANDLE * hSMem, LARGE_INTEGER * SMemSize);

	DLL_IMP_API int mapMsgView(SMCtrl *smCtrl);							//Maps Msg area - ALL ACCESS
	DLL_IMP_API int mapGameDataView(SMCtrl *smCtrl, DWORD permission);	//Maps GameData area - READ/WRITE


