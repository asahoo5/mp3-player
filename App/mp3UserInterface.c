/*
    mp3UserInterface.c
    MP3 Player User Interfacing implementation.
    This is the implementation of the mp3UserInterface.h driver interface exposed to applications.

    Developed for University of Washington embedded systems programming certificate
    
    2021/3 Abhilash Sahoo wrote it
*/

#include "mp3UserInterface.h"

#define DEAFULT_VOL_POS   7U
// Button Intialization list
static ButtonParameter playerParamList[PLAYERBUTTONNUM] = 
{
    {"Play",  PLAY_XCOORD,    PLAY_YCOORD,     STD_BUTTON_WIDTH,  STD_BUTTON_HEIGHT },
    {"Pause", PAUSE_XCOORD,   PAUSE_YCOORD,    STD_BUTTON_WIDTH,  STD_BUTTON_HEIGHT },
    {"Stop",  STOP_XCOORD,    STOP_YCOORD,     STD_BUTTON_WIDTH,  STD_BUTTON_HEIGHT },
    {"Rewind",REWIND_XCOORD,  REWIND_YCOORD,   STD_BUTTON_WIDTH,  STD_BUTTON_HEIGHT },
    {"FF",    FF_XCOORD,      FF_YCOORD,       STD_BUTTON_WIDTH,  STD_BUTTON_HEIGHT },
    {"Up",    UP_XCOORD,      UP_YCOORD,       SM_BUTTON_WIDTH,   SM_BUTTON_HEIGHT},
    {"Down",  DOWN_XCOORD,    DOWN_YCOORD,     SM_BUTTON_WIDTH,   SM_BUTTON_HEIGHT},
    {"Vol+",  VOLPLUS_XCOORD, VOLPLUS_YCOORD,  SM_BUTTON_WIDTH,   SM_BUTTON_HEIGHT },
    {"Vol-",  VOLMINUS_XCOORD,VOLMINUS_YCOORD, SM_BUTTON_WIDTH,   SM_BUTTON_HEIGHT }
};

static char* player_status[]       = {"Select&Play",
                                      "Playing....",
                                      "Paused.....",
                                      "Stopped...."};

// Active Buttons Count
static INT8U activeMenuBtnCnt = 0;
static INT8U currMenuSelectCounter = 0;
// Adafruit LCD Controller Instance
Adafruit_ILI9341 lcdCtrl = Adafruit_ILI9341();
    
// Status Bar Count
static INT8S statusBarBtnCnt = -1;
static INT8S volumeBarBtnCnt = DEAFULT_VOL_POS;

// Private function definitions
static void buttonPressResponse(Adafruit_GFX_Button *button);
static void buttonReleaseResponse(Adafruit_GFX_Button *button);
static void setMenuToSelectState(Adafruit_GFX_Button *menu);
static void updateStatusBar(Adafruit_GFX_Button *status, BOOLEAN state);
static void updateVolumeBar(Adafruit_GFX_Button *vol, BOOLEAN state);
static void setMenuToInactiveState(Adafruit_GFX_Button *menu);
static INT8U getActiveButtonCount (boolean upDownFlag);
static void drawPlayStatus(char *status);
static void PrintCharToLcd(char c);

void InitMenuLabels(PlayerWindow *pWindow, boolean upDownFlag);

/*******************************************************************************
 * Function:  SetUpLCDInterface
 * 
 * Description: Set Up LCD Controller and related interface.
 * 
 * Arguments:   
 * 
 * Return Value: None
 *
 ******************************************************************************/
void SetUpLCDInterface (void)
{
    PjdfErrCode pjdfErr;
    INT32U length;
        
    // Open handle to the LCD driver
    HANDLE hLcd = Open(PJDF_DEVICE_ID_LCD_ILI9341, 0);
    //hang here for debugging purpose - to be replaced with Error log if project scope is expanded
    if (!PJDF_IS_VALID_HANDLE(hLcd)) while(1); 
    
    // Open handle to the SPI driver
    HANDLE hSPI = Open(LCD_SPI_DEVICE_ID, 0);
    if (!PJDF_IS_VALID_HANDLE(hSPI)) while(1);
    
    length = sizeof(HANDLE);
    pjdfErr = Ioctl(hLcd, PJDF_CTRL_LCD_SET_SPI_HANDLE, &hSPI, &length);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);
    
    // Now Initialize the LCD Controller
    lcdCtrl.setPjdfHandle(hLcd);
    lcdCtrl.begin();
}

