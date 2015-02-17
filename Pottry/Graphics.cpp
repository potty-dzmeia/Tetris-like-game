#include "Graphics.h"


inline void Plot_Pixel_Faster16(int x, int y, int red, int green, int blue,
								unsigned short *video_buffer, int lpitch16)
{
	// this function plots a pixel in 16-bit color mode
	// assuming that the caller already locked the surface
	// and is sending a pointer and byte pitch to it

	// first build up color WORD
	unsigned short pixel=_RGB16BIT565(red, green, blue);

	//write the data
	video_buffer[x+y*lpitch16]=pixel;
}

inline void Plot_Pixel_24(int x, int y, int red, int green, int blue,
								unsigned short *video_buffer, int lpitch16)
{
	// this function plots a pixel in 24-bit color mode
	// assuming that the caller already locked the surface
	// and is sending a pointer and byte pitch to it

	//in byte math(8-bit) the proper address is: 3*x+y*lpitch
	//This is the address of the low order byte which is the Blue channel
	//since the data is in RGB order
	DWORD pixel_addr= (x+x+x)+y*lpitch16;

	//write the data, first blue
	video_buffer[pixel_addr]=blue;

	
	video_buffer[pixel_addr+1]=green;
	
	//write the data, first blue
	video_buffer[pixel_addr+2]=red;
}



inline void Plot_Pixel_32(int x, int y, 
                          int alpha,int red, int green, int blue, 
                          UINT *video_buffer, int lpitch32)
{
// this function plots a pixel in 32-bit color mode
// assuming that the caller already locked the surface
// and is sending a pointer and DWORD aligned pitch to it

// first build up color WORD
UINT pixel = _RGB32BIT(alpha,red,green,blue);

// write the data
video_buffer[x + y*lpitch32] = pixel;

} // end Plot_Pixel_32



void Blit_Clipped(int x, int y,			//possition to draw bitmap
				  int width, int height,//size of bitmap in pixels
				  UCHAR *bitmap,		//pointer to bitmap data
				  UCHAR *video_buffer,	//pointer to video buffer surface
				  int	mempitch)		//video pitch per line
{
// this function blits and clips the image sent in bitmap to the 
// destination surface pointed to by video_buffer
// the function assumes a 640x480x8 mode
	
	// first do trivial rejections of bitmap, is it totally invisible?
	if ((x >= SCREEN_WIDTH) || (y>= SCREEN_HEIGHT) ||
		((x + width) <= 0) || ((y + height) <= 0))
	return;

	// clip source rectangle
	// pre-compute the bounding rect to make life easy
	int x1 = x;
	int y1 = y;
	int x2 = x1 + width - 1;
	int y2 = y1 + height -1;

	// upper left hand corner first
	if (x1 < 0)
	   x1 = 0;

	if (y1 < 0)
	   y1 = 0;

	// now lower left hand corner
	if (x2 >= SCREEN_WIDTH)
		x2 = SCREEN_WIDTH-1;

	if (y2 >= SCREEN_HEIGHT)
		y2 = SCREEN_HEIGHT-1;

	// now we know to draw only the portions of the bitmap from (x1,y1) to (x2,y2)
	// compute offsets into bitmap on x,y axes, we need this to compute starting point
	// to rasterize from
	int x_off = x1 - x;
	int y_off = y1 - y;

	// compute number of columns and rows to blit
	int dx = x2 - x1 + 1;
	int dy = y2 - y1 + 1;

	// compute starting address in video_buffer 
	video_buffer += (x1 + y1*mempitch);

	// compute starting address in bitmap to scan data from
	bitmap += (x_off + y_off*width);

	// at this point bitmap is pointing to the first pixel in the bitmap that needs to
	// be blitted, and video_buffer is pointing to the memory location on the destination
	// buffer to put it, so now enter rasterizer loop

	UCHAR pixel; // used to read/write pixels

	for (int index_y = 0; index_y < dy; index_y++)
		 {
		 // inner loop, where the action takes place
		 for (int index_x = 0; index_x < dx; index_x++)
			  {
			  // read pixel from source bitmap, test for transparency and plot
			  if ((pixel = bitmap[index_x]))
				  video_buffer[index_x] = pixel;

			  } // end for index_x
	     
			  // advance pointers
			  video_buffer+=mempitch;  // bytes per scanline
			  bitmap      +=width;     // bytes per bitmap row

		 } // end for index_y

} // end Blit_Clipped



LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAW7 lpdd,			
									     LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list)
{
// this function creates a clipper from the sent clip list and attaches
// it to the sent surface

	int index;                         // looping var
	LPDIRECTDRAWCLIPPER lpddclipper;   // pointer to the newly
									   // created dd clipper
	LPRGNDATA region_data;             // pointer to the region
									   // data that contains
									   // the header and clip list

	// first create the direct draw clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipper,NULL)))
	   return(NULL);

	// now create the clip list from the sent data

	// first allocate memory for region data
	region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+
				   num_rects*sizeof(RECT));

	// now copy the rects into region data
	memcpy(region_data->Buffer, clip_list, sizeof(RECT)*num_rects);

	// set up fields of header
	region_data->rdh.dwSize          = sizeof(RGNDATAHEADER);
	region_data->rdh.iType           = RDH_RECTANGLES;
	region_data->rdh.nCount          = num_rects;
	region_data->rdh.nRgnSize        = num_rects*sizeof(RECT);
	region_data->rdh.rcBound.left    =  64000;
	region_data->rdh.rcBound.top     =  64000;
	region_data->rdh.rcBound.right   = -64000;
	region_data->rdh.rcBound.bottom  = -64000;

	// find bounds of all clipping regions
	for (index=0; index<num_rects; index++)
		{
		// test if the next rectangle unioned with
		// the current bound is larger
		if (clip_list[index].left < region_data->rdh.rcBound.left)
		   region_data->rdh.rcBound.left = clip_list[index].left;

		if (clip_list[index].right > region_data->rdh.rcBound.right)
		   region_data->rdh.rcBound.right = clip_list[index].right;

		if (clip_list[index].top < region_data->rdh.rcBound.top)
		   region_data->rdh.rcBound.top = clip_list[index].top;

		if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
		   region_data->rdh.rcBound.bottom = clip_list[index].bottom;

		} // end for index

	// now we have computed the bounding rectangle region and set up the data
	// now let's set the clipping list

	if (FAILED(lpddclipper->SetClipList(region_data, 0)))
	   {
	   // release memory and return error
	   free(region_data);
	   return(0);
	   } // end if

	// now attach the clipper to the surface
	if (FAILED(lpdds->SetClipper(lpddclipper)))
	   {
	   // release memory and return error
	   free(region_data);
	   return(NULL);
	   } // end if

	// all is well, so release memory and
	// send back the pointer to the new clipper
	free(region_data);
	return(lpddclipper);

} // end DDraw_Attach_Clipper


