/* 
 * File:   RS232.h
 * Author: KHJ
 *
 * Created on 2018? 2? 7? (?), ?? 4:09
 */

//-----------------LCD Comm(RS-232)---------------------------//
//extern void Init_Uart2(void);					// for RS-232
extern void Serial_Process(void);				// Continuous Data TX
extern void Receive_Data(void);					// Continuous Data RX
extern void Set_Value(void);					// Set Data TX
extern void Response_Serial(void);				// Data Response Select
extern void Response_FB(void);					// Set Data Feedback RX
extern void Check_Data(void);					//Set Data Chek & Rety
extern void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch);

//-------------------------------------------------------------------------
// RS-232 Flag
extern UINT8	Set_OK;
extern UINT8	f_SData;
extern UINT8	f_EData;
extern UINT8	Rindex;
extern UINT8	RxBuf[36];
extern UINT8	TxBuf[36];
extern UINT8	TxIndex;
extern UINT8	EOT_Flag;
extern unsigned char   Key_Cnt;

// Input_Voltage
extern FLOAT32	Input_Volt_R;
extern FLOAT32	Input_Volt_S;
extern FLOAT32	Input_Volt_T;
extern FLOAT32	Input_Spare1;
extern FLOAT32	Input_Spare2;
extern FLOAT32	Input_Spare3;
extern FLOAT32	Input_Spare4;

// Alram_status Bit
extern UINT8    Rx_Char;
extern UINT8    Rx_Spare1;
extern UINT8    Rx_Spare2;
extern UINT8    BATT_A;

// LCD Display Command
extern FLOAT32   Set_ARef;
extern FLOAT32   Set_CL;
extern FLOAT32   Set_VRef;
extern FLOAT32   Set_UV;
extern FLOAT32   Set_OV;
extern FLOAT32   Set_BattAmp;
extern FLOAT32   Set_OT;
extern UINT8     ADJ_VGain;
extern UINT8     ADJ_CGain;
extern UINT8     CHG_Info;

// Module Receive Command Feedback
extern FLOAT32   Set_ARef_FB;
extern FLOAT32   Set_CL_FB;
extern FLOAT32   Set_VRef_FB;
extern FLOAT32   Set_UV_FB;
extern FLOAT32   Set_OV_FB;
extern FLOAT32   Set_BattAmp_FB;
extern FLOAT32   Set_OT_FB;
extern UINT8     ADJ_VGain_FB;
extern UINT8     ADJ_CGain_FB;
extern UINT8     CHG_Info_FB;

// Module Continuous Data
extern FLOAT32   Voltage_Out;
extern FLOAT32   Current_Out;
extern FLOAT32   Temperature;
extern FLOAT32   Input_Volt_R_FB;
extern FLOAT32   Input_Volt_S_FB;
extern FLOAT32   Input_Volt_T_FB;
extern UINT8    UnderVoltageStatus;
extern UINT8    OverVoltageStatus;
extern UINT8    OverCurrentStatus;
extern UINT8    OverTemperatureStatus;
extern bool8    CHG_Sts;

// RS-232 Count
extern UINT8	CheckDataCnt;
extern UINT8	Output_OV,Output_UV;
extern unsigned int    tmr2Count3;
