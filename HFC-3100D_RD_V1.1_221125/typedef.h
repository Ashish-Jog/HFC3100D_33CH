/*
******************************************************************************************
* PROJECT : HFC3000(100A) MAIN CONTROL
* FILE NAME: TYPEDEF.H
* DEVICE : dsPIC30F6012A
* COMPILER : MPLAB-C V3.02 
* REVISION HISTORY
* 1) 2011.12.30 (V0.9)
* 
****************************************************************************************** */
#ifndef __TYPEDEF_H
#define __TYPEDEF_H
/********************************************************************************
 * Definitions
 ********************************************************************************/
#define sdiv(a,b)			__builtin_divsd(a,b)
#define abs(a)				((a)>0?(a):-(a))
#define max(a,b)			((a)>(b)?(a):(b))
#define min(a,b)			((a)<(b)?(a):(b))
#define limit(a,min,max)	((a)=((a)=(a)<(min)?(min):(a))>(max)?(max):(a))
 //-------------------------------------------------------

#define HIGH    					(1)
#define LOW     					(!HIGH)
		//
#define LEFT						(0)
#define RIGHT 					(1)
		//
#define ON      					(0)   //active low//
#define OFF     					(!ON)
#define	INPUT					(1)
#define	OUTPUT				(0)
		//
#define line1	0
#define line2	1
#define line3	2
#define line4	3
#define sline1 (0x80)
#define sline2 (0xc0)
#define sline3 (0x90)
#define sline4 (0xd0)
//---------------------------------------
#define CheckBit(x,y) 	((x)>>(y)) & (0x1)
#define SetBit(x,y) 		(x)|=((0x1)<<(y))
#define ClrBit(x,y) 		(x)&=~((0x1)<<(y))
				
/***************************************************************************
 *  generic type definitiON
 ***************************************************************************/
typedef signed char     CHAR ;							//8BIT
typedef unsigned char   UCHAR ;
typedef int             INT ;							//16BIT
typedef unsigned int    UINT ;
typedef long           	LONG ;						//32BIT
typedef unsigned long   ULONG ;
typedef float           FLOAT ;
typedef double          DOUBLE ;



typedef unsigned char  		BYTE ;
typedef unsigned int   		WORD;
typedef enum {false,true} 	BOOL;

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


#endif
//=======================================================

