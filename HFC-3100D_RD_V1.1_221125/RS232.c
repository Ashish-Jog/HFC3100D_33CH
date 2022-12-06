/* 
 * File:   RS232.c
 * Author: KHJ
 *
 * Created on 2018? 2? 7? (?), ?? 4:09
 */
#include <xc.h>
//#include <libpic30.h>
#include    "Definition.h"
#include    "RS232.h"
//#include    "ADC.h"
#include "mcc_generated_files/adc1.h"
#include    "Alram_Sts.h"
#include    "variables.h"
//#include    "mcc_generated_files/pin_manager.h"
#include    "mcc_generated_files/uart1.h"
#include "mcc_generated_files/pin_manager.h"

//-----------------LCD Comm(RS-232)---------------------------//
//void Init_Uart2(void);                      // for RS-232
void Serial_Process(void);                 // Continuous Data TX
void Receive_Data(void);                    // Continuous Data RX
void Set_Value(void);                       // Set Data TX
void Response_Serial(void);                // Data Response Select
void Response_FB(void);                    // Set Data Feedback RX
void Check_Data(void);                     //Set Data Chek & Rety
void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch);

//-------------------------------------------------------------------------
// RS232 Variable
UINT8       f_SData;
UINT8       f_EData;
UINT8       Rindex=0;
UINT8       RxBuf[36];
UINT8       TxBuf[36];
UINT8       TxIndex=0;
UINT8       Set_OK=0;
UINT8		EOT_Flag=0;

// Input_Voltage
FLOAT32     Input_Volt_R = 0;
FLOAT32     Input_Volt_S = 0;
FLOAT32     Input_Volt_T = 0;
FLOAT32     Input_Spare1 = 0;
FLOAT32     Input_Spare2 = 0;
FLOAT32     Input_Spare3 = 0;
//FLOAT32     Input_Spare4 = 0;

// Alram_status Bit
UINT8       Rx_Char = 0x30;
UINT8       Rx_Spare1 = 0x30;
UINT8       Rx_Spare2 = 0x30;
UINT8       BATT_A = 6;

// LCD Diplay Command Data
FLOAT32     Set_ARef = ARef;
FLOAT32     Set_CL = OCL;
FLOAT32     Set_VRef = RefOutputVoltage_PB;
FLOAT32     Set_UV = MinOutputVoltage_PB;
FLOAT32     Set_OV = MaxOutputVoltage_PB;
FLOAT32     Set_BattAmp = RefBattAmp;
FLOAT32     Set_OT = OverTempLimitmax;
UINT8       ADJ_VGain = 0;
UINT8       ADJ_CGain = 0;
UINT8       CHG_Info = 0;

// Module Receive Feedback Data
FLOAT32     Set_ARef_FB = 0;
FLOAT32     Set_CL_FB = 0;
FLOAT32     Set_VRef_FB = 0;
FLOAT32     Set_UV_FB = 0;
FLOAT32     Set_OV_FB = 0;
FLOAT32     Set_BattAmp_FB = 0;
FLOAT32     Set_OT_FB = 0;
UINT8       ADJ_VGain_FB = 0;
UINT8       ADJ_CGain_FB = 0;
UINT8       CHG_Info_FB = 0;


// Module Continous Data
FLOAT32     Voltage_Out = 0;
FLOAT32     Current_Out = 0;
FLOAT32     Temperature = 0;
FLOAT32     Input_Volt_R_FB = 0;
FLOAT32     Input_Volt_S_FB = 0;
FLOAT32     Input_Volt_T_FB = 0;
UINT8       UnderVoltageStatus = 0;
UINT8       OverVoltageStatus = 0;
UINT8       OverCurrentStatus = 0;
UINT8       OverTemperatureStatus = 0;

bool8       CHG_Sts={0,0,0,0,0,0,0,0};

// RS-232 Count
UINT8	CheckDataCnt=0;

