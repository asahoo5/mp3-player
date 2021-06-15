/*
    events.h
    Header file for for event-definition of the application and some helping functions for managing OS Events.

    Developed for University of Washington embedded systems programming certificate
    
    2021/3 Abhilash Sahoo wrote/arranged it
*/

#ifndef __EVENTS_H
#define __EVENTS_H

#define EVENT_QUEUE_SIZE     20U
#define NUM_TYPE_OF_EVENTS   10U // display events group

// Specific for controlling Display events
typedef enum
{
        
    // Play Button Event
    EVENT_PLAY_PRESS = 0,
    EVENT_PLAY_RELEASE,
    
    // Pause Button Event
    EVENT_PAUSE_PRESS = 10,
    EVENT_PAUSE_RELEASE,
    
    // Stop Button Event
    EVENT_STOP_PRESS = 20,
    EVENT_STOP_RELEASE,
    
    // Rewind Button Event
    EVENT_REWIND_PRESS = 30,
    EVENT_REWIND_RELEASE,
    
    // Fast Forward Button Event
    EVENT_FF_PRESS = 40,
    EVENT_FF_RELEASE,
    
    // UP Button Event
    EVENT_UP_PRESS = 50,
    EVENT_UP_RELEASE,
    
    // Down Button Event
    EVENT_DOWN_PRESS = 60,
    EVENT_DOWN_RELEASE,
    
    // Volume Increase Button Event
    EVENT_VOLPLUS_PRESS = 70,
    EVENT_VOLPLUS_RELEASE,
    
    // Volume Decrease Button Event
    EVENT_VOLMINUS_PRESS = 80,
    EVENT_VOLMINUS_RELEASE,
    
    // Status Bar Increment and Decrement Update Event
    EVENT_STATUSBAR_INC,
    EVENT_STATUSBAR_DEC,
    
    EVENT_NONE
} Event_Type;

// MP3 Decoder Task event flags -- not using
/*
typedef enum
{
    setPlayFlag = (OS_FLAGS)0x1,
    setPauseFlag,       // pause event works if setPlayFlag is set
    setStopFlag,        // stop event works  if setStopFlag is set
    setFastForwardFlag, // fast forward event works if setPlayFlag is set
    setRewindFlag,      // Rewind flag works if setPlayFlag is set
    setVolUpFlag,
    setVolDownFlag    
} Flag_Type;
*/
// OS Events
extern OS_EVENT *touchEventsMbox;
extern OS_EVENT *mp3EventsMbox;

//extern OS_EVENT *mtxFileFetch;

// Event flags for controlling MP3 Streaming Events
/*
extern OS_FLAG_GRP *mp3Flags;
extern OS_FLAGS MP3PlayFlag;
*/

extern OS_EVENT *displayQMsg;
extern void * displayQMsgPtrs[EVENT_QUEUE_SIZE];

#endif