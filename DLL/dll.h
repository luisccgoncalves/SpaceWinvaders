#include <windows.h>
#include <tchar.h>

#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

#define SMEM_NAME		TEXT("SWInvadersMem")		//Name of the shared memory
#define STRUCT_SYNC		TEXT("Structsync")			//Name of the struct sync mutex 
#define	EVE_BOOT		TEXT("LetsBoot")
#define EVE_1ST_PIPE	TEXT("1stPipeInstance")
#define	EVE_SERV_UP		TEXT("SMServerUpdate")
#define	EVE_GATE_UP		TEXT("SMGatewayUpdate")

#define PIPE_NAME		TEXT("\\\\.\\pipe\\SpaceWPipe")

#define SMALL_BUFF		20							//Used for small strings (Ex:username)
#define	SMEM_BUFF		10
#define XSIZE			80							//Terminal max collumn number
#define YSIZE			25							//Terminal max row number

#define MAX_PLAYERS		3							//Maximum number of concurrent players
#define MAX_INVADER		57							//Maximum invaders by level
#define INVADER_SPEED	1000						//Regular path invader speed in miliseconds
#define MAX_BOMBS		6							//Maximum bombs per invaders by level (TEMP: 10% invaders - consider min cases)

typedef struct {

	int		id;							//populated with pid of each client
	TCHAR	username[SMALL_BUFF];		//probably needed for remote pipe usage
	TCHAR	password[SMALL_BUFF];		//unhashed password
	int		high_score;

	int		score;
	int		lives;			//ship is a one shot kill, but has several lives
	int		x;				//ship x,y position
	int		y;
	
	//powerups (only player specific)
	int		shield;			//If shield is true, lives won't go down.
	int		drunk;			//If true, controls are inverted.
	int		turbo;			//Player will move faster. -------(?)-------
	int		laser_shots;	//kills all invaders in sight
							//add more
} Ship;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		x_init;			//ship x,y initial position
	int		y_init;			//needed for relative coordinates

	int		hp;				//ship hit points
	int		rand_path;		//true for random trajectory, false for zig-zag
} Invader;

//typedef struct {
//	int		x;				//ship x,y position
//	int		y;
//	int		hp;
//} Barriers;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		fired;			//dead or alive
	//int		speed;		//May not be sensible to have it here - RECONSIDER
} PowerUp;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		fired;			//dead or alive
	//int		speed;		//May not be sensible to have it here - RECONSIDER
} InvaderBomb;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		fired;			//dead or alive
	int		speed;
} ShipShot;

typedef struct {							//Game data to use in pipes
	Invader			invad[MAX_INVADER];		//Array of maximum number invaders at one time
	InvaderBomb		bomb[MAX_BOMBS];		//Percent of bombers (until some defined minimum)
	Ship			ship[MAX_PLAYERS];		//number of ships/players in game
	ShipShot		shot[25];				//temporary number of shots
	PowerUp			pUp;					//One powerUp only at any given time

	int xsize;								//max y size of play area
	int ysize;								//max x size of play area

	/*This are needed on server only*/
	int invaders_speed;						//invaders speed
	int ship_speed;							//ship speed

	int projectiles_speed;					//bombs, shots and powerUps base speed

	/*Eventualy find a way to use them from projectiles speed*/
	int invaders_bombs_speed;
	int ship_shot_speed;					//defenders shot speed
	int power_up_speed;
	/**/

	int num_players;						//number of players per game
	int max_invaders;						//total of invaders
	int max_rand_invaders;					//number of rand invaders
	int max_bombs;							//max boms on game (%invaders?)

} GameData;

typedef struct {
	//int		msgID;						//probably unnecessary - event driven approach	
	int		owner;							//player1, player2, server, gateway, etc..
	int		instruction;					//up, down, right, left, fire, shutdown,etc...	

	//maybe a string?
	//if this aproach is to follow we may need to consider constants
}Packet;

typedef struct {							//Message to use in the message view

	Packet	buffer[SMEM_BUFF];

}SMMessage;

typedef struct {
	HANDLE			hSMem;					//Handle to shared memory
	LARGE_INTEGER	SMemSize;				//Stores the size of the mapped file

	HANDLE			hSMServerUpdate;		//Handle to event. Warns gateway about updates in shared memory
	LARGE_INTEGER	SMemViewServer;			//Stores the size of the view

	HANDLE			hSMGatewayUpdate;		//Handle to event. Warns server about updates in shared memory
	LARGE_INTEGER	SMemViewGateway;		//Stores the size of the view
	
	GameData		*pSMemGameData;			//Pointer to shared memory's structure server>gateway
	SMMessage		*pSMemMessage;			//Pointer to shared memory's structure gateway<>server

	HANDLE			mhStructSync;			//Handle to mutex, grants pSMemGameData integrity
	HANDLE			mhGameData;				//Handle to mutex to control GameData read and write
	HANDLE			mhProdConsMut;			//Handle to mutex, grants psMemMsg integrity
	HANDLE			shVacant;				//Handle to vacants fields semaphor
	HANDLE			shOccupied;				//Handle to occupied fields semaphor

	int				ThreadMustGoOn;			//Flag for thread shutdown

	GameData		gameData;				//structure that holds the local game
	SMMessage		msg;					//structure that holds the local msg
} SMCtrl;

typedef struct {							//Message to use in pipes
	TCHAR		username[SMALL_BUFF];		//string		
	TCHAR		password[SMALL_BUFF];		//string
	int			logged;						//BOOL
	int			SGMsg;

}PipeMsgs;

	DLL_IMP_API int sharedMemory(HANDLE * hSMem, LARGE_INTEGER * SMemSize);

	DLL_IMP_API int mapMsgView(SMCtrl *smCtrl);							//Maps Msg area - ALL ACCESS
	DLL_IMP_API int mapGameDataView(SMCtrl *smCtrl, DWORD permission);	//Maps GameData area - READ/WRITE

	/**/
	DLL_IMP_API HANDLE createGameDataMutex();
	DLL_IMP_API HANDLE createProdConsMutex();
	DLL_IMP_API HANDLE createOccupiedSemaphore();
	DLL_IMP_API HANDLE createVacantSemaphore();

	DLL_IMP_API Packet consumePacket(SMCtrl *smCtrl, int *next);
	DLL_IMP_API int writePacket(SMCtrl *smCtrl, int *nextIn, Packet localPacket);

	DLL_IMP_API int writeGameData(HANDLE *sharedMemory, HANDLE *localGame, HANDLE *mutex);
