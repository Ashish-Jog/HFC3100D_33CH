#define tm0 tmrCnt[0]
#define tm1 tmrCnt[1]   
#define tm2 tmrCnt[2]   
#define tm3 tmrCnt[3]   
#define tm4 tmrCnt[4]   
#define tm5 tmrCnt[5]   
#define tm6 tmrCnt[6]   
#define tm7 tmrCnt[7]   
#define tm8 tmrCnt[8]   
#define tm9 tmrCnt[9]   

extern void adOp(void);
extern void Mode_Check(void);


extern volatile unsigned int tmrCnt[10];
extern volatile unsigned int tmr485;
extern volatile unsigned int tmr232;
extern volatile unsigned int tmr485Send;
//extern unsigned int timeOutModule1; //-->tm8
//extern unsigned int timeOutModule2; //-->tm9 
extern volatile unsigned int tmrContact1;
extern volatile unsigned int tmrContact2;
extern volatile unsigned int tmrContact3;
//
extern unsigned int fNormalStateTimer;
extern unsigned int fNormalState;
//------------------------------------------------------------------------------
// RS485 Variable

extern UINT8       f_SData_485;                // Start Character Packet from Module
extern UINT8       f_EData_485;                // End Character Packet from Module 
extern UINT8       EOT_Flag485;                // End of Transmit Flag from Module
extern UINT8       Rindex_485;                 // Rxbuffer Index Variable from Module
extern UINT8       RxBuf_485[36];                  // Rxbuffer for Debug from Module
extern char       TxBuf_485[36];                  // Txbuffer for Debug from Module
extern UINT8       TxIndex_485;                // Tx 1Byte buffer for Debug from Module

// Module #N Continous Data
extern FLOAT32     Voltage_Out[4];                 // Modules Voltage Output
extern FLOAT32     Current_Out[4];                 // Modules Current Output
extern FLOAT32     Temperature[4];                 // Modules Temperature
extern FLOAT32     Input_Volt_R[4];                // Modules Input Voltage R
extern FLOAT32     Input_Volt_S[4];                // Modules Input Voltage S
extern FLOAT32     Input_Volt_T[4];                // Modules Input Voltage T
extern UINT8       Rx_Char[4];                     // Modules Status
extern UINT8       Rx_Char2[4];                    // Modules Warning Status                   
extern UINT8       Rx_Char3[4];
extern UINT8       Rx_Spare1[4];

extern bool8       Alarm_Sts[5];                   // Modules Operation Status
extern bool8       Warning_Sts[5];                 // Modules Warning Status
extern bool8       CHG_Info[5];                    // Module Charge type

// Module #N Set Data
extern UINT8       ID_485;                     // Modules No. initial First No. 1

//Operation Mode Variable
extern UINT8       OP_MODE;                    // OP_MODE => '0' : Manual / '1': Auto 
extern UINT8       OP_CMD;                     // Command Change Operation Command Module2
extern UINT8       OP_Type;                    // 0 : Stand-alone or 1 : Load Share Mode

//==============================================================================
//rs232//
//==============================================================================
extern bool8       LED_Sts1;         // NFB LED Status Variable for LCD
extern bool8       LED_Sts2;         // NFB LED Status Variable for LCD
extern bool8       LED_Sts3;         // Spare1 LED Status Variable for LCD
extern bool8       LED_Sts4;         // Spare2 LED Status Variable for LCD

extern bool8       Module1_Sts;      // Module1 Operation Status for LCD
extern bool8       Module2_Sts;      // Module2 Operation Status for LCD
extern bool8       Module3_Sts;      // Module3 Operation Status for LCD
extern bool8       Module4_Sts;      // Module4 Operation Status for LCD

extern bool8       Batt_Sts;         // Battery Condition Status for LCD
extern bool8       Module_Sts;       // Module Status for LCD
extern bool8       CHG_Sts;       // Spare Status1 for LCD
extern bool8       Spare_Sts1;       // Spare Status2 for LCD
// RS232 -Debug Variable
extern UINT8       f_SData_Debug;              // Start Character Packet for Debug
extern UINT8       f_EData_Debug;              // End Character Packet for Debug 
extern UINT8       EOT_Flag_Debug;             // End of Transmit Flag for Debug
extern UINT8       Rindex_Debug;               // Rxbuffer Index Variable for Debug
extern UINT8       RxBuf_Debug[36];                // Rxbuffer for Debug for Debug            
extern UINT8       TxBuf_Debug[36];                // Txbuffer for Debug for Debug
extern UINT8       Tx_Char_Debug[12];              // Tx 1Byte buffer for Debug
extern UINT8       TxIndex_Debug;                // Txbuffer Index Variable for Debug