/*******************************************************************************
 * Function:  InitPlayerWindow
 * 
 * Description: Intialize PlayerWindow Datastructure
 *              To be called at the beginning of display task
 * 
 * Arguments:    None
 * 
 * Return Value: None
 *
 ******************************************************************************/
 void InitPlayerWindow(PlayerWindow *pWindow)
 {
     OS_CPU_SR cpu_sr;
     
     OS_ENTER_CRITICAL(); 
     
     // allow slow lower pri drawing operation to finish without preemption
     // Clear Screen - And set up a background color for the screen
     lcdCtrl.fillScreen(ILI9341_WHITE);
     
     // Init List of menu
     for (int i = 0; i < MAXMENULIST; i++)
     {
         pWindow->menu_list[i] = Adafruit_GFX_Button();
         pWindow->menu_list[i].initButton(                       
                                   &lcdCtrl,                       
                                   MENU_XCOORD_BEGIN, 
                                   MENU_YCOORD_BEGIN + (i*MENU_BUTTON_HEIGHT),                       
                                   MENU_BUTTON_WIDTH, 
                                   MENU_BUTTON_HEIGHT,                      
                                   ILI9341_WHITE,                  // outline                       
                                   ILI9341_BLUE,                   // fill                       
                                   ILI9341_WHITE,                  // text color                       
                                   "",                       
                                   1);
     }
     
     // If there are active menu buttons, first item is by default selected
     activeMenuBtnCnt = getActiveButtonCount(true);
     
     InitMenuLabels(pWindow, true);
     
     if(activeMenuBtnCnt > 0)
     {
         setMenuToSelectState(&pWindow->menu_list[0]);
         pWindow->menu_list[0].drawButton(false, true);
     }
     
     // Initialize Status Box and Status Bar
     pWindow->status_box = Adafruit_GFX_Button();
     pWindow->status_box.initButton(                       
                                   &lcdCtrl,                       
                                   MENU_XCOORD_BEGIN, 
                                   MENU_YCOORD_BEGIN + (MAXMENULIST*MENU_BUTTON_HEIGHT)+3,                       
                                   MENU_BUTTON_WIDTH, 
                                   MENU_BUTTON_HEIGHT,                      
                                   ILI9341_BLACK,                                        
                                   ILI9341_WHITE,                                         
                                   ILI9341_WHITE,                                         
                                   "",                       
                                   1);
     pWindow->status_box.drawButton(false, false);

    for (int i = 0; i < STATUSBARSNUM; i++)
     {
         pWindow->status_bar[i] = Adafruit_GFX_Button();
         pWindow->status_bar[i].initButton(                       
                                   &lcdCtrl,                       
                                   BAR_BUTTON_XCOORD + (i * BAR_BUTTON_SPACE), 
                                   MENU_YCOORD_BEGIN + (MAXMENULIST*MENU_BUTTON_HEIGHT)+3,                       
                                   BAR_BUTTON_WIDTH, 
                                   BAR_BUTTON_HEIGHT,                      
                                   ILI9341_BLUE,                                       
                                   ILI9341_WHITE,                                          
                                   ILI9341_WHITE,                                       
                                   "",                       
                                   1);
         //pWindow->status_bar[i].drawButton(false, false);
     }
   
     // Initialize Status Box and Status Bar
     pWindow->vol_box = Adafruit_GFX_Button();
     pWindow->vol_box.initButton(                       
                                   &lcdCtrl,                       
                                   VOLBOX_XCOORD, 
                                   VOLBOX_YCOORD,                       
                                   BOX_BUTTON_WIDTH, 
                                   BOX_BUTTON_HEIGHT,                      
                                   ILI9341_BLACK,                                        
                                   ILI9341_WHITE,                                         
                                   ILI9341_WHITE,                                         
                                   "",                       
                                   1);
     pWindow->vol_box.drawButton(false, false);
     
     for (int i = 0; i < VOLUMEBARSNUM; i++)
     {
         pWindow->vol_bar[i] = Adafruit_GFX_Button();
         pWindow->vol_bar[i].initButton(                       
                                   &lcdCtrl,                       
                                   STK_BUTTON_XCOORD, 
                                   306 - (i * 7),                       
                                   STK_BUTTON_WIDTH, 
                                   STK_BUTTON_HEIGHT,                      
                                   ILI9341_WHITE,                                       
                                   ILI9341_BLUE,                                          
                                   ILI9341_WHITE,                                       
                                   "",                       
                                   1);
     }
    
    // display default volume psoition
    for (int i = 0; i <= volumeBarBtnCnt; i++)
    {
        pWindow->vol_bar[i].drawButton(false, false);
    }
    
     // Initialize a list of player buttons
     for (int i = 0; i < PLAYERBUTTONNUM; i++)
     {
         pWindow->button_list[i] = Adafruit_GFX_Button();
         pWindow->button_list[i].initButton(                       
                                   &lcdCtrl,                       
                                   playerParamList[i].x_coordinate, 
                                   playerParamList[i].y_coordinate,                       
                                   playerParamList[i].width, 
                                   playerParamList[i].height,                      
                                   ILI9341_BLACK,                      // outline                       
                                   ILI9341_WHITE,                      // fill                       
                                   ILI9341_BLACK,                      // text color                       
                                   playerParamList[i].label,                       
                                   1);
          pWindow->button_list[i].drawButton(false, false);
     }
     
     drawPlayStatus(player_status[SELECT]);
     
     OS_EXIT_CRITICAL();
 }
 
