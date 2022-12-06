/*
*************************************************************************
* REVISION HISTORY: Draft                                               *
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
* Author : Martin Kim,   Date : 18.04.10,  revision: V1.0               *
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
* Seoul Electric Power System co.,Ltd First release of source					*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
** ADDITIONAL NOTES:        										    *
*************************************************************************
 */
#include "dsp.h"
#include "p33fxxxx.h"
//===============================================================
#define RCVersion 140 //V1.40 //max 10.23// 
//===============================================================
#define RS232_Period        65
#define RS485_Period        100
#define FCY 				40000000
#define BAUDRATE			9600                      //KHJ 115200                    //org. 9600
#define BRGVAL				((FCY/BAUDRATE)/4-1)        //KHJ ((FCY/BAUDRATE)/4-1)      //org. ((FCY/BAUDRATE)/16-1)		//baudrate Comm1,Comm2 9600bps
//===============================================================
#define PWMClockFreq		118E6	//APSTSCLR=4,(ACLK /8)	//(7.37Mhz*16 * 8) /8 = 117.92
#define PWMSwitchingFreq	20E3//20Khz,  //15E3-15Khz	

#define PWMPeriod			(int)((PWMClockFreq/PWMSwitchingFreq) - 8) 	// 20K-7850 - 8 =>7850  //15K-7858
#define PWMDuty				(int)((PWMClockFreq/PWMSwitchingFreq)*0.5) 	// 20K-3929				//15K-3933
#define HALFPERIOD      	(int)(PWMPeriod/2)							// 20K-3925				//15K-3929

#define MaxPhase			(int)((PWMClockFreq/PWMSwitchingFreq)* 0.5) // 20K-7858/2=3929      //15K-7866/2=3933
#define MaxDeltaPhase 		(int)((PWMClockFreq/PWMSwitchingFreq)* 0.05)// 20K-392              //15K-393
#define trigger 			(int)((PWMClockFreq/PWMSwitchingFreq)* 0.95)// 20K-465 0.95			//15K-7472

#define Deadtime			142 		//ACLK= (7.37M * 16)/8(APSTSCLR) =117.92Mhz/8=
										//DTRx=[14.74M * 8 * x us]	
#define ALTDeadtime			142			//deadtime = 1.2us  HFC-3100D(+125Vdc) 

#define PSFBCurrent			ADCBUF0		// Vout Current	
#define OutputVoltage		ADCBUF1		// Vout Current	
#define Temperature			ADCBUF2		// Module Temp
#define BatteryCurrent      ADCBUF3     // Batt (Dis)Charge Current
#define VBus				ADCBUF4     // VBus Voltage
#define LoadShare			ADCBUF5		//
#define VAdj				ADCBUF14    // Variable Resistor
#define AAdj				ADCBUF15    // Variable Resistor
//
#define Cutoff_Freq         7           // (2^n) - 1 ex = 3, 7, 15, 31
#define Volt_Gain           0.165      // 0.167 or 0.165 //10bit ADC Voltage Gain
#define Current_Gain        0.134      // 0.135 or 0.134 //10bit ADC Voltage Gain

// For 10kohm -Thermistor => 1kohm
#define Temp_Gain_TO       0.00000021           // 0.0000003352            //Third Order
#define Temp_Gain_SO       -0.00035103          // -0.0004941283           //Second Order
#define Temp_Gain_FO       0.28993517           // 0.3333871792            //First Order

#define Temp_Offset        -4.00415921          // 6.572910//-10.4006020812          //Offset

#define VoRef  				0.78125                 //110 / 140.8 = 0.78125//250/255 = 0.98      // 11.42/14=0.815714  //12.22/14=0.8728//  	
#define VoRefQ15 			Q15(VoRef)              //Q15 Format
//
#define V_FND		0x76	//I2C Top FND driver address
#define	A_FND		0x70	//I2C Bottom FND driver address

//Active Low
#define RXLED		0x01 	//0xFE 1111-1110 
#define TXLED		0x02 	//0xFD 1111-1101 
#define FAILLED		0x04	//0xFB 1111-1011
#define	NORLED		0x08	//0xF7 1111-0111
#define	STBLED		0x10	//0xEF 1110-1111
#define LEDOFF		0x00	//0xFF

#define TRUE		1
#define FALSE		0

