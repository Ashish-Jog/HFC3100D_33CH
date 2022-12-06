/* 
 * File:   Alram_Sts.h
 * Author: KHJ
 *
 * Created on 2018? 2? 13? (?), ?? 2:00
 */


//=================RS232 Module Define=============================================================

//---------------------------------------------------------------------------------------------------------------------------
// PB Type
#define MinOutputVoltage_PB         90.0    //98.6
#define MinOutputVoltage_PB1        124.7
#define MinOutputVoltage_PB2        127.6

#define MidOutputVoltage_PB         132.0 //shutdown min volt max limit//

#define MaxOutputVoltage_PB         150.0
#define MaxOutputVoltage_PB1        141.0
#define MaxOutputVoltage_PB2        147.0
#define RefOutputVoltage_PB         131.0

//100VDC
#define MinOutputVoltage2_PB        90.0
#define MinOutputVoltage2_PB1       90.0
#define MinOutputVoltage2_PB2       90.0
#define MidOutputVoltage2_PB        120.0
#define MaxOutputVoltage2_PB        150.0
#define MaxOutputVoltage2_PB1       130.0
#define MaxOutputVoltage2_PB2       150.0
#define RefOutputVoltage2_PB        124.0   //115.0


// Li-Ion Type
#define MinOutputVoltage_LI    90.0 //96.0
#define MidOutputVoltage_LI    118.4
#define MaxOutputVoltage_LI    135.0
#define RefOutputVoltage_LI    125.0

#define OverTempLimitmax 	95.0	//18.06.19 95 deg - KHJ 

#define RefBattAmp			30.0
#define MinBattAmp          1.0
#define MaxBattAmp          40.0

//380VAC Type
#define MinInputVoltage_380 342.0	
#define MaxInputVoltage_380 418.0  	
#define RefInputVoltage_380 380.0
#define ShutdownVoltage_380 228.0

//220VAC Type
#define MinInputVoltage_220 198.0	
#define MaxInputVoltage_220 242.0  	
#define RefInputVoltage_220 220.0
#define ShutdownVoltage_220 132.0
//==============================================================================
#define UCL           80.0           //Init Under Current Limit   18.01.15  - KHJ
#define ARef          110.0         //Init Current Reference
#define OCL           120.0         //Init Over Current Limit   18.01.15  - KHJ
//==============================================================================
extern void Alram_Sts(void);
extern void CHG_CMD(void);
extern void CHG_Check(void);
extern void CHG_Time(void);
extern void hex16_3digt(unsigned char hex_data_8);
extern void hex16_4digt(unsigned int hex_data);

extern float   Input_UV_Ref;	//18.06.18 -10% - KHJ
extern float   Input_OV_Ref;  	//18.06.18 +10% - KHJ
extern float   Input_V_Ref;
extern float   Input_Shutdown_Ref;   //18.06.18 -40% - Order by GJJ 
extern UINT8   CHG_Mode;
extern UINT8   CHG_SEL;
extern UINT8   CHG_EN;
extern UINT8   Input_OV;
extern UINT8   Input_UV;
extern UINT8   Input_Shutdown;
extern UINT8   Res_Year[10]; 
extern UINT8   Res_Month[10];
extern UINT8   Res_Day[10];
extern UINT8   Res_Hour[10];
extern UINT8   Res_Min[10];
extern UINT8   Res_Sec[10];

//External Function & Variable
extern UINT8 rm_bcd(UINT8 data);
extern UINT8 read_ds1302(UINT8 cmd);
extern float   Set_Input_UV;
extern float   Set_Input_OV;




