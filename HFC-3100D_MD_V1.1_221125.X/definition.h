/*
**********************************************************
* PROJECT : HFC3100D MAIN DISPLAY
* FILE NAME: Definition.h
* DEVICE : dsPIC33CH128MP506
* COMPILER : MPLABX(6.0)-XC16(2.0)
* REVISION HISTORY
* 1) 2022.05.24 (V1.0~~)
* 
********************************************************** */
#ifndef __DEFINITION_H
#define __DEFINITION_H

#include "var.h"

//EEPROM Address
#define INIT_STATE				1
#define BATT_TYPE               2 //0 pb125, 1 pb110, 2 liIon
#define SYS_TYPE                3
#define OP_MODE                 4
#define ALARM_STOP				9
//
#define PASSWD_0				10
#define PASSWD_1			    11
#define PASSWD_2			    12
#define PASSWD_3			    13
#define PASSWD_4			    14
#define PASSWD_5			    15
#define PASSWD_6			    16
#define PASSWD_7			    17
#define PASSWD_8			    18
#define PASSWD_9			    19

#define SPASSWD_0				30
#define SPASSWD_1			    31
#define SPASSWD_2			    32
#define SPASSWD_3			    33
#define SPASSWD_4			    34
#define SPASSWD_5			    35
#define SPASSWD_6			    36
#define SPASSWD_7			    37
#define SPASSWD_8			    38
#define SPASSWD_9			    39

//
#define BATTVOLT_0				20
#define BATTVOLT_1				21
#define BATTVOLT_2				22
#define BATTVOLT_3				23
#define BATTVOLT_4				24
#define BATTVOLT_5				25
#define BATTVOLT_6				26
#define BATTVOLT_7				27
#define BATTVOLT_8				28
#define BATTVOLT_9				29

#define RESTARTTIME_0			40
#define RESTARTTIME_1			41

//Alarm History
#define ALARM_HISTORY_CNT	    50
#define ALARM_STORAGE		    51

#define HIGH    			(1)
#define LOW     			(!HIGH)
		//
#define LEFT				(0)
#define RIGHT 				(1)
		//
#define ON     				(0)   //active low//
#define OFF    				(!ON)
#define	INPUT				(1)
#define	OUTPUT				(0)


////////////// Key Define ////////////////////////////////
#define KEY_ENTER		1
#define KEY_LEFT				2
#define KEY_UP					3
#define KEY_RIGHT				4
#define KEY_DOWN				5
#define KEY_AUTOMANUAL			6
#define KEY_NO_SELECT			7
#define KEY_LAMP				8
#define KEY_BZ_OFF				9

#define F_KEY_ENTER				0
#define F_KEY_LEFT				1
#define F_KEY_UP				2
#define F_KEY_RIGHT				3
#define F_KEY_DOWN				4
#define F_KEY_AUTOMANUAL		5
#define F_KEY_NO_SELECT			6
#define F_KEY_LAMP				7
#define F_KEY_BZ_OFF			8

#define SEQ_NOP         (0)
#define SEQ_SOME        (1)
#define SEQ_ON_TIME     (2)
#define SEQ_CATCH       (3)
#define SEQ_REJECT      (4)
#define SEQ_OFF         (5)
#define SEQ_OFF_TIME    (6)
#define SEQ_END         (7)

#define KEY_ON_TIME     (3) 
#define KEY_REJECT_TIME (36)
#define KEY_OFF_TIME    (5) 

#define     keyNULL						(0)
#define     keyKEY_ENTER				(1)
#define     keyKEY_LEFT					(2)
#define     keyKEY_UP					(3)
#define     keyKEY_RIGHT				(4)
#define     keyKEY_DOWN	 				(5)
#define     keyKEY_AUTOMANUAL			(6)
#define     keyKEY_NO_SELECT			(7)
#define     keyKEY_LAMP					(8)
#define     keyKEY_BZ_OFF				(9)
#define     keyKEY_ENTERfast			(10)
#define     keyKEY_LEFTfast				(11)
#define     keyKEY_UPfast				(12)
#define     keyKEY_RIGHTfast			(13)
#define     keyKEY_DOWNfast				(14)
#define     keyKEY_AUTOMANUALfast		(15)
#define     keyKEY_NO_SELECTfast		(16)
#define     keyKEY_LAMPfast				(17)
#define     keyKEY_BZ_OFFfast			(18)
//#define		keyKEY_L_R			(20)

#define DELAY_LONG_PRESSED (300)//(750) //org 500
//
#define     RS232_Period    300         //ms
//typedef signed char     	INT8 ;      /* Signed 8-bit integer    */
//typedef unsigned char      	UINT8 ;     /* Unsigned 8-bit integer  */
//typedef signed short int   	INT16 ;     /* Signed 16-bit integer   */
//typedef unsigned short int 	UINT16 ;    /* Unsigned 16-bit integer */
//typedef long int           	INT32 ;     /* Signed 32-bit integer   */
//typedef unsigned long      	UINT32 ;
//typedef float              	FLOAT32 ;   /* 32-bit IEEE single precisiON */
//typedef double             	FLOAT64 ;   /* 64-bit IEEE double precisiON */
//
//typedef unsigned char  		BYTE ;
//typedef unsigned int   		WORD;

//typedef union
//{
//    UINT8 _byte;
//    struct
//    {
//        unsigned lsb :4,
//                 msb :4;
//    };
//}BYTE_T;

/***************************************************************************
 *  specific definitiONs
 ***************************************************************************/
//typedef struct 
//{
//	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;	
//} bool8;
//
//typedef struct 
//{
//	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;
//	unsigned b8 :1,b9 :1,b10:1,b11:1,b12:1,b13:1,b14:1,b15:1;
//} bool16;



typedef struct  Tds1302_Time
{      
 		UINT8 Second;
 		UINT8 Minute;
 		UINT8 Hour;
 		UINT8 Date;
 		UINT8 Month;
 		UINT8 Year;
 		UINT8 Am;
 		UINT8 Md;
} ds1302_Time;
//struct Tds1302_Time  Ds1302_Time;

#define _NOP()		__asm__ volatile ("NOP")


////Display
//#define M0_LOGO					1
//#define M0_OUTBATT				2
//#define M0_LOAD					3
//#define M0_SETUP				4
//#define M0_HISTORY				5
//#define M0_TIME					6
////
//#define M1_SETUPSET				10
//#define M1_BATTALARM			11
//#define M1_MODULERESTART		12
//#define M1_TIMESET				13
//#define M1_SYSTEMINIT			14
//#define M1_PASSWORD				15
//#define M1_ALARMSTOP			16
////
//#define M2_BATTALARMSET			20
//#define M2_MODULERESTARTSET		21
//#define M2_SYSTEMINITSET		22
//#define M2_PASSWORDSET			23
//#define M2_ALARMSTOPSET			24
////
//#define M3_System				30
//#define M3_BATTTYPE             31
//#define M3_INPUTVOLT            32
//#define M3_OPERATIONMODE		33
//#define M3_SYSTYPE              34
////
//#define M4_SystemSet			40
//#define M4_BATTTYPESET          41
//#define M4_INPUTVOLTSET         42
//#define M4_OPERATIONMODESET		43
//#define M4_SYSTYPESET           44


#endif
//=======================================================


