#define WIN32_LEAN_AND_MEAN	//No MFC
#define DEBUG
#define INITGUID

//#include <windows.h>
#include "WavFilePlayer.h"
#include "OtherMacros.h"
#include "Graphics.h"
#include "Tetramino.h"
#include "resource1.h"
#include "SoundDefinitions.h"	
//#include <windows.h>
//#include <strsafe.h>

//#include <objbase.h>

/*--------------Game Functions---------------------*/
int Game_Main(void *parms = NULL, int num_parms = 0);
int Game_Init(void *parms = NULL, int num_parms = 0);
int Game_Shutdown(void *parms = NULL, int num_parms = 0);
/*--------------------------------------------------*/

/*----------------Some Other Functions-----------------*/
LPDIRECTDRAWSURFACE7	lpDDBlockSurfaces[7];

//Loads all the 7 possible pictures in the lpDDBlockSurfaces[]
int LoadBlockSurfaces(LPDIRECTDRAWSURFACE7 &lpDDSurface,int type);
//Checks for full lines and deletes them 
int DeleteFullLines();		//returns how many line were deleted
int DrawIncomingTetramino();//draws the next tetramino to come
int DrawTexts();			//Draw all the texts: Records, speed, rows
void LoadScoreBoard();		//Loads the score board from file(score.pty)
void SaveScoreBoard();		//Saves the score board to file(score.pty)
void DrawGameOver();		//Draws the gameover.bmp
void StartNewGame();		//resets all the variables so a new game may start
void LoadSounds();			//Loads all the Sounds so they are ready to be played
void PlayGameSound(int SoundID);//Plays a specific sound, check in SoundDefinitions.h fo IDs
void StopGameSound(int SoundID);//Stops a specific sound, check in SoundDefinitions.h fo IDs
/*-----------------------------------------------------*/					 

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ScoreBoardDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK StartNewGameQuestionDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK StartTheGameNowDlgProc(HWND, UINT, WPARAM, LPARAM);
// GLOBALS ////////////////////////////////////////////////
HWND			main_window_handle = NULL;	// globally track main window
HINSTANCE		hinstance_app      = NULL;	// globally track hinstance

/* Variables for Game status */
bool     		bGameClosed=0;		  //if the game is to be closed
bool			bGameOver;			  //true/false if Game Over
bool			bGamePaused;		  //If games is paused is true
bool			bSound=1;			  //Game Sound on-1 or off-0	
int				iIncomingTetramino;   //The next tetramino to come
int				bTetraminoState;      //Shows if the current tetramino is moving
								      //1-if the tetramino is moving, 0-if it's stopped
DWORD           tetramino_speed;	  //Speed(in mseconds)- dificulty lvl
DWORD			tetramino_timing;	  //timer, so we know when to move the tetramino
DWORD			dwCurrentScore;		  //Shows the current score of the player
int			    iLastLineDroppedAt;	  //timer that shows how many seconds ago a line was dropped, if =-1 the games just started...
int				iDroppedLines;	      //Shows how many line has been dropped since game started

WavFilePlayer   *GameSounds;//Sounds for the game

BITMAP_FILE		bitmap_file;		  // a bitmap file


//The structure to hold the SCOREBOARD
struct {
	TCHAR	szName[MAX_PATH];
	DWORD	dwScore;
} ScoreBoard[3];


PlayGround	PG[12][23];
/*						10
PG[12][23];			0	|		|
						|		|
						|		|
				9___	|		|
						|_______| ___20
			

*/	
Tetramino		*currentTetr;

//it's used, so the input can't be to quick		
DWORD			key_down_cool_down,
				key_left_cool_down,
				key_right_cool_down,
				key_rotate_cool_down,
				key_drop_down_cool_down,
				key_pause_cool_down;


//--------directdraw stuff --------
LPDIRECTDRAW7         lpdd         = NULL;   // dd object
LPDIRECTDRAWSURFACE7  lpddsprimary = NULL;   // dd primary surface
LPDIRECTDRAWSURFACE7  lpddsback    = NULL;   // dd back surface
LPDIRECTDRAWSURFACE7  lpddsbackground    = NULL;   // dd back surface
LPDIRECTDRAWSURFACE7  lpddssoundmark    = NULL;//The mark that will be drawn if soudn is on
LPDIRECTDRAWCLIPPER   lpddclipper  = NULL;   // dd clipper
DDSURFACEDESC2        ddsd;                  // a direct draw surface description struct
DDBLTFX               ddbltfx;               // used to fill
DDSCAPS2              ddscaps;               // a direct draw surface capabilities struct
HRESULT               ddrval;                // result back from dd calls



//Some temporary variables
HRESULT			hResultTemp;
TCHAR			szBuffer[MAX_PATH];				// general printing buffer
int				iTemp;
size_t			size_tTemp;
DWORD		    dwTemp;

/*------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------*/




