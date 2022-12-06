/**
  CHARLCD1 Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    charlcd1.h

  @Summary
    This is the generated header file for the CHARLCD1 driver using Board Support Library

  @Description
    This header file provides APIs for driver for CHARLCD1.
    Generation Information :
        Product Revision  :  Board Support Library - 2.0.0
        Device            :  dsPIC33CH128MP506
        Driver Version    :  0.95
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.50
        MPLAB 	          :  MPLAB X v5.40
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#ifndef CHARLCD1_H
#define CHARLCD1_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
//====================================================================
//Display
#define M0_LOGO					1
#define M0_OUTBATT				2
#define M0_LOAD					3
#define M0_SETUP				4
#define M0_HISTORY				5
#define M0_TIME					6
//
#define M1_SETUPSET				10
#define M1_BATTALARM			11
#define M1_MODULERESTART		12
#define M1_TIMESET				13
#define M1_SYSTEMINIT			14
#define M1_PASSWORD				15
#define M1_ALARMSTOP			16
//
#define M2_BATTALARMSET			20
#define M2_MODULERESTARTSET		21
#define M2_SYSTEMINITSET		22
#define M2_PASSWORDSET			23
#define M2_ALARMSTOPSET			24
//
#define M3_System				30
#define M3_BATTTYPE             31
#define M3_INPUTVOLT            32
#define M3_OPERATIONMODE		33
#define M3_SYSTYPE              34
//
#define M4_SystemSet			40
#define M4_BATTTYPESET          41
#define M4_INPUTVOLTSET         42
#define M4_OPERATIONMODESET		43
#define M4_SYSTYPESET           44

/*********************************************************************
* Function: bool CHARLCD1_Initialize(void);
*
* Overview: Initializes the LCD screen.  Can take several hundred
*           milliseconds.
*
* PreCondition: none
*
* Input: None
*
* Output: true if successful, false otherwise
*
********************************************************************/
bool CHARLCD1_Initialize(void);

/*********************************************************************
* Function: void CHARLCD1_PutString(char* inputString, uint16_t length);
*
* Overview: Puts a string on the LCD screen.  Unsupported characters will be
*           discarded.  May block or throw away characters is LCD is not ready
*           or buffer space is not available.  Will terminate when either a
*           null terminator character (0x00) is reached or the length number
*           of characters is printed, which ever comes first.
*
* PreCondition: already initialized via CHARLCD1_PutString()
*
* Input: char* - string to print
*        uint16_t - length of string to print
*
* Output: None
*
********************************************************************/
void CHARLCD1_PutString(char* inputString, uint16_t length);

/*********************************************************************
* Function: void CHARLCD1_PutChar(char);
*
* Overview: Puts a character on the LCD screen.  Unsupported characters will be
*           discarded.  May block or throw away characters is LCD is not ready
*           or buffer space is not available.
*
* PreCondition: already initialized via CHARLCD1_PutChar()
*
* Input: char - character to print
*
* Output: None
*
********************************************************************/
void CHARLCD1_PutChar(char);

/*********************************************************************
* Function: void CHARLCD1_ClearScreen(void);
*
* Overview: Clears the screen, if possible.
*
* PreCondition: already initialized via CHARLCD1_ClearScreen()
*
* Input: None
*
* Output: None
*
********************************************************************/
void CHARLCD1_ClearScreen(void);

