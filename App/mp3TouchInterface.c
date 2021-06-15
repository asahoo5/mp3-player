/*
    mp3TouchInterface.c
    MP3 Player Touch Interface Implementation

    Developed for University of Washington embedded systems programming certificate
    
    2021/3 Abhilash Sahoo wrote/arranged it
*/

#include "mp3TouchInterface.h"

static long MapTouchToScreen(long x, long in_min, long in_max, long out_min, long out_max);
Event_Type event;

// The touch controller Instance
Adafruit_FT6206 touchCtrl = Adafruit_FT6206();

// Button Status
// It's 0th - 8th bits (mapped with PlayerButtonIndex) are used to track button status
// Set to 1 if button press activity is carried out, else default 0 (release/noactivity)
static unsigned int ButtonStatus = 0;
//static HANDLE hI2c = 0;

// Function Definitions
static boolean IsAnyButtonsPressed (void);
static void SetButtonStatus (unsigned int index);
static void ResetButtonStatus (unsigned int index);
static unsigned int WhichButtonWasPressed (void);

/*******************************************************************************
 * Function:  SetUpTouchInterface
 * 
 * Description: Set Up Touch Controller and related interface.
 * 
 * Arguments:  
 * 
 * Return Value: None
 *
 ******************************************************************************/
void SetUpTouchInterface(void)
{
    PjdfErrCode pjdfErr;
        
    // Open a HANDLE for accessing device PJDF_DEVICE_ID_I2C1
    HANDLE hI2c = Open(PJDF_DEVICE_ID_I2C1, 0);
    if (!PJDF_IS_VALID_HANDLE(hI2c)) while(1);
    
    // Call Ioctl on that handle to set the I2C device address to FT6206_ADDR
    INT8U addressFT6206 = (FT6206_ADDR << 1);
    pjdfErr = Ioctl(hI2c, PJDF_CTRL_I2C_SET_DEVICE_ADDRESS, &addressFT6206, NULL);
    if(PJDF_IS_ERROR(pjdfErr)) while(1);
    
    // Call setPjdfHandle() on the touch contoller to pass in the I2C handle
    touchCtrl.setPjdfHandle(hI2c);

    if (! touchCtrl.begin(40)) {  // pass in 'sensitivity' coefficient
        //PrintWithBuf(buf, BUFSIZE, "Couldn't start FT6206 touchscreen controller\n");
        while (1);
    }
}

/*******************************************************************************
 * Function:  TouchEventGenerator
 * 
 * Description: It maps the touch points w.r.t screen orientation, if touch points
 *              match with any MP3 Player window buttons - updates button's press 
 *              status. And then based on press/release status, it sends events in mailbox
 * Note:        Ensures not to send multiple press/release events of a button
 *              Also handles one button press/release event at a time 
 * 
 * Arguments:  
 * 
 * Return Value: None
 *
 ******************************************************************************/
void TouchEventGenerator (PlayerWindow pWindow)
{
    boolean touched;
    
    // Get touch status
    touched = touchCtrl.touched();
    
    if(IsAnyButtonsPressed() & !touched)
    {
        unsigned int button;
        // implies button was released, so send a release event for the respective button
        button = WhichButtonWasPressed();
        
        // button events were numbered w.r.t to number of events in system - release is just a numeric up
        event  = (Event_Type) (button * NUM_TYPE_OF_EVENTS + BUTTON_RELEASED);
        OSMboxPost(touchEventsMbox, (void*)&event);
        // Rest button Status
        ResetButtonStatus(button);
    }
    else if( !(IsAnyButtonsPressed()) & touched)
    {
        TS_Point point;
        TS_Point p = TS_Point();
        
        point = touchCtrl.getPoint();
        
        if (point.x == 0 && point.y == 0)
        {
            //break; // usually spurious, so ignore
        }
        
        // transform touch orientation to screen orientation.
        p.x = MapTouchToScreen(point.x, 0, ILI9341_TFTWIDTH, ILI9341_TFTWIDTH, 0);
        p.y = MapTouchToScreen(point.y, 0, ILI9341_TFTHEIGHT, ILI9341_TFTHEIGHT, 0);
        
        // If button is pressed, set button status and send button status event
        for(unsigned int button = PLAY; button < PLAYERBUTTONNUM; ++button)
        {
            if (pWindow.button_list[button].contains(p.x, p.y)) {
                
                event  = (Event_Type) (button * NUM_TYPE_OF_EVENTS + BUTTON_PRESSED);
                OSMboxPost(touchEventsMbox, (void*)&event);
                
                SetButtonStatus(button);
            }
        }            
    }
    else
    {
        // TBD
    }
        
}

/*******************************************************************************
 * Function:  MapTouchToScreen 
 * 
 * Description: Transform touch orientation to the screen orientation 
 * 
 * Arguments:    x - x or y coordinate value
 *               in_main - starting x or y axis value
 *               in_max - end x or y axis value
 *               out_min - end x or y axis value
 *               out_max - starting x or y axis value
 * 
 * Return Value: screen equivalent touch point (x or y coordinate)
 *
 ******************************************************************************/
static long MapTouchToScreen(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*******************************************************************************
 * Function:  IsAnyButtonsPressed
 * 
 * Description: Sends true, if any of the button is pressed at any given time
 *
 * Note:        Buttons could be either pressed or released state
 * 
 * Arguments:  None 
 * 
 * Return Value: boolean
 *
 ******************************************************************************/
static boolean IsAnyButtonsPressed (void)
{
    return (ButtonStatus > 0);
}

/*******************************************************************************
 * Function:  SetButtonStatus
 * 
 * Description: Sets respective index (identified as button) of ButtonStatus
 * 
 * Arguments:  index 
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void SetButtonStatus (unsigned int index)
{
    ButtonStatus |= (1 << index);
}

/*******************************************************************************
 * Function:  ResetButtonStatus
 * 
 * Description: Resets respective index (identified as button) of ButtonStatus
 * 
 * Arguments:  index 
 * 
 * Return Value: None
 *
 ******************************************************************************/
static void ResetButtonStatus (unsigned int index)
{
    ButtonStatus &= ~(1 << index);
}

/*******************************************************************************
 * Function:  WhichButtonWasPressed
 * 
 * Description: Get the index of button that was pressed
 *              To be used if any button press event had occured (
 *              i.e. call IsAnyButtonsPressed() before)
 * 
 * Arguments:  None 
 * 
 * Return Value: unsigned int -> button index value
 *
 ******************************************************************************/
static unsigned int WhichButtonWasPressed (void)
{
    unsigned int index = PLAY;
    int item = ButtonStatus;
    while (item > 1)
    {
        item >>= 1;
        ++index;
    }
    return index;
}