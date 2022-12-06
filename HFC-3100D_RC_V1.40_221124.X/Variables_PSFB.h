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
#include "p33FJ64GS606.h"
//=======================

// Measure Data Variable
extern int VDcQ15 ;                     // Output Voltage Q15 Format Value 
extern int VDcQ15Old;                   // Previous Output Voltage Q15 Format Value
extern int IpsfbQ15;                    // Bus Current Q15 Value
extern int VBusQ15;                     // Bus Voltage Q15 Format Value	
extern int IpsfbQ15_Flt;
extern int prevIpsfbQ15;
extern int BattIpsfbQ15;                 // Battery Current Q15 Value
extern int BattIpsfbQ15_Flt;
extern int prevBattIpsfbQ15;
extern int iVdcVbus;
extern int TDI_count;

// Control Command
extern unsigned char OP_MODE;
extern unsigned char OP_CMD;
extern unsigned char OP_Type;

// PI Control Variable
extern char firstPass;                   // power on flag//	
extern char softstart;                   // Softstart 
extern int VoReference;                 // Reference Voltage Q15 Value 
extern int Verror;                      // Voltage error Q15 Value
extern int Vpreverror;                  // Previous Voltage error Q15 Value
extern long VRVoltRef;                  // Adjustable VR Voltage Reference
extern int VRCurrRef;                   // Adjustable VR Current Reference
extern int toggleLed;                   // for current limit operation indicated led
extern int PhaseZVT;                    // Limited PIOutput Value
extern long VoltageIntegralOutput,VoltagePropOutput,VoltageDiffOutput,PIOutput;
extern long CurrentIntegralOutput,CurrentPropOutput,CurrentDiffOutput,CurrentPIOutput;
extern long CurrentRef, CurrentRef_Org;
extern int VoltageDecoupleTerm;         // Rate of Bus Voltage Value
extern long PIOutput;                   // PI Output Value
extern int Sumcount;                    // Pi Control Average Summation Count
extern int AveragePIOutput;             // Average PI Output Result
extern int modifier;                    // Q15(0.75)
extern long SumPIOutput;                // Summation PI Output
extern int SaturationFlag;              // Control Saturation Flag
extern int duty;                        // PWM Duty

//Alram Status Bit
extern char Input_OV;                   // Input Over Voltage Status
extern char Input_UV;                   // Input Under Voltage Status
extern char Input_Shutdown;             // Input Shutdown Status
extern char Input_Spare;
extern char UnderVoltageStatus;         // Output Under Voltage Status
extern char OverVoltageStatus;          // Output Over Voltage Status
extern char OverCurrentStatus;          // Output Over Current Status
extern char OverTemperatureStatus;      // Over Temperature Status
extern char OV_Fail, UV_Fail, OT_Fail;                // Fail Status

// Module_status Bit
extern char Module_OV;                  // Input Over Voltage Status
extern char Module_UV;                  // Input Under Voltage Status
extern char Module_Input;               // Module Input Status
extern char Module_Output;              // Module Output Status
extern char Module_Spare1;             
extern char BATT_A_FB;   

//extern int VBusQ15;
extern unsigned char E_Stop;            // Emergency Stop Flag(Operation Mode Select)
extern unsigned char FaultStatus;       // Module Fault Flag(Module Status - OV,OC, OT)
extern unsigned char FLT_Flag;          // Module Fault Flag(HWOCL, IGBT Fault)
extern unsigned char SystemState;       // System Status
extern unsigned char SystemSet;         // Maincontroller Command Set

extern int prevVerror;        // Previous Error
extern int Currenterror;      // Current Error
extern int VoltRef[];                   // Set Voltage Reference Table 
extern int AmpRef[];                    // Set Current Reference Table
extern int BattAmpRef[];                // Set Batt Current Reference Table

/* old Variable
//Set Voltage Reference Table Index Number	
extern volatile int VoltRefIndex;		//Minimum Voltage Reference Table Index Number, 
extern volatile int VoltRefIndexBackup; //Backup Voltage Reference Table Index Number

//Set Current Reference Table Index Number
extern volatile int AmpRefIndex;		//Maximum Current Reference Table Index Number, 
extern volatile int AmpRefIndexBackup;

//Set Battery Current Reference Table Index Number
extern volatile int BattAmpRefIndex;		//Maximum Batt Current Reference Table Index Number, 
extern volatile int BattAmpRefIndexBackup;
*/