/*******************************************************************************
 * Function:  drawPlayerWindow
 * 
 * Description: Display the MP3 Player Window
 * 
 * Arguments:   None
 * 
 * Return Value: None
 *
 ******************************************************************************/
void DrawPlayerWindow(PlayerWindow *pWindow)
{
     // Display a list of songs fetched
     for (int i = 0; i < MAXMENULIST; i++)
     {
         pWindow->menu_list[i].drawButton(false, true);
     }
     
     // Display buttons
     for (int i = 0; i < PLAYERBUTTONNUM; i++)
     {
         pWindow->button_list[i].drawButton(false, false);
     }   
}

/*******************************************************************************
 * Function:  InitMenuLabel
 * 
 * Description: Init menu buttons
 * 
 * Arguments:   None
 * 
 * Return Value: None
 *
 ******************************************************************************/
void InitMenuLabels(PlayerWindow *pWindow, boolean upDownFlag)
{
     // Fetch a list of songs from SD card and initialize in the list
     for (int i = 0; i < activeMenuBtnCnt; i++)
     {
          if(upDownFlag)
          {
              pWindow->menu_list[i].relabelButton(listOfSongs[currSongFilePntr+i]);
              pWindow->menu_list[i].drawButton(false, true);
          }
          else
          {
                // Should scroll over - feed lower menu to upwards
              pWindow->menu_list[(activeMenuBtnCnt-1) - i].relabelButton(listOfSongs[currSongFilePntr-i]); 
              pWindow->menu_list[(activeMenuBtnCnt-1) - i].drawButton(false, true);
          }
     }
     
     //count is not equal to menu count, fill remaining with NULL labelling
     if(activeMenuBtnCnt != MAXMENULIST)
     {
         for(int i = activeMenuBtnCnt; i < MAXMENULIST; ++i)
         {
             pWindow->menu_list[i].relabelButton("");
             pWindow->menu_list[i].drawButton(false, true);
         }
     } 
}

/*******************************************************************************
 * Function:  getActiveButtonCount
 * 
 * Description: Determine active list of menu buttons to be created
 * 
 * Arguments:   size
 *              index pointer
 *              updown event: true - Down event, false - Up event
 * 
 * Return Value: None
 *
 ******************************************************************************/