//                                                       Module <-> Module LCD-KHJ                                                            //
//==================================================================================================================================================================//
//                                                      Request Data Protocol Packet                                                                               //
// '[' + 'D'+ '0' + 'S' + TxBuf[0~19](Set_ARef[0~3] / Set_CL[4~7] / Set_VRef[8~11] / Set_UV[12~15] / Set_OV[16~19] / Set_BCL[20~23] 
//  + SET_OT[24] + ADJ_VGain[25] + ADJ_CGain[26] + CHG_Info[27]) + ']'  
// 
//                                                     Response Data Protocol Packet
// '<' + 'U'+ '0' + 'R' + TxBuf[0~19](Set_ARef_FB[0~3] / Set_CL_FB[4~7] / Set_VRef_FB[8~11] / Set_UV_FB[12~15] / Set_OV_FB[16~19] / Set_BCL_FB[20~23]
//  + SET_OT_FB[24] + ADJ_VGain_FB[25] + ADJ_CGain_FB[26] + CHG_Info_FB[27]) + '>'
//==================================================================================================================================================================//

void Response_Serial(void)
{
	switch(RxBuf[1])                            //Unit Select
	{
        case 'U':
            switch(RxBuf[2])                    //Module ID Select          
            {                
                ////////////////////////////////////////////////////////////////
                //-----------------------Module-----------------------------//               
                //////////////////////////////////////////////////////////////// 
                case '0':                       //Module 1
                    switch(RxBuf[3])            //Ctrl Code Select
                    {
                        case 'R':               //Setting Data Set Start
                        Response_FB();          //Module Response Data Recieve
                        Check_Data();
                        break;                  //Setting Data Set Start END
                        
                        case 'C':
                        Receive_Data();           //LCD Continues Data Recieve
                        break;
                      
                        default:
                        break;
                    }                           //Ctrl Code Select
                break;                          //Module 1
                               
                default:
                break;
            }
        break;
        
        default:
        break;
	}
  	f_SData = 0;
    f_EData = 0;
    EOT_Flag = 0;
}

void Check_Data(void)
{
	if(CheckDataCnt < 3)
	{
		CheckDataCnt++;
		if(Set_ARef != Set_ARef_FB || Set_CL != Set_CL_FB || Set_VRef != Set_VRef_FB || Set_UV != Set_UV_FB || Set_OV != Set_OV_FB || Set_OT != Set_OT_FB || Set_BattAmp != Set_BattAmp_FB)
		{
			Set_OK = 1;
		}
	}
    else
    {
        CheckDataCnt=0;
    }
}


void Response_FB(void)
{
    int tmpvalue = 0;
    unsigned char a,b,c,d;
    // Current Reference Setting Feedback
    a = RxBuf[4];
    b = RxBuf[5];
    c = RxBuf[6];
    d = RxBuf[7];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    Set_ARef_FB = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
                               
     // Current Limit Setting Feedback
    a = RxBuf[8];
    b = RxBuf[9];
    c = RxBuf[10];
    d = RxBuf[11];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Set_CL_FB = tmpvalue * 0.1;    //  xxxx -> xxx.x
                }
            }
        }
    }

    // Voltage Reference Setting Feedback
    a = RxBuf[12];
    b = RxBuf[13];
    c = RxBuf[14];
    d = RxBuf[15];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Set_VRef_FB = tmpvalue * 0.1;      //  xxxx -> xxx.x
                }
            }
        }		
    }
    // Under Voltage Limit Feedback
    a = RxBuf[16];
    b = RxBuf[17];
    c = RxBuf[18];
    d = RxBuf[19];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Set_UV_FB = tmpvalue * 0.1;      //xxxx -> xxx.x
                }
            }
        }		
    }
    // Over Voltage Limit Feedback
    a = RxBuf[20];
    b = RxBuf[21];
    c = RxBuf[22];
    d = RxBuf[23];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;	
                    Set_OV_FB = tmpvalue * 0.1;        //xxxx -> xxx.x
                }
            }
        }		
    }
    // Over Batt limit Feedback
    a = RxBuf[24];
    b = RxBuf[25];
    c = RxBuf[26];
    d = RxBuf[27];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;	
                    Set_BattAmp_FB = tmpvalue * 0.1;        //xxxx -> xxx.x
                }
            }
        }
    }
    
    // Over temp limit Feedback
    a = RxBuf[28];
    if(a > 0 && a <= OverTempLimitmax)      // Set OT < Max
    {
        Set_OT_FB = a;
    }

    // ADC Gain Set
    ADJ_VGain_FB    =  RxBuf[29];
    ADJ_CGain_FB    =  RxBuf[30];
    CHG_Info_FB   =  RxBuf[31];
}

void Set_Value()
{
    unsigned char i = 0,checksum=0;
    int tmpvalue = 0;
    tmpvalue = Set_ARef * 10;
    Send_Serial(tmpvalue,1,'S');
    tmpvalue = Set_CL * 10;
    Send_Serial(tmpvalue,2,'S');
    tmpvalue = Set_VRef * 10;
    Send_Serial(tmpvalue,3,'S');
    tmpvalue = Set_UV * 10;
    Send_Serial(tmpvalue,4,'S');
    tmpvalue = Set_OV * 10;
    Send_Serial(tmpvalue,5,'S');
    tmpvalue = Set_BattAmp * 10;
    Send_Serial(tmpvalue,6,'S');
    
    CHG_Info = CHG_Sts.b7;                          // Charge Auto/Manual
    CHG_Info = (CHG_Info << 1) | CHG_Sts.b6;        // Charge Equal/Float Type
    CHG_Info = (CHG_Info << 1) | CHG_Sts.b5;        // Manual Operation Mode
    CHG_Info = (CHG_Info << 1) | CHG_Sts.b4;        // BATT Type
    CHG_Info = (CHG_Info << 1) | CHG_Sts.b3;        // 125/110VDC
    CHG_Info = (CHG_Info << 1) | CHG_Sts.b2;
    CHG_Info = (CHG_Info << 1) | CHG_Sts.b1;
    CHG_Info = (CHG_Info << 1) | CHG_Sts.b0;

    TxBuf[0] = '[';                      //Start of Transmission Packet - SOT => 0x5B '['
    TxBuf[1] = 'D';                      //Display Unit      => 0x44 'D'
    TxBuf[2] = '0';                      //Module ID data   => 0
    TxBuf[3] = 'S';                      //Data Type => Response => 0x53 'S'
	
    TxBuf[28] = Set_OT;
    TxBuf[29] = ADJ_VGain;
    TxBuf[30] = ADJ_CGain;
    TxBuf[31] = CHG_Info;
    
	for(i=0;i<32;i++)
	{
		checksum^=TxBuf[i];
	}
	TxBuf[32] = (checksum>>4)+0x30;
	TxBuf[33] = (checksum&0x0F)+0x30;
	TxBuf[34] = ']';					//End of Transmission Packet - EOT => 0x5D ']'
    for(i=0; i<35; i++)                 //Data Packet 28Byte
    {
        UART1_Write(TxBuf[i]);   //#2                
        while(!U1STAbits.TRMT); 
        //Sp2_LED_Toggle();
//        U2TXREG = TxBuf[i];
//        while(!U2STAbits.TRMT);
    }
}


void Receive_Data(void)
{
    int tmpvalue = 0;
    unsigned char a,b,c,d;
    // Voltage Output
    a = RxBuf[4];
    b = RxBuf[5];
    c = RxBuf[6];
    d = RxBuf[7];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    Voltage_Out = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
                               
    // Current Output
    a = RxBuf[8];
    b = RxBuf[9];
    c = RxBuf[10];
    d = RxBuf[11];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Current_Out = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }
    }

    // Temperature
    a = RxBuf[12];
    b = RxBuf[13];
    c = RxBuf[14];
    d = RxBuf[15];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Temperature = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }  
    
    // Input Voltage R
    a = RxBuf[16];
    b = RxBuf[17];
    c = RxBuf[18];
    d = RxBuf[19];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    Input_Volt_R_FB = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
                               
    // Input Voltage S
    a = RxBuf[20];
    b = RxBuf[21];
    c = RxBuf[22];
    d = RxBuf[23];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Input_Volt_S_FB = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }
    }

    // Input Voltage T
    a = RxBuf[24];
    b = RxBuf[25];
    c = RxBuf[26];
    d = RxBuf[27];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Input_Volt_T_FB = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }
    
    // Input Status
    Rx_Char = RxBuf[28];
    Rx_Spare1 = RxBuf[29];
    Rx_Spare2 = RxBuf[30];
    BATT_A = RxBuf[31];  
    
}


//                                              Module -> Module LCD Transmit-KHJ                                    //
//===================================================================================================================//
//                                       Continuous Data Protocol Packet                                             //
//  '<' + 'U'+ '0' + 'C' + TxBuf[0~7](Vlotage[0~3] / Ampere[4~7]) / Temperature[8~11]                  //   
//  Input_Volt_R[12~15] / Input_Volt_S[16~19] / Input_Volt_S[20~23]) / STS[24] / SP1[25] / SP2[26] / SP2[27] + '>'   //
//--------------------------------------------------------------------------------------------------------------------------------------------//
//                                        Response Data Protocol Packet                                                                       //
// '<' + 'U'+ '0' + 'R' + TxBuf[0~19](C_Ref[0~3] / OC_Limt[4~7] / V_Ref[8~11] / UV_Limit[12~15] / OV_Limit[16~19]) + '>'      //
//============================================================================================================================================//
void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch)
{
	unsigned int tmpval;
	unsigned char a,b,c,d,e;
	//==================================================================// 
	//TxBuf[0~7] ===> 'C' [0~3] / [4~7]									//
	//TxBuf[0~19] ===> 'R' [1~3] / [4~7] / [8~11] / [12~15] / [16~19]	//
	//==================================================================//
	tmpval = ival;
	//
	a=__builtin_divud((long)tmpval,10000);
	b=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000)) ),1000);
	c=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000)) ),100);
	d=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000) + __builtin_muluu(c,100)) ),10);
	e= tmpval%10;
	//
    switch(ch)		//KHJ - RS-232 & RS-485
    {
        case 'C':	//Tx Type -Continuous
            switch(pindex)
            {
                case 1:
                    TxBuf[4]= b + 0x30;                 //Volt_Out_BUS/1000 => 1000
                    TxBuf[5]= c + 0x30;                 //Volt_Out_BUS/100 => 100
                    TxBuf[6]= d + 0x30;                 //Volt_Out_BUS/10  => 10
                    TxBuf[7]= e + 0x30;                 //Volt_Out_BUS%1  => 1
                break;
                        //
                case 2:
                    TxBuf[8]= b + 0x30;                 //Currentt_Out/1000 => 1000
                    TxBuf[9]= c + 0x30;                 //Currentt_Out/100 => 100
                    TxBuf[10]= d + 0x30;                 //Currentt_Out/10  => 10
                    TxBuf[11]= e + 0x30;                 //Currentt_Out%1  => 1
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                 //Temp/1000 => 1000
                    TxBuf[13]= c + 0x30;                 //Temp/100 => 100
                    TxBuf[14]= d + 0x30;                //Temp/10  => 10
                    TxBuf[15]= e + 0x30;                //Temp%1  => 1
                break;
                
                case 4:
                    TxBuf[16]= b + 0x30;                 //Input_Volt_R/1000 => 1000
                    TxBuf[17]= c + 0x30;                 //Input_Volt_R/100 => 100
                    TxBuf[18]= d + 0x30;                 //Input_Volt_R/10  => 10
                    TxBuf[19]= e + 0x30;                 //Input_Volt_R%1  => 1
                break;
                
                case 5:
                    TxBuf[20]= b + 0x30;                 //Input_Volt_S/1000 => 1000
                    TxBuf[21]= c + 0x30;                 //Input_Volt_S/100 => 100
                    TxBuf[22]= d + 0x30;                 //Input_Volt_S/10  => 10
                    TxBuf[23]= e + 0x30;                 //Input_Volt_S%1  => 1
                break;
                
                case 6:
                    TxBuf[24]= b + 0x30;                 //Input_Volt_T/1000 => 1000
                    TxBuf[25]= c + 0x30;                 //Input_Volt_T/100 => 100
                    TxBuf[26]= d + 0x30;                 //Input_Volt_T/10  => 10
                    TxBuf[27]= e + 0x30;                 //Input_Volt_T%1  => 1
                break;

                default:
                break;
            }
        break; 
        
        case 'S':		//Tx Type - Request
            switch(pindex)
            {
                case 1:
                    TxBuf[4]= b + 0x30;                 //Set_ARef/1000 => 1000
                    TxBuf[5]= c + 0x30;                 //Set_ARef/100 => 100
                    TxBuf[6]= d + 0x30;                 //Set_ARef/10  => 10
                    TxBuf[7]= e + 0x30;                 //Set_ARef%1  => 1
                break;
                //
                case 2:
                    TxBuf[8]= b + 0x30;                 //Set_CL/1000 => 1000
                    TxBuf[9]= c + 0x30;                 //Set_CL/100 => 100
                    TxBuf[10]= d + 0x30;                 //Set_CL/10  => 10
                    TxBuf[11]= e + 0x30;                 //Set_CL%1  => 1
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                 //Set_VRef/1000 => 1000
                    TxBuf[13]= c + 0x30;                 //Set_VRef/100 => 100
                    TxBuf[14]= d + 0x30;                //Set_VRef/10  => 10
                    TxBuf[15]= e + 0x30;                //Set_VRef%1  => 1
                break;
                //
                case 4:
                    TxBuf[16]= b + 0x30;                 //Set_UV/1000 => 1000
                    TxBuf[17]= c + 0x30;                 //Set_UV/100 => 100
                    TxBuf[18]= d + 0x30;                 //Set_UV/10  => 10
                    TxBuf[19]= e + 0x30;                 //Set_UV%1  => 1
                break;
                
                case 5:
                    TxBuf[20]= b + 0x30;                 //Set_OV/1000 => 1000
                    TxBuf[21]= c + 0x30;                 //Set_OV/100 => 100
                    TxBuf[22]= d + 0x30;                 //Set_OV/10  => 10
                    TxBuf[23]= e + 0x30;                 //Set_OV%1  => 1
                break;
                
                case 6:
                    TxBuf[24]= b + 0x30;                 //Set_BattAmp/1000 => 1000
                    TxBuf[25]= c + 0x30;                 //Set_BattAmp/100 => 100
                    TxBuf[26]= d + 0x30;                 //Set_BattAmp/10  => 10
                    TxBuf[27]= e + 0x30;                 //Set_BattAmp%1  => 1
                break;
                   
                default:
                break;
                
            }
        break; 
     
        default:
        break;
    }

}

void Serial_Process(void)
{
    int tmpvalue=0;
    unsigned char i=0,checksum=0;
    unsigned char hibyte, lobyte;

    if(f_SData && f_EData)
    {
        Response_Serial();
    }
    //========================================================================//
    //                 Continuous Data Protocol Packet                        //
    //           '[' + 'U'+ '0' + 'C' + TxBuf[0~24]+ SP4 + ']'                // 
    //========================================================================//
    else
    {    
        //-------------------------------------------------------------//
        //                  Control Request Data Send                  //
        //-------------------------------------------------------------//
        //if(tmr2Count3 >= RS232_Period)				//elapse time about 500ms~750ms
        if(tmr232 >= RS232_Period)//200ms interval//
        {    
            
            Sp1_LED_Toggle(); //goooo
            tmr232 = 0;
            if(Set_OK == 1)
            {
                if(Key_Cnt == 0)
                {
                    Key_Cnt = 1;
                    Set_Value();
                    Set_OK = 0; 
                }
                else
                {
                    Set_OK = 0;
                }
            }
            //----------------------------------------------------------//
            //              Metering & Alarm Data Transmit-KHJ          //
            //----------------------------------------------------------//   
            else if(Set_OK == 0)
            {
                Key_Cnt = 0;         
                ///////////////////////////////////////////
                //          Metering data(24byte)         //
                ///////////////////////////////////////////   
                Input_Volt_R = ACV_R_filter;
                Input_Volt_S = ACV_S_filter;
                Input_Volt_T = ACV_T_filter;
                Input_Spare1 = ACI_R_filter;
                Input_Spare2 = ACI_S_filter;
                Input_Spare3 = ACI_T_filter;

                tmpvalue = Input_Volt_R*10;             //Input_Volt_R
                Send_Serial(tmpvalue,1,'C');

                tmpvalue = Input_Volt_S*10;             //Input_Volt_S
                Send_Serial(tmpvalue,2,'C');

                tmpvalue = Input_Volt_T*10;             //Input_Volt_T
                Send_Serial(tmpvalue,3,'C');                 

                tmpvalue = Input_Spare1*10;             //ACI_R_filter 4bytes
                Send_Serial(tmpvalue,4,'C');

                Input_Spare1 = ACI_S;        
                tmpvalue = Input_Spare1*10;             //ACI_S_filter 4bytes
                Send_Serial(tmpvalue,5,'C');

                Input_Spare1 = ACI_T;    
                tmpvalue = Input_Spare1*10;             //ACI_T_filter 4bytes[24,25,26,27]
                Send_Serial(tmpvalue,6,'C');
                //RDVERSION=============================================== 
                //hibyte + lobyte//
                hibyte = 0x00;
                lobyte = 0x00;
                hibyte = (unsigned char)(RDVERSION>>8);//0000-0000|0110-1110
                hibyte |= (0x08);//xxxx-1000
                lobyte = (unsigned char)(RDVERSION & 0x00ff);
                TxBuf[26] = hibyte; //xxxx-10nn | REVERSION
                TxBuf[27] = lobyte; //xxxx-10xx`
                //---------------------------------------------------------
                //RS-232 Module
                TxBuf[0] = '[';                      //Start of Transmission Packet - SOT => 0x5B '[' 
                TxBuf[1] = 'D';                      //Chrger Unit      => 0x44 'D'
                TxBuf[2] = '0';                      //Module ID data   => 0x30 '0'
                TxBuf[3] = 'C';                      //Data Type => Continues 'C'
                //        
                TxBuf[28] = Input_OV + Input_UV + Input_Shutdown;		//Alram Status Packet
                TxBuf[29] = Output_OV + Output_UV;
                TxBuf[30] = Auto_Flag;
                TxBuf[31] = BATT_A;

                for(i=0;i<32;i++)
                {
                    checksum^=TxBuf[i];
                }
                TxBuf[32]=(checksum>>4)+0x30;
                TxBuf[33]=(checksum&0x0F)+0x30;
                TxBuf[34]=']';						//End of Transmission Packet - EOT
                for(i=0; i<35; i++)                 //Data Packet 28Byte
                {
                    UART1_Write(TxBuf[i]);      //#1              
                    while(!U1STAbits.TRMT); 
                } 
            }
        }
    }
}//End Serial_Process(void)


////==================================== RS232 ===================================
//void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt() //RS232
////void __attribute__((interrupt,auto_psv)) _U2RXInterrupt()
//{//RS-232
//	unsigned char RcvData,tmp=0,checksum=0,i;	
//    if(U2STAbits.OERR == 1)     //Clear Overflow Flag - KHJ
//    {
//        U2STAbits.OERR = 0;
//    }
//    RcvData = U2RXREG;              //RXbuff 
//    if(RcvData == '<' && EOT_Flag == 0)             //Start of Transmission Packet - SOT => 0x3C '<'
//	{
//        f_SData=1;                  //start serial data flag 
//		Rindex=0;
//		EOT_Flag=1;
//	}
//    //
//	else if((f_SData == 1) && (RcvData == 0x3E))//EOT of Transmission Packet - EOT => 0x3E '>'
//	{
//		for(i=0;i<32;i++)
//		{
//			checksum^=RxBuf[i];
//		}
//		tmp=((RxBuf[32]-0x30)<<4)&0xF0;
//		tmp=(tmp+((RxBuf[33]-0x30)&0x0F));
//	
//		if(tmp == checksum)
//		{
//			f_EData=1;
//			EOT_Flag=0;
//		}	
//		else
//		{
//			f_SData=0;
//		}		
//	}		
//	RxBuf[Rindex]=RcvData;
//	Rindex++;
//	//
//	if(Rindex>36)	//Occur overflow -> Reset buffer//
//	{	
//		Rindex=0;	
//		f_SData=0;
//		f_EData=0;
//		EOT_Flag=0;
//	}
//    
//	IFS1bits.U2RXIF=0;
//}  
//
//void __attribute__((__interrupt__, no_auto_psv)) _U2TXInterrupt()   //RS232
//{//RS-232
//	IFS1bits.U2TXIF=0;
//}  
///////////////////////////////////////////////////////////////////////////////

