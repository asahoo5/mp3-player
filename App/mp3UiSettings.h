/*
    uiSettings.h
    Header file for controlling objects on UI of LCD Display - carries display object coordinates

    Developed for University of Washington embedded systems programming certificate
    
    2021/3 Abhilash Sahoo wrote/arranged it
*/

#ifndef __MP3UISETTINGS_H
#define __MP3UISETTINGS_H

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
/*
    Definition: MP3 Main Player Window and its labels UI Interface
    Note:       Contains all coordinate related Information/definitions
                Window space is divided into two halves - first half (240*160) music file related display & progress bar
                And second half is MP3 controller, that contains buttons
                (0, 0) - Top Left Coordinate

        -------------------------------------------------------------
        |       <.Song Title....................(Wrap Around)>      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |           Music BitMap File                |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       |                                            |      |
        |       ----------------------------------------------      |
        |-----------------------------------------------------------|
        |                   Progress Bar                            |
        |-----------------------------------------------------------|
        |   ----------------  ----------------  ----------------    |
        |   |              |  |              |  |              |    |
        |   |   Play       |  |    Pause     |  |    Stop      |    |
        |   |              |  |              |  |              |    |
        |   ----------------  ----------------  ----------------    |
        |             ---------------  --------------               |
        |             |             |  |            |               |
        |             |    Prev     |  |   Next     |               |
        |             |             |  |            |               |
        |             ---------------  --------------               |
        |                                            -------------  |
        |   ----------------                         |           |  |
        |   |              |                         |   Vol+    |  |
        |   |   Menu       |     <Play Status>       |           |  |
        |   |              |                         -------------  |
        |   ----------------                         -------------  |
        |                                            |           |  |
        |                                            |    Vol-   |  |
        |                                            |           |  |
        |                                            -------------  |
        --------------------------------------------------------------

Alternatively,
        -------------------------------------------------------------
        |       <.Song Title1....................(Wrap Around)>     |
        |       <.Song Title2....................(Wrap Around)>     |
        |       <.Song Title3....................(Wrap Around)>     |
        |       <.Song Title4....................(Wrap Around)>     |
        |       <.Song Title5....................(Wrap Around)>     |
        |       <.Song Title6....................(Wrap Around)>     |
        |       <.Song Title7....................(Wrap Around)>     |
        |       <.Song Title8....................(Wrap Around)>     |
        |       <.Song Title9....................(Wrap Around)>     |
        |       <.Song Title10...................(Wrap Around)>     |
        |-----------------------------------------------------------|
        |                   Progress Bar                            |
        |-----------------------------------------------------------|
        |   ----------------  ----------------  ----------------    |
        |   |              |  |              |  |              |    |
        |   |   Play       |  |    Pause     |  |    Stop      |    |
        |   |              |  |              |  |              |    |
        |   ----------------  ----------------  ----------------    |
        |             ---------------  --------------               |
        |             |             |  |            |               |
        |             |    Rewind   |  |     FF     |               |
        |             |             |  |            |               |
        |             ---------------  --------------               |
        |   -------------                            -------------  |
        |   |           |                            |           |  |
        |   |   UP      |                            |   Vol+    |  |
        |   |           |        <Play Status>       |           |  |
        |   -------------                            -------------  |
        |   -------------                            -------------  |
        |   |           |                            |           |  |
        |   |   DOWN    |                            |    Vol-   |  |
        |   |           |                            |           |  |
        |   -------------                            -------------  |
        --------------------------------------------------------------

*/

// Standard button width and height
#define STD_BUTTON_WIDTH   68U
#define STD_BUTTON_HEIGHT  30U

// Small button width and height
#define SM_BUTTON_WIDTH    60U
#define SM_BUTTON_HEIGHT   30U

// Menu button width and height
#define MENU_BUTTON_WIDTH  240U
#define MENU_BUTTON_HEIGHT 20U

// Status Bar button width and height
#define BAR_BUTTON_WIDTH   30U//18U
#define BAR_BUTTON_HEIGHT  18U

#define BAR_BUTTON_XCOORD  15U//14U
// Space between status bar
#define BAR_BUTTON_SPACE   24U // x-center of rectangle + 6 (distance between bars) 


// Volume Box and Stack button width and height
#define BOX_BUTTON_WIDTH   18U
#define BOX_BUTTON_HEIGHT  105U

#define STK_BUTTON_WIDTH   16U//18U
#define STK_BUTTON_HEIGHT  10U

#define STK_BUTTON_XCOORD  225U//14U

#define STK_BUTTON_SPACE   2U 

// Player Window Buttons (Designed on lower half) and other Objects Coordinate Mapping
#define MENU_XCOORD_BEGIN  120U
#define MENU_YCOORD_BEGIN  10U

#define PLAY_XCOORD      40U
#define PLAY_YCOORD      185U

#define PAUSE_XCOORD     120U
#define PAUSE_YCOORD     185U

#define STOP_XCOORD      200U
#define STOP_YCOORD      185U

#define REWIND_XCOORD    65U
#define REWIND_YCOORD    222U

#define FF_XCOORD        145U
#define FF_YCOORD        222U

//#define MENU_XCOORD    85U
//#define MENU_YCOORD    230U

#define UP_XCOORD        40U
#define UP_YCOORD        259U

#define DOWN_XCOORD      40U
#define DOWN_YCOORD      299U

#define VOLPLUS_XCOORD   180U
#define VOLPLUS_YCOORD   259U

#define VOLMINUS_XCOORD  180U
#define VOLMINUS_YCOORD  299U

#define STATUS_XCOORD    80U
#define STATUS_YCOORD    272U

#define VOLBOX_XCOORD    225U
#define VOLBOX_YCOORD    260U

#define PLAYERBUTTONNUM  9U
#define MAXMENULIST      7U
#define STATUSBARSNUM    10U
#define VOLUMEBARSNUM    14U

#define DEFAULTVOLINDX   5U

// MP3 Player Button Indexes
typedef enum
{
    PLAY = 0,
    PAUSE,
    STOP,
    REWIND,
    FF,
    UP,
    DOWN,
    VOLPLUS,
    VOLMINUS
} PlayerButtonIndex;

// Declare MP3 Player Winodows here

// The Player Window - where functions of controlling MP3 events are provided
typedef struct _PlayerWindow
{
    // -----------Start of Upper Half of the display--------------------//
    // Music Title & Music Bitmap File
    Adafruit_GFX_Button menu_list[MAXMENULIST];
    
    // Music Progress bar
    Adafruit_GFX_Button status_box;
    Adafruit_GFX_Button status_bar[STATUSBARSNUM];
    
    // -----------End of Upper Half of the display--------------------//
  
    // -----------Start of Lower Half of the display--------------------//
    // A list of MP3 Player buttons (Play, Pause, Stop, Prev, Next, Menu, Vol+, Vol-)
    Adafruit_GFX_Button button_list[PLAYERBUTTONNUM];
    
    // Current Play Status - Text Content
    char *player_status;
    
    // Volume Bar
    Adafruit_GFX_Button vol_box;
    Adafruit_GFX_Button vol_bar[VOLUMEBARSNUM];
    // -----------End of Lower Half of the display--------------------//
} PlayerWindow;

#endif