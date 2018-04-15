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
	int		bombs;			//number of bombs available to drop
	bool randpath;			//true for random trajectory, false for zig-zag
}invader;






#endif /* STRUCTS_H */