//-------------------------------------------------------------
#define Saturation 				8190		//1100 // 30A???? limit ??? 2? ??.//
#define Refshift 				2			//5 //prev value is 3// App. Org. 5            	
#define Voltagedecouple         Q15(0.4733)  	
//==============================================================================
//125VDC
#define MinOutputVoltage_PB         98.6
#define MinOutputVoltage_PB1        124.7
#define MinOutputVoltage_PB2        127.6
#define MidOutputVoltage_PB         132.0
#define MaxOutputVoltage_PB         150.0
#define MaxOutputVoltage_PB1        141.0
#define MaxOutputVoltage_PB2        147.0
#define RefOutputVoltage_PB         131.0
#define STNDBYOutputVoltage_PB      125.0   

//110VDC
//#define MinOutputVoltage2_PB        105.0
//#define MinOutputVoltage2_PB1       110.0
//#define MinOutputVoltage2_PB2       120.0
//#define MidOutputVoltage2_PB        120.0
//#define MaxOutputVoltage2_PB        150.0
//#define MaxOutputVoltage2_PB1       130.0
//#define MaxOutputVoltage2_PB2       150.0
//#define RefOutputVoltage2_PB        115.0
//#define STNDBYOutputVoltage2_PB     110.0   //
#define MinOutputVoltage2_PB        60.0
#define MinOutputVoltage2_PB1       60.0
#define MinOutputVoltage2_PB2       60.0
#define MidOutputVoltage2_PB        120.0
#define MaxOutputVoltage2_PB        150.0
#define MaxOutputVoltage2_PB1       130.0
#define MaxOutputVoltage2_PB2       150.0
#define RefOutputVoltage2_PB        115.0
#define STNDBYOutputVoltage2_PB     110.0   //

#define MinOutputVoltage_LI         60.0	//18.03.02  Li-ion 96Vdc = 3.2/Cell - KHJ
#define MidOutputVoltage_LI         118.4
#define MaxOutputVoltage_LI         135.0  	//18.03.02 135Vdc = 4.2/Cell - KHJ
#define RefOutputVoltage_LI         125.0
#define STNDBYOutputVoltage_LI      125.0  


//#define FaultOutputVoltage      60.0    // Fault Condition Vout (< 70)
//==============================================================================
#define UCL           80.0           //Init Under Current Limit   18.01.15  - KHJ
#define ARef          110.0         //Init Current Reference
#define OCL           120.0         //Init Over Current Limit   18.01.15  - KHJ
//==============================================================================
#define UBCL          0.0           //Init Under Batt Charge Current Limit   18.01.15  - KHJ
#define OBCL          30.0          //Init Over Batt Charge Current Limit  - KHJ
//==============================================================================

//==============================================================================	
// 	MCP9700 ADC Voltage -> Degree Value Transfer function: 3-order EQ 	
#define OverTempLimitmax 		95    
//==============================================================================

#define PHASEZVT  			PHASE1
#define PHASEZVT3  			PHASE3

#define System_STANDBY		2
#define System_ON  			1				
#define System_OFF 			0		

//=================================================================
// Current Limit
//=================================================================
#define	Io_CurrLimitRefShift		2           //2
#define	Io_BattCurrLimitRefShift	2           //2
#define	CurrLimitSaturation			Q15(0.99)   //131072      //65535	//32767 //8192
#define	CurrLimitSaturationZero		Q15(0.0)
#define	BattCurrLimitSaturation		Q15(0.99)   //131072      //65535	//32767 //8192
#define	BattCurrLimitSaturationZero	Q15(0.0)

//------------------------------
// Control Gain Define
//------------------------------
// Voltage Control Define
#define KpVoltage  			Q15(0.99)       //18.02.27 38uH : 0.7 ~ 0.99        						
#define KiVoltage   		Q15(0.008)      //Q15(0.018)//18.02.27 38uH : 0.003 ~ 0.008     

// Load Current Dropping Voltage Define
#define	KpCurrLimit			Q15(0.1)        // Q15(0.1)      
#define	KiCurrLimit			Q15(0.005)      // Q15(0.005) 

// Battery Current Dropping Voltage Define 
#define	KpBattCurrLimit		Q15(0.03)        // Q15(0.1)  HHW      
#define	KiBattCurrLimit		Q15(0.001)    // Q15(0.005) HHW

// Load Share Define 
#define KpLoadShare         Q15(0.2)        //Q15(0.1 ~ 0.4)    
#define KiLoadShare         Q15(0.01)       //Q15(0.005 ~ 0.02)

//==================================================================
#define		ON				1
#define		OFF				0

//----------------from MainControl 485 comm. ---------------------//
#define 	tstport1			(LATEbits.LATE4)
#define 	tstport2			(LATEbits.LATE5)
#define 	sysLed              (LATEbits.LATE6)
#define		WDT					(LATDbits.LATD9)		//DS1232 care #43//	
#define		tstport4			(LATDbits.LATD10)
#define		Relay				(LATFbits.LATF6)
#define		Nfb_Trip			(LATGbits.LATG9)
#define		tstport5			(LATEbits.LATE7)
#define		DEBOUNCE            100
#define		RS485_TX			(PORTFbits.RF2)
#define		RS485_RX			(LATFbits.LATF3)
#define		RS485_CTRL          (LATDbits.LATD11)

//--------------- From Module CAN -------------------------------//
/* CAN Baud Rate Configuration 		*/
#define FCAN  	      	40000000
#define BITRATE 		100000	//100kb(100000)//1M(1000000)
#define NTQ 			20		// 20 Time Quanta in a Bit Time
#define BRP_VAL			((FCAN/(2*NTQ*BITRATE))-1)

/* defines used in ecan.c */
/* ECAN message buffer length */
#define ECAN1_MSG_BUF_LENGTH 	4

/* ECAN message type identifiers */
#define CAN_MSG_DATA	0x01
#define CAN_MSG_RTR		0x02
#define CAN_FRAME_EXT	0x03
#define CAN_FRAME_STD	0x04
#define CAN_BUF_FULL	0x05
#define CAN_BUF_EMPTY	0x06

/* filter and mask defines */
/* Macro used to write filter/mask ID to Register CiRXMxSID and 
CiRXFxSID. For example to setup the filter to accept a value of 
0x123, the macro when called as CAN_FILTERMASK2REG_SID(0x123) will 
write the register space to accept message with ID ox123 
USE FOR STANDARD MESSAGES ONLY */
#define CAN_FILTERMASK2REG_SID(x) ((x & 0x07FF)<< 5)
/* the Macro will set the "MIDE" bit in CiRXMxSID */
#define CAN_SETMIDE(sid) (sid | 0x0008)
/* the macro will set the EXIDE bit in the CiRXFxSID to 
accept extended messages only */
#define CAN_FILTERXTD(sid) (sid | 0x0008)
/* the macro will clear the EXIDE bit in the CiRXFxSID to 
accept standard messages only */
#define CAN_FILTERSTD(sid) (sid & 0xFFF7)
/* Macro used to write filter/mask ID to Register CiRXMxSID, CiRXMxEID and 
CiRXFxSID, CiRXFxEID. For example to setup the filter to accept a value of 
0x123, the macro when called as CAN_FILTERMASK2REG_SID(0x123) will 
write the register space to accept message with ID 0x123 
USE FOR EXTENDED MESSAGES ONLY */
#define CAN_FILTERMASK2REG_EID0(x) (x & 0xFFFF)
#define CAN_FILTERMASK2REG_EID1(x) (((x & 0x1FFC)<< 3)|(x & 0x3))

/* DMA CAN Message Buffer Configuration */
typedef unsigned int ECAN1MSGBUF [ECAN1_MSG_BUF_LENGTH][8];
extern ECAN1MSGBUF  ecan1msgBuf __attribute__((space(dma)));

#define     sdiv(a,b)			__builtin_divsd(a,b)

typedef struct 
{
	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;
} bool8;

typedef struct 
{
	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;
	unsigned b8 :1,b9 :1,b10:1,b11:1,b12:1,b13:1,b14:1,b15:1;
} bool16;

/* message structure in RAM */
typedef struct{
	/* keep track of the buffer status */
	unsigned char buffer_status;
	/* RTR message or data message */
	unsigned char message_type;
	/* frame type extended or standard */
	unsigned char frame_type;
	/* buffer being used to send and receive messages */
	unsigned char buffer;
	/* 29 bit id max of 0x1FFF FFFF 
	*  11 bit id max of 0x7FF */
	unsigned long id; 
	unsigned char data[8];	
	unsigned char data_length;
}mID;