int Game_Init(void *parms, int num_parms){/****************************************************/
	
	//First load the Sounds
	LoadSounds();

	//Than setup the DirectX
	if(FAILED(DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL))){
		MessageBox(main_window_handle, TEXT("Error when calling DirectDrawCreateEx()"),TEXT("Error"), MB_OK);
		return 0;
	}

	if(FAILED(lpdd->SetCooperativeLevel(main_window_handle,DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX |
														   DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT))){
		MessageBox(main_window_handle, TEXT("Error when calling SetCooperativeLevel()"),TEXT("Error"), MB_OK);
		return(1);
	}

	if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,0,0)))
    {
		MessageBox(main_window_handle,  TEXT("Error when calling SetDisplayMode()"),TEXT("Error"), MB_OK);
		return(1);
	}

	// clear ddsd and set size
	DD_INIT_STRUCT(ddsd);

	// enable valid fields
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

	// set the backbuffer count field to 1
	ddsd.dwBackBufferCount = 1;

	// request a complex, flippable
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE |
                      DDSCAPS_COMPLEX | DDSCAPS_FLIP;

	// create the primary surface
	if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL))){
	    MessageBox(main_window_handle,  TEXT("Error when calling CreateSurface(primary)"),TEXT("Error"), MB_OK);
		return(0);
	}

	// now query for attached surface from the primary surface

	// this line is needed by the call
	ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

	if (FAILED(lpddsprimary->GetAttachedSurface(&ddsd.ddsCaps, &lpddsback))){
		MessageBox(main_window_handle,  TEXT("Error when calling CreateSurface(back)"),TEXT("Error"), MB_OK);
		return 0;
	}

	// set clipper up on back buffer since that's where well clip
	RECT screen_rect= {0,0,SCREEN_WIDTH-1,SCREEN_HEIGHT-1};
	lpddclipper = DDraw_Attach_Clipper(lpdd,lpddsback,1,&screen_rect);

	//-------------Load the StartPicture and draw it--------------
	GetCurrentDirectory(MAX_PATH,szBuffer);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Bitmaps\\StartWindow.bmp"));
	if(!Load_Bitmap_File(&bitmap_file, szBuffer)){
		MessageBox(main_window_handle,  TEXT("Couldn't load \"StartWindow.bmp\""),TEXT("Error"), MB_OK);
		return 0;
	}

	DD_INIT_STRUCT(ddsd);
	if(FAILED(lpddsprimary->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR||DDLOCK_WAIT, NULL))){
		MessageBox(main_window_handle,  TEXT("Couldn't lock primary surface"),TEXT("Error"), MB_OK);
		return 0;
	}

	// get video pointer to primary surfce
	DWORD *image_buffer = (DWORD *)ddsd.lpSurface;
 
		//copy 
	for(int y=0;y<SCREEN_HEIGHT; y++){
		for(int x=0; x<SCREEN_WIDTH; x++){
			UCHAR blue  =(bitmap_file.buffer[3*x+y*SCREEN_WIDTH*3+0]),
				  green =(bitmap_file.buffer[3*x+y*SCREEN_WIDTH*3+1]),
				  red	=(bitmap_file.buffer[3*x+y*SCREEN_WIDTH*3+2]);
			image_buffer[x+(y*ddsd.lPitch >> 2)]=_RGB32BIT(0, red, green, blue);
		}
	}


	if(FAILED(lpddsprimary->Unlock(NULL))){
		MessageBox(main_window_handle,  TEXT("Couldn't unlock primary surface"),TEXT("Error"), MB_OK);
		return 0;
	}
		
	Unload_Bitmap_File(&bitmap_file);
	//End loading the StartPicture
	
	
	PlayGameSound(7);
	//w8 for  few seconds
	Sleep(4000);

	//----------Create the off-screen surface for the background image----------
	//----------		and copy BackGround.bmp image there			  ----------
	if((lpddsbackground=DDraw_Create_Surface(lpdd,800,600,DDSCAPS_VIDEOMEMORY,-1))==NULL){
			MessageBox(main_window_handle,  TEXT("Couldn't create off-screen surface"),TEXT("Error"), MB_OK);
			return 0;
	}

	//Load the background.bmp image 
	GetCurrentDirectory(MAX_PATH,szBuffer);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Bitmaps\\BackGround.bmp"));
	if(!Load_Bitmap_File(&bitmap_file,szBuffer)){
		MessageBox(main_window_handle,  TEXT("Couldn't load \"BackGround.bmp.bmp\""),TEXT("Error"), MB_OK);
		return 0;
	}

			
	DD_INIT_STRUCT(ddsd);
	if(FAILED(lpddsbackground->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR||DDLOCK_WAIT, NULL))){
		MessageBox(main_window_handle,  TEXT("Couldn't lock() the off-screen surface"),TEXT("Error"), MB_OK);
		return 0;
	}
	DWORD *background=(DWORD *)ddsd.lpSurface;

	for(int y=0; y<SCREEN_HEIGHT; y++){
		for(int x=0; x<SCREEN_WIDTH; x++){
				UCHAR blue  =(bitmap_file.buffer[3*x+y*SCREEN_WIDTH*3+0]),
					  green =(bitmap_file.buffer[3*x+y*SCREEN_WIDTH*3+1]),
					  red	=(bitmap_file.buffer[3*x+y*SCREEN_WIDTH*3+2]);
			background[x+(y*ddsd.lPitch >> 2)]=_RGB32BIT(0, red, green, blue);
		}
	}

	Unload_Bitmap_File(&bitmap_file);
	
	//End filling the off-screen surface for the background
	if(FAILED(lpddsbackground->Unlock(NULL))){
		MessageBox(main_window_handle,  TEXT("Couldn't Unlock() the off-screen surface"),TEXT("Error"), MB_OK);
		return 0;
	}
	
	//Create surfaces for the blocks to hold the bitmaps
	for(int i=0; i<7;i++){
		if(!LoadBlockSurfaces(lpDDBlockSurfaces[i],i)){
			MessageBox(main_window_handle,  TEXT("Couldn't Load Block Surfaces"),TEXT("Error"), MB_OK);
			return 0;
		}
		if(!lpDDBlockSurfaces[i]){
			MessageBox(main_window_handle, TEXT("Couldn't Load Block Surfaces(surface null)"),TEXT("Error"), MB_OK);
			return 0;
		}
	}

	
    //----------Create the off-screen surface for the sound mark image----------
	//----------		and copy chovka.bmp image there			  ----------
	if((lpddssoundmark=DDraw_Create_Surface(lpdd,48,40,DDSCAPS_VIDEOMEMORY,1))==NULL){
		MessageBox(main_window_handle,  TEXT("Couldn't create off-screen surface"),TEXT("Error"), MB_OK);
		return 0;
	}

	//Load the background.bmp image 
	GetCurrentDirectory(MAX_PATH,szBuffer);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Bitmaps\\chovka.bmp"));
	if(!Load_Bitmap_File(&bitmap_file, szBuffer)){
		MessageBox(main_window_handle,  TEXT("Couldn't load \"chovka.bmp\""),TEXT("Error"), MB_OK);
		return 0;
	}
			
	DD_INIT_STRUCT(ddsd);
	if(FAILED(lpddssoundmark->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR||DDLOCK_WAIT, NULL))){
		MessageBox(main_window_handle,  TEXT("Couldn't lock() the off-screen surface- lpddssoundmark"),TEXT("Error"), MB_OK);
		return 0;
	}
	DWORD *chovchica=(DWORD *)ddsd.lpSurface;

	for(int y=0; y<40; y++){
		for(int x=0; x<48; x++){
				UCHAR blue  =(bitmap_file.buffer[3*x+y*48*3+0]),
					  green =(bitmap_file.buffer[3*x+y*48*3+1]),
					  red	=(bitmap_file.buffer[3*x+y*48*3+2]);
			chovchica[x+(y*ddsd.lPitch >> 2)]=_RGB32BIT(0, red, green, blue);
		}
	}

	Unload_Bitmap_File(&bitmap_file);
	
	//End filling the off-screen surface for the background
	if(FAILED(lpddssoundmark->Unlock(NULL))){
		MessageBox(main_window_handle,  TEXT("Couldn't Unlock() the off-screen surface- lpddssoundmark"),TEXT("Error"), MB_OK);
		return 0;
	}

	
	
	//Inits game variables
	StartNewGame();
    PlayGameSound(MUSIC_SOUND);
	return 1;
}




