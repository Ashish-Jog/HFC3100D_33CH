/*
 * File:   var.c
 * Author: RN04
 *
 * Created on May 13, 2022, 3:48 PM
 */

/***************************************************************************
 *  declared Variables//
 ***************************************************************************/
#include "var.h"

volatile UINT16 tmr1Cnt[10] = {0,0,0,0,0,0,0,0,0,0};//increment counter//
volatile UINT16 tmr1Tick=0;    
volatile UINT16 tmr1Tick2=0;
volatile UINT16 tmr1Dec=0;//decrement counter//
//
UINT16 iTestVal=0;
UINT8 iHr=0, iMin=0, iSec=0, iSecOld;
UINT16 iMainCnt=0;
UINT16 iMainTmp=0;
//
UINT8 iLcdBackLightTm=0;
//====================================================================================//
//                          Variables Declaration                                     //
//====================================================================================//
//Global
UINT8 data_8,gCnt_8,buff_8;
UINT16 data_16,gCnt_16,buff_16;
UINT8 gCnt;
//Timer
UINT16 tmr1Count0=0,tmr1Count1=0,tmr1Count2=0,tmr1Count3=0,tmr1Count4=0;
UINT16 tmr2Count0=0,tmr2Count1=0,tmr2Count2=0,tmr2Count3=0,tmr2Count4=0;
UINT16 tmr50ms=0,tmr100ms=0,tmr200ms=0,tmr500ms=0,tmr1s=0;
UINT16 cursor100ms=0,cursor1s=0;
UINT16 lcdBlink=0;
UINT8 errBz100ms=0;
UINT16 key1s=0;
UINT16 taskCnt=0;
UINT16 serialCnt=0;
UINT8 tmrBz=0;
UINT8 fBzOn=0;
//Key
UINT8 uc_key=0,fKey=0,SetKeyOk=0,fKeyAutoManual=0,fKeyNoSelect=0;
//Memory
UINT8 gPass[10]={'0','0','0','0','0','0','0','0','0','0'};
UINT8 SgPass[10]={'1','1','1','1','1','1','1','1','1','1'};
UINT8 gBattVoltSet[10]={'1','5','0','0','1','0','5','0','0','0'};
UINT8 gRestartTime[2]={'1',};
//Flag
//Count
UINT8 Key_Cnt=0;

//comm.
//
UINT8   Set_OK = 0;
//UINT8	gOperationMode = 0;
//UINT8   gSysTypeMode = 0;
//float BattHiVoltage=0;
//float BattLowVoltage=0;
UINT8   gBattTypeMode=0;
UINT8   gInVoltMode=0;
//
UINT8 gauge_empty[8] =  {0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111};    // empty middle piece
UINT8 gauge_fill_1[8] = {0b11111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111};    // filled gauge - 1 column
UINT8 gauge_fill_2[8] = {0b11111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11111};    // filled gauge - 2 columns
UINT8 gauge_fill_3[8] = {0b11111, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11100, 0b11111};    // filled gauge - 3 columns
UINT8 gauge_fill_4[8] = {0b11111, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11111};    // filled gauge - 4 columns
UINT8 gauge_fill_5[8] = {0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111};    // filled gauge - 5 columns
UINT8 heart_icon[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000};     // warning icon - just because we still have one custom character left
//
UINT8 gauge_left[8] =   {0b11111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111};    // left part of gauge - empty
UINT8 gauge_right[8] =  {0b11111, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b00001, 0b11111};    // right part of gauge - empty

UINT8 gauge_mask_left[8] = {0b01111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b01111};  // mask for rounded corners for leftmost character
UINT8 gauge_mask_right[8] = {0b11110, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11110}; // mask for rounded corners for rightmost character
//
UINT8 gauge_left_dynamic[8];   // left part of gauge dynamic - will be set in the loop function
UINT8 gauge_right_dynamic[8];  // right part of gauge dynamic - will be set in the loop function
//
int cpu_gauge = 0;       // value for the CPU gauge
char buffer[17];         // helper buffer to store C-style strings (generated with sprintf function)
int move_offset = 0;     // used to shift bits for the custom characters
int gauge_size_chars=16;
char gauge_string[17]; // string that will include all the gauge character to be printed

//RTC(ds1302N)
//UINT8 day,mth,year,dow,hour,min,sec,sec_old;

