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
#include "Variables_PSFB.h"
#include "Define_PSFB.h"
#include "dsp.h"

// Measure Data Variable
int     VDcQ15 = 0;                     // Output Voltage Q15 Format Value 
int     VDcQ15Old = 0;                  // Previous Output Voltage Q15 Format Value
int     VBusQ15 = 0;                    // Bus Voltage Q15 Format Value
int     IpsfbQ15 = 0;                   // Bus Current Q15 Value
int		IpsfbQ15_Flt = 0;
int		prevIpsfbQ15 = 0;
int     BattIpsfbQ15 = 0;               // Battery Current Q15 Value
int		BattIpsfbQ15_Flt = 0;
int		prevBattIpsfbQ15 = 0;
int     iVdcVbus = 0;
int     TDI_count = 0;
// Control Command
unsigned char OP_MODE = 0;
unsigned char OP_CMD = 0;
unsigned char OP_Type = 0;


// PI Control Variable
char    firstPass = 1;                  // power on flag//	
char    softstart = 1;                  // Softstart 
int     VoReference = 0;                // Reference Voltage Q15 Value 
int     Verror = 0;                     // Voltage error Q15 Value
int     Vpreverror = 0;                 // Previous Voltage error Q15 Value          
long    VRVoltRef = 0;                  // Adjustable VR Voltage Reference
int     VRCurrRef = 0;                  // Adjustable VR Current Reference 
int     toggleLed = 0;                  // for current limit operation indicated led
int     PhaseZVT = 0;                   // Limited PIOutput Value
long    VoltageIntegralOutput = 0,VoltagePropOutput = 0,VoltageDiffOutput = 0,VoltagePIOutput = 0;
long    CurrentIntegralOutput = 0,CurrentPropOutput = 0,CurrentDiffOutput = 0,CurrentPIOutput = 0;
long    CurrentRef = 0, CurrentRef_Org = 0;
int     VoltageDecoupleTerm = 0;        // Rate of Bus Voltage Value
long    PIOutput = 0;                   // PI Output Value
int     Sumcount = 0;                   // Pi Control Average Summation Count
int     AveragePIOutput = 0;            // Average PI Output Result
int     modifier = Q15(0.75);           // Q15(0.75)
long    SumPIOutput = 0;                // Summation PI Output 
int     SaturationFlag = 0;             // Saturation Flag
int     duty = 0;                       // PWM Duty 

// Alram_status Bit
char    Input_OV = 0;                   // Input Over Voltage Status
char    Input_UV = 0;                   // Input Under Voltage Status
char    Input_Shutdown = 0;             // Input Shutdown Status
char    Input_Spare = 0;     
char    UnderVoltageStatus = 0;         // Output Under Voltage Status
char    OverVoltageStatus = 0;          // Output Over Voltage Status
char    OverCurrentStatus = 0;          // Output Over Current Status
char    OverTemperatureStatus = 0;      // Over Temperature Status

char    OV_Fail = 0, UV_Fail = 0, OT_Fail = 0;                // Fail Status

// Module_status Bit
char    Module_OV = 0;                  // Module Over Voltage Status
char    Module_UV = 0;                  // Module Under Voltage Status
char    Module_Input = 0;               // Module Input Status
char    Module_Output = 0;              // Module Output Status
char    Module_Spare1 = 0;             
char    BATT_A_FB = 0;    

//VBusQ15;
unsigned char E_Stop = 0;               // Emergency Stop Flag(Operation Mode Select)
unsigned char FaultStatus = 0;          // Module Fault Flag(Module Status - OV,OC, OT)
unsigned char FLT_Flag = 0;             // Module Fault Flag(Set_CL 125%, IGBT Fault)
unsigned char SystemState = 0;                // System Status
unsigned char SystemSet = 0;                  // Maincontroller Command Set

int     prevVerror = 0;                 // Previous Error
int     Currenterror = 0;               // Current Error

