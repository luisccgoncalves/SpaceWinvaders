#define _CRT_RAND_S
#include <stdlib.h>

#include <windows.h>
#include <tchar.h>
#include <time.h>


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

#define PIPE_NAME		TEXT("\\\\.\\pipe\\SpaceWPipe")
#define REG_SUBKEY		TEXT("Software\\SpaceWinvaders\\HighScores")

#define SMALL_BUFF		20							//Used for small strings (Ex:username)
#define	SMEM_BUFF		10

#define XSIZE			800							//Terminal max collumn number
#define YSIZE			600							//Terminal max row number

#define INV_WIDTH		10
#define INV_HEIGHT		6
#define PROJ_SIZE		5
#define SHIP_WIDTH		20
#define SHIP_HEIGHT		10

#define MAX_PLAYERS		5							//Maximum number of concurrent players
#define MAX_INVADER		20							//Maximum invaders by level
#define MAX_BOMBS		30							//Maximum bombs in the screen at any time for one invader
#define MAX_SHOTS		999							//Maximum shots a defender can have on the screen at same time
#define RAND_INVADER	10							//Number of random path invaders

#define INVADER_BY_ROW	11							//Number of maximum invaders by row

#define INVADER_SPEED	100							//Regular path invader speed in miliseconds
#define PROJECTL_SPEED	20							//Base speed for Powerups and invader bombs
#define	BOMB_RATE		100							//Number of steps between bomb launches 
#define SHOT_RATE		500							//Number that represents shots for milliseconds
#define MOVE_RATE		4							//Number that represents defender movement for milliseconds

#define POWERUP_DUR		10000						//Duration of a powerup buff

typedef struct {
	int		x;										//ship x,y position
	int		y;
	int		width;									//ship width and height
	int		height;
	int		fired;									//dead or alive
	int		speed;									//Not being used right now
} ShipShot;

typedef struct {

	int			id;									//populated with pid of each client
	TCHAR		username[SMALL_BUFF];				//probably needed for remote pipe usage
	//TCHAR		password[SMALL_BUFF];				//unhashed password

	int			lives;								//ship is a one shot kill, but has several lives
	int			x;									//ship x,y position
	int			y;
	int			width;								//ship width and height
	int			height;

	ShipShot	shots[MAX_SHOTS];					//shots per ship
	DWORD		shotTimeStamp;						//stores last shot timestamp
	DWORD		moveTimeStamp;						//stores last ship movement timestamp
	
	//powerups (only player specific)
	int			shield;								//If shield is true, lives won't go down.
	int			drunk;								//If true, controls are inverted.

	int			turbo;								//Player will move faster. -(Ours)-
	int			laser_shots;						//kills all invaders in sight -(Ours)-
} Ship;

typedef struct {
	TCHAR	username[SMALL_BUFF];
	int		Id;										//Unique ID from client

	BOOL	isReady;								//Ready to play flag
}Player;

typedef struct {
	int			x;									//ship x,y position
	int			y;
	int			width;								//ship width and height
	int			height;
	int			fired;								//dead or alive
} InvaderBomb;

typedef struct {
	int			x;									//ship x,y position
	int			y;
	int			width;								//ship width and height
	int			height;

	int			x_init;								//ship x,y initial position
	int			y_init;								//needed for relative coordinates

	InvaderBomb		bomb[MAX_BOMBS];		
	int				bombRateCounter;				//Steps into invaders path (to bomb rate)
	
	int				hp;								//ship hit points
	int				rand_path;						//true for random trajectory, false for zig-zag
	int				direction;						//ship movement

	} Invader;

//typedef struct {
//	int		x;				//ship x,y position
//	int		y;
//	int		hp;
//} Barriers;

typedef struct {
	int			x;									//ship x,y position
	int			y;
	int			width;								//ship width and height
	int			height;
	int			fired;								//dead or alive
	int			type;								//0-shield 1-drunk 2-turbo 3-laser_shot
	int			duration;
} PowerUp;

typedef struct {									//structure to use on regestry
	TCHAR	timestamp[SMALL_BUFF];
	DWORD	score;
}HighScore;

