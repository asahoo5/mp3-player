/*
    mp3UserInterface.h
    MP3 Player User Interfacing Objects and functions for controlling the LCD Display related activities.

    Developed for University of Washington embedded systems programming certificate
    
    2021/2 Abhilash Sahoo wrote it
*/

#ifndef __MP3USERINTERFACE_H
#define __MP3USERINTERFACE_H

#include "bsp.h"
#include "print.h"

#include "events.h"
#include "globals.h"

#include "mp3UserInterface.h"
#include "mp3UiSettings.h"

#define BUFFERSIZE  32U

// Play Status
typedef enum
{
    SELECT = 0,
    PLAYING,
    PAUSED,
    STOPPED
} PlayStatus;

typedef struct{
    char     *label;
    uint16_t x_coordinate;
    uint16_t y_coordinate;
    uint8_t  width;
    uint8_t  height;
}ButtonParameter;
                                      

// LCD Controller Setup and Inialization
void SetUpLCDInterface ();
        
// MP3 Player Window Control Functions 
void InitPlayerWindow(PlayerWindow *pWindow);
//void DeinitPlayerWindow(PlayerWindow *pWindow);
void DrawPlayerWindow(PlayerWindow *pWindow);

void PlayerWindowStateMachine(PlayerWindow *pWindow, Event_Type winEvent);

void PrintToLcdWithBuf(char *buf, int size, char *format, ...);

#endif
