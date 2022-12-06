/*
 * File:   funcData.c
 * Author: goobo
 *
 * Created on June 15, 2022, 10:01 AM
 */
#include "xc.h"
#include "typeDef.h"
#include "funcData.h"
#include "i2c_master_eeprom_24fc256.h"
//variables//
//timer1
volatile unsigned int tmrCnt[10]={0,};
volatile unsigned int tmr485 = 0;
volatile unsigned int tmr232 = 0;
volatile unsigned int tmr485Send =0;
//unsigned int timeOutModule1 =0; //-->tm8
//unsigned int timeOutModule2 =0; //-->tm9
volatile unsigned int tmrContact1=0;
volatile unsigned int tmrContact2=0;
volatile unsigned int tmrContact3=0;
unsigned int fNormalStateTimer=50;
unsigned int fNormalState=0;
//------------------------------------------------------------------------------
// RS485 Variable

UINT8       f_SData_485 = 0;                // Start Character Packet from Module
UINT8       f_EData_485 = 0;                // End Character Packet from Module 
UINT8       EOT_Flag485 = 0;                // End of Transmit Flag from Module
UINT8       Rindex_485 = 0;                 // Rxbuffer Index Variable from Module
UINT8       RxBuf_485[36]={0,};                  // Rxbuffer for Debug from Module
char       TxBuf_485[36]={0,};                  // Txbuffer for Debug from Module
UINT8       TxIndex_485 = 0;                // Tx 1Byte buffer for Debug from Module

// Module #N Continous Data
FLOAT32     Voltage_Out[4];                 // Modules Voltage Output
FLOAT32     Current_Out[4];                 // Modules Current Output
FLOAT32     Temperature[4];                 // Modules Temperature
FLOAT32     Input_Volt_R[4];                // Modules Input Voltage R
FLOAT32     Input_Volt_S[4];                // Modules Input Voltage S
FLOAT32     Input_Volt_T[4];                // Modules Input Voltage T
UINT8       Rx_Char[4];                     // Modules Status
UINT8       Rx_Char2[4];                    // Modules Warning Status                   
UINT8       Rx_Char3[4];
UINT8       Rx_Spare1[4];

bool8       Alarm_Sts[5];                   // Modules Operation Status
bool8       Warning_Sts[5];                 // Modules Warning Status
bool8       CHG_Info[5];                    // Module Charge type

// Module #N Set Data
UINT8       ID_485 = 1;                     // Modules No. initial First No. 1

//Operation Mode Variable
UINT8       OP_MODE = 1;                    // OP_MODE => '0' : Manual / '1': Auto 
UINT8       OP_CMD = 1;                     // Command Change Operation Command Module2
UINT8       OP_Type = 0;                    // 0 : Stand-alone or 1 : Load Share Mode

//==============================================================================
//rs232//
//==============================================================================
bool8       LED_Sts1 = {0,0,0,0,0,0,0,0};         // NFB LED Status Variable for LCD
bool8       LED_Sts2 = {0,0,0,0,0,0,0,0};         // NFB LED Status Variable for LCD
bool8       LED_Sts3 = {0,0,0,0,0,0,0,0};         // Spare1 LED Status Variable for LCD
bool8       LED_Sts4 = {0,0,0,0,0,0,0,0};         // Spare2 LED Status Variable for LCD

bool8       Module1_Sts = {0,0,0,0,0,0,0,1};      // Module1 Operation Status for LCD
bool8       Module2_Sts = {0,0,0,0,0,0,0,1};      // Module2 Operation Status for LCD
bool8       Module3_Sts = {0,0,0,0,0,0,0,1};      // Module3 Operation Status for LCD
bool8       Module4_Sts = {0,0,0,0,0,0,0,1};      // Module4 Operation Status for LCD

bool8       Batt_Sts = {0,0,0,0,0,0,0,0};         // Battery Condition Status for LCD
bool8       Module_Sts = {0,0,0,0,0,0,0,0};       // Module Status for LCD
bool8       CHG_Sts = {0,0,0,0,0,0,0,0};       // Spare Status1 for LCD
bool8       Spare_Sts1 = {0,0,0,0,0,0,0,0};       // Spare Status2 for LCD

// RS232 -Debug Variable
UINT8       f_SData_Debug = 0;              // Start Character Packet for Debug
UINT8       f_EData_Debug = 0;              // End Character Packet for Debug 
UINT8       EOT_Flag_Debug = 0;             // End of Transmit Flag for Debug
UINT8       Rindex_Debug = 0;               // Rxbuffer Index Variable for Debug
UINT8       RxBuf_Debug[36];                // Rxbuffer for Debug for Debug            
UINT8       TxBuf_Debug[36];                // Txbuffer for Debug for Debug
UINT8       Tx_Char_Debug[12];              // Tx 1Byte buffer for Debug
UINT8       TxIndex_Debug=0;                // Txbuffer Index Variable for Debug

