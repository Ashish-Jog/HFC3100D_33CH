//file name : mc. typeDef.h//

#define SerialDebug  0
#define adcTest  0  //0 is disable
//-------------------------------------------
#define CHG125V  1 //1 is 125, 0 is 110V
//------------------------------------------
//125V
#if(CHG125V == 1 )
    #define     Batt_UV_Ref     96.0           // 18.02.13 110 - KHJ
#else
    #define     Batt_UV_Ref     95.0           // 18.02.13 110 - KHJ
#endif

#if(CHG125V == 1 )
    #define     Batt_OV_Ref     150.0           // 18.02.13 143 - KHJ
#else
    #define     Batt_OV_Ref     135.0           // 18.02.13 143 - KHJ 110V
#endif

#define     RST_Time_Min    0.0             // Minimum Recovery System Restart Time
#define     RST_Time_Max    60.0            // Maximum Recovery System Restart Time 

#define		LDI_OFFSET		10
#define		ADCNT			15				//moving average counter.
#define		CHKERRCNT		50
#define		BATI_OFFSET		2014
#define     RS485_Period    1000             //ms
#define     RS232_RX_Delay  100              //ms


#define     sdiv(a,b)			__builtin_divsd(a,b)

#define Request		0x00				//Request Module's Voltage & Current
#define SetAll		0x01				//Set	 All Module's Voltage
#define SetID 		0x02				//Set ID's Module Set.
#define SetInit		0x00				//?????????.

#define CheckBit(x,y) 	((x)>>(y)) & (0x1)
#define SetBit(x,y) 		(x)|=((0x1)<<(y))
#define ClrBit(x,y) 		(x)&=~((0x1)<<(y))
				
#define     _NOP()		__asm__ volatile ("NOP")

#define     SYS_TYPE         10
/***************************************************************************
 *  generic type definitiON
 ***************************************************************************/

typedef int             		INT ;							//16BIT
typedef unsigned int            UINT ;


/* Compiler-independent, fixed size types */
typedef signed char         	INT8 ;      /* Signed 8-bit integer    */
typedef unsigned char       	UINT8 ;     /* Unsigned 8-bit integer  */
typedef signed short int    	INT16 ;     /* Signed 16-bit integer   */
typedef unsigned short int  	UINT16 ;    /* Unsigned 16-bit integer */
typedef long int            	INT32 ;     /* Signed 32-bit integer   */
typedef unsigned long       	UINT32 ;
typedef float               	FLOAT32 ;   /* 32-bit IEEE single precisiON */
typedef double              	FLOAT64 ;   /* 64-bit IEEE double precisiON */

typedef unsigned char  			BYTE ;
typedef unsigned int   			WORD;
//typedef enum{false,true} 		BOOL;

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
