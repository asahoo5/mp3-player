/*
    mp3TouchInterface.h
    MP3 Player Touch Interfacing Objects and functions for controlling the LCD Touch related activities.

    Developed for University of Washington embedded systems programming certificate
    
    2021/3 Abhilash Sahoo wrote/arranged it
*/
#ifndef __MP3TOUCHINTERFACE_H
#define __MP3TOUCHINTERFACE_H

#include "bsp.h"
#include "print.h"
#include "events.h"

#include "mp3UiSettings.h"
#include <Adafruit_FT6206.h>

// Button Status
typedef enum
{
    BUTTON_PRESSED = 0,
    BUTTON_RELEASED
} ButtonState;

// LCD Controller Setup and Inialization
void SetUpTouchInterface (void);

// Touch Event Generator
void TouchEventGenerator (PlayerWindow pWindow);

#endif