int Game_Main(void *parms, int num_parms){/****************************************************/

	
	if(bGameClosed)
	{
		return 1;
	}
	

	if(bGameOver){
		StartNewGame();
		bGameOver=false;
	}
	
	DWORD frame_time = GetTickCount();//get time so we can draw with 30fps

	/*---------------------------------------------------------------------------------------
									Get Input
	/---------------------------------------------------------------------------------------*/
	


	if(KEYDOWN(VK_ESCAPE)){	//Close game
		PostMessage(main_window_handle, WM_CLOSE, 0,0);
		bGameClosed=true;
		return 1;
	}

	
	if(KEYDOWN(0x50)){		//Pause is hitted (P key)
		if(GetTickCount()-key_pause_cool_down>400){
			if(bGamePaused==false)
				bGamePaused=true;
			else
				bGamePaused=false;
			key_pause_cool_down=GetTickCount();
		}	
	}
	if(bGamePaused){		//If games is Paused
		return 1;
	}


	if(KEYDOWN(VK_LEFT)){  //Move LEFT Tetramino
		if(GetTickCount()-key_left_cool_down>100){	
			key_left_cool_down=GetTickCount();           

			currentTetr->Translate(LEFT,PG);
		}
	}

	if(KEYDOWN(VK_RIGHT)){  //Move RIGHT Tetramino
		if(GetTickCount()-key_right_cool_down>100){	
			key_right_cool_down=GetTickCount();			

			currentTetr->Translate(RIGHT,PG);
		}
	}

	if(KEYDOWN(0x5A)){		//Rotate to the Left
		if(GetTickCount()-key_rotate_cool_down>150){	
			key_rotate_cool_down=GetTickCount();			
			PlayGameSound(ROTATE_SOUND);
			currentTetr->Rotate(LEFT,PG);
		}
	}

	if(KEYDOWN(0x58)){		//Rotate to the RIGHT
		if(GetTickCount()-key_rotate_cool_down>150){	
			key_rotate_cool_down=GetTickCount();			
			PlayGameSound(ROTATE_SOUND);
			currentTetr->Rotate(RIGHT,PG);
		}
	}

	/*Dealing with movement comes last...*/
	if(KEYDOWN(VK_DOWN)){	//Move Down Tetramino
		if(GetTickCount()-key_down_cool_down>50){	//The input can't be faster than 50ms				                    
			key_down_cool_down=GetTickCount();           
			
			//Move down the tetramino and get the current state
			bTetraminoState=currentTetr->Translate(DOWN,PG);
			
			tetramino_timing= GetTickCount();		//Restart the speed timer
		}
	}

	if(KEYDOWN(VK_SPACE) && bTetraminoState){		//Drop Down Tetramino(also it must be moving-state=1)
		if(GetTickCount()-key_drop_down_cool_down>200){	//The input cant be faster than 200ms				                    
			key_drop_down_cool_down=GetTickCount();          
			PlayGameSound(DROP_DOWN_SOUND);
			//Move the tetramino while stops(returns 0)
			while(currentTetr->Translate(DOWN,PG));
			bTetraminoState=false;//write the state so we can create new tetramino 
								//in the collision detection section
			tetramino_timing= GetTickCount();		//Restart the speed timer
		}
	}

	
	/*----------------------------------------------------------------------------------------
								  Animation
	/---------------------------------------------------------------------------------------*/
	
	//On every tetramino_speed(in mseconds), move the Tetramino
	if((GetTickCount() - tetramino_timing) > tetramino_speed){
		if(bTetraminoState)//If the tetramino is moving, make the periodical movement
			bTetraminoState=currentTetr->Translate(DOWN,PG);
		
		tetramino_timing= GetTickCount();
	}

	/*---------------------------------------------------------------------------------------
							  Collision detection
	/---------------------------------------------------------------------------------------*/
	
	int iLinesDeleted=0;//shows how many lines have been deleted, so later we can put appropriate sound
	/***If the Tetramino Stops: ***/ 
	if(!bTetraminoState){
		
		//Create new tetreamino
		delete(currentTetr);
		
		currentTetr=new Tetramino(iIncomingTetramino,PG);//Create another one
		//draw the next tetramino
		srand(GetTickCount());
		iIncomingTetramino=(rand()%7);
		
		bTetraminoState=true;//Set that the new tetramino is moving
		
		/*---------------------------- GAME OVER--------------------------------*/
		if(currentTetr->endgame){ 
		    WavFilePlayer *gameOverSound=new WavFilePlayer();
			PlayGameSound(GAME_OVER_SOUND);
			DrawGameOver();//draw the text Game Over
			Sleep(3000);	//w8 a sec
			//If the score is enought for the ScoreBoard show the dialog
			for(int i=0;i<3;i++)
				if(dwCurrentScore>ScoreBoard[i].dwScore){
					lpdd->FlipToGDISurface();
					DialogBox(hinstance_app,MAKEINTRESOURCE(IDD_DIALOG1),main_window_handle,ScoreBoardDlgProc);
					break;
				}
			SaveScoreBoard();//Save the scoreboard to file
			
			lpdd->FlipToGDISurface();
			//Ask the player if he/she wants to play again?
			//if No, the Dialog sets bGameClosed=1;   if Yes, do nothing
			DialogBox(hinstance_app,MAKEINTRESOURCE(IDD_DIALOG2),main_window_handle,StartNewGameQuestionDlgProc);
			if(bGameClosed==1)
				return 1;
			StartNewGame();//Start new game
			return 1;		
		}
		
		//Check the PG array for full lines and delete them
		iLinesDeleted=DeleteFullLines();
		
		iDroppedLines+=	iLinesDeleted;	  //Add the current deleted lines to the general count
		tetramino_speed-=5*iLinesDeleted; //Make the Tetramino move faster with every dropped line
		
		//Calculate the score 
		if(iLinesDeleted>0){
			dwCurrentScore+=(iLinesDeleted*iLinesDeleted*200000)/(GetTickCount()-iLastLineDroppedAt+1);
			iLastLineDroppedAt=GetTickCount();
		}
	}
		
	

	/*----------------------------------------------------------------------------------------
								Sound and Music
	/-----------------------------------------------------------------------------------------*/
	
	//Play the dropping line sounds
	if(iLinesDeleted==4)
		PlayGameSound(FOUR_LINES_DELETED_SOUND);
	else if(iLinesDeleted>0)
		PlayGameSound(LINE_DELETED_SOUND);

	

	/*----------------------------------------------------------------------------------------
								 Reader Frame
	/----------------------------------------------------------------------------------------*/
	
	//Draw background
	HRESULT hResultTemp=DDraw_Draw_Surface(lpddsbackground, 0,0,SCREEN_WIDTH, SCREEN_HEIGHT,
												lpddsback,0);
	if(FAILED(hResultTemp)){
		switch (hResultTemp){
			case DDERR_SURFACEBUSY:
					MessageBox(main_window_handle,  TEXT("Error in Blt(): DDERR_SURFACEBUSY"),TEXT("Error"), MB_OK);
					return 0;
			case DDERR_SURFACELOST:
					MessageBox(main_window_handle,  TEXT("Alt+Tab not supported at this time!"),TEXT("Note"), MB_OK);
					break;
			case DDERR_UNSUPPORTED:
					MessageBox(main_window_handle,  TEXT("Error in Blt(): DDERR_UNSUPPORTED"),TEXT("Error"), MB_OK);
					return 0;
			case DDERR_WASSTILLDRAWING:
					MessageBox(main_window_handle,  TEXT("Error in Blt(): DDERR_WASSTILLDRAWING"),TEXT("Error"), MB_OK);
					return 0;
			case DDERR_GENERIC:
					MessageBox(main_window_handle,  TEXT("Error in  Blt(): DDERR_GENERIC"),TEXT("Error"), MB_OK);
					return 0;
			default:
					MessageBox(main_window_handle,  TEXT("Error Blt(): else"),TEXT("Error"), MB_OK);
					return 0;
		}
	}//if

	//Draw the Tetramino
	hResultTemp=currentTetr->Draw(lpddsback,lpDDBlockSurfaces[currentTetr->type]);
	if(FAILED(hResultTemp)){
		MessageBox(main_window_handle,  TEXT("Error when drawing Tetramino"),TEXT("Error"), MB_OK);
		return 0;
	}
	
	//Draw PG[12][23] - draws all the busy blocks
	for(int x=0;x<12;x++){
		for(int y=0;y<23;y++){
			if(PG[x][y].state){//if block is occupied, draw it...
			 
				hResultTemp= DDraw_Draw_Surface( 
								lpDDBlockSurfaces[PG[x][y].type],	//source surface to draw
								PG_COORD_X+(x*BLOCK_WIDTH),			//position to draw at
								PG_COORD_Y+(y*BLOCK_WIDTH),			//position to draw at
								BLOCK_WIDTH, BLOCK_WIDTH,			//size of source surface
								lpddsback,							//destination surface
 								0);									//Use color_key

				if(FAILED(hResultTemp)){
					MessageBox(NULL,TEXT("Error when trying to draw a block from PG[][]."),TEXT("Error"),MB_OK);
					return 0;
				}
			}
		}//for
	}//for

	//Draw the Incoming Tetramino in the NEXT: area
	if(!DrawIncomingTetramino()){
		MessageBox(NULL,TEXT("Could not draw the incoming tetramino."),TEXT("Error"),MB_OK);
		return 0;
	}	

	//Draw the text 
	if(!DrawTexts()){
		MessageBox(NULL,TEXT("Could not draw the texts."),TEXT("Error"),MB_OK);
		return 0;
			
	}

	//Draw the Sound on/off Mark
	if(bSound){
		if(FAILED(DDraw_Draw_Surface(
							lpddssoundmark,  //The surface to hold the needed bitmap
							106,		//The x coordinates where the block must be placed
							266,//The x coordinates where the block must be placed
							48,	//width 
							40,	//height 
							lpddsback,		//destination surface
							1))){
			MessageBox(NULL,TEXT("Could not draw the the sound mark"),TEXT("Error"),MB_OK);
			return 0;
		}
	}


	// flip pages
	while (FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT)));



	//wait a sec
	while ((GetTickCount() - frame_time) < 30);

	return 1;
}



