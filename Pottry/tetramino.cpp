#include "Tetramino.h"


//-------------------------Constructor------------------------
Tetramino::Tetramino(int type,PlayGround PG[][23])//,LPDIRECTDRAW7 lpdd)
{
	ASSERT(/*lpdd!=NULL &&*/ PG!=NULL);	//test the arguments

	ASSERT(type>=0 && type<=6);//check for legal types
	
	this->type=type;
	endgame=false;

	//Set the starting point of the tetramino
	//relative to the PG[][] struct.
	begins_atX=4;
	begins_atY=2;

	//Init each of the four building blocks
	InitBlocks();
	
	//Write the tetramino into PG struct
	for(int i=0;i<4; i++){
		//if TRUE, there is no more room in the PLayground, so the game must end.
		if(PG[begins_atX+blocks[i].x][begins_atY+blocks[i].y].state==1)
			endgame=true;
	}
}


//-------------------------------Destructor-----------------------------
Tetramino::~Tetramino(void)
{

}


//Draws the current tetramino into the lpDestinationSurface
int Tetramino :: Draw(LPDIRECTDRAWSURFACE7 lpDestinationSurface,LPDIRECTDRAWSURFACE7 lpSourceSurface){
	
	ASSERT(lpDestinationSurface!=NULL && lpSourceSurface!=NULL);
	
	//darw each of the four blocks
	for(int i=0; i<4;i++){
			HRESULT hResultTemp= DDraw_Draw_Surface(lpSourceSurface,  // source surface to draw
					PG_COORD_X+((begins_atX+blocks[i].x)*BLOCK_WIDTH),// position to draw at
					PG_COORD_Y+((begins_atY+blocks[i].y)*BLOCK_WIDTH),// position to draw at
					BLOCK_WIDTH, BLOCK_WIDTH,						  // size of source surface
					lpDestinationSurface,						      //destination surface
 					0);										          //Use color_key
			if(FAILED(hResultTemp)){
				errorCode=GRAPHICS_ERROR;
				return hResultTemp;
			}
	}

	return 1;
}

int Tetramino ::Translate(int direction, PlayGround PG[][23]){
	
	ASSERT(PG!=NULL);
	ASSERT(direction==DOWN || direction==LEFT || direction==RIGHT);
	

	//if Moving down:
	switch(direction){
		case DOWN:
				
			for(int i=0; i<4; i++){//for every block: 	
				//---Test if the new block hits another block - stop the tetramino
				if(PG[begins_atX+blocks[i].x][begins_atY+blocks[i].y+1].state){
					
					for(int i=0;i<4;i++){//Make the blocks into the PG array valid and set their type
						PG[begins_atX+blocks[i].x][begins_atY+blocks[i].y].state=1;
						PG[begins_atX+blocks[i].x][begins_atY+blocks[i].y].type=type;
					}
					return 0;//Show that the tetramino has stopped...
				}
				
				//---Test if the the new block is going out of the PLayground(stop the tetramino)
				if(begins_atY+blocks[i].y+1>19){
					
					for(int i=0;i<4;i++){//Make the blocks into the PG array valid and set their type
						PG[begins_atX+blocks[i].x][begins_atY+blocks[i].y].state=1;
						PG[begins_atX+blocks[i].x][begins_atY+blocks[i].y].type=type;
					}
					return 0;//Show that the tetramino has stopped...
				}	
			}
			
			//If all OK - move the coordinates of the tetramino
			begins_atY++;		
			break;

		case LEFT:
			
			for(int i=0; i<4; i++){//for every block 
				
				//---Test if the block hits another block(do nothing)---
				if(PG[begins_atX+blocks[i].x-1][begins_atY+blocks[i].y].state==1)		
					return 1;
		
				//---Test if the the block is going out of the PLayground(do nothing)---
				if(begins_atX+blocks[i].x-1<0){	
					return 1;
				}
			}
			
			//Now it's OK to move the coordinates of the tetramino
			begins_atX--;
			break;

		case RIGHT:
			
			for(int i=0; i<4; i++){//for every block starting from  
				
				//---Test if the block hits another block(do nothing)---
				if(PG[begins_atX+blocks[i].x+1][begins_atY+blocks[i].y].state==1)
					return 1;
		
				//---Test if the the block is going out of the PLayground(do nothing)---
				if(begins_atX+blocks[i].x+1>9)
					return 1;
			}
			
			//Move the coordinates of the tetramino
			begins_atX++;
			break;
	}


	return 1;
}