int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, WCHAR *filename)
{
// this function opens a bitmap file and loads the data into bitmap
	DWORD	dwTemp;		//temp value
	HANDLE  file_handle;// the file handle
    int		index;      // looping index
	DWORD	size;		//the real size of the file


	char	szBuffer[200]; //temp buffer;


	//UCHAR   *temp_buffer = NULL; // used to convert 24 bit images to 16 bit


	// open the file if it exists
	file_handle = CreateFile((LPCWSTR)filename,GENERIC_READ,
								  FILE_SHARE_READ,  NULL,
								  OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,
								  NULL);

	if(file_handle==INVALID_HANDLE_VALUE)
								  return(0);

	//load the bitmap file header
	ReadFile(file_handle, &bitmap->bitmapfileheader, sizeof(BITMAPFILEHEADER),&dwTemp,NULL);

	if(dwTemp!=sizeof(BITMAPFILEHEADER)){
		CloseHandle(file_handle);
		return 0;
	}
	if(bitmap->bitmapfileheader.bfType!=BITMAP_ID){
		CloseHandle(file_handle);
		return 0;
	}

	//if(bitmap->bitmapfileheader.bfType!=BITMAP_ID){
	//	CloseHandle(file_handle);
	//	return 0;
	//}

	

	size=bitmap->bitmapfileheader.bfSize-bitmap->bitmapfileheader.bfOffBits;
	
	//read the bitmap info header

	ReadFile(file_handle, &bitmap->bitmapinfoheader, sizeof(BITMAPINFOHEADER),&dwTemp,NULL);
	if(dwTemp!=sizeof(BITMAPINFOHEADER)){
		CloseHandle(file_handle);
		return 0;
	}

	//load the color palette if there is one
	if(bitmap->bitmapinfoheader.biBitCount==8)
	{
		ReadFile(file_handle, &bitmap->palette, MAX_COLORS_PALETTE*sizeof(PALETTEENTRY),
				&dwTemp,NULL);

		//now set all the flags in the palette correctly
		//and fix the reversed
		//BGR RGBQUAD data format
		 for (index=0; index < MAX_COLORS_PALETTE; index++)
		 {
			  // reverse the red and green fields
			  int temp_color                = bitmap->palette[index].peRed;
			  bitmap->palette[index].peRed  = bitmap->palette[index].peBlue;
			  bitmap->palette[index].peBlue = temp_color;

			  // always set the flags word to this
			  bitmap->palette[index].peFlags = PC_NOCOLLAPSE;
		  } // end for index

	} // end if

	

	// now read in the image
	if (bitmap->bitmapinfoheader.biBitCount==8 ||
		bitmap->bitmapinfoheader.biBitCount==16 ||
		bitmap->bitmapinfoheader.biBitCount==24){
	   // delete the last image if there was one
	   if (bitmap->buffer)
		   free(bitmap->buffer);

	   // allocate the memory for the image
	   if (!(bitmap->buffer =
		              (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage))){
		  // close the file
		  CloseHandle(file_handle);
		  // return error
		  return(0);
	    } // end if

	   SetFilePointer(file_handle,
		bitmap->bitmapfileheader.bfOffBits,NULL,FILE_BEGIN);//-(int)(bitmap->bitmapinfoheader.biSizeImage),NULL,FILE_END);
	   
	   // now read it in
	   ReadFile(file_handle,bitmap->buffer,
			  bitmap->bitmapinfoheader.biSizeImage,
			  &dwTemp,NULL);
	   if(bitmap->bitmapinfoheader.biSizeImage!=dwTemp){
		  CloseHandle(file_handle);
		  // return error
		  return(0);	
		}
	} // end if
	else
	   {
	   // serious problem
	   return(0);

	   } // end else

	// close the file
	CloseHandle(file_handle);

/*	fstream logfile("logfile.log", ios::out);
	logfile.seekp(0,ios::end);
	sprintf(szBuffer,"\nfilename:%s \nsize=%d \nwidth=%d \nheight=%d \nbitsperpixel=%d \ncolors=%d \nimpcolors=%d  \n BytesRead=%d \nTHE new size =%d !",
        filename,
        bitmap->bitmapinfoheader.biSizeImage,
        bitmap->bitmapinfoheader.biWidth,
        bitmap->bitmapinfoheader.biHeight,
		bitmap->bitmapinfoheader.biBitCount,
        bitmap->bitmapinfoheader.biClrUsed,
        bitmap->bitmapinfoheader.biClrImportant,
		dwTemp,
		size);
	logfile<<szBuffer;

	logfile.close();
*/
	// flip the bitmap
	Flip_Bitmap(bitmap->buffer,
				bitmap->bitmapinfoheader.biWidth*
				(bitmap->bitmapinfoheader.biBitCount/8),
				bitmap->bitmapinfoheader.biHeight);

	// return success
	return(1);

} // end Load_Bitmap_File


int Flip_Bitmap(UCHAR *image, int bytes_per_line, int height)
{
// this function is used to flip bottom-up .BMP images

	UCHAR *buffer; // used to perform the image processing
	int index;     // looping index

	// allocate the temporary buffer
	if (!(buffer = (UCHAR *)malloc(bytes_per_line*height)))
	   return(0);

	// copy image to work area
	memcpy(buffer,image,bytes_per_line*height);

	// flip vertically
	for (index=0; index < height; index++)
		memcpy(&image[((height-1) - index)*bytes_per_line],
			   &buffer[index*bytes_per_line], bytes_per_line);

	// release the memory
	free(buffer);

	// return success
	return(1);

} // end Flip_Bitmap


