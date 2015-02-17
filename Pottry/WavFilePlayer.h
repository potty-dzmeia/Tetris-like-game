#ifndef WAVEFILEPLAYER_H
#define WAVEFILEPLAYER_H


#include <windows.h>
#include <Mmreg.h>
#include <dsound.h>
#include <strsafe.h>


class WavFilePlayer{
private:
	BYTE					*pbData;		//the sound data read from the file
	unsigned				iSizeSoundData; //The size of the sound data	
	
	LPDIRECTSOUND			lpDS;				//pointer to the directsound object
	LPDIRECTSOUNDBUFFER		lpDsb;				//secondary buffer
	
	WAVEFORMATEX			wfx;			//The format of the audio data

public:
	WavFilePlayer(void);
public:
	~WavFilePlayer(void);

/*If funcitons return 0 there is some error, 1 is OK*/

	
public:
	/*Function that loads all the data(into pbData) from the Sound File*/
	bool LoadWavFile(TCHAR *szFileName);

	/*Play the data streaming it*/
	bool Play(HWND hwnd,bool Looping=0);//if TRUE play the sound looping

	/*Stops playing the data*/
	bool Stop();

	//Clears the memory  
	void ShutDown();

};

#endif