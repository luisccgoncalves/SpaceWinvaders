#ifndef STRUCTS_H
#define STRUCTS_H

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

TCHAR SMName[] = TEXT("SWInvadersMem"); //Name of the shared memory, this has to be known between processes




#endif /* STRUCTS_H */