/*********************************************************************
* Function: void CHARLCD1_CursorEnable(bool enable)
*
* Overview: Enables/disables the cursor
*
* PreCondition: None
*
* Input: bool - specifies if the cursor should be on or off
*
* Output: None
*
********************************************************************/
void CHARLCD1_CursorEnable(bool enable);
//--------------------------------------------------------------
void setcurLCD(unsigned char CurX, unsigned char CurY);
//
void LCD_S_Delay(void);
//
void createChar(unsigned char location, unsigned char charmap[]);
void customChar(unsigned char index);
//==============================================================================
extern void MainProcess(void);
extern void DispLogo(void);
extern void DispOutBatt(void);
extern void DispLoad(void);
extern void DispSetup(void);
extern void SysSetup(void);
extern void SysSetupSet(void);
extern void DispSetupSet(void);
extern void DispBattAlarm(void);
extern void DispBattAlarmSet(void);
//
extern void DispModuleRestart(void);
extern void DispModuleRestartSet(void);
extern void DispSystemInit(void);
extern void DispSystemInitSet(void);
extern void DispPassword(void);
extern void DispPasswordSet(void);
extern void DispHistory(void);
extern void DispTime(void);
extern void DispTimeSet(void);
extern void DispAlarmStop(void);
extern void DispAlarmStopSet(void);
extern void DispBattType(void);
extern void DispBattTypeSet(void);
extern void DispInputVoltType(void);
extern void DispInputVoltTypeSet(void);
extern void DispOperationMode(void);
extern void DispOperationModeSet(void);
extern void DispSysType(void);
extern void DispSysTypeSet(void);

extern void SelectModule(void);
extern void AutoManual(void);

//ALARMHISTORY
extern void AlarmHistoryDisplay(uint8_t index);
extern void AlarmOccurrenceApply(unsigned int packetsize);
//RTC
extern void rtc_get_date();
extern void rtc_get_time();
//LED Status, Module Status
extern void LedStatus(void);
extern void ModuleStatus(void);
extern void BatteryStatus(void);
extern void WarningStatus(void);
extern void ModuleStartupStatus(void);
extern void ChargeStatus(void);
extern void dcGroundCheck(void);
//Buzzer
extern void bzOperation(void);
//
//Global Variable
extern uint8_t gTmp;

//Flag Variable
extern uint8_t fTimeSet;
extern uint8_t fAlarmStop;

extern uint16_t Temp_LCD_DATA;
extern uint8_t Temp_CMD;
extern uint8_t Str_Temp;
extern uint8_t iMenuIndex;
extern uint8_t iMenuCnt;

extern uint16_t Vvalue;
extern uint16_t Avalue;
extern uint8_t KeyValue;
//UINT8 uc_key=0;

extern int8_t inputCash;
extern uint8_t fPassSet;
extern uint8_t passWord[10];
extern uint8_t SpassWord[10];

//AlarmHistory
extern uint8_t gAlarmHistory[200][9];
extern uint8_t gAlarmHistoryCnt;
extern uint8_t alarmList[38];
extern uint8_t gAlarmStatus[25];
extern uint8_t gC_AlarmHistoryCnt;
extern uint8_t prev_alarm_name;
extern uint8_t gOldState;
extern uint8_t dcGroundCnt;
extern uint8_t ErrCode;
extern uint8_t ErrCnt;
extern uint8_t fCheckErr;
extern uint8_t fNfbCheckErr;
extern uint8_t gPacketsize;
//RTC
extern uint8_t day,mth,year,dow,hour,min,sec,sec_old;

//LED STATUS
extern uint8_t StatusBuf[20];

//Memory Set Data

//NFB STATUS
extern uint8_t fM1_NFB,fM2_NFB;

//Buzzer
extern uint8_t ferrBz[5];

//Battery Charge,Discharge Flag
extern uint8_t fCharge;

extern uint8_t Module1_Sts[8];
extern uint8_t Module2_Sts[8];
extern uint8_t Battery_Sts[8];
extern uint8_t Warning_Sts[8];
extern uint8_t ModuleStartup_Sts[2];
extern uint8_t Module_CHG_Sts[8];


extern uint8_t prev_Module1_Sts[8];
extern uint8_t prev_Module2_Sts[8];
extern uint8_t prev_Battery_Sts[8];
extern uint8_t prev_Warning_Sts[8];
extern uint8_t prev_ModuleStartup_Sts[2];
extern uint8_t prev_Module_CHG_Sts[8];
extern uint8_t fNewErr;
extern uint8_t test[10];

#endif
