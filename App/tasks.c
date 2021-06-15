/************************************************************************************

Copyright (c) 2001-2016  University of Washington Extension.

Module Name:

    tasks.c

Module Description:

    The tasks that are executed by the test application.

2016/2 Nick Strathy adapted it for NUCLEO-F401RE 

************************************************************************************/
#include <stdarg.h>

#include "bsp.h"
#include "events.h"
#include "globals.h"
#include "print.h"

#include "mp3Util.h"
#include "mp3UserInterface.h"
#include "mp3TouchInterface.h"

#include <Adafruit_FT6206.h>

#define PENRADIUS 3

//#include "train_crossing.h"

#define BUFSIZE         256

char listOfSongs[MAXLISTOFSONGS][SUPPFILENAMESIZE];    // An array to prefetch a list of songs array
unsigned int sizeOfList = 0;
unsigned int currSongFilePntr = 0;


BOOLEAN isPlaying    = OS_FALSE;
BOOLEAN isStopSong   = OS_FALSE;
BOOLEAN isFastForward= OS_FALSE;
BOOLEAN isRewind     = OS_FALSE;
BOOLEAN isVolUp      = OS_FALSE;
BOOLEAN isVolDown    = OS_FALSE;

INT32U currPlayingSongFilePntr = INT_MAX;

/************************************************************************************

   Allocate the stacks for each task.
   The maximum number of tasks the application can have is defined by OS_MAX_TASKS in os_cfg.h

************************************************************************************/

static OS_STK   LcdDisplayTaskStk[APP_DISPLAY_TASK_EQ_STK_SIZE];
static OS_STK   Mp3StreamTaskStk[APP_MP3STREAM_TASK_EQ_STK_SIZE];
static OS_STK   CmdControllerTaskStk[APP_CMD_TASK_EQ_STK_SIZE];
static OS_STK   LcdTouchTaskStk[APP_TOUCH_TASK_EQ_STK_SIZE];

     
// Task prototypes
void LcdDisplayTask(void* pdata);
void Mp3StreamTask(void* pdata);
void CmdControllerTask(void* pdata);
void LcdTouchTask(void* pdata);

// Globals
PlayerWindow pWindow;                   // Player Window Instance

//OS Events
OS_EVENT *touchEventsMbox;
OS_EVENT *mp3EventsMbox;

OS_EVENT *displayQMsg;
void * displayQMsgPtrs[EVENT_QUEUE_SIZE];
/************************************************************************************

   This task is the initial task running, started by main(). It starts
   the system tick timer and creates all the other tasks. Then it deletes itself.

************************************************************************************/
void StartupTask(void* pdata)
{
	char buf[BUFSIZE];

    PjdfErrCode pjdfErr;
    INT32U length;
    INT8U err = 0;
    static HANDLE hSD = 0;
    static HANDLE hSPI = 0;

	//PrintWithBuf(buf, BUFSIZE, "StartupTask: Begin\n");
	//PrintWithBuf(buf, BUFSIZE, "StartupTask: Starting timer tick\n");

    // Start the system tick
    SetSysTick(OS_TICKS_PER_SEC);
    
    // Initialize SD card
    //PrintWithBuf(buf, PRINTBUFMAX, "Opening handle to SD driver: %s\n", PJDF_DEVICE_ID_SD_ADAFRUIT);
    hSD = Open(PJDF_DEVICE_ID_SD_ADAFRUIT, 0);
    if (!PJDF_IS_VALID_HANDLE(hSD)) while(1);


    //PrintWithBuf(buf, PRINTBUFMAX, "Opening SD SPI driver: %s\n", SD_SPI_DEVICE_ID);
    // We talk to the SD controller over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to 
    // the SD driver.
    hSPI = Open(SD_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);
    
    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hSD, PJDF_CTRL_SD_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);
    
    if (!SD.begin(hSD)) 
    {
        //PrintWithBuf(buf, PRINTBUFMAX, "Attempt to initialize SD card failed.\n");
    }

    // Create the test tasks
    PrintWithBuf(buf, BUFSIZE, "StartupTask: Creating the application tasks\n");
    
    // Create Mailbox
    touchEventsMbox = OSMboxCreate(NULL);
    mp3EventsMbox   = OSMboxCreate(NULL);
    
    //Create Queue
    displayQMsg = OSQCreate(displayQMsgPtrs, EVENT_QUEUE_SIZE);
    
    //Create Event Flag -- not using
    //mp3Flags = OSFlagCreate( 0x1, &err);

    // The maximum number of tasks the application can have is defined by OS_MAX_TASKS in os_cfg.h
    OSTaskCreate(Mp3StreamTask, (void*)0, &Mp3StreamTaskStk[APP_MP3STREAM_TASK_EQ_STK_SIZE-1], APP_TASK_TEST4_PRIO);
    OSTaskCreate(LcdDisplayTask, (void*)0, &LcdDisplayTaskStk[APP_DISPLAY_TASK_EQ_STK_SIZE-1], APP_TASK_TEST2_PRIO);
    OSTaskCreate(LcdTouchTask,   (void*)0, &LcdTouchTaskStk[APP_TOUCH_TASK_EQ_STK_SIZE-1],   APP_TASK_TEST1_PRIO);
    OSTaskCreate(CmdControllerTask,   (void*)0, &CmdControllerTaskStk[APP_CMD_TASK_EQ_STK_SIZE-1],   APP_TASK_TEST3_PRIO);

    // Delete ourselves, letting the work be done in the new tasks.
    PrintWithBuf(buf, BUFSIZE, "StartupTask: deleting self\n");
	OSTaskDel(OS_PRIO_SELF);
}

