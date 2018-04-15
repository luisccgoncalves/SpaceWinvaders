#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
	int		id;				
	int		score;
	int		lives;			//ship is a one shot kill, but has several lives
	int		x;				//ship x,y position
	int		y;

	//powerups (only player specific)
	bool	shield;			//If shield is true, lives won't go down.
	bool	drunk;			//If true, controls are inverted.
	bool	turbo;			//Player will move faster.
	//add more
}playership;

typedef struct {
	int		x;				//ship x,y position
	int		y;
	int		hp;				//ship hit points
	int		bombrate;		//bomb drop rate
	bool	randpath;		//true for random trajectory, false for zig-zag
}invader;

typedef struct {
	int		speed;
}invaderbomb;

typedef struct {
	int		speed;
}shipshot;

typedef struct {
	int		matrix[20][20];
}map;






#endif /* STRUCTS_H */