//================================================================//
// Current Limit(����) ���� ���� �߰����� 2013-10-07 ���� 11:21:51//
//================================================================//
/*
extern float	Io_CurrLimitErr;
extern float Io_CurrLimitPropOutput;
extern float	Io_CurrLimitIntegralOutput;
extern float	Vo_CurrLimitRef;
extern int	VoReferenceLimitOut;

extern float	Io_BattCurrLimitErr;
extern float Io_BattCurrLimitPropOutput;
extern float	Io_BattCurrLimitIntegralOutput;
extern float	Vo_BattCurrLimitRef;
extern int	VoReferenceLimitOut2;
*/
extern long	Io_CurrLimitErr;
extern long Io_CurrLimitPropOutput;
extern long	Io_CurrLimitIntegralOutput;
extern long	Vo_CurrLimitRef;
extern long	VoReferenceLimitOut;

extern long	Io_BattCurrLimitErr;
extern long Io_BattCurrLimitPropOutput;
extern long	Io_BattCurrLimitIntegralOutput;
extern long	Vo_BattCurrLimitRef;
extern long	VoReferenceLimitOut2;

//-------------------------------
extern unsigned int 		tstduty;
extern unsigned int			tmrtick1;
extern unsigned int 		tmrtick2;
extern unsigned int			tmrtick3;
extern unsigned int			tmrtick4;
extern unsigned int 		tmrtick5;
extern unsigned int         tmrtick6;
extern unsigned int         tmrtick7;
extern unsigned int 		step;
//
extern unsigned char		Rindex;
extern unsigned char		f_SData;
extern unsigned char		f_EData;
extern unsigned char        EOT_Flag;
extern unsigned char		RxBuf[35];
extern unsigned char		TxBuf[35];
extern unsigned char		TxIndex;

//
extern unsigned char		Rindex_CAN;
extern unsigned char		f_SData_CAN;
extern unsigned char		f_EData_CAN;
extern unsigned char        EOT_Flag_CAN;
extern unsigned char		RxBuf_CAN[16];
extern unsigned char		TxBuf_CAN[16];
extern unsigned char		TxIndex_CAN;
//
//-------------------------------
extern unsigned char		f_SData485;     // Start Character Packet from Maincontrol
extern unsigned char		f_EData485;     // End Character Packet from MainControl
extern unsigned char        EOT_Flag485;    // End of Transmit Flag form MainControl
extern unsigned char		Rindex485;      // Rxbuffer Index Variable from MainControl
extern unsigned char		RxBuf485[35];   // Rxbuffer for Debug from MainControl
extern unsigned char 		TxBuf485[35];   // Txbuffer for Debug from MainControl
extern unsigned char		TxIndex485;     // Tx 1Byte buffer for Debug from Module

extern unsigned char 		f_get_Req;      //request, module send voltage & Current to MainControl//	
extern unsigned char		f_get_Set;      //request, module set voltage & Current//
//-------------------------------
extern unsigned int			gain_OutV;
extern unsigned int			gain_VBus;
extern unsigned int			gain_OutI;
extern unsigned int			gain_IBus;
extern unsigned int			gain_Temp;
extern unsigned int			gain_BattI;

extern int                  Pre_tmpvalue;       //Previous Output Voltage ADC
extern int                  Pre_tmpvalue2;      //Previous Bus Voltage ADC
extern int                  Pre_tmpvalue3;      //Previous VR Voltage ADC
extern int                  Pre_tmpvalue4;      //Previous VR Current ADC
extern int                  Pre_tmpvalue5;      //Previous Output Current
extern int                  Pre_tmpvalue6;      //Previous Temperature
extern int                  Pre_tmpvalue7;      //Previous Batt Current

extern int					OutCurrent[100];
extern int					OutVolt[32];
extern int					VBusVolt[32];
extern int					VAdjVolt[32];
extern int					AAdjVolt[32];
extern int					Temp[32];
extern int					BattCurrent[100];
extern unsigned char 		ocCnt;
extern unsigned char		scCnt;
extern unsigned char		ovCnt;
extern unsigned int         avgCnt;
extern float                SumCurr;
extern unsigned int         avgCnt2;
extern float                SumVolt;
extern unsigned int         avgCnt3;
extern float                SumBattCurr;