int RDVersion;
// Set Voltage Reference Table 
int VoltRef[]={
//16174,16224,16274,16324,16374,16424,16474,16524,16574,16624,
//16674,16724,16774,16824,16874,16924,16974,17024,17074,17124,
//17174,17224,17274,17324,17374,17424,17474,17524,17574,17624, 
//17674,17724,17774,17824,17874,17924,17974,18024,18074,18124, 
//18174,18224,18274,18324,18374,18424,18474,18524,18574,18624,  
//18674,18724,18774,18824,18874,18924,18974,19024,19074,19124,    
//19174,19224,19274,19324,19374,19424,19474,19524,19574,19624,    
//19674,19724,19774,19824,19874,19924,19974,20024,20074,20124,    
//20174,20224,20274,20324,20374,20424,20474,20524,20574,20624,   
//20674,20724,20774,20824,20874,20924,20974,21024,21074,21124,
21174,21224,21274,21325,21375,21426,21476,21526,21577,21627, 	//(#0~9)    90.0~91.8                	    
21678,21728,21778,21829,21879,21930,21980,22031,22081,22131,    //(#10~19)  92.0~93.8                       
22182,22232,22283,22333,22383,22434,22484,22535,22585,22635,    //(#20~29)  94.0~95.8                       
22686,22736,22787,22837,22887,22938,22988,23039,23089,23140,    //(#30~39)  96.0~97.8                       
23190,23240,23291,23341,23392,23442,23492,23543,23593,23644,    //(#40~49)  98.0~99.8                       
23694,23744,23795,23845,23896,23946,23997,24047,24097,24148,    //(#50~59)  100.0~101.8                       
24198,24249,24299,24349,24400,24450,24501,24551,24601,24652,    //(#60~69)  102.0~103.8                       
24702,24753,24803,24854,24904,24954,25005,25055,25106,25156,    //(#70~79)  104.0~105.8                       
25206,25257,25307,25358,25408,25458,25509,25559,25610,25660,    //(#80~89)  106.0~107.8
25710,25761,25811,25862,25912,25963,26013,26063,26114,26164,    //(#90~99)  108.0~109.8                       
26215,26265,26315,26366,26416,26467,26517,26567,26618,26668,    //(#100~109)110.0~111.8                       
26719,26769,26820,26870,26920,26971,27021,27072,27122,27172,    //(#110~119)112.0~113.8	                    
27223,27273,27324,27374,27424,27475,27525,27576,27626,27677,    //(#121~129)114.0~115.8                                
27727,27777,27828,27878,27929,27979,28029,28080,28130,28181,    //(#131~139)116.0~117.8                                
28231};                                                         //(#140)    118.0

// Set Current Reference Table
int AmpRef[]={
11565, 12047, 12529, 13010, 13492, 13974, 14456, 14938, 15420, 15902,   //50~68A(index  0~9)
16384, 16865, 17347, 17829, 18311, 18793, 19275, 19757, 20238, 20720,   //70~88A(index 10~19)
21202, 21684, 22166, 22648, 23130, 23612, 24093, 24575, 25057, 25539,   //90~108A(index 20~29)
26021, 26503, 26985, 27466, 27948, 28430, 28912, 29394, 29876, 30358,   //110~128A(index 30~39)
30840};                                                                 //130A	  (index 40)	

// Set BattCurrent Reference Table
int BattAmpRef[]={
0, 241,482, 723, 964, 1205, 1446, 1687, 1928, 2169,                     //2~11(index 0~9)
2409, 2650, 2891, 3132, 3373, 3614, 3855, 4096, 4337, 4578,             //12~21(index 10~19)
4819, 5060, 5301, 5542, 5782, 6023, 6264, 6505, 6746, 6987,             //22~31(index 20~29)    
7228, 7469, 7710, 7951, 8192, 8433, 8674, 8915, 9156, 9397,             //32~41(index 30~39)
9637, 9878, 10119, 10360, 10601, 10842, 11083, 11324, 11565, 11806,     //42~51(index 40~49)
12047, 12288, 12529, 12770, 13010, 13251, 13492, 13733, 13974, 14215,   //52~61(index 50~59)
14456, 14697, 14938, 15179, 15420, 15661, 15902, 16143, 16384, 16865,   //62~71(index 60~69)
17347, 17829, 18311, 18793, 19275, 19757, 20238, 20720, 21202, 21684,   //70~88A(index 10~19)
22166, 22648, 23130, 23612, 24093, 24575, 25057, 25539, 26021, 26503,   //90~108A(index 20~29)
26985, 27466, 27948, 28430, 28912, 29394, 29876, 30358, 30840};         //110~128A(index 30~39)
                                                                

