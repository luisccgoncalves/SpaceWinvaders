#include <windows.h>
#include <tchar.h>

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#define SMEM_NAME		TEXT("SWInvadersMem")		//Name of the shared memory
#define STRUCT_SYNC		TEXT("Structsync")			//Name of the struct sync mutex name

#define SMALL_BUFF		20							//Used for small strings (Ex:username)
#define XSIZE			80							//Terminal max collumn number
#define YSIZE			25							//Terminal max row number

#define MAX_PLAYERS		1							//Maximum number of concurrent players
#define MAX_INVADER		57							//Maximum invaders by level
#define INVADER_SPEED	1000						//Regular path invader speed in miliseconds
#define MAX_BOMBS		6							//Maximum bombs per invaders by level (TEMP: 10% invaders - consider min cases)

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

//typedef struct {
//	int		x;				//ship x,y position
//	int		y;
//	int		hp;
//}barriers;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		fired;			//dead or alive
	//int		speed;		//May not be sensible to have it here - RECONSIDER
}powerup;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		fired;			//dead or alive
	//int		speed;		//May not be sensible to have it here - RECONSIDER
}invaderbomb;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		fired;			//dead or alive
	int		speed;
}ship_shot;

typedef struct {							//Message to use in the game data view
	invader			invad[MAX_INVADER];		//Array of maximum number invaders at one time
	invaderbomb		bomb[MAX_BOMBS];		//Percent of bombers (until some defined minimum)
	ship			ship[MAX_PLAYERS];		//number of ships/players in game
	ship_shot		shot[25];				//temporary number of shots
	powerup			pUp;					//One powerUp only at any given time

	int xsize;						//max y size of play area
	int ysize;						//max x size of play area
	int invaders_speed;				//invaders speed
	int invaders_bombs_speed;
	int ship_shot_speed;			//defenders shot speed
	int power_up_speed;				
	
	int num_players;				//number of players per game
	int max_invaders;
	int max_bombs;

}SMGameData;

typedef struct {							//Message to use in the message view
	//int		msgID;						//probably unnecessary - event driven approach	
	int		owner;							//player1, player2, server, gateway, etc..
	int		instruction;					//movement, powerUp, shutdown, updateHighscore, etc...
	int		details;						//up, down, right, left, highscore, etc...

	//if this aproach is to follow we may need to consider constants
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