/************************************************************************************

   Runs LCD Display code

************************************************************************************/
void LcdDisplayTask(void* pdata)
{
    INT8U err;
    Event_Type winEvent;
    
    Mp3FetchFileNames();
        
    SetUpLCDInterface();
    
    InitPlayerWindow(&pWindow);
        
    while (1) { 
        
        winEvent = *((Event_Type*)OSQPend(displayQMsg, 0, &err));
        
        if(err != OS_ERR_NONE) while(1);
        
        PlayerWindowStateMachine(&pWindow, winEvent);
        
        OSTimeDly(10);
    }
}

/************************************************************************************

   Runs LCD Display code

************************************************************************************/
void LcdTouchTask(void* pdata)
{     
    SetUpTouchInterface();
    
    while (1) {         
              
        TouchEventGenerator(pWindow);
        
        OSTimeDly(10);
    }
}

/************************************************************************************

   Runs LCD Display code

************************************************************************************/
void CmdControllerTask(void* pdata)
{     
    Event_Type receivedEvent = EVENT_NONE;
    INT8U err = 0;
    
    while (1) {

        receivedEvent = *((Event_Type*)OSMboxPend(touchEventsMbox, 0, &err));
        if(err != OS_ERR_NONE) while(1);

     
        // Send their respective operating events to MP3 Decoder Task and Display Tasks
        switch (receivedEvent)
        {
        case EVENT_PLAY_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_PLAY_RELEASE:
            
            //if(!isPaused){
            if(currPlayingSongFilePntr != currSongFilePntr)
            {
                // exit from song loop
                isStopSong = OS_TRUE;
                OSMboxPost(mp3EventsMbox, (void*)&receivedEvent);
                
            }
            OSTimeDly(50);

            isPlaying = OS_TRUE;
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            
            break;
        case EVENT_PAUSE_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_PAUSE_RELEASE:
            isPlaying = OS_FALSE;
            
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_STOP_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_STOP_RELEASE:
            if(isPlaying || (currPlayingSongFilePntr == currSongFilePntr))
            {
                isStopSong = OS_TRUE;
            }
            else
            {
                err = OSQPost(displayQMsg, (void*)&receivedEvent);
            }
            break;
        case EVENT_REWIND_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_REWIND_RELEASE:
            isRewind = OS_TRUE;            
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_FF_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_FF_RELEASE:
            isFastForward = OS_TRUE;
            
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_UP_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_UP_RELEASE:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_DOWN_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_DOWN_RELEASE:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_VOLPLUS_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_VOLPLUS_RELEASE:
            if(isPlaying)
                isVolUp  = OS_TRUE;
            else
                OSMboxPost(mp3EventsMbox, (void*)&receivedEvent);
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_VOLMINUS_PRESS:
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_VOLMINUS_RELEASE:
            if(isPlaying)
                isVolDown  = OS_TRUE;
            else
                OSMboxPost(mp3EventsMbox, (void*)&receivedEvent);
            err = OSQPost(displayQMsg, (void*)&receivedEvent);
            break;
        case EVENT_NONE:
            break;
        default:
            break;
        }       
        
        OSTimeDly(5);
    }
}
/************************************************************************************

   Runs MP3 demo code

************************************************************************************/
void Mp3StreamTask(void* pdata)
{
    Event_Type  mp3Event;
    PjdfErrCode pjdfErr;
    INT8U err;
    INT32U length;
    
    char buf[BUFSIZE];
    PrintWithBuf(buf, BUFSIZE, "Mp3DemoTask: starting\n");

    PrintWithBuf(buf, BUFSIZE, "Opening MP3 driver: %s\n", PJDF_DEVICE_ID_MP3_VS1053);
    // Open handle to the MP3 decoder driver
    HANDLE hMp3 = Open(PJDF_DEVICE_ID_MP3_VS1053, 0);
    if (!PJDF_IS_VALID_HANDLE(hMp3)) while(1);

	PrintWithBuf(buf, BUFSIZE, "Opening MP3 SPI driver: %s\n", MP3_SPI_DEVICE_ID);
    // We talk to the MP3 decoder over a SPI interface therefore
    // open an instance of that SPI driver and pass the handle to 
    // the MP3 driver.
    HANDLE hSPI = Open(MP3_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);

    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hMp3, PJDF_CTRL_MP3_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);

    // Send initialization data to the MP3 decoder and run a test
	PrintWithBuf(buf, BUFSIZE, "Starting MP3 device test\n");
    
//    OS_ENTER_CRITICAL();
//    Mp3FetchFileNames();
//    OS_EXIT_CRITICAL();
    
    while (1)
    {
        //OSTimeDly(50);
        mp3Event = *((Event_Type*)OSMboxPend(mp3EventsMbox, 0, &err));
        if(err != OS_ERR_NONE) while(1);
        
        // Process their respective operating events to MP3 Decoder Task and Display Tasks
        switch (mp3Event)
        {
        case EVENT_PLAY_RELEASE:
             Mp3StreamCycle(hMp3);
            break;
        
        case EVENT_VOLPLUS_RELEASE:
            Mp3VolumeControl(hMp3, VOLUP);
            break;
        
        case EVENT_VOLMINUS_RELEASE:
            Mp3VolumeControl(hMp3, VOLDOWN);
            break;
            
        default:
            break;
        }
         
    }
}
