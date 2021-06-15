/*
    mp3Util.c
    Some utility functions for controlling the MP3 decoder.

    Developed for University of Washington embedded systems programming certificate
    
    2016/2 Nick Strathy wrote/arranged it

    2021/2 Abhilash Sahoo updated to support MP3 Streaming task for the project
*/

#include "mp3Util.h"

#define DEFAULT_VOLUME_INDEX 8

void delay(uint32_t time);


static File dataFile;
static INT8U  mp3Buf[MP3_DECODER_BUF_SIZE];
static INT32U iBufPos = 0;
static INT32U iDataFileMovPos = 0;
static INT32U iDataFileCurPos = 0;
static INT32U iDataFileBegPos = 0;
static INT32U progressCounter = 0;
static INT8U  volProgressCounter = DEFAULT_VOLUME_INDEX;
static Event_Type mp3Event;


extern BOOLEAN isFileStart;
extern BOOLEAN isPlaying;
extern BOOLEAN isStopSong;
extern BOOLEAN isFastForward;
extern BOOLEAN isRewind;
extern BOOLEAN isVolUp;
extern BOOLEAN isVolDown;

extern INT32U currPlayingSongFilePntr;

void Mp3StreamInit(HANDLE hMp3)
{
    INT32U length;
    
    // Place MP3 driver in command mode (subsequent writes will be sent to the decoder's command interface)
    Ioctl(hMp3, PJDF_CTRL_MP3_SELECT_COMMAND, 0, 0);
    
    // Reset the device
    length = BspMp3SoftResetLen;
    Write(hMp3, (void*)BspMp3SoftReset, &length);
    
    length = BspMp3SetClockFLen;
    Write(hMp3, (void*)BspMp3SetClockF, &length);
 
    // Set volume
    length = BspMp3SetVolLen;
    Write(hMp3, (void*)BspMp3SetVolRange[volProgressCounter], &length);

    // To allow streaming data, set the decoder mode to Play Mode
    length = BspMp3PlayModeLen;
    Write(hMp3, (void*)BspMp3PlayMode, &length);
   
    // Set MP3 driver to data mode (subsequent writes will be sent to decoder's data interface)
    Ioctl(hMp3, PJDF_CTRL_MP3_SELECT_DATA, 0, 0);
}

// Volume Controller for the application
void Mp3VolumeControl(HANDLE hMp3, VolumeCounter state)
{
    INT32U length;
    
    if(state == VOLUP)
    {
        // Volume Range Index Sanity Check
        if(volProgressCounter == MP3_VOLUME_RANGE-1) return;
        
        ++volProgressCounter;
    }
    else
    {
        // Volume Range Index Sanity Check
        if(volProgressCounter == 0) return;
        
        --volProgressCounter;
    }
    
    // Place MP3 driver in command mode
    Ioctl(hMp3, PJDF_CTRL_MP3_SELECT_COMMAND, 0, 0);
    
    // Set volume
    length = BspMp3SetVolLen;
    Write(hMp3, (void*)BspMp3SetVolRange[volProgressCounter], &length);

    // To allow streaming data, set the decoder mode to Play Mode
    length = BspMp3PlayModeLen;
    Write(hMp3, (void*)BspMp3PlayMode, &length);
   
    // Set MP3 driver to data mode
    Ioctl(hMp3, PJDF_CTRL_MP3_SELECT_DATA, 0, 0);
        
}

// Mp3GetRegister
// Gets a register value from the MP3 decoder.
// hMp3: an open handle to the MP3 decoder
// cmdInDataOut: on entry this buffer must contain the 
//     command to get the desired register from the decoder. On exit
//     this buffer will be OVERWRITTEN by the data output by the decoder in 
//     response to the command. Note: the buffer must not reside in readonly memory.
// bufLen: the number of bytes in cmdInDataOut.
// Returns: PJDF_ERR_NONE if no error otherwise an error code.
PjdfErrCode Mp3GetRegister(HANDLE hMp3, INT8U *cmdInDataOut, INT32U bufLen)
{
    PjdfErrCode retval;
    if (!PJDF_IS_VALID_HANDLE(hMp3)) while (1);
    
    // Place MP3 driver in command mode (subsequent writes will be sent to the decoder's command interface)
    Ioctl(hMp3, PJDF_CTRL_MP3_SELECT_COMMAND, 0, 0);
    
    retval = Read(hMp3, cmdInDataOut, &bufLen);
    return retval;
}

// Mp3FetchFileNames
// Fetches a list of file names from the SD card 
// Note:  This function needs to be called at the beginning of the MP3Task()
// list - A string array where a list of songs will be stored
// size - tracks size of array as it grows
// maxSize - maximum allowable list
//void Mp3FetchFileNames(char **list, int maxRow, int col, int *sizeOfList)
void Mp3FetchFileNames()
{
    
    // Endless loop to read files from the root directory and stream them
    File dir = SD.open("/");
    char buf[13];
    while (sizeOfList <= MAXLISTOFSONGS)
    {
        File entry = dir.openNextFile();
        if (!entry)
        {
            break;
        }
        if (entry.isDirectory()) // skip directories
        {
            entry.close();
            continue;
        }
        
        char *filename = entry.name();
        for(int i = 0; i < SUPPFILENAMESIZE; ++i)
            listOfSongs[sizeOfList][i] = filename[i];
        PrintWithBuf(buf, 13, listOfSongs[sizeOfList]);
        
        sizeOfList++;
        entry.close();
    }
    dir.seek(0); // reset directory file to read again;
}