static INT8U getActiveButtonCount (boolean upDownFlag)
{
    int count = 0;
    if(sizeOfList == 0)
        return count;
    if(upDownFlag)
         // Determine the number of labels to be displayed
         count = (((sizeOfList-1) - currSongFilePntr) >= MAXMENULIST) ? MAXMENULIST : ((sizeOfList-1) - currSongFilePntr);
    else
         // Upper will always be filled with 7 values, wont allow creation of 
         // buttons if current point is at 0
         count = (currSongFilePntr > 0) ? MAXMENULIST : 0;
    return count;
}
/*******************************************************************************
 * Function:  buttonPressResponse
 * 
 * Description: A unique display for button on a press event
 * 
 * Arguments:   Adafruit_GFX_Button - button background color value to be updated
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void buttonPressResponse(Adafruit_GFX_Button *button)
{
    button->refillButton(ILI9341_DARKGREY);
}

/*******************************************************************************
 * Function:  buttonReleaseResponse
 * 
 * Description: A unique display for button on a release event
 * 
 * Arguments:   Adafruit_GFX_Button - button background color value to be updated
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void buttonReleaseResponse(Adafruit_GFX_Button *button)
{
    button->refillButton(ILI9341_WHITE);
}

/*******************************************************************************
 * Function:  setMenuToSelectState
 * 
 * Description: A unique display for menu on a select event
 * 
 * Arguments:   Adafruit_GFX_Button - button background color value to be updated
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void setMenuToSelectState(Adafruit_GFX_Button *menu)
{
    menu->refillButton(ILI9341_BLACK);
}

/*******************************************************************************
 * Function:  updateStatusBar
 * 
 * Description: A unique display for status button on a active/inactive event
 * 
 * Arguments:   Adafruit_GFX_Button - button background color value to be updated
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void updateStatusBar(Adafruit_GFX_Button *status, BOOLEAN state = OS_FALSE)
{
    if(state)
    {
        status->refillButton(ILI9341_BLUE);
        status->reoutlineButton(ILI9341_BLUE);
    }
    else
    {
        status->refillButton(ILI9341_WHITE);
        status->reoutlineButton(ILI9341_WHITE);
    }
}

/*******************************************************************************
 * Function:  updateVolumeBar
 * 
 * Description: A unique display for volume bar on a active/inactive event
 * 
 * Arguments:   Adafruit_GFX_Button - button background color value to be updated
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void updateVolumeBar(Adafruit_GFX_Button *vol, BOOLEAN state = OS_FALSE)
{
    if(state)
    {
        vol->refillButton(ILI9341_BLUE);
    }
    else
    {
        vol->refillButton(ILI9341_WHITE);
    }
}

/*******************************************************************************
 * Function:  setMenuToSelectActive
 * 
 * Description: A unique display for menu on a active event
 * 
 * Arguments:   Adafruit_GFX_Button - button background color value to be updated
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void setMenuToActiveState(Adafruit_GFX_Button *menu)
{
    menu->refillButton(ILI9341_BLUE);
}

/*******************************************************************************
 * Function:  setMenuToSelectActive
 * 
 * Description: A unique display for menu on a active event
 * 
 * Arguments:   Adafruit_GFX_Button - button background color value to be updated
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void setMenuToInactiveState(Adafruit_GFX_Button *menu)
{
    menu->refillButton(ILI9341_WHITE);
}

/*******************************************************************************
 * Function:  drawPlayStatus
 * 
 * Description: Display Current Play Status
 * 
 * Arguments:   char[] - status value to be displayed
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void drawPlayStatus(char *status)
{
    char buf[BUFFERSIZE];
    
    lcdCtrl.setCursor(STATUS_XCOORD, STATUS_YCOORD);
    lcdCtrl.setTextColor(ILI9341_BLACK, ILI9341_WHITE);  
    lcdCtrl.setTextSize(1);
    PrintToLcdWithBuf(buf, BUFFERSIZE, status);
}

// Renders a character at the current cursor position on the LCD
static void PrintCharToLcd(char c)
{
    lcdCtrl.write(c);
}
 /************************************************************************************

   Print a formated string with the given buffer to LCD.
   Each task should use its own buffer to prevent data corruption.

************************************************************************************/
void PrintToLcdWithBuf(char *buf, int size, char *format, ...)
{
    va_list args;
    va_start(args, format);
    PrintToDeviceWithBuf(PrintCharToLcd, buf, size, format, args);
    va_end(args);
}