typedef struct {									//Game data to use in communication
	int				gameRunning;			
	int				gameCreated;

	Invader			invad[MAX_INVADER];				//Array of maximum number invaders at one time
	Ship			ship[MAX_PLAYERS];				//number of ships in game
	Player			logged[MAX_PLAYERS];			//Numer of players logged on server (playing or not)
	PowerUp			pUp;							//One powerUp only at any given time

	int				xsize;							//max y size of play area
	int				ysize;							//max x size of play area

	/*Environment variables*/
	int				invaders_speed;	  				//invaders speed
	int				ship_speed;		  				//ship speed
	int				projectiles_speed;				//bombs, shots and powerUps base speed
	int				ship_shot_speed;				//defenders shot speed

	int				shotRate;						//shots launch rate
	int				bombRate;		  				//bomb drop rate
	int				moveRate;						//defenders rate

	int				num_players;					//number of players per game
	int				max_invaders;					//total of invaders
	int				max_rand_invaders;				//number of rand invaders
	int				max_bombs;						//max boms on game (%invaders?)

	int				pup_duration;
	int				ice;							//Powerup effect invaders (stops all for some time)
	int				plusSpeed;						//PowerUp effect invaders(speeds up inv for some time)
	int				battery;						//PowerUp effect speed up shots (all players)

	int	score;										//actual score if game is happening
	HighScore	top10[10];							//Top 10 highest scores
} GameData;

typedef struct {
	int		owner;									//Index in ship array
	DWORD	Id;										//Unique ID from client
	int		instruction;							//up, down, right, left, fire, shutdown,etc...	

	TCHAR	username[SMALL_BUFF]; 
}Packet;

typedef struct {									//Message to use in the message view
	Packet	buffer[SMEM_BUFF];						//Big buffer
}SMMessage;

typedef struct {
	HANDLE			hSMem;							//Handle to shared memory
	LARGE_INTEGER	SMemSize;						//Stores the size of the mapped file

	HANDLE			hSMServerUpdate;				//Handle to event. Warns gateway about updates in shared memory
	LARGE_INTEGER	SMemViewServer;					//Stores the size of the view
	LARGE_INTEGER	SMemViewGateway;				//Stores the size of the view
	
	GameData		*pSMemGameData;					//Pointer to shared memory's structure server>gateway
	SMMessage		*pSMemMessage;					//Pointer to shared memory's structure gateway<>server

	HANDLE			mhStructSync;					//Handle to mutex, grants pSMemGameData integrity
	HANDLE			mhGameData;						//Handle to mutex to control GameData read and write
	HANDLE			mhProdConsMut;					//Handle to mutex, grants psMemMsg integrity
	HANDLE			shVacant;						//Handle to vacants fields semaphor
	HANDLE			shOccupied;						//Handle to occupied fields semaphor

	int				ThreadMustGoOn;					//Flag for thread shutdown

	GameData		localGameData;					//structure that holds the local game
} SMCtrl;

typedef struct {
	int players, maxInvaders, hardInvaders;
	int invadersSpeed, projectileSpeed, pUpDuration;
	int bombRate, shotRate, moveRate;
} ConfigurableVars;

	DLL_IMP_API int sharedMemory(HANDLE * hSMem, LARGE_INTEGER * SMemSize);

	DLL_IMP_API int mapMsgView(SMCtrl *smCtrl);														//Maps Msg area - ALL ACCESS
	DLL_IMP_API int mapGameDataView(SMCtrl *smCtrl, DWORD permission);								//Maps GameData area - READ/WRITE

	/* This section creates the mutexes and semaphores needed to */
	/* Semi-abstract the send and read funcions below */
	DLL_IMP_API HANDLE createGameDataMutex();
	DLL_IMP_API HANDLE createProdConsMutex();
	DLL_IMP_API HANDLE createOccupiedSemaphore();
	DLL_IMP_API HANDLE createVacantSemaphore();

	DLL_IMP_API Packet consumePacket(SMCtrl *smCtrl);												//Read from Consumer-Productor style array
	DLL_IMP_API int writePacket(SMCtrl *smCtrl, Packet localPacket);								//Write(copy) in to Consumer-Productor style array

	DLL_IMP_API GameData consumeGameData(GameData *sharedMemory, HANDLE *mutex);					//Read from shared memory
	DLL_IMP_API int writeGameData(GameData *sharedMemory, GameData *localGame, HANDLE mutex);		//Write(copy) in to shared memory

	DLL_IMP_API int RandomValue(int value);															//Self explanatory - [0, value-1]