// RS232 Variable
extern UINT8       f_SData;                    // Start Character Packet from Module
extern UINT8       f_EData;                    // End Character Packet from Module 
extern UINT8       EOT_Flag;                   // End of Transmit Flag from Module
extern UINT8       Rindex;                     // Rxbuffer Index Variable from Module
extern UINT8       RxBuf[36];                      // Rxbuffer for Debug from Module
extern char       TxBuf[36];                      // Txbuffer for Debug from Module
extern UINT8       Tx_Char[12];                    // Tx 1Byte buffer for Debug from Module
extern UINT8       TxIndex;                      // Txbuffer Index Variable from Module

// LCD Continous Data
extern FLOAT32     Batt_Volt;                  // Battery Voltage from Module
extern FLOAT32     Batt_Amp;                   // Battery Current from Module
extern FLOAT32     Load_Volt;                  // Load Voltage from Module
extern FLOAT32     Load_Amp;                   // Load Current from Module
extern UINT8       LED_Module1_NFB;            // Module1_NFB LED from Module
extern UINT8       LED_Module2_NFB;            // Module2_NFB LED from Module
extern UINT8       LED_Module3_NFB;            // Module3_NFB LED from Module
extern UINT8       LED_Module4_NFB;            // Module4_NFB LED from Module
extern UINT8       LED_DC1_NFB;                    // DC1 NFB LED from Module
extern UINT8       LED_DC2_NFB;                    // DC2 NFB LED from Module
extern UINT8       LED_DC3_NFB;                    // DC3 NFB LED from Module
extern UINT8       LED_DC4_NFB;                    // DC4 NFB LED from Module
extern UINT8       LED_Battery_NFB;                // Battery NFB LED from Module
extern UINT8       LED_Spare1_NFB;                 // Spare1 NFB LED from Module
extern UINT8       LED_Spare2_NFB;                 // Spare2 NFB LED from Module
extern UINT8       LED_Load_NFB;                   // Load NFB LED from Module
extern UINT8       LED_Spare3_NFB;                 // Spare3 NFB LED from Module 
extern UINT8       LED_Spare4_NFB;                 // Spare4 NFB LED from Module
extern UINT8       LED_Spare5_NFB;                 // Spare5 NFB LED from Module
extern UINT8       LED_Spare6_NFB;                 // Spare6 NFB LED from Module
extern UINT8       SystemState_STS[4];             // Module State Status from Module
extern UINT8       Input_OV_STS[4];                // Input Over Voltage Status from Module
extern UINT8       Input_UV_STS[4];                // Input Under Voltage Status from Module
extern UINT8       Input_SD_STS[4];                // Input Shutdown Status from Module
extern UINT8       Output_OV_STS[4];               // Output Over Voltage Status from Module
extern UINT8       Output_UV_STS[4];               // Output Under Voltage Status from Module
extern UINT8       Output_OC_STS[4];               // Output Over Current Status from Module
extern UINT8       Output_OT_STS[4];               // Over Temperature Status from Module
extern UINT8       Module_OV_STS[4];               // Output Over Voltage Warning Status from Module
extern UINT8       Module_UV_STS[4];               // Output Under Voltage Warning Status from Module
extern UINT8       CHG_MODE[4];                    // Charge Mode(Pb Type)
extern UINT8       CHG_STS[8];                     // Charge Status(Pb Type)
extern UINT8       Batt_OV_STS;                // Battery Over Voltage Status from Module
extern UINT8       Batt_UV_STS;                // Battery Ounder Voltage Status from Module
extern UINT8       Batt_Mode_STS;              // Battery Mode Status from Module

extern UINT8		bNfbState1,bNfbState1Old;		//nfb state : on/off
extern UINT8		bNfbState2,bNfbState2Old;		//nfb state : on/off

// Module Response Data
extern FLOAT32     Set_Batt_OV;      // Load the Reference Data into a Battery Over Voltage Value 
extern FLOAT32     Set_Batt_UV;      // Load the Reference Data into a Battery Under Voltage Value
extern FLOAT32     RST_TIME;        // Load the Reference Data into a Minimum Restart Time Value
extern FLOAT32     Spare2;                     
extern FLOAT32     Spare3;
extern FLOAT32     Spare4;

// Module Response backup Data
extern FLOAT32     Set_Batt_OV_B;    // Load the Reference Data into a Battery Over Voltage Value
extern FLOAT32     Set_Batt_UV_B;    // Load the Reference Data into a Battery Under Voltage Value
extern FLOAT32     RST_Time_B;      // Load the Reference Data into a Minimum Restart Time Value
extern FLOAT32     Spare2_B;
extern FLOAT32     Spare3_B;
extern FLOAT32     Spare4_B;

extern UINT8   SYS_Type;                    // Module Status from Module

