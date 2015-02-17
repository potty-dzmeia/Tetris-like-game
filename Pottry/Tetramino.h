#ifndef TETRAMINO_H
#define	TETRAMINO_H

#define DEBUG

#include <stdio.h>
#include <math.h>
#include "Graphics.h"
#include "MYASSERT.h"

//State of the tetramino
#define MOVING 1
#define STOPPED 2
#define REMOVE 3

//PlayGround starting graphical coordinates
#define PG_COORD_X	260	
#define PG_COORD_Y	  0

//Error Codes
#define OK_ERROR			 1
#define MOVE_ERROR		 	 101
#define GRAPHICS_ERROR	 	 102
#define ARRAY_ERROR			 103
#define MISSING_IMAGES_ERROR 104
#define END_GAME_ERROR		 105
#define UNKNOWN_ERROR		 120

//Possible directions for the Tetramino to move 
#define DOWN	1
#define LEFT	2
#define RIGHT	3

#define BLOCK_WIDTH	30	//Block width in pixels


//Block structure
typedef struct {
	int x;			//Coordinates of the block relative to the Tetramino center
	int y;			
}Block, *pBlock;

//The structure to hold all the information fot the playground area
typedef struct{
	int		state;//the play ground area is divied into cubes, if true the cube is occupied, else its empty
	int		type; //tells the type of the occupied cube
}PlayGround,*pPlayGround;	


/* Tetramino Class*/
class Tetramino
{
private:
	Block					blocks[4];		//Each Tetramino is made of four blocks
	int						begins_atX,		//the coordinates of the Tetramino in 
							begins_atY;		//the Playground array			

public:				
	bool					endgame;		//the game must end(used in the constructor)
	int						errorCode;
	int						type;		    //0,...,6
	
public:
	Tetramino(int type, PlayGround PG[][23]);//,LPDIRECTDRAW7 lpdd);
	~Tetramino(void);

    //------------------------------------------------
	void InitBlocks();//Depending of the tetramino type, returns the coordinates
					  //of each block 

	int Draw(LPDIRECTDRAWSURFACE7 lpDestinationSurface,//Draws the tetramino 
			 LPDIRECTDRAWSURFACE7 lpSourceSurface);// 
														 

	int Translate(int direction, PlayGround PG[][23]);
	int Rotate(int direction, PlayGround PG[][23]);
};


#endif