int Tetramino :: Rotate(int direction, PlayGround PG[][23]){
	
	ASSERT(PG!=NULL);
	ASSERT(direction==RIGHT || direction==LEFT);

	int newX, newY;
	double dAngle = 3.1415926535/2;
	
	//Debuging	
/*	char szBuffer[255];
	

	fstream logfile("logfile.log", ios::out);
	logfile.seekp(0,ios::end);
	sprintf(szBuffer,"(int)sin(90)= %d" ,
        (int)sin((float)pi/2));
	logfile<<szBuffer;

	logfile.close();
*/

	if(direction==RIGHT)
		dAngle=dAngle;
	else
		dAngle=-dAngle;

	if(type==1)//If a cube, do not rotate;
		return 1;
	
	else {
		
		for(int i=0;i<4;i++){//get the new coordinates of the rotation, and check them
			if(blocks[i].x==0 && blocks[i].y==0)//if the block has coordinates (0,0)- there's no need rotating it
				continue;
			newX=blocks[i].x*((int)cos(dAngle))-blocks[i].y*((int)sin(dAngle));//rotation formulas
			newY=blocks[i].x*((int)sin(dAngle))+blocks[i].y*((int)cos(dAngle));

			//test the new coordinates if PG array if busy
			if(PG[begins_atX+newX][begins_atY+newY].state)//if not free, do not rotate and..
				return 1;
		
			//test the new coordinates if going out of boundary 
			if(begins_atX+newX<0 || begins_atX+newX>9 || begins_atY+newY>19)
				return 1;
		}//for

		//If all the coordinates are free, rotate the tetramino
		for(int i=0;i<4;i++){//rotate every column except the one with coordinates(0,0)
			if(blocks[i].x==0 && blocks[i].y==0)//if the block has coordinates (0,0)- there's no need rotating it
				continue;
			newX=blocks[i].x*((int)cos(dAngle))-blocks[i].y*(int)sin(dAngle);//rotation formulas
			newY=blocks[i].x*((int)sin(dAngle))+blocks[i].y*(int)cos(dAngle);
			
			blocks[i].x=newX; 
			blocks[i].y=newY;
		}
	}
	return 1;
}



void Tetramino :: InitBlocks(){
							//					0
	if(type==0){			//block type:		|
							//			   -2 	L (block3)  
		blocks[3].x=0;		//			   -1	L (block2)
		blocks[3].y=-2;		//			    0	L (block1)	 
							//				1   L (block0)
		blocks[2].x=0;		//
		blocks[2].y=-1;		//
							//
		blocks[1].x=0;
		blocks[1].y=0;

		blocks[0].x=0;
		blocks[0].y=1;

		return;
	}

	else if(type==1){		//					 0	
							//block type:		 |
							//			   -1	L L
		blocks[3].x=-1;		//				0->	L L
		blocks[3].y=-1;		//			  		  	
							//
		blocks[2].x=0;		//
		blocks[2].y=-1;		//
							
		blocks[1].x=-1;		//
		blocks[1].y=0;		//
			
		blocks[0].x=0;
		blocks[0].y=0;

		return;

	}
	else if(type==2){
					
		blocks[3].x=0;		//			LL LL
		blocks[3].y=-1;		//			LL  	
							//
		blocks[2].x=-1;		//
		blocks[2].y=-1;		//
							
		blocks[1].x=-1;		//
		blocks[1].y=0;		//
			
		blocks[0].x=-1;
		blocks[0].y=1;
		return;

	}
	else if(type==3){

		blocks[3].x=-1;		//		 LL LL 
		blocks[3].y=-1;		//			LL  	
							//			LL  	
							//
		blocks[2].x=0;		//
		blocks[2].y=-1;		//
							
		blocks[1].x=0;		//
		blocks[1].y=0;		//
			
		blocks[0].x=0;
		blocks[0].y=1;
		return ;

	}
	else if(type==4){
		blocks[3].x=-1;		//			LL 
		blocks[3].y=-1;		//			LL LL
							//			   LL	
							//
		blocks[2].x=-1;		//
		blocks[2].y=0;		//
							
		blocks[1].x=0;		//
		blocks[1].y=0;		//
			
		blocks[0].x=0;
		blocks[0].y=1;
		return;

	}
	else if(type==5){
		blocks[3].x=0;		//			   LL
		blocks[3].y=-1;		//			LL LL
							//			LL   	
							//
		blocks[2].x=0;		//
		blocks[2].y=0;		//
							
		blocks[1].x=-1;		//
		blocks[1].y=0;		//
			
		blocks[0].x=-1;
		blocks[0].y=1;
		return;

	}
	else if(type==6){
		blocks[3].x=0;		//			   LL	
		blocks[3].y=-1;		//			LL LL LL
							//			  
							//
		blocks[2].x=1;		//
		blocks[2].y=0;		//
							
		blocks[1].x=0;		//
		blocks[1].y=0;		//
			
		blocks[0].x=-1;
		blocks[0].y=0;
		return;

	}
}