int Game_Shutdown(void *parms, int num_parms){
	//Close DirectDraw
	if(lpddsback){
		lpddsback->Release();
		lpddsback=NULL;
	}
	if(lpddsprimary){
		lpddsprimary->Release();
		lpddsprimary=NULL;
	}
	if(lpdd){
		lpdd->Release();
		lpdd=NULL;
	}
	
	for(int i=0;i<8;i++)
		GameSounds[i].ShutDown();

	return 1;
}


/**************************WINMain***************************/
int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, 
										LPSTR lpcmdline,int ncmdshow)
{
	HWND			hwnd;	     // window handle
	MSG				msg;		 // message
	WNDCLASSEX		winclass;

	// first fill in the window class stucture
	winclass.cbSize         = sizeof(WNDCLASSEX);
	winclass.style			= CS_DBLCLKS | CS_OWNDC | 
							  CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc	= WindowProc;
	winclass.cbClsExtra		= 0;
	winclass.cbWndExtra		= 0;
	winclass.hInstance		= hinstance;
	winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor		= LoadCursor(NULL, IDC_ARROW); 
	winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName	= NULL;
	winclass.lpszClassName	= TEXT("Pottry");
	winclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

	// save hinstance in global
	hinstance_app = hinstance;
	// register the window class
	if (!RegisterClassEx(&winclass))
		return(0);

	// create the window
	// create the window
	hwnd = CreateWindowEx(NULL,							// extended style
						winclass.lpszClassName,			// class
						TEXT("Pottry 0.9b"),			// title
						WS_POPUP | WS_VISIBLE,
					 	0,0,							// initial x,y
						SCREEN_WIDTH,SCREEN_HEIGHT,		// initial width, height
						NULL,							// handle to parent 
						NULL,							// handle to menu
						hinstance,						// instance of this application
						NULL);							// extra creation parms
	if(!hwnd){
		MessageBox(hwnd, TEXT("Could not create the Main Window!"), TEXT("Error"), MB_OK);
		return(0);
	}

	// save main window handle
	main_window_handle = hwnd;

	// initialize game here
	if(!Game_Init())
	{
		MessageBox(hwnd, TEXT("Error in Game_Init()"), TEXT("Error"), MB_OK);
		bGameClosed=1;
	}
	
	// enter main event loop
	while(TRUE)
	{
	   
		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			// test if this is a quit
			if(msg.message == WM_QUIT || bGameClosed){
				PlayGameSound(QUIT_SOUND);
				Sleep(3000);
				break;
			}
			// translate any accelerator keys
			TranslateMessage(&msg);
			// send the message to the window proc
			DispatchMessage(&msg);
		}

		// main game processing goes here
		if(!Game_Main()){
			MessageBox(hwnd, TEXT("Error in Game_Main()"), TEXT("Error"), MB_OK);
			break;
		}

	} // end while