extern int  fSaveDone;                          // EEPROM Save Flag
extern long sumOutVolt;                        // Summary Output Voltage
extern long sumVBusVolt;                       // Summary Bus Voltage
extern long sumVAdjVolt;                       // Summary Adjustable VR Voltage
extern long sumAAdjVolt;                       // Summary Adjustable VR Current
extern long sumOutCurrent;                     // Summary Output Current
extern long sumTemp;                           // Summary Temperature
extern long sumBattCurrent;                     // Summary Battery Current

//----------------------------------
//7-segment display variables
extern unsigned int 	Mmm[3];
extern unsigned int 	mIndex;
extern unsigned char	f_maxmin;
extern unsigned int		VoltFnd;
extern unsigned int 	VBusFnd;
extern unsigned int 	VBusFnd_Avg;
extern unsigned int 	VAdjFnd;
extern unsigned int 	AAdjFnd;
extern unsigned int 	AmpFnd;
extern unsigned int 	prev_AmpFnd;
extern unsigned int     TempFnd;
extern unsigned int 	BattAmpFnd;
extern unsigned int 	prev_BattAmpFnd;

extern float    Temp_Val;
extern float	VoltFnd_Avg;
extern float    Vout_Val;
extern float    VBus_Val;
extern float 	AmpFnd_Avg;
extern float    Amp_Val;
extern float 	BattAmpFnd_Avg;
extern float    BattAmp_Val;
extern char     BATT_A;

extern unsigned char	LedFnd;
//0x01(tx),0x02(rx),0x10(fail),0x04(normal),0x08(standby)
extern unsigned char	ID485;

extern int				tmrDec1;
extern int				tmrDec2;

extern unsigned char cntNormalLedBlink;
extern unsigned char f_VoltAdj;
//
extern unsigned char amp100;
extern unsigned char amp10;
extern unsigned char amp1;

extern float   Input_Volt_R;                // Modules Input Voltage R
extern float   Input_Volt_S;                // Modules Input Voltage S
extern float   Input_Volt_T;                // Modules Input Voltage T
extern float   Input_Spare1;
extern float   Input_Spare2;
extern float   Input_Spare3;
extern float   Input_Spare4;

extern float   Set_ARef;
extern float   Set_CL;
extern float   Set_VRef;
extern float   Set_UV;
extern float   Set_OV;
extern float   Set_BCL;
extern float   Set_OT;
extern char  ADJ_VGain;
extern char  ADJ_CGain;
extern char  CHG_Info;

extern float   Set_ARef_B;
extern float   Set_CL_B;
extern float   Set_VRef_B;
extern float   Set_UV_B;
extern float   Set_OV_B;
extern float   Set_BCL_B;
extern float   Set_OT_B;

extern int   Set_CL_ID;     //Array convert
extern int   Set_VRef_Q15;
extern int   Set_BCL_ID;     //Array convert

extern int   Set_CL_ID_B;      //Array convert Backup
extern int   Set_VRef_Q15_B;
extern int   Set_BCL_ID_B;      //Array convert Backup

extern unsigned char 	b_DeTime,f_press1,f_press2;
extern unsigned char 	Txdata[8],Tx_Char,Tx_Char485,Tx_Char485_2;    

/*******************************************************************************
                            CAN Comm Variables  
*******************************************************************************/
extern unsigned char canData0;
extern unsigned char canData1;
extern unsigned char canData2;
extern unsigned char canData3;
extern unsigned char canData4;
extern unsigned char canData5;
extern unsigned char canData6;
extern unsigned char canData7;

extern long LoadShare_Err;
extern long LoadShare_prevErr;
extern long LoadShare_IntegralOutput;
extern long LoadShare_PropOutput;
extern int  LoadShareControlOut;
extern int canShare;
extern int canShare_B;
extern int canShareSum;
extern int iCanShareCnt;
extern int canMax;
extern int canShareQ15;
extern int	f_canReceive;
extern int cntNotCanData;

extern float CanModAmp[8];

extern int RDVersion;//received RD version//
