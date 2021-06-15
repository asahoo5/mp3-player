/*
    bspMp3.c

    Board support for controlling Adafruit Music Maker VS1053 MP3 decoder shield via NUCLEO-F401RE MCU

    Developed for University of Washington embedded systems programming certificate
    
    2016/2 Nick Strathy wrote/arranged it
*/

#include "bsp.h"

// some command strings to send to the VS1053 MP3 decoder:
const INT8U BspMp3SineWave[] =  { 0x53, 0xEF, 0x6E, 0x44, 0, 0, 0, 0 };
const INT8U BspMp3Deact[] =     { 0x45, 0x78, 0x69, 0x74, 0, 0, 0, 0 };
const INT8U BspMp3TestMode[] =  { 0x02, 0x00, 0x08, 0x20 };
const INT8U BspMp3PlayMode[] =  { 0x02, 0x00, 0x08, 0x00 };
const INT8U BspMp3SoftReset[] = { 0x02, 0x00, 0x08, 0x04 };
const INT8U BspMp3SetClockF[] = { 0x02, 0x03, 0x98, 0x00 };
//const INT8U BspMp3SetVol1010[] = { 0x02, 0x0B, 0x10, 0x10 };
//const INT8U BspMp3SetVol6060[] = { 0x02, 0x0B, 0x60, 0x60 };
const INT8U BspMp3ReadVol[] = { 0x3, 0x0B, 0x00, 0x00 };

//Definition of a Volume Range (Min to Max) going down by 1/10th
const INT8U BspMp3SetVolRange[MP3_VOLUME_RANGE][DATA_FRAME] = {
                                                { 0x02, 0x0B, 0x80, 0x80 },
                                                { 0x02, 0x0B, 0x70, 0x70 },
                                                { 0x02, 0x0B, 0x60, 0x60 },
                                                { 0x02, 0x0B, 0x58, 0x58 },
                                                { 0x02, 0x0B, 0x50, 0x50 },
                                                { 0x02, 0x0B, 0x48, 0x48 },
                                                { 0x02, 0x0B, 0x40, 0x40 },
                                                { 0x02, 0x0B, 0x38, 0x38 },
                                                { 0x02, 0x0B, 0x30, 0x30 },
                                                { 0x02, 0x0B, 0x28, 0x28 },
                                                { 0x02, 0x0B, 0x20, 0x20 },
                                                { 0x02, 0x0B, 0x18, 0x18 },
                                                { 0x02, 0x0B, 0x10, 0x10 },
                                                { 0x02, 0x0B, 0x0A, 0x0A }
                                                };

// Lengths of the above commands
const INT8U BspMp3SineWaveLen = sizeof(BspMp3SineWave);
const INT8U BspMp3DeactLen = sizeof(BspMp3Deact);
const INT8U BspMp3TestModeLen = sizeof(BspMp3TestMode);
const INT8U BspMp3PlayModeLen = sizeof(BspMp3PlayMode);
const INT8U BspMp3SoftResetLen = sizeof(BspMp3SoftReset);
const INT8U BspMp3SetClockFLen = sizeof(BspMp3SetClockF);
//const INT8U BspMp3SetVol1010Len = sizeof(BspMp3SetVol1010);
const INT8U BspMp3SetVolLen = sizeof(BspMp3SetVolRange[0]);
const INT8U BspMp3ReadVolLen = sizeof(BspMp3ReadVol);




// Initializes GPIO pins for the VS1053 MP3 device.
void BspMp3InitVS1053()
{
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);

    LL_GPIO_InitTypeDef GPIO_InitStruct;
    
    /*-------- Configure MCS ChipSelect Pin, asserts low, selects command interface PA8--------*/ 
 
    GPIO_InitStruct.Pin = MP3_VS1053_MCS_GPIO_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
     
    LL_GPIO_Init(MP3_VS1053_MCS_GPIO, &GPIO_InitStruct);
    MP3_VS1053_MCS_DEASSERT();
 
    /*-------- Configure DCS ChipSelect Pin, asserts low, selects data interface PB10--------*/ 
 
    GPIO_InitStruct.Pin = MP3_VS1053_DCS_GPIO_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
     
    LL_GPIO_Init(MP3_VS1053_DCS_GPIO, &GPIO_InitStruct);
    MP3_VS1053_DCS_DEASSERT();
   
    /*-------- Configure DREQ data request interrupt, asserts high when ready for data PB3--------*/ 
 
    GPIO_InitStruct.Pin = MP3_VS1053_DREQ_GPIO_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_DOWN;
     
    LL_GPIO_Init(MP3_VS1053_DREQ_GPIO, &GPIO_InitStruct);
}