Game_Shutdown();

return(msg.wParam);
} // end WinMain



LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
PAINTSTRUCT		ps;		
HDC				hdc;
int				iX, iY;
switch(msg)
	{	
	case WM_CREATE:  
			
			return(0);

	case WM_LBUTTONDOWN:
			
			iX=	LOWORD(lparam);
			iY= HIWORD(lparam);
			
			//if QUIT button was pressed
			if(iX>21 && iX<161 && iY>356 && iY<422)
				bGameClosed=1;
			//->restart<- button was pressed
			if(iX>38 && iX<103 && iY>313 && iY<323)
				StartNewGame();
			//Sound button was pressed
			if(iX>13 && iX<74 && iY>281 && iY<293){
				if(bSound){
					bSound=0;
					//if the sound is OFF, turn off the music
				    StopGameSound(MUSIC_SOUND);
				}
				else {
					bSound=1;
				    PlayGameSound(MUSIC_SOUND);
				}
			}
			return 0;

	case WM_PAINT: 	
   			hdc = BeginPaint(hwnd,&ps);	 

			EndPaint(hwnd,&ps);
			return(0);

	case WM_DESTROY: 
		
			PostQuitMessage(0);
			return(0);

    }

	return (DefWindowProc(hwnd, msg, wparam, lparam));
} // end WinProc

int LoadBlockSurfaces(LPDIRECTDRAWSURFACE7 &lpDDSurface,int type){
	
	TCHAR szCurrentDir[MAX_PATH];
	
	/*Creates surface and loads a bitmap(that must be called for each of the 7 blocks) */

	ASSERT(lpdd!=0 && type>=0 && type<7);	//test the argument

	//bitmap_file.buffer=NULL;

	//Create the off-screen surface
	//-and copy BackGround.bmp image there-
	if((lpDDSurface=DDraw_Create_Surface(lpdd,BLOCK_WIDTH,BLOCK_WIDTH,DDSCAPS_VIDEOMEMORY,-1))==NULL){
		return 0;
	}

	if(!DDraw_Fill_Surface(lpDDSurface, _RGB32BIT(0, 0,0,0))){
		return 0;
	}

	//-Load the bmp image -
	//-The files are from type: tetramino(TYPE).bmp-
	GetCurrentDirectory(MAX_PATH,szCurrentDir);
	
	StringCchPrintf(szBuffer,MAX_PATH, TEXT("//Bitmaps//tetramino%d.bmp"), type);
	
	StringCchCat(szCurrentDir,MAX_PATH,szBuffer);

	if(!Load_Bitmap_File(&bitmap_file, szCurrentDir)){
		return 0;
	}
			
	DD_INIT_STRUCT(ddsd);
	if(FAILED(lpDDSurface->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR||DDLOCK_WAIT, NULL))){
		return 0;
	}

	DWORD *surface=(DWORD *)ddsd.lpSurface;
	UCHAR *bitmap_buffer=(UCHAR *)bitmap_file.buffer;

	for(int y=0; y<BLOCK_WIDTH; y++){
		
		for(int x=0; x<BLOCK_WIDTH; x++){
		/*	UCHAR blue  =(bitmap_file.buffer[3*x+3*y*BLOCK_WIDTH+0]),
				  green =(bitmap_file.buffer[3*x+3*y*BLOCK_WIDTH+1]),
				  red	=(bitmap_file.buffer[3*x+3*y*BLOCK_WIDTH+2]);
			surface[x+y*(ddsd.lPitch >> 2)]=_RGB32BIT(0, red, green, blue);
		*/
			/*!!!!First, each line of data must end on a DWORD(4 byte) boundary. 
			If the width of your image does not fall on a DWORD boundary, 
			then it will be padded with zeros to fill it out. For example, 
			if you have an image that is 254x254, the DWORD boundary for this image is 256. 
			The image data loaded from this file will be 256x254. Before the data can be used,
			the padding must be removed*/

			UCHAR	blue  = *(bitmap_buffer+0),
					green = *(bitmap_buffer+1),
					red   = *(bitmap_buffer+2);
			*surface=_RGB32BIT(0, red, green, blue);
			
			surface++;
			bitmap_buffer+=3;
		}
		surface=(DWORD *)ddsd.lpSurface+y*(ddsd.lPitch>>2);//move to the next line of the surface
											// >>2 equals /4, because each pixel is 4bytes
		bitmap_buffer+=6;
	}
		
	Unload_Bitmap_File(&bitmap_file);

	if(FAILED(lpDDSurface->Unlock(NULL))){
		return 0;
	}
	return 1;
}