int Scan_Image_Bitmap_24to32(BITMAP_FILE_PTR bitmap,     // bitmap file to scan image data from
                      LPDIRECTDRAWSURFACE7 lpdds, // surface to hold data
                      int cx, int cy)             // cell to scan image from
{
// this function extracts a bitmap out of a bitmap file

	UCHAR *source_ptr,   // working pointers
		  *dest_ptr;

	DDSURFACEDESC2 ddsd;  //  direct draw surface description 

	// get the addr to destination surface memory

	// set size of the structure
	DD_INIT_STRUCT(ddsd);

	// lock the display surface
	lpdds->Lock(NULL,
				&ddsd,
				DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
				NULL);

	// compute position to start scanning bits from
	cx = cx*(ddsd.dwWidth+1) + 1;
	cy = cy*(ddsd.dwHeight+1) + 1;

	int gwidth  = ddsd.dwWidth;
	int gheight = ddsd.dwHeight;

	// extract bitmap data
	source_ptr = bitmap->buffer + cy*bitmap->bitmapinfoheader.biWidth+cx;

	// assign a pointer to the memory surface for manipulation
	dest_ptr = (UCHAR *)ddsd.lpSurface;

	// iterate thru each scanline and copy bitmap
	for (DWORD index_y=0; index_y < ddsd.dwHeight; index_y++)
		{
		// copy next line of data to destination
		memcpy(dest_ptr, source_ptr, ddsd.dwWidth);

		// advance pointers
		dest_ptr   += (ddsd.lPitch); // (ddsd.dwWidth);
		source_ptr += bitmap->bitmapinfoheader.biWidth;
		} // end for index_y

	// unlock the surface 
	lpdds->Unlock(NULL);

	// return success
	return(1);

} // end Scan_Image_Bitmap


LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(LPDIRECTDRAW7 lpdd,int width, int height, int mem_flags, int ckey)
{
	
	// this function creates an offscreen plain surface

	DDSURFACEDESC2 ddsd;         // working description
	LPDIRECTDRAWSURFACE7 lpdds;  // temporary surface
	    
	// set to access caps, width, and height
	DD_INIT_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT ;

	// set dimensions of the new bitmap surface
	ddsd.dwWidth  =  width;
	ddsd.dwHeight =  height;

	// set surface to offscreen plain
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | mem_flags;

	// create the surface
	if (FAILED(lpdd->CreateSurface(&ddsd,&lpdds,NULL)))
	   return(NULL);

	// test if user wants a color key
	if (ckey >= 0)
	 {
	   // set color key to color 0
	   DDCOLORKEY color_key; // used to set color key
	   color_key.dwColorSpaceLowValue  = _RGB32BIT(0,255,255,255);
	   color_key.dwColorSpaceHighValue = _RGB32BIT(0,255,255,255);

	   // now set the color key for source blitting
	   lpdds->SetColorKey(DDCKEY_SRCBLT, &color_key);
	 } // end if


	// return surface
	return(lpdds);
} // end DDraw_Create_Surface
 


int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap)
{
//this function release all memory associated with the bitmap

	if(bitmap->buffer)
	{
		free(bitmap->buffer);

		bitmap->buffer=NULL;
	}

	return 1;
}

int DDraw_Draw_Surface(LPDIRECTDRAWSURFACE7 source, // source surface to draw
                      int x, int y,                 // position to draw at
                      int width, int height,        // size of source surface
                      LPDIRECTDRAWSURFACE7 dest,    // surface to draw the surface on
					  int transparent)          // transparency flag



	{
// draw at the x,y 
// on the destination surface defined in dest

	HRESULT		hResult;
	RECT dest_rect,   // the destination rectangle
		 source_rect; // the source rectangle                             

	// fill in the destination rect
	dest_rect.left   = x;
	dest_rect.top    = y;
	dest_rect.right  = x+width-1;
	dest_rect.bottom = y+height-1;

	// fill in the source rect
	source_rect.left    = 0;
	source_rect.top     = 0;
	source_rect.right   = width-1;
	source_rect.bottom  = height-1;

	// test transparency flag

	if (transparent)
	   // enable color key blit
	   // blt to destination surface
	   hResult=dest->Blt(&dest_rect, source, &source_rect,(DDBLT_WAIT | DDBLT_KEYSRC),NULL);
	else
	   // perform blit without color key
	   // blt to destination surface
	   hResult=dest->Blt(&dest_rect, source, &source_rect ,DDBLT_WAIT, NULL);
	
	
	return(hResult);
} // end DDraw_Draw_Surface


int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds,DWORD color)
{
	DDBLTFX ddbltfx; // this contains the DDBLTFX structure

	// clear out the structure and set the size field 
	DD_INIT_STRUCT(ddbltfx);

	// set the dwfillcolor field to the desired color
	ddbltfx.dwFillColor = color; 

	// ready to blt to surface
	if(FAILED(lpdds->Blt(NULL,       // ptr to dest rectangle
			   NULL,       // ptr to source surface, NA            
			   NULL,       // ptr to source rectangle, NA
			   DDBLT_COLORFILL | DDBLT_WAIT,   // fill and wait                   
			   &ddbltfx)))  // ptr to DDBLTFX structure
		return 0;
	// return success
	return 1;
} // end DDraw_Fill_Surface