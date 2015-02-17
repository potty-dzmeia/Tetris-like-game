#include "WavFilePlayer.h"

WavFilePlayer::WavFilePlayer(void)
{
	pbData=NULL;
	lpDS=NULL;
	lpDsb=NULL;
}

WavFilePlayer::~WavFilePlayer(void)
{
	ShutDown();
}


/*-----------------------------------------------------------------------------------*/
bool WavFilePlayer :: LoadWavFile(TCHAR *szFileName){
/*This function loads the sound data from wave File into pbData*/
	
	MMCKINFO mmckinfoParent;	 //for the group header
	MMCKINFO mmckinfoSubchunk;   //for finding chunks within the Group 
	HMMIO	 hMMFile;

	unsigned iTemp;

	StringCchLength(szFileName,MAX_PATH,&iTemp);		
	if(iTemp<3){
		MessageBox(NULL, TEXT("Invalid file name"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	

	hMMFile=mmioOpen(szFileName, 0, MMIO_READ);//Opening file
	if(hMMFile==NULL){
		MessageBox(NULL, TEXT("Error opening multimedia file"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	
	
	// Set up WAV format structure. 
	memset(&wfx, 0, sizeof(WAVEFORMATEX));

	/* Tell Windows to locate a WAVE Group header somewhere in the file, and read it in.
	This marks the start of any embedded WAVE format within the file */
	mmckinfoParent.fccType = mmioStringToFOURCC(TEXT("WAVE"), 0); 
	if(MMSYSERR_NOERROR!=mmioDescend(hMMFile, (LPMMCKINFO)&mmckinfoParent, 0, MMIO_FINDRIFF)){
		MessageBox(NULL, TEXT("This file doesn't contain a WAVE!"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		mmioClose(hMMFile,0);
		return 0;
	}
	
	/* Tell Windows to locate the WAVE's "fmt " chunk and read in its header */
	mmckinfoSubchunk.ckid = mmioStringToFOURCC(TEXT("fmt \0"),0); 
	if(MMSYSERR_NOERROR!=mmioDescend(hMMFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)){
		MessageBox(NULL, TEXT("Error locating the fmt chunk!"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		mmioClose(hMMFile,0);
		return 0;
	}
	
	/* Tell Windows to read in the "fmt " chunk into a WAVEFORMATEX structure */
	if(mmioRead(hMMFile, (HPSTR)&wfx, mmckinfoSubchunk.cksize) != (LRESULT)mmckinfoSubchunk.cksize){
		MessageBox(NULL, TEXT("Error reading the fmt chunk!"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		mmioClose(hMMFile,0);
		return 0;
	}
		
	//Ascend from "fmt" sub-chunk
	mmioAscend(hMMFile, &mmckinfoSubchunk, 0);
	//Locate the "data" chunk
	mmckinfoSubchunk.ckid = mmioStringToFOURCC(TEXT("data\0"), 0); 
	if (MMSYSERR_NOERROR!=mmioDescend(hMMFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)){
		MessageBox(NULL, TEXT("Error locating the data chunk!"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		mmioClose(hMMFile,0);
		return 0;
	}
		
	iSizeSoundData=mmckinfoSubchunk.cksize;//write the size of the Data into the global variable	
	
	//if the file is bigger than 200MB do not open it
	if(iSizeSoundData>209715200){
		MessageBox(NULL, TEXT("File is bigger than 200MB and won't be open"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		mmioClose(hMMFile,0);
		return 0;
	}
	
	pbData=new BYTE[iSizeSoundData];

	//Loads all the file into pbData
	if(mmioRead(hMMFile, (HPSTR)pbData, iSizeSoundData)!=(LRESULT)iSizeSoundData){
		MessageBox(NULL, TEXT("Couldn't read the sound data from the file"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
	
	mmioClose(hMMFile,0);
	return 1;
}

/*-----------------------------------------------------------------------------------*/
bool WavFilePlayer :: Play(HWND hwnd, bool bLooping){
/*this functions play the data that's pointed by pbData*/
	DSCAPS				 dscaps;        //Structure specifies the capabilities of a DirectSound device for use by the IDirectSound::GetCaps 
	DSBUFFERDESC		 dsbdesc;		//describes the necessary characteristics of a new DirectSoundBuffer object
	
	HRESULT				 hresult;
	TCHAR szBuffer[MAX_PATH];
	
	
	if(DS_OK!=DirectSoundCreate(NULL,&lpDS,NULL)){ //create direct sound object
       MessageBox(NULL,TEXT("Error when calling DirectSoundCreate"),TEXT("Error"),MB_OK | MB_ICONEXCLAMATION);
       return 0;	
	}
	//DirectSound Object created...do some work with it
    dscaps.dwSize = sizeof(DSCAPS); 
	hresult= lpDS->GetCaps(&dscaps); //fills structure with information for the SoundDevice.
	if(hresult) 
		MessageBox(NULL, TEXT("Error getting sound device info"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
	//prints some fields from the dscaps structure
	//TextOut(GetDC(hwdn),15, 15,buffer , wsprintf(buffer,TEXT("%i"), dscaps.dwFreeHwMemBytes));
		
	if(DS_OK!=lpDS->SetCooperativeLevel(hwnd, DSSCL_NORMAL)){
		IDirectSound_Release(lpDS);
        MessageBox(NULL,TEXT("Set Cooperative Level didn't Work!"),TEXT("Error"),MB_OK | MB_ICONEXCLAMATION);
        return 0;	
    }
     

	// Set up DSBUFFERDESC structure. 
	memset(&dsbdesc,0, sizeof(dsbdesc));
	
	dsbdesc.dwSize=sizeof(dsbdesc);
	dsbdesc.dwFlags= DSBCAPS_GLOBALFOCUS;
	//The Size of the buffer
	dsbdesc.dwBufferBytes=iSizeSoundData; 
	dsbdesc.lpwfxFormat=&wfx;

	// Create buffer. 
	if(DS_OK!=lpDS->CreateSoundBuffer(&dsbdesc, &lpDsb, NULL)){
		MessageBox(NULL, TEXT("Error Creating Basic Buffer"),TEXT( "error"), MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}
	
	
	//Copy and Play the data
	LPVOID lpvWrite;
	DWORD  dwLength;

	if(DS_OK == lpDsb->Lock(
      0,          // Offset at which to start lock.
      iSizeSoundData,          // Size of lock; ignored because of flag.
      &lpvWrite,  // Gets address of first part of lock.
      &dwLength,  // Gets size of first part of lock.
      NULL,       // Address of wraparound not needed. 
      NULL,       // Size of wraparound not needed.
      DSBLOCK_ENTIREBUFFER))  // Flag.
	{
	
		memcpy(lpvWrite, pbData, dwLength);
		lpDsb->Unlock(lpvWrite,   // Address of lock start.
					  dwLength,   // Size of lock.
					  NULL,       // No wraparound portion.
					  0);         // No wraparound size.
	}
	else{
		MessageBox(NULL, TEXT("Error When locking the sound buffer"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		return 0;  // Add error-handling here.
	}

	//To play the buffer, call IDirectSoundBuffer8::Play, as in the following example:
    lpDsb->SetCurrentPosition(0);
	
	if(bLooping){
		hresult = lpDsb->Play(
		0,  // Unused.
		0,  // Priority for voice management.
		DSBPLAY_LOOPING); // Flags.
	}
	else{
		hresult = lpDsb->Play(
			0,  // Unused.
			0,  // Priority for voice management.
			0); // Flags.
	}
	if (FAILED(hresult))
	{
		MessageBox(NULL, TEXT("Error When playing the sound buffer"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		return 0;  // Add error-handling here.  // Add error-handling here.
	}

	
	
	
	return 1;
}

bool WavFilePlayer :: Stop(){
	if(FAILED(lpDsb->Stop())){
		MessageBox(NULL, TEXT("Error When stoping sound buffer"), TEXT("error"), MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	return 1;
}


/*-----------------------------------------------------------------------------------*/
void WavFilePlayer :: ShutDown(){
	
	if(pbData!=NULL){
		delete(pbData);
		pbData=NULL;
	}

	if(lpDsb!=NULL){
		lpDsb->Release();
		lpDsb=NULL;
	}
	if(lpDS!=NULL){
		lpDS->Release();
		lpDS=NULL;
	}
	

}