int DeleteFullLines(){
	/*This functions checks the PG array for full lines and deletes them
	  The return value is number of lines deleted, if 0 - no lines were deleted*/
	
	int iFullBlock=0;			//counts how many blocks in the line are busy
	int iFullRows=0;			//shows how many lines are full and have been deleted


	//Check which rows are to be deleted
	for(int y=0;y<20;y++){//for every row
		
		for(int x=0;x<10;x++){
			if(PG[x][y].state)
				iFullBlock++;
		}
		//when full line is found: Move all the lines above it one row down 
		if(iFullBlock==10){
			
			for(int i=y;i>0;i--){
				for(int z=0;z<10;z++){
					PG[z][i].state=PG[z][i-1].state;//Move the block one row down
					PG[z][i].type =PG[z][i-1].type;
					
					if(i==1){//if we reach the second line, zero all the elements in the first one.
						PG[z][i-1].state=0;
						PG[z][i-1].type=-1;
					}
				}
			}
			
			iFullRows++;
		}//when full block is found
		
		iFullBlock=0;
	}//for every row


	return iFullRows;
}
int DrawIncomingTetramino(){
	/*Draws the next tetramino to come. Reads the value of iIncomingTetramino and
	  draws it at coordinates: x=150, y=510.
	  To know what orientation has the tetramino when it's comming look in the
	  Class Tetramino - InitBlocks().
	  To draw the different blocks we read from the already loaded surface
	  lpDDBlockSurfaces[7].
	
	  The function is very long written...but I want to keep the design clear.
	*/
	
	//The position(on the display) where the tetramino will be drawn
	int iStartsAtX=150,iStartsAtY=540;

	
	for(int i=0;i<7;i++)
		ASSERT(lpDDBlockSurfaces[i]);

	//If the Tetramino is from the type 0(long one)
	if(iIncomingTetramino==0){
		
		/*Looking at Class Tetramino function InitBlocks() we see how tetramino type 0
		is inited: 
		blocks[3].x=0;		//			   
		blocks[3].y=-2;		//			    
							
		blocks[2].x=0;		//
		blocks[2].y=-1;		//
							//
		blocks[1].x=0;
		blocks[1].y=0;

		blocks[0].x=0;
		blocks[0].y=1;
		We use the information to draw the blocks of the incoming tetramino
		*/
		
		//draw block 0 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],  //The surface to hold the needed bitmap
							iStartsAtX,		//The x coordinates where the block must be placed
							iStartsAtY+(-2*BLOCK_WIDTH),//The x coordinates where the block must be placed
							BLOCK_WIDTH,	//width of the building block
							BLOCK_WIDTH,	//height of the building block
							lpddsback,		//destination surface
							0);				//
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 0) block 0."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 1 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 0) block 1."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 0) block 2."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY+BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 0) block 3."),TEXT("Error"),MB_OK);
			return 0;
		}
	}

	else if(iIncomingTetramino==1){//---------------------------1
		//draw block 0 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 1) block 0."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 1 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 1) block 1."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 1) block 2."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 1) block 3."),TEXT("Error"),MB_OK);
			return 0;
		}		
	}

	else if(iIncomingTetramino==2){//------------------------2
		//draw block 0 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 2) block 0."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 1 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 2) block 1."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 2) block 2."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY+BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 2) block 3."),TEXT("Error"),MB_OK);
			return 0;
		}
	}

	else if(iIncomingTetramino==3){//-----------------------------3
		//draw block 0 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 3) block 0."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 1 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 3) block 1."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 3) block 2."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY+BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 3) block 3."),TEXT("Error"),MB_OK);
			return 0;
		}
	}

	else if(iIncomingTetramino==4){//--------------------------4
		//draw block 0 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 4) block 0."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 1 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 4) block 1."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 4) block 2."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY+BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 4) block 3."),TEXT("Error"),MB_OK);
			return 0;
		}					
	}

	else if(iIncomingTetramino==5){//----------------------5
			//draw block 0 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 5) block 0."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 1 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 5) block 1."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 5) block 2."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY+BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 5) block 3."),TEXT("Error"),MB_OK);
			return 0;
		}			
	}

	else if(iIncomingTetramino==6){//---------------------6
			//draw block 0 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY-BLOCK_WIDTH,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 6) block 0."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 1 
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX+BLOCK_WIDTH,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 6) block 1."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 6) block 2."),TEXT("Error"),MB_OK);
			return 0;
		}	

		//draw block 2
		hResultTemp = DDraw_Draw_Surface(
							lpDDBlockSurfaces[iIncomingTetramino],
							iStartsAtX-BLOCK_WIDTH,
							iStartsAtY,
							BLOCK_WIDTH,
							BLOCK_WIDTH,
							lpddsback,
							0);
		if(FAILED(hResultTemp)){
			MessageBox(NULL,TEXT("Could not draw the incoming tetramino(type 6) block 3."),TEXT("Error"),MB_OK);
			return 0;
		}					
	}
	else{
		MessageBox(NULL,TEXT("Error iIncomingTetramino value."),TEXT("Error"),MB_OK);
		return 0;
	}
	
	return 1;
}
int DrawTexts(){
	/*This function draws all the text on the Offscreen surface. */
	HDC		xdc;
	
	//Get the DC for offscreen surface
 	if(FAILED(lpddsback->GetDC(&xdc))){
		MessageBox(NULL,TEXT("Could not get the DC for the offscreen surface"),TEXT("Error"),MB_OK);
		return 0;
	}
	SetTextColor(xdc,RGB(230,230,100));
	SetBkMode(xdc,TRANSPARENT);

	//Draw the speed text
	StringCchPrintf(szBuffer, MAX_PATH,TEXT("%d"), tetramino_speed);
	StringCchLength(szBuffer,MAX_PATH,&size_tTemp);
	TextOut(xdc,716,524,szBuffer, size_tTemp);

	//Draw the dropped lines text
	StringCchPrintf(szBuffer, MAX_PATH,TEXT("%d"), iDroppedLines);
	StringCchLength(szBuffer,MAX_PATH,&size_tTemp);
	TextOut(xdc,675,567,szBuffer, size_tTemp);

	//Draw current score:
	StringCchPrintf(szBuffer, MAX_PATH,TEXT("%d"), dwCurrentScore);
	StringCchLength(szBuffer,MAX_PATH,&size_tTemp);
	TextOut(xdc,680,483,szBuffer, size_tTemp);

	//Draw the score board
	//Score 1
	StringCchLength(ScoreBoard[0].szName,MAX_PATH,&size_tTemp);
	TextOut(xdc,608,149,ScoreBoard[0].szName, size_tTemp);

	StringCchPrintf(szBuffer, MAX_PATH,TEXT("%d"), ScoreBoard[0].dwScore);
	StringCchLength(szBuffer,MAX_PATH,&size_tTemp);
	TextOut(xdc,702,149,szBuffer, size_tTemp);

	//Score 2
	StringCchLength(ScoreBoard[1].szName,MAX_PATH,&size_tTemp);
	TextOut(xdc,608,197,ScoreBoard[1].szName, size_tTemp);

	StringCchPrintf(szBuffer, MAX_PATH,TEXT("%d"), ScoreBoard[1].dwScore);
	StringCchLength(szBuffer,MAX_PATH,&size_tTemp);
	TextOut(xdc,702,197,szBuffer, size_tTemp);
	
	//Score 3
	StringCchLength(ScoreBoard[2].szName,MAX_PATH,&size_tTemp);
	TextOut(xdc,616,238,ScoreBoard[2].szName, size_tTemp);

	StringCchPrintf(szBuffer, MAX_PATH,TEXT("%d"), ScoreBoard[2].dwScore);
	StringCchLength(szBuffer,MAX_PATH,&size_tTemp);
	TextOut(xdc,712,238,szBuffer, size_tTemp);

	
	lpddsback->ReleaseDC(xdc);
	return 1;
}