/* old Variable
//Set Voltage Reference Table Index Number		  
volatile int VoltRefIndex = 0;          //Minimum Voltage Reference Table Index Number, 
volatile int VoltRefIndexBackup = 0;    //Backup Voltage Reference Table Index Number

//Set Current Reference Table Index Number
volatile int AmpRefIndex = (int)((OCL - 50 ) / 2) ;         //Current Limit 17.12.21 35 - KHJ (125Vdc/120A)
volatile int AmpRefIndexBackup = 35;                        //Current Limit 17.12.21 35 - KHJ (125Vdc/120A)

//Set Batt Current Reference Table Index Number
volatile int BattAmpRefIndex = (int)(OBCL - 2) ;            //Battery Current Limit - KHJ (125Vdc/30A)
volatile int BattAmpRefIndexBackup = 28;                    //Battery Current Limit - KHJ (125Vdc/30A)
*/

//============================================================================//
//        Current Limit Variable - Voltage Dropping                           //
//============================================================================//
/*
float	Io_CurrLimitErr = 0;                    // Over Current Limit Error Value 
float	Io_CurrLimitIntegralOutput = 0;         // Over Current Limit Error Integral Value
float	Io_CurrLimitPropOutput = 0;             // Over Current Limit Error Proportinal Value
float	Vo_CurrLimitRef = 0;                    // Over Current Limit Reference Value
int 	VoReferenceLimitOut = 0;                // Voltage Dropping for Current Saturation Value

float	Io_BattCurrLimitErr = 0;                // Over Batt Current Limit Error Value 
float	Io_BattCurrLimitIntegralOutput = 0;     // Over Batt Current Limit Error Integral Value
float	Io_BattCurrLimitPropOutput = 0;         // Over Batt Current Limit Error Proportinal Value
float	Vo_BattCurrLimitRef = 0;                // Over BattCurrent Limit Reference Value
int     VoReferenceLimitOut2 = 0;               // Voltage Dropping for Battery Current Saturation Value
*/

long	Io_CurrLimitErr = 0;                    // Over Current Limit Error Value 
long	Io_CurrLimitIntegralOutput = 0;         // Over Current Limit Error Integral Value
long	Io_CurrLimitPropOutput = 0;             // Over Current Limit Error Proportinal Value
long	Vo_CurrLimitRef = 0;                    // Over Current Limit Reference Value
long	VoReferenceLimitOut = 0;                // Voltage Dropping for Current Saturation Value
long	Io_BattCurrLimitErr = 0;                // Over Batt Current Limit Error Value 
long	Io_BattCurrLimitIntegralOutput = 0;     // Over Batt Current Limit Error Integral Value
long	Io_BattCurrLimitPropOutput = 0;         // Over Batt Current Limit Error Proportinal Value
long	Vo_BattCurrLimitRef = 0;                // Over BattCurrent Limit Reference Value
long	VoReferenceLimitOut2 = 0;               // Voltage Dropping for Battery Current Saturation Value

unsigned int 	AAdjFnd = 0;                      // Adjustable Current VR Raw Value
unsigned int	VoltFnd = 0;                      // Output Voltage Raw Value
unsigned int 	VBusFnd = 0;                      // Bus Voltage Raw Value
unsigned int 	VBusFnd_Avg = 0;                  // Bus Voltage Average Raw Value(for LCD)
unsigned int	VAdjFnd = 0;                      // Adjustable Voltage VR Raw Value
unsigned int 	AmpFnd = 0;                       // Bus Current Raw Value
unsigned int    prev_AmpFnd = 0;                  // Previous Current Raw Value
unsigned int    TempFnd = 0;                      // Temperature Raw Value
unsigned int 	BattAmpFnd = 0;                   // Battery Current Raw Value
unsigned int    prev_BattAmpFnd = 0;              // Previous Battery Current Raw Value

float   Temp_Val = 0;                             // Temperature Value
float	VoltFnd_Avg = 0;                          // Bus Voltage Average Value
float   Vout_Val = 0;                             // Output Voltage Value
float   VBus_Val = 0;                             // Output Voltage Value
float 	AmpFnd_Avg = 0;                           // Bus Current Average Value
float   Amp_Val = 0;                              // Bus Current Value
float 	BattAmpFnd_Avg = 0;                       // Battery Current Average Value
float   BattAmp_Val = 0;                          // Battery Current Value
char    BATT_A = 0;
//0x01(tx),0x02(rx),0x10(fail),0x04(normal),0x08(standby)
unsigned char 	ID485 = 0;
//
unsigned int	tstduty = 0;
unsigned int	tmrtick1 = 0;                       //1ms
unsigned int 	tmrtick2 = 0;	
unsigned int 	tmrtick3 = 0;
unsigned int	tmrtick4 = 0;
unsigned int	tmrtick5 = 0;
unsigned int	tmrtick6 = 0;
unsigned int	tmrtick7 = 0;

