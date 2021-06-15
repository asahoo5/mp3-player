/*
    mp3Util.h
    Some utility functions for controlling the MP3 decoder.

    Developed for University of Washington embedded systems programming certificate
    
    2016/2 Nick Strathy wrote/arranged it
*/

#ifndef __MP3UTIL_H
#define __MP3UTIL_H

#include "SD.h"
#include "globals.h"
#include "events.h"
#include "bsp.h"
#include "print.h"

typedef enum {
    VOLUP = 0,
    VOLDOWN
} VolumeCounter;

PjdfErrCode Mp3GetRegister(HANDLE hMp3, INT8U *cmdInDataOut, INT32U bufLen);
void Mp3StreamInit(HANDLE hMp3);
//void Mp3Test(HANDLE hMp3);
//void Mp3Stream(HANDLE hMp3, INT8U *pBuf, INT32U bufLen);
//void Mp3StreamSDFile(HANDLE hMp3, char *pFilename);
void Mp3VolumeControl(HANDLE hMp3, VolumeCounter state);

void Mp3FetchFileNames();
//void Mp3FetchFileNames(char **list, int maxRow, int col, int *size);
void Mp3StreamCycle(HANDLE hMp3);

#endif