void LoadScoreBoard(){
/*This function loads the score board(into ScoreBoard struct) from file(score.pty)*/
	
	HANDLE hFile;

	//Open file to read the score board
	hFile=CreateFile(TEXT("score.pty"), GENERIC_READ, 0, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	
	//if file can not be open init the ScoreBoard[3] struct
	if(hFile==INVALID_HANDLE_VALUE){
		for(int i=0;i<3;i++){
			StringCchCopy(ScoreBoard[i].szName, MAX_PATH, TEXT(""));
			ScoreBoard[i].dwScore=0;
		}
		CloseHandle(hFile);
		return;
	}

	//if could not read from the file, init the score board
	if(!ReadFile(hFile,ScoreBoard, sizeof(ScoreBoard), &dwTemp, NULL) || dwTemp!=sizeof(ScoreBoard))
	{
		for(int i=0;i<3;i++){
			StringCchCopy(ScoreBoard[i].szName, MAX_PATH, TEXT("Empty"));
			ScoreBoard[i].dwScore=0;
		}
		CloseHandle(hFile);
		return;
	}
			
	CloseHandle(hFile);
	return;
}
void SaveScoreBoard(){
	/*Saves the score board(from ScoreBoard struct) to file(score.pty)*/

	HANDLE hFile;

	//Open file to read the score board
	hFile=CreateFile(TEXT("score.pty"), GENERIC_WRITE, 0, NULL,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
	if(hFile==INVALID_HANDLE_VALUE){
		MessageBox(NULL,TEXT("Could not open score.pty"),TEXT("ERROR"),MB_OK);
		return;
	}

	WriteFile(hFile, ScoreBoard, sizeof(ScoreBoard), &dwTemp,NULL);
	if(dwTemp!=sizeof(ScoreBoard)){
		MessageBox(NULL,TEXT("Could not write the ScoreBoard "),TEXT("ERROR"),MB_OK);
		return;
	}
	CloseHandle(hFile);
	return;
}

BOOL CALLBACK ScoreBoardDlgProc (HWND hDlg, UINT message, 
                            WPARAM wParam, LPARAM lParam)
{
	 static int iCurrentStanding;
     
	 //Pause the game so we dont draw over the DialogBox
	 //bGamePaused=true;

	 switch (message)
     {
     case WM_INITDIALOG :
		 SetFocus(GetDlgItem(hDlg, IDC_NAME));
		 //Limit the text for the edit box
		 SendMessage(GetDlgItem(hDlg, IDC_NAME),(UINT)EM_LIMITTEXT,(WPARAM)10,0);

		 //get the currents standing
		 for(iCurrentStanding =0;iCurrentStanding<3;iCurrentStanding++)
			if(dwCurrentScore>ScoreBoard[iCurrentStanding].dwScore)
				break;

		 switch(iCurrentStanding){
			 case 0:
				SetWindowText(hDlg,TEXT("New Record!!!"));
				//Move the other two standing down
				ScoreBoard[2].dwScore=ScoreBoard[1].dwScore;
				StringCchCopy(ScoreBoard[2].szName,MAX_PATH,ScoreBoard[1].szName);

				ScoreBoard[1].dwScore=ScoreBoard[0].dwScore;
				StringCchCopy(ScoreBoard[1].szName,MAX_PATH,ScoreBoard[0].szName);
				break;
			 case 1:
				 SetWindowText(hDlg,TEXT("Second Place!!"));
				 //Move the second standing one place down
				 ScoreBoard[2].dwScore=ScoreBoard[1].dwScore;
				 StringCchCopy(ScoreBoard[2].szName,MAX_PATH,ScoreBoard[1].szName);
				 break;
			 case 2:
				 SetWindowText(hDlg,TEXT("Third Place!"));
				 break;
		 }
		 return TRUE ;
          
     case WM_COMMAND :
		 switch (LOWORD (wParam))
          {
          case IDC_BUTTON1 :
          case IDCANCEL :
			  GetWindowText(GetDlgItem(hDlg, IDC_NAME),ScoreBoard[iCurrentStanding].szName,MAX_PATH);
			  ScoreBoard[iCurrentStanding].dwScore=dwCurrentScore;
			  SaveScoreBoard();//Save the score-board
			  //UnPause the game
			  //bGamePaused=false;
			  EndDialog (hDlg, 0) ;
              return TRUE ;
          }
          break ;
     }
     return FALSE ;
}

void DrawGameOver(){
	//make sure the surfaces are valid
	ASSERT(lpdd && lpddsprimary);

	DDSURFACEDESC2		ddsd;
	
	LPDIRECTDRAWSURFACE7 lpDDSurfaceTemp;

	//Create the surface for the gameover.bmp
	lpDDSurfaceTemp=DDraw_Create_Surface(lpdd, 360, 144, DDSCAPS_VIDEOMEMORY, 1);
	if(!lpDDSurfaceTemp){
		MessageBox(NULL,TEXT("Could not create lpDDSurfaceTemp"),TEXT("ERROR"),MB_OK);
		abort();
	}

	//Load the bitmap
	GetCurrentDirectory(MAX_PATH,szBuffer);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Bitmaps\\gameover.bmp"));
	StringCchCopy(szBuffer,MAX_PATH, szBuffer);
	if(!Load_Bitmap_File(&bitmap_file, szBuffer)){
		MessageBox(NULL,TEXT("Could not find gameover.bmp."),TEXT("ERROR"),MB_OK);
		abort();
	}


	DD_INIT_STRUCT(ddsd);
	if(FAILED(lpDDSurfaceTemp->Lock(NULL, &ddsd,DDLOCK_SURFACEMEMORYPTR||DDLOCK_WAIT, NULL))){
		MessageBox(NULL,TEXT("Error when locking lpDDSurfaceTemp."),TEXT("ERROR"),MB_OK);
		abort();
	}

	DWORD *surface=(DWORD *)ddsd.lpSurface;
	UCHAR *bitmap_buffer=(UCHAR *)bitmap_file.buffer;

	for(int y=0; y<144; y++){
		for(int x=0; x<360; x++){
			UCHAR	blue  = *(bitmap_buffer+0),
					green = *(bitmap_buffer+1),
					red   = *(bitmap_buffer+2);
			*surface=_RGB32BIT(0, red, green, blue);
			
			surface++;
			bitmap_buffer+=3;
		}
		surface=(DWORD *)ddsd.lpSurface+y*(ddsd.lPitch>>2);//move to the next line of the surface
		//bitmap_buffer+=6;
	}
		
	Unload_Bitmap_File(&bitmap_file);

	if(FAILED(lpDDSurfaceTemp->Unlock(NULL))){
		MessageBox(NULL,TEXT("Error when unlocking lpDDSurfaceTemp."),TEXT("ERROR"),MB_OK);
		abort();
	}

	DDraw_Draw_Surface(lpDDSurfaceTemp, 231,214,350,140,lpddsprimary,1);


}

void StartNewGame(){
//This function inits all  Game variables, Tetramino and PG stuff----
//So a new game may start
	
	//Load the score board from the file
	LoadScoreBoard();
    
	lpdd->FlipToGDISurface();
	//Asko the player if ready to play?
	DialogBox(hinstance_app,MAKEINTRESOURCE(IDD_DIALOG3),main_window_handle,StartTheGameNowDlgProc);
	
	//Init the playground structure
	for(int x=0;x<12;x++)
		for(int y=0;y<23;y++){
			PG[x][y].state=0;
			PG[x][y].type=-1;
		}

	//Create the Tetramino object
	currentTetr=new Tetramino(iIncomingTetramino, PG);
	//draw the next tetramino
	srand(GetTickCount());
	iIncomingTetramino=rand()%7;

	if(!currentTetr){
		MessageBox(main_window_handle,  TEXT("Couldn't create Tetreamino object"),TEXT("Error"), MB_OK);
		abort();
	}
	
	//Init Game variables here:
	bGameClosed		=false;	//if the game is to be closed
	bGamePaused		=false;	//If games is paused is true
	bTetraminoState	=true;	//Shows if the current tetramino is moving
	bGameOver		=false;	//1-if the tetramino is moving, 0-if it's stopped	
	
	tetramino_speed	=600;	//Init the tetramino speed(move per miliseconds)
	iLastLineDroppedAt	=-1;//timer that shows when the game did start, if =-1 the games just started...
	iDroppedLines	=0;		//Shows how many line has been dropped sin
	tetramino_timing=GetTickCount();//Init tetramino_timing
	dwCurrentScore	=0;		//Set the current score to 0

	//Init key_cool_down timing
	key_down_cool_down		=GetTickCount();
	key_left_cool_down		=GetTickCount();
	key_right_cool_down		=GetTickCount();
    key_rotate_cool_down	=GetTickCount();
	key_drop_down_cool_down	=GetTickCount();
	key_pause_cool_down		=GetTickCount();

	iLastLineDroppedAt=GetTickCount();//Start the counter which shows how many secs ago a line was dropped
}
BOOL CALLBACK StartNewGameQuestionDlgProc (HWND hDlg, UINT message, 
                            WPARAM wParam, LPARAM lParam)
{
/*ThisDialog Asks the player if he/she wants to play again*/
	 switch (message)
     {
     case WM_INITDIALOG :
		 SetFocus(hDlg);
		 return TRUE ;
          
     case WM_COMMAND :
		 switch (LOWORD (wParam))
          {
          case IDC_YES :
			  EndDialog (hDlg, 0) ;
			  return TRUE ;
		  case IDC_NOO :
			  bGameClosed=1;
			  EndDialog (hDlg, 0) ;
              return TRUE ;
          }
          break ;
     }
     return FALSE ;
}

BOOL CALLBACK StartTheGameNowDlgProc (HWND hDlg, UINT message, 
                            WPARAM wParam, LPARAM lParam)
{
/*ThisDialog Asks the player if he/she is ready to start the Game*/
	 switch (message)
     {
     case WM_INITDIALOG :
		 SetFocus(hDlg);
		 return TRUE ;
          
     case WM_COMMAND :
		 switch (LOWORD (wParam))
          {
          case IDC_START :
			  EndDialog (hDlg, 0) ;
			  return TRUE ;
          }
          break ;
     }
     return FALSE ;
}


void LoadSounds(){
	TCHAR szCurrentDir[MAX_PATH];
	GameSounds= new WavFilePlayer[8];
	//Create the objects for all the sounds
	//GameSounds[0].
	GetCurrentDirectory(MAX_PATH,szCurrentDir);
	
	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\MUSIC_SOUND.wav"));
	GameSounds[0].LoadWavFile(szBuffer);
	
	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\GAME_OVER_SOUND.wav"));
	GameSounds[1].LoadWavFile(szBuffer);

	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\ROTATE_SOUND.wav"));
	GameSounds[2].LoadWavFile(szBuffer);

	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\DROP_DOWN_SOUND.wav"));
	GameSounds[3].LoadWavFile(szBuffer);

	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\LINE_DELETED_SOUND.wav"));
	GameSounds[4].LoadWavFile(szBuffer);
	
	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\FOUR_LINES_DELETED_SOUND.wav"));
	GameSounds[5].LoadWavFile(szBuffer);
	
	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\QUIT_SOUND.wav"));
	GameSounds[6].LoadWavFile(szBuffer);
	
	StringCchCopy(szBuffer,MAX_PATH,szCurrentDir);
	StringCchCat(szBuffer,MAX_PATH,TEXT("\\Sounds\\GAME_START_SOUND.wav"));
	GameSounds[7].LoadWavFile(szBuffer);
	

}

void PlayGameSound(int SoundID){
	if(!bSound)
		return;
	
	//IF the Music is started, looping play is needed
	if(SoundID==MUSIC_SOUND){
		GameSounds[SoundID].Play(main_window_handle,1);
		return;
	}
	
	//else normal playing
	GameSounds[SoundID].Play(main_window_handle);
}

void StopGameSound(int SoundID){
	GameSounds[SoundID].Stop();
}