unsigned int step=0;
//-----------------------------------
unsigned char	f_SData_CAN = 0;                // Start Character Packet from LCD
unsigned char	f_EData_CAN = 0;                // End Character Packet from LCD
unsigned char   EOT_Flag_CAN = 0;               // End of Transmit Flag from LCD
unsigned char 	Rindex_CAN = 0;                 // Rxbuffer Index Variable from LCD
unsigned char	RxBuf_CAN[16];                  // Rxbuffer for Debug from LCD
unsigned char 	TxBuf_CAN[16];                  // Txbuffer for Debug from LCD
unsigned char	TxIndex_CAN = 0;                // Txbuffer Index Variable from LCD

//---------------------------------
unsigned char	f_SData = 0;                // Start Character Packet from LCD
unsigned char	f_EData = 0;                // End Character Packet from LCD
unsigned char   EOT_Flag = 0;               // End of Transmit Flag from LCD
unsigned char 	Rindex = 0;                 // Rxbuffer Index Variable from LCD
unsigned char	RxBuf[35];                  // Rxbuffer for Debug from LCD
unsigned char 	TxBuf[35];                  // Txbuffer for Debug from LCD
unsigned char	TxIndex = 0;                // Txbuffer Index Variable from LCD
//------------------------------
unsigned char	f_SData485 = 0;             // Start Character Packet from MainControl
unsigned char	f_EData485 = 0;             // End Character Packet from MainControl
unsigned char   EOT_Flag485 = 0;            // End of Transmit Flag form MainControl
unsigned char	Rindex485 = 0;              // Rxbuffer Index Variable from MainControl
unsigned char	RxBuf485[35];               // Rxbuffer for Debug from MainControl
unsigned char 	TxBuf485[35];               // Txbuffer for Debug from MainControl
unsigned char	TxIndex485=0;               // Tx 1Byte buffer for Debug from Module

unsigned char 	f_get_Req=0;                // request, module send voltage & Current to MainControl//	
unsigned char	f_get_Set=0;                // request, module set voltage & Current//

// ADC Data Average Filter Variable
int             Pre_tmpvalue = 0;       //Previous Output Voltage ADC
int             Pre_tmpvalue2 = 0;      //Previous Bus Voltage ADC
int             Pre_tmpvalue3 = 0;      //Previous VR Voltage ADC
int             Pre_tmpvalue4 = 0;      //Previous VR Current ADC
int             Pre_tmpvalue5 = 0;      //Previous Output Current
int             Pre_tmpvalue6 = 0;      //Previous Temperature
int             Pre_tmpvalue7 = 0;      //Previous Batt Current


int 			OutCurrent[100];            // Output Current Buffer Array for ADC Data Save
int				OutVolt[32];                // Output Voltage Buffer Array for ADC Data Save
int				VBusVolt[32];               // Bus Voltage Buffer Array for ADC Data Save
int				VAdjVolt[32];               // Adjustable Voltage VR Buffer Array for ADC Data Save
int				AAdjVolt[32];               // Adjustable Current VR Buffer Array for ADC Data Save
int             Temp[32];                   // Temperture Buffer Array for ADC Data Save
int 			BattCurrent[100];            // Battery Current Buffer Array for ADC Data Save

unsigned char	ovCnt = 0;                  // Index Count
unsigned int	avgCnt = 0;                 // Average Filter Index Array
unsigned int	avgCnt2 = 0;                // Average Filter Index Array
unsigned int	avgCnt3 = 0;                // Average Filter Index Array
float           SumVolt = 0;                // SumVoltage Temporary Variable
float           SumCurr = 0;                // SumCurrent Temporary Variable
float           SumBattCurr = 0;            // SumBattCurrent Temporary Variable

int             fSaveDone = 0;              // EEPROM Save Flag
long            sumOutVolt = 0;             // Summary Output Voltage
long            sumVBusVolt = 0;            // Summary Bus Voltage
long            sumVAdjVolt = 0;            // Summary Adjustable VR Voltage
long            sumAAdjVolt = 0;            // Summary Adjustable VR Current
long            sumOutCurrent = 0;          // Summary Output Current
long            sumTemp = 0;                // Summary Temperature
long            sumBattCurrent = 0;          // Summary Battery Current
int				tmrDec1 = 0;                // Timer Decrease Variable
int				tmrDec2 = 0;                // Timer Decrease Variable

//-------------------------------------------------------------------------
// RS232 Receive Command
float   Input_Volt_R = 0;               // Modules Input Voltage R
float   Input_Volt_S = 0;               // Modules Input Voltage S
float   Input_Volt_T = 0;               // Modules Input Voltage T
float   Input_Spare1 = 0;
float   Input_Spare2 = 0;
float   Input_Spare3 = 0;
float   Input_Spare4 = 0;


float   Set_ARef = ARef;                // Curren Warning Value
float   Set_CL = OCL;                   // Curren Limit Value(Voltage Dopping)
float   Set_VRef = RefOutputVoltage_LI;    // Voltage Reference Value
float   Set_UV = MinOutputVoltage_PB;      // Under Voltage Value
float   Set_OV = MaxOutputVoltage_PB;      // Over Voltage Value
float   Set_BCL = OBCL;                 // Batt Current Limit Value(Voltage Dopping)
float   Set_OT = OverTempLimitmax;      // Over Trmperature Value
char ADJ_VGain = 0;
char ADJ_CGain = 0;
char CHG_Info = 0;

float   Set_ARef_B = ARef;              // Current Warning Value Backup
float   Set_CL_B = OCL;                 // Current Limit Value Backup
float   Set_VRef_B = RefOutputVoltage_LI;  // Voltage Reference Value Backup
float   Set_UV_B = MinOutputVoltage_PB;    // Under Voltage Value Backup
float   Set_OV_B = MaxOutputVoltage_PB;    // Over Voltage Value Backup
float   Set_BCL_B = OBCL;               // Batt Current Limit Value Backup
float   Set_OT_B = OverTempLimitmax;    // Over Trmperature Value Backup

int   Set_CL_ID = (OCL - 50) / 2;       //Q15 convert
int   Set_BCL_ID = OBCL - 2;            //Q15 convert
int   Set_VRef_Q15 = -(0.1239 * RefOutputVoltage_LI * RefOutputVoltage_LI) + (216.8044 * RefOutputVoltage_LI) - 3219.3999;         // y = 186.70x - 1510.26  => Voltage Reference -> Q15 Convert EQ

int   Set_CL_ID_B = (OCL - 50) / 2;     //Q15 convert
int   Set_BCL_ID_B = OBCL - 2;          //Q15 convert
int   Set_VRef_Q15_B = -(0.1239 * RefOutputVoltage_LI * RefOutputVoltage_LI) + (216.8044 * RefOutputVoltage_LI) - 3219.3999;       // y = 186.70x - 1510.26  => Voltage Reference -> Q15 Convert EQ


//#ifdef BD2
//===============//
//board#2        //
//===============//
int	VoComm = 0;			//adjust VoReference through Serial comm. ++1,--1// 
//
unsigned int	gain_OutV;//=1000;//999;	
//===============//
unsigned int	gain_OutI;//=1000;
unsigned int	gain_IBus;//=1000;
unsigned int	gain_VBus;//=1000;//1002;
unsigned int	gain_Temp;//=1000;
unsigned int    gain_BattI;//=1000;

/*******************************************************************************
                            Globals Variables  
*******************************************************************************/
unsigned char 	b_DeTime=0,f_press1=0,f_press2=0;
unsigned char 	Txdata[8],Tx_Char = 0x30,Tx_Char485 = 0x30,Tx_Char485_2 = 0x30;    

/*******************************************************************************
                            CAN Comm Variables  
*******************************************************************************/
unsigned char canData0 = 0;
unsigned char canData1 = 0;
unsigned char canData2 = 0;
unsigned char canData3 = 0;
unsigned char canData4 = 0;
unsigned char canData5 = 0;
unsigned char canData6 = 0;
unsigned char canData7 = 0;

long LoadShare_Err = 0;
long LoadShare_prevErr = 0;
long LoadShare_IntegralOutput = 0;
long LoadShare_PropOutput = 0;
int  LoadShareControlOut = 0;

int f_canReceive = 0; // Compl. Rx. 
int canShare = 0;
int canShare_B = 0;
int canShareSum = 0;
int iCanShareCnt = 0;
int canMax = 0;
int canShareQ15 = 0;
int	Set_Test = 0;
int	AvrCurQ15 =0;
int cntNotCanData = 0;

float CanModAmp[8] = {0,0,0,0,0,0,0,0};


