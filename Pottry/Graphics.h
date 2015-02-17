#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <windows.h>
#include <ddraw.h> 
#include <io.h>
#include "myDDmacros.h"
#include <fstream>

using namespace std;

#define INITGUID // make sure directX guids are included

#define MAX_COLORS_PALETTE	 256
#define BITMAP_ID            0x4D42 // universal id for a bitmap


//------ default screen size -----------
#define SCREEN_WIDTH    800  // size of screen
#define SCREEN_HEIGHT   600
#define SCREEN_BPP      32   // bits per pixel
//------------------------------------


//---- struct to hold the bitmap image -----
typedef struct BITMAP_FILE_TAG
{
        BITMAPFILEHEADER bitmapfileheader;  // this contains the
                                            // bitmapfile header
        BITMAPINFOHEADER bitmapinfoheader;  // this is all the info
                                            // including the palette
        PALETTEENTRY     palette[256];		// we will store the palette here
        UCHAR            *buffer;			// this is a pointer to the data

} BITMAP_FILE, *BITMAP_FILE_PTR;






/*********************** FUNCTIONS here ***************************/


void Blit_Clipped(int x, int y,			//possition to draw bitmap
				  int width, int height,//size of bitmap in pixels
				  UCHAR *bitmap,		//pointer to bitmap data
				  UCHAR *video_buffer,	//pointer to video buffer surface
				  int	mempitch);		//video pitch per line


LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAW7 lpdd,			
									     LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list);

//Bitmap functions
// reads bitmap file and enters information into BITMAP_FILE_PTR structure
int	Load_Bitmap_File(BITMAP_FILE_PTR bitmap, WCHAR *filename);
// this function extracts a bitmap out of a bitmap file      
int Scan_Image_Bitmap_24to32(BITMAP_FILE_PTR bitmap, LPDIRECTDRAWSURFACE7 lpdds, int cx, int cy) ;
// clears the BITMAP_FILE_PTR structure
int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap);
// flips the upside-down the bitmap
int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height);
// this function creates an offscreen plain surface
LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(LPDIRECTDRAW7 lpdd,int width, int height,int mem_flags, int color_key = 0);
// this functions Blits off-screen surface to primary or backbuffer
int DDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source,int x, int y, int width, 
					   int height,LPDIRECTDRAWSURFACE7 dest,int transparent = 1);        

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,DWORD color);//Fills a surface with specific color
#endif