/*******************************************************************************
 * Function:  PlayerWindowStateMachine
 * 
 * Description: State Machine for MP3 Player Window UI Inerface
 * 
 * Arguments:   Event_Type
 * 
 * Return Value: None
 *
 ******************************************************************************/
void PlayerWindowStateMachine(PlayerWindow *pWindow, Event_Type winEvent)
{
    OS_CPU_SR cpu_sr;
    switch (winEvent)
    {
    case EVENT_PLAY_PRESS:
        buttonPressResponse(&pWindow->button_list[PLAY]);
        pWindow->button_list[PLAY].drawButton();
             
        break;
    case EVENT_PLAY_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[PLAY]);
        pWindow->button_list[PLAY].drawButton();
                
        drawPlayStatus(player_status[PLAYING]);
        break;
    case EVENT_PAUSE_PRESS:
        buttonPressResponse(&pWindow->button_list[PAUSE]);
        pWindow->button_list[PAUSE].drawButton();
        break; 
    case EVENT_PAUSE_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[PAUSE]);
        pWindow->button_list[PAUSE].drawButton();
        
        drawPlayStatus(player_status[PAUSED]);
        break;
    case EVENT_STOP_PRESS:
        buttonPressResponse(&pWindow->button_list[STOP]);
        pWindow->button_list[STOP].drawButton();
        break;
    case EVENT_STOP_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[STOP]);
        pWindow->button_list[STOP].drawButton();
        
        setMenuToActiveState(&pWindow->status_box);
        pWindow->status_box.drawButton(false, false);
        
        OSTimeDly(50);
        
        setMenuToInactiveState(&pWindow->status_box);
        pWindow->status_box.drawButton(false, false);
        
        statusBarBtnCnt = -1;
        
        drawPlayStatus(player_status[STOPPED]);
        break;
    case EVENT_REWIND_PRESS:
        buttonPressResponse(&pWindow->button_list[REWIND]);
        pWindow->button_list[REWIND].drawButton();
        break;
    case EVENT_REWIND_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[REWIND]);
        pWindow->button_list[REWIND].drawButton();
        
        break;
    case EVENT_FF_PRESS:
        buttonPressResponse(&pWindow->button_list[FF]);
        pWindow->button_list[FF].drawButton();
        break;
    case EVENT_FF_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[FF]);
        pWindow->button_list[FF].drawButton();
            
        break;
    case EVENT_UP_PRESS:
        buttonPressResponse(&pWindow->button_list[UP]);
        pWindow->button_list[UP].drawButton();
        
        break;
    case EVENT_UP_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[UP]);
        pWindow->button_list[UP].drawButton();
        
        if((currMenuSelectCounter-1) == -1)
        {
            if(getActiveButtonCount(false) > 0)
            {
                activeMenuBtnCnt = getActiveButtonCount(false);
                
                // Set last button to active state before resetting button values
                setMenuToActiveState(&pWindow->menu_list[currMenuSelectCounter]);
                pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
                
                currSongFilePntr--;
                
                // Reinit with new values
                InitMenuLabels(pWindow,false);
                
                currMenuSelectCounter = activeMenuBtnCnt-1;
                // Set last menu button as selected
                setMenuToSelectState(&pWindow->menu_list[currMenuSelectCounter]);
                pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
            }
            else
            {
                //need to be dealt here
                activeMenuBtnCnt = getActiveButtonCount(true);
            }
        }
        else if(activeMenuBtnCnt != 0)
        {
            setMenuToActiveState(&pWindow->menu_list[currMenuSelectCounter]);
            pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
            
            OS_ENTER_CRITICAL();
            currSongFilePntr--;
            OS_EXIT_CRITICAL();

            currMenuSelectCounter--;
            
            setMenuToSelectState(&pWindow->menu_list[currMenuSelectCounter]);
            pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
        }
        else
        {
        }
        break;
    case EVENT_DOWN_PRESS:
        buttonPressResponse(&pWindow->button_list[DOWN]);
        pWindow->button_list[DOWN].drawButton();
        break;
    case EVENT_DOWN_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[DOWN]);
        pWindow->button_list[DOWN].drawButton();
        
        if((currMenuSelectCounter+1) == activeMenuBtnCnt)
        {
            if(getActiveButtonCount(true) > 0)
            {
                activeMenuBtnCnt = getActiveButtonCount(true);
                    
                // Set last button to active state before resetting button values
                setMenuToActiveState(&pWindow->menu_list[currMenuSelectCounter]);
                pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
                
                OS_ENTER_CRITICAL(); ;
                currSongFilePntr++;
                OS_EXIT_CRITICAL();
                
                // Reinit with new values
                InitMenuLabels(pWindow,true);
                
                currMenuSelectCounter = 0;
                
                // Set first menu button as selected
                setMenuToSelectState(&pWindow->menu_list[currMenuSelectCounter]);
                pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
            }
            else
            {
                activeMenuBtnCnt = getActiveButtonCount(false);
            }
        }
        else if(activeMenuBtnCnt != 0 && currSongFilePntr != sizeOfList -1)
        {
            setMenuToActiveState(&pWindow->menu_list[currMenuSelectCounter]);
            pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
            
            OS_ENTER_CRITICAL();
            currSongFilePntr++;
            OS_EXIT_CRITICAL();

            currMenuSelectCounter++;
            
            setMenuToSelectState(&pWindow->menu_list[currMenuSelectCounter]);
            pWindow->menu_list[currMenuSelectCounter].drawButton(false, true);
        }
            
        break;
    case EVENT_VOLPLUS_PRESS:
        buttonPressResponse(&pWindow->button_list[VOLPLUS]);
        pWindow->button_list[VOLPLUS].drawButton();
        
        break;
    case EVENT_VOLPLUS_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[VOLPLUS]);
        pWindow->button_list[VOLPLUS].drawButton();
        
        if(volumeBarBtnCnt < (INT8S)(VOLUMEBARSNUM-1))
        {
            volumeBarBtnCnt += 1;
            updateVolumeBar(&pWindow->vol_bar[volumeBarBtnCnt], OS_TRUE);
            pWindow->vol_bar[volumeBarBtnCnt].drawButton();
        }
        break;
    case EVENT_VOLMINUS_PRESS:
        buttonPressResponse(&pWindow->button_list[VOLMINUS]);
        pWindow->button_list[VOLMINUS].drawButton();
        
        break;
    case EVENT_VOLMINUS_RELEASE:
        buttonReleaseResponse(&pWindow->button_list[VOLMINUS]);
        pWindow->button_list[VOLMINUS].drawButton();
        
        if(volumeBarBtnCnt >= 0)
        {
            updateVolumeBar(&pWindow->vol_bar[volumeBarBtnCnt]);
            pWindow->vol_bar[volumeBarBtnCnt].drawButton();
            volumeBarBtnCnt--;
        }
        break;
    case EVENT_STATUSBAR_INC:
        if(statusBarBtnCnt < (INT8S)(STATUSBARSNUM-1))
        {
            statusBarBtnCnt += 1;
            updateStatusBar(&pWindow->status_bar[statusBarBtnCnt], OS_TRUE);
            pWindow->status_bar[statusBarBtnCnt].drawButton();
        }
        break;
    case EVENT_STATUSBAR_DEC:
        // MP3 ensures status increment and decrement in the range of 1/10th
        
        if(statusBarBtnCnt >= 0)
        {
            updateStatusBar(&pWindow->status_bar[statusBarBtnCnt]);
            pWindow->status_bar[statusBarBtnCnt].drawButton();
            statusBarBtnCnt--;
        }
        
        break;
    case EVENT_NONE:
        break;
    default:
        break;
    }
}