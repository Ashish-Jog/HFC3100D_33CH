#ifndef __VAR_H
#define __VAR_H

/***************************************************************************
 *  generic type definitiON
 ***************************************************************************/
typedef signed char     	CHAR ;							//8BIT
typedef unsigned char       UCHAR ;
typedef int             	INT ;							//16BIT
typedef unsigned int    	UINT ;
typedef long           	 	LONG ;						//32BIT
typedef unsigned long       ULONG ;
typedef float           	FLOAT ;
typedef double          	DOUBLE ;

/* Compiler-independent, fixed size types */
typedef signed char         INT8 ;      /* Signed 8-bit integer    */
typedef unsigned char       UINT8 ;     /* Unsigned 8-bit integer  */
typedef signed short int    INT16 ;     /* Signed 16-bit integer   */
typedef unsigned short int  UINT16 ;    /* Unsigned 16-bit integer */
typedef long int            INT32 ;     /* Signed 32-bit integer   */
typedef unsigned long       UINT32 ;
typedef float               FLOAT32 ;   /* 32-bit IEEE single precisiON */
typedef double              FLOAT64 ;   /* 64-bit IEEE double precisiON */

typedef unsigned char  		BYTE ;
typedef unsigned int   		WORD;

//typedef unsigned short int  uint16_t;    /* Unsigned 16-bit integer */


typedef union
{
    UINT8 _byte;
    struct
    {
        unsigned lsb :4,
                 msb :4;
    };
}BYTE_T;

/***************************************************************************
 *  specific definitiONs
 ***************************************************************************/
typedef struct 
{
	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;	
} bool8;

typedef struct 
{
	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;
	unsigned b8 :1,b9 :1,b10:1,b11:1,b12:1,b13:1,b14:1,b15:1;
} bool16;

/***************************************************************************
 *  Define 
 ***************************************************************************/

#define HEARTTMR    tmr1Cnt[0]
#define TM1         tmr1Cnt[1]
#define TM2         tmr1Cnt[2]
#define TM3         tmr1Cnt[3]
#define TM4         tmr1Cnt[4]
#define TM5         tmr1Cnt[5]    
#define TM6         tmr1Cnt[6]
#define TM7         tmr1Cnt[7]    
#define TM8         tmr1Cnt[8]  //uart period//  
#define WATCHTMR    tmr1Cnt[9]

#define CheckBit(x,y) 	((x)>>(y)) & (0x1)
#define SetBit(x,y) 	(x)|=((0x1)<<(y))
#define ClrBit(x,y) 	(x)&=~((0x1)<<(y))
/***************************************************************************
 *  Declared Variables//
 ***************************************************************************/
extern volatile UINT16 tmr1Cnt[10];//increment counter//
extern volatile UINT16 tmr1Tick;   
extern volatile UINT16 tmr1Tick2;
extern volatile UINT16 tmr1Dec;//decrement counter//
//
extern UINT16 iTestVal;
extern UINT8 iHr, iMin, iSec, iSecOld;
extern UINT16 iMainCnt;
extern UINT16 iMainTmp;
//
extern UINT8 iLcdBackLightTm;
//====================================================================================//
//                          Variables Declaration                                     //
//====================================================================================//
//Global
extern UINT8 data_8,gCnt_8,buff_8;
extern UINT16 data_16,gCnt_16,buff_16;
extern UINT8 gCnt;
//Timer
extern UINT16 tmr1Count0,tmr1Count1,tmr1Count2,tmr1Count3,tmr1Count4;
extern UINT16 tmr2Count0,tmr2Count1,tmr2Count2,tmr2Count3,tmr2Count4;
extern UINT16 tmr50ms,tmr100ms,tmr200ms,tmr500ms,tmr1s;
extern UINT16 cursor100ms,cursor1s;
extern UINT16 lcdBlink;
extern UINT8 errBz100ms;
extern UINT16 key1s;
extern UINT16 taskCnt;
extern UINT16 serialCnt;
extern UINT8 tmrBz;
extern UINT8 fBzOn;
//Key
extern UINT8 uc_key,fKey,SetKeyOk,fKeyAutoManual,fKeyNoSelect;
//Memory
extern UINT8 gPass[10];
extern UINT8 SgPass[10];
extern UINT8 gBattVoltSet[10];
extern UINT8 gRestartTime[2];
//Flag
//Count
extern UINT8 Key_Cnt;
//
//comm.
extern UINT8   Set_OK;
//extern UINT8	gOperationMode;
//extern UINT8   gSysTypeMode;
//extern float BattHiVoltage;
//xtern float BattLowVoltage;
extern UINT8   gBattTypeMode;
extern UINT8   gInVoltMode;
//
extern UINT8 gauge_empty[8];
extern UINT8 gauge_fill_1[8];
extern UINT8 gauge_fill_2[8];
extern UINT8 gauge_fill_3[8];
extern UINT8 gauge_fill_4[8];
extern UINT8 gauge_fill_5[8];
extern UINT8 heart_icon[8];
//
extern UINT8 gauge_left[8];
extern UINT8 gauge_right[8];

extern UINT8 gauge_mask_left[8];
extern UINT8 gauge_mask_right[8];
//
extern UINT8 gauge_left_dynamic[8];   // left part of gauge dynamic - will be set in the loop function
extern UINT8 gauge_right_dynamic[8];  // right part of gauge dynamic - will be set in the loop function
//
extern int cpu_gauge;       // value for the CPU gauge
extern char buffer[17];         // helper buffer to store C-style strings (generated with sprintf function)
extern int move_offset;     // used to shift bits for the custom characters

extern int gauge_size_chars;       // width of the gauge in number of characters
extern char gauge_string[17]; // string that will include all the gauge character to be printed

//RTC
//extern UINT8 day,mth,year,dow,hour,min,sec,sec_old;
#endif