// Mp3StreamSDFile
// Streams the given file from the SD card to the given MP3 decoder.
// hMP3: an open handle to the MP3 decoder
// pFilename: The file on the SD card to stream. 

void Mp3StreamCycle(HANDLE hMp3)
{
    
    INT32U length;
    INT8U err = 0;
    
    Mp3StreamInit(hMp3);
    
	//char printBuf[PRINTBUFMAX];
    
    dataFile = SD.open(listOfSongs[currSongFilePntr], O_READ);
    if (!dataFile) 
    {
        //PrintWithBuf(printBuf, PRINTBUFMAX, "Error: could not open SD card file '%s'\n", listOfSongs[currSongFilePntr]);
        return;
    }

    // Initialize flags
    
    isStopSong = OS_FALSE;
    isFastForward = OS_FALSE;
    isRewind = OS_FALSE;
    isVolUp  = OS_FALSE; 
    isVolDown  = OS_FALSE; 
    
    progressCounter = 1;
    currPlayingSongFilePntr = currSongFilePntr;
    
    
    // this value will be used for increment/decrement song position .
    // A song data will be seen as ten parts and it will move accordingly
    iDataFileMovPos = dataFile.size()/10;
    iDataFileBegPos = dataFile.position();
    iDataFileCurPos = iDataFileBegPos;
        
    while (dataFile.available())
    {
        
        iBufPos = 0;
        //iDataFileCurPos =  dataFile.position();
        
        // if Paused stays in the loop and then picks when played again
        if(isPlaying)
        {
            while (dataFile.available() && iBufPos < MP3_DECODER_BUF_SIZE)
            {
                mp3Buf[iBufPos] = dataFile.read();
                
                if((dataFile.position() - iDataFileBegPos) == (progressCounter * iDataFileMovPos))
                {
                    mp3Event = EVENT_STATUSBAR_INC;
                    err = OSQPost(displayQMsg, (void*)&mp3Event);
                    
                    progressCounter++;
                }
                
                //delay(30);
                iBufPos++;
            }
           
            Write(hMp3, mp3Buf, &iBufPos);
            //OSTimeDly(5);
            
        }

        // new data file position
        iDataFileCurPos =  dataFile.position();
        
        // Fast Forward, Rewind, Vol+, Vol- and Stop functions should work if it
        // is Playing or Paused
        
        if(isFastForward)
        {
            //set new position
            iDataFileCurPos += iDataFileMovPos;
            //set the value in file datastructure
            dataFile.seek(iDataFileCurPos);
            //Send a Status Bar Update Event to display task
            mp3Event = EVENT_STATUSBAR_INC;
            err = OSQPost(displayQMsg, (void*)&mp3Event);
            
             progressCounter++;
                
            isFastForward = OS_FALSE;
            //OSFlagPost(mp3Flags, setFastForwardFlag, OS_FLAG_SET, &err);
        }
            
        if(isRewind)
        {
            iDataFileCurPos = (iDataFileMovPos >= iDataFileCurPos) ? iDataFileBegPos : 
                                            (iDataFileCurPos - iDataFileMovPos);
            //set the value in file datastructure
            dataFile.seek(iDataFileCurPos);
            
            //Send a Status Bar Update Event to display task
            mp3Event = EVENT_STATUSBAR_DEC;
            err = OSQPost(displayQMsg, (void*)&mp3Event);
            
             progressCounter--;
                
            
            isRewind = OS_FALSE;
        }
        
        if(isVolUp)
        {
            Mp3VolumeControl(hMp3, VOLUP);
            isVolUp = OS_FALSE;
        }
        
        if(isVolDown)
        {
            Mp3VolumeControl(hMp3, VOLDOWN);
            isVolDown = OS_FALSE;
        }
        
        if(isStopSong)
        {
            isStopSong = OS_FALSE;
            break;
        }
        
    }
    currPlayingSongFilePntr = INT_MAX;
    isPlaying = OS_FALSE;
    
    dataFile.close();
    
    mp3Event = EVENT_STOP_RELEASE;
    err = OSQPost(displayQMsg, (void*)&mp3Event);
    
    //OSFlagPost(mp3Flags, setPlayFlag | setPauseFlag, OS_FLAG_WAIT_SET_ALL, &err);
    
    Ioctl(hMp3, PJDF_CTRL_MP3_SELECT_COMMAND, 0, 0);
    length = BspMp3SoftResetLen;
    Write(hMp3, (void*)BspMp3SoftReset, &length);
}