// RS232 Variable
UINT8       f_SData = 0;                    // Start Character Packet from Module
UINT8       f_EData = 0;                    // End Character Packet from Module 
UINT8       EOT_Flag = 0;                   // End of Transmit Flag from Module
UINT8       Rindex = 0;                     // Rxbuffer Index Variable from Module
UINT8       RxBuf[36];                      // Rxbuffer for Debug from Module
char       TxBuf[36];                      // Txbuffer for Debug from Module
UINT8       Tx_Char[12];                    // Tx 1Byte buffer for Debug from Module
UINT8       TxIndex=0;                      // Txbuffer Index Variable from Module

// LCD Continous Data
FLOAT32     Batt_Volt = 0;                  // Battery Voltage from Module
FLOAT32     Batt_Amp = 0;                   // Battery Current from Module
FLOAT32     Load_Volt = 0;                  // Load Voltage from Module
FLOAT32     Load_Amp = 0;                   // Load Current from Module
UINT8       LED_Module1_NFB = 1;            // Module1_NFB LED from Module
UINT8       LED_Module2_NFB = 1;            // Module2_NFB LED from Module
UINT8       LED_Module3_NFB = 1;            // Module3_NFB LED from Module
UINT8       LED_Module4_NFB = 1;            // Module4_NFB LED from Module
UINT8       LED_DC1_NFB = 1;                    // DC1 NFB LED from Module
UINT8       LED_DC2_NFB = 1;                    // DC2 NFB LED from Module
UINT8       LED_DC3_NFB = 1;                    // DC3 NFB LED from Module
UINT8       LED_DC4_NFB = 1;                    // DC4 NFB LED from Module
UINT8       LED_Battery_NFB;                // Battery NFB LED from Module
UINT8       LED_Spare1_NFB;                 // Spare1 NFB LED from Module
UINT8       LED_Spare2_NFB;                 // Spare2 NFB LED from Module
UINT8       LED_Load_NFB;                   // Load NFB LED from Module
UINT8       LED_Spare3_NFB;                 // Spare3 NFB LED from Module 
UINT8       LED_Spare4_NFB;                 // Spare4 NFB LED from Module
UINT8       LED_Spare5_NFB;                 // Spare5 NFB LED from Module
UINT8       LED_Spare6_NFB;                 // Spare6 NFB LED from Module
UINT8       SystemState_STS[4];             // Module State Status from Module
UINT8       Input_OV_STS[4];                // Input Over Voltage Status from Module
UINT8       Input_UV_STS[4];                // Input Under Voltage Status from Module
UINT8       Input_SD_STS[4];                // Input Shutdown Status from Module
UINT8       Output_OV_STS[4];               // Output Over Voltage Status from Module
UINT8       Output_UV_STS[4];               // Output Under Voltage Status from Module
UINT8       Output_OC_STS[4];               // Output Over Current Status from Module
UINT8       Output_OT_STS[4];               // Over Temperature Status from Module
UINT8       Module_OV_STS[4];               // Output Over Voltage Warning Status from Module
UINT8       Module_UV_STS[4];               // Output Under Voltage Warning Status from Module
UINT8       CHG_MODE[4];                    // Charge Mode(Pb Type)
UINT8       CHG_STS[8];                     // Charge Status(Pb Type)
UINT8       Batt_OV_STS = 0;                // Battery Over Voltage Status from Module
UINT8       Batt_UV_STS = 0;                // Battery Ounder Voltage Status from Module
UINT8       Batt_Mode_STS = 0;              // Battery Mode Status from Module

UINT8		bNfbState1,bNfbState1Old;		//nfb state : on/off
UINT8		bNfbState2,bNfbState2Old;		//nfb state : on/off

// Module Response Data
FLOAT32     Set_Batt_OV = Batt_OV_Ref;      // Load the Reference Data into a Battery Over Voltage Value 
FLOAT32     Set_Batt_UV = Batt_UV_Ref;      // Load the Reference Data into a Battery Under Voltage Value
FLOAT32     RST_TIME = RST_Time_Min;        // Load the Reference Data into a Minimum Restart Time Value
FLOAT32     Spare2 = 0;                     
FLOAT32     Spare3 = 0;
FLOAT32     Spare4 = 0;

// Module Response backup Data
FLOAT32     Set_Batt_OV_B = Batt_OV_Ref;    // Load the Reference Data into a Battery Over Voltage Value
FLOAT32     Set_Batt_UV_B = Batt_UV_Ref;    // Load the Reference Data into a Battery Under Voltage Value
FLOAT32     RST_Time_B = RST_Time_Min;      // Load the Reference Data into a Minimum Restart Time Value
FLOAT32     Spare2_B = 0;
FLOAT32     Spare3_B = 0;
FLOAT32     Spare4_B = 0;

UINT8 SYS_Type = 0;//module status from module//

//function//
void adcOp(void)
{
    
}

void Mode_Check(void)
{
    //write_EE(SYS_TYPE,SYS_Type);
    //SYS_Type = read_EE(SYS_TYPE);
    EEPROMWrite(SYS_TYPE,SYS_Type);
    SYS_Type = EEPROMRead(SYS_TYPE);
}

