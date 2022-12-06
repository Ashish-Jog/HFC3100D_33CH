/****************************************************************************************
* Application : HFC-3100D														      	*
* Seoul Electric Power System co.,Ltd  	  2018/04/10  	First release of source file  	*
* Device : Rectifier(Microchip dspic33FJ64GS606)										*
* Target PCB No :                                                            *
* Compiler : mplab_C V3.31											 					*
****************************************************************************************/
#include "p33FJ64GS606.h"
#include "Define_PSFB.h"
#include "Variables_PSFB.h"

void Serial_Process(void);
void Response_Serial(void);
void Set_Value(void);
void BittoByte(void);
void BytetoBit(void);
void Continuous_FB(void);
void Response_FB(void);
void Input_Volt(void);
void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch);

bool8 Alram_Sts={0,0,0,0,0,0,0,0};

//-----------------------------------------------------------//
//                  Control Request & Response               //
//-----------------------------------------------------------//
void Serial_Process(void)
{
    if(f_SData && f_EData)          // Check the Data Recieve Complete
    {
        if(tmrtick3 >= RS232_Period)
        {
            tmrtick3 = RS232_Period;
            Response_Serial();          // Order Process
        }        
    }        
}

//                          Module <-> Module LCD-KHJ                         //
//============================================================================//
//                        Request Data Protocol Packet                        //
// '[' + 'D'+ '0' + 'S' + TxBuf[0~27](Set_ARef[0~3] / Set_CL[4~7] /           //
//  Set_VRef[8~11] / Set_UV[12~15] / Set_OV[16~19] / Set_BCL[20~23] /          //
//  Set_OT[24] + ADJ_VGain[25] + ADJ_CGain[26] + CHG_Info[27]) + Checksum[28~29] + ']'                                     //
// ===========================================================================//
//                       Response Data Protocol Packet                        //
// '<' + 'U'+ '0' + 'R' + TxBuf[0~27](Set_ARef[0~3] / Set_CL[4~7] /  //
//  Set_VRef[8~11] / Set_UV[12~15] / Set_OV[16~19] / Set_BCL[20~23] /          //
//  Set_OT[24] + ADJ_VGain[25] + ADJ_CGain[26] + CHG_Info[27]) + Checksum[28~29] + '>'                                     //
//============================================================================//
void Response_Serial(void)
{
	switch(RxBuf[1])                            //Unit Select
	{
        case 'D':                               //Destination : Information from Display
            switch(RxBuf[2])                    //Module Select : '0'       
            {
                ////////////////////////////////////////////////////////////////
                //-----------------------Module-------------------------------//               
                //////////////////////////////////////////////////////////////// 
                
                case '0':                       //Module
                    switch(RxBuf[3])            //Ctrl Code Select
                    {
                        case 'S':               //Setting Data Set Start Start
                        Set_Value();            //LCD Request Data Recieve
                        Response_FB();          //Module Response Data Send
                        break;                  //Setting Data Set Start END
                        
                        case 'C':
                        Input_Volt();           //LCD Continuous Data Recieve
                        Continuous_FB();        //Module Continuous Data Send
                        break;
                      
                        default:
                        break;
                    }                           
                break;                         
                
  
                default:
                break;
            }
        break;
        
        default:
        break;
	}
  	f_SData = 0;                        // Recieve Buffer Clear
    f_EData = 0;                        // Recieve Buffer Clear
    EOT_Flag = 0;
}

//-----------------------------------------------------------//
//                      Set Value for RS-232                 //
//-----------------------------------------------------------//
void Set_Value(void)
{
    int tmpvalue = 0;
    unsigned char a = 0, b = 0, c = 0, d = 0;
    
    CHG_Info = RxBuf[31];
    
    // Current Reference Setting
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
                    Set_ARef = tmpvalue * 0.1;  //  xxxx -> xxx.x
                    if(Set_ARef >= UCL && Set_ARef <= OCL)     // Min < ARef < Current Limit
                    {
                         Set_ARef_B = Set_ARef;
                    }               
                    else
                    {
                        Set_ARef = Set_ARef_B;
                    }
                }
            }
        }		
    }                          
                               
    // Current Limit Setting
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
                    Set_CL = tmpvalue * 0.1;    //  xxxx -> xxx.x
                    if(Set_CL >= UCL && Set_CL <= OCL)      // ARef < Current Limit <= Current Max
                    {
                        Set_CL_B = Set_CL;          // Pre Value Backup
                    }
                    else
                    {
                        Set_CL = Set_CL_B;
                    }
                    Set_CL_ID = (Set_CL - 50) / 2;	// y = 2x + 50
                }
            }
        }
    }

    // Voltage Reference Setting
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
                    Set_VRef = tmpvalue * 0.1;      //  xxxx -> xxx.x
                    if((CHG_Info & 0x10) == 0x10)
                    {
                        if(Set_VRef >= MinOutputVoltage_LI && Set_VRef <= MaxOutputVoltage_LI)      // Set Min < VRef < Set Max
                        {
                            Set_VRef_B = Set_VRef;
                        }
                        else
                        {
                            Set_VRef = Set_VRef_B;
                        }
                    }
                    else if((CHG_Info & 0x10) == 0x00)
                    {
                        if((CHG_Info & 0x08) == 0x00)
                        {
                            if(Set_VRef >= MinOutputVoltage_PB1 && Set_VRef <= MaxOutputVoltage_PB)      // Set Min < VRef < Set Max
                            {
                                Set_VRef_B = Set_VRef;
                            }
                            else
                            {
                                Set_VRef = Set_VRef_B;
                            }
                        }
                        else if((CHG_Info & 0x08) == 0x08)
                        {
                            if(Set_VRef >= MinOutputVoltage2_PB1 && Set_VRef <= MaxOutputVoltage2_PB)      // Set Min < VRef < Set Max
                            {
                                Set_VRef_B = Set_VRef;
                            }
                            else
                            {
                                Set_VRef = Set_VRef_B;
                            }
                        }
                    }
                }
            }
        }		
    }
    
    // Under Voltage Limit 
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
                    //
                    if(tmpvalue>150)tmpvalue = tmpvalue - 150;//???????(min)?? -15.0V gooooo
                     //
                    Set_UV = tmpvalue * 0.1;      //xxxx -> xxx.x
                    if((CHG_Info & 0x10) == 0x10)
                    {
                        if((CHG_Info & 0x20) == 0x20)
                        {
                            Set_UV = Set_UV_B * 0.85;
                        }
                        else if((CHG_Info & 0x20) == 0x00) 
                        {
                            if(Set_UV >= MinOutputVoltage_LI && Set_UV <= MidOutputVoltage_LI)     // Min <= Set Min < VRef
                            {   
                                Set_UV_B = Set_UV;
                            }
                            else
                            {
                                Set_UV = Set_UV_B;
                            }
                        }   
                    }
                    else if((CHG_Info & 0x10) == 0x00)
                    {
                        if((CHG_Info & 0x08) == 0x00)
                        {
                            if((CHG_Info & 0x20) == 0x20)
                            {
                                Set_UV = Set_UV_B * 0.85;
                            }
                            else if((CHG_Info & 0x20) == 0x00) 
                            {
                                if(Set_UV >= MinOutputVoltage_PB && Set_UV <= MidOutputVoltage_PB)     // Min <= Set Min < VRef
                                {   
                                    Set_UV_B = Set_UV;
                                }
                                else
                                {
                                    Set_UV = Set_UV_B;
                                }
                            }
                        }
                        else if((CHG_Info & 0x08) == 0x08)
                        {
                            if((CHG_Info & 0x20) == 0x20)
                            {
                                Set_UV = Set_UV_B * 0.85;
                            }
                            else if((CHG_Info & 0x20) == 0x00) 
                            {
                                if(Set_UV >= MinOutputVoltage2_PB && Set_UV <= MidOutputVoltage2_PB)     // Min <= Set Min < VRef
                                {   
                                    Set_UV_B = Set_UV;
                                }
                                else
                                {
                                    Set_UV = Set_UV_B;
                                }
                            }
                        }
                    }
                }
            }
        }		
    }
    
    // Over Voltage Limit 
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
                    Set_OV = tmpvalue * 0.1;        //xxxx -> xxx.x
                    
                    if((CHG_Info & 0x10) == 0x10)
                    {
                        if(Set_OV >= MidOutputVoltage_LI && Set_OV <= MaxOutputVoltage_LI)     // VRef < Set Max < Max
                        {
                            Set_OV_B = Set_OV;
                        }

                        else 
                        {
                            Set_OV = Set_OV_B;
                        }
                    }
                    else if((CHG_Info & 0x10) == 0x00)
                    {
                        if((CHG_Info & 0x08) == 0x00)
                        {
                            if(Set_OV >= MinOutputVoltage_PB2 && Set_OV <= MaxOutputVoltage_PB)     // VRef < Set Max < Max
                            {
                                Set_OV_B = Set_OV;
                            }
                            else 
                            {
                                Set_OV = Set_OV_B;
                            }
                        }
                        else if((CHG_Info & 0x08) == 0x08)
                        {
                            if(Set_OV >= MinOutputVoltage2_PB2 && Set_OV <= MaxOutputVoltage2_PB)     // VRef < Set Max < Max
                            {
                                Set_OV_B = Set_OV;
                            }
                            else 
                            {
                                Set_OV = Set_OV_B;
                            }
                        }
                    }
                }
            }
        }		
    }
    
    // Battery Current Limit Setting
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
                    Set_BCL = tmpvalue * 0.1;    //  xxxx -> xxx.x
                    if(Set_BCL >= UBCL && Set_BCL <= OBCL)      // Current Min <= Current Limit <= Current Max
                    {
                        if(Set_BCL < 2)
                        {
                            Set_BCL = 2;              // Minimum Control Range 
                        }
                        Set_BCL_B = Set_BCL;          // Pre Value Backup
                    }                                            
                    else
                    {
                        Set_BCL = Set_BCL_B;
                    }
                    Set_BCL_ID = Set_BCL - 2;	// y = x + 2
                }
            }
        }
    }
    
    // Over Temperature Limit 
    a = RxBuf[28];
    if(a > 0 && a <= OverTempLimitmax)      // Set OT < Max
    {
        Set_OT = a;
        Set_OT_B = Set_OT;
    }
    else 
    {
        Set_OT = Set_OT_B;
    }

    // ADC Gain Set
    if(RxBuf[29] < 0x80)
    {
        ADJ_VGain = RxBuf[29];
    }
    else
    {
        ADJ_VGain =  RxBuf[29] - 256;
    }
    if(RxBuf[30] < 0x80)
    {
        ADJ_CGain = RxBuf[30];
    }
    else
    {
        ADJ_CGain =  RxBuf[30] - 256;
    }
    
}

//-------------------------------------------------------------//
//              Bit to Byte Convert for RS-232                 //
//-------------------------------------------------------------//
void BittoByte(void)
{
    //////////////////////////////////////////
    //          Alram data(1byte : 8bit)    //
    //////////////////////////////////////////
    Alram_Sts.b7 = SystemState;             //Operation
    Alram_Sts.b6 = Input_OV;//1;            //Input_OV
    Alram_Sts.b5 = Input_UV;//1;            //Input_UV
    Alram_Sts.b4 = Input_Shutdown;//1;      //Input_Shutdown
    Alram_Sts.b3 = OverVoltageStatus;       //OV
    Alram_Sts.b2 = UnderVoltageStatus;      //UV
    Alram_Sts.b1 = OverCurrentStatus;       //OC
    Alram_Sts.b0 = OverTemperatureStatus;   //OT
    
    ////////////////////////////////////////////////////////////////////////////
    //                      Alram Status Byte Package                         //
    //  MSB ------------------------------------------------------------- LSB //
    //  0    |     0    |     0    |        0       |  0   |  0  |  0   |  0  //
    //  OP   | Input_OV | Input_UV | Input_Shutdown |  OV  | UV  |  OC  |  OT //
    ////////////////////////////////////////////////////////////////////////////
    Tx_Char = Alram_Sts.b7;                          
    Tx_Char = (Tx_Char << 1) | Alram_Sts.b6;
    Tx_Char = (Tx_Char << 1) | Alram_Sts.b5;
    Tx_Char = (Tx_Char << 1) | Alram_Sts.b4;
    Tx_Char = (Tx_Char << 1) | Alram_Sts.b3;
    Tx_Char = (Tx_Char << 1) | Alram_Sts.b2;
    Tx_Char = (Tx_Char << 1) | Alram_Sts.b1;
    Tx_Char = (Tx_Char << 1) | Alram_Sts.b0;
}

//============================================================================//
//                          Continuous Data Protocol Packet                   //
// '<' + 'U'+ '0' + 'C' + TxBuf[0~27](Vlot_Fnd[0~3] / Amp_Val[4~7])  //
//  Temp_Val[8~11] / Input_Volt_R[12~15] / Input_Volt_S[16~19] /              //
//  Input_Volt_T[20~23] / Status[24] / Spare1[25] / Spare2[26] / BATT_A[27])  //
//  + Checksum[28~29] + '>'                                                   // 
//============================================================================//
void Continuous_FB(void)
{
    unsigned char i = 0;
    unsigned int Checksum = 0;
    int tmpvalue = 0;
    
    //----------------------------------------------------------//
    //              Metering & Alram Data Transmit-KHJ          //
    //----------------------------------------------------------//            
    ///////////////////////////////////////////
    //          Metering data(8byte)         //
    ///////////////////////////////////////////   

    tmpvalue = VoltFnd_Avg*10;              //Volt_out_Bus              //Vout_Val*10;
    Send_Serial(tmpvalue,1,'C');
    tmpvalue = AmpFnd_Avg*10;               //Current_out
    Send_Serial(tmpvalue,2,'C');
    tmpvalue = Temp_Val*10;                 //Current_out
    Send_Serial(tmpvalue,3,'C');
    tmpvalue = Input_Volt_R*10;             //Input_Volt_R
    Send_Serial(tmpvalue,4,'C');
    tmpvalue = Input_Volt_S*10;             //Input_Volt_S
    Send_Serial(tmpvalue,5,'C');
    tmpvalue = Input_Volt_T*10;             //Input_Volt_T
    Send_Serial(tmpvalue,6,'C');

    BittoByte();                            //Alram Status Conversion

    //RS-232 Module
    TxBuf[0] = '<';                     //Start of Transmission Packet - SOT => 0x3C '<' 
    TxBuf[1] = 'U';                     //Chrger Unit      => 0x55 'U'
    TxBuf[2] = '0';						//Module ID data   => 0x30 '0'                
    TxBuf[3] = 'C';                     //Data Type => Continues 'C'
                                        //TxBuf[4]~[27]
    TxBuf[28] = Tx_Char;                //Alram Status Packet
    TxBuf[29] = '0';                    //SP1      
    TxBuf[30] = '0';                    //SP2
    
    BATT_A = (char)BattAmpFnd_Avg;
    TxBuf[31] = BATT_A;                   //SP3
    
    for(i=0 ; i<32 ; i++)
    {
        Checksum ^= TxBuf[i];
    }

    TxBuf[32] = ((Checksum & 0xF0) >> 4) + 0x30;        //Checksum High nibble
    TxBuf[33] = (Checksum & 0x0F) + 0x30;               //Checksum Low nibble
    TxBuf[34] = '>';                                    //End of Transmission Packet - EOT
    
    for(i=0; i<35; i++)                                 //Data Packet
    {
        U2TXREG = TxBuf[i];
        while(!U2STAbits.TRMT);
    }
    
}
  
//-----------------------------------------------------------//
//              Converting Bit to Byte for RS-485            //
//-----------------------------------------------------------//
void Response_FB(void)
{
    unsigned char i = 0;
    unsigned int Checksum = 0;
    int tmpvalue = 0;
    
    tmpvalue = Set_ARef * 10;       // Current Set Data Feedback
    Send_Serial(tmpvalue,1,'R');
    tmpvalue = Set_CL * 10;         // Current Limit Set Data Feedback
    Send_Serial(tmpvalue,2,'R');
    tmpvalue = Set_VRef * 10;       // Voltage Set Data Feedback
    Send_Serial(tmpvalue,3,'R');
    tmpvalue = Set_UV * 10;         // Under Voltage Set Data Feedback
    Send_Serial(tmpvalue,4,'R');
    tmpvalue = Set_OV * 10;         // Over Voltage Set Data Feedback
    Send_Serial(tmpvalue,5,'R');
    tmpvalue = Set_BCL * 10;        // Battery Current Limit Set Data Feedback
    Send_Serial(tmpvalue,6,'R');

   //RS-232 Module
    TxBuf[0] = '<';                     //Start of Transmission Packet - SOT => 0x3C '<' 
    TxBuf[1] = 'U';                     //Chrger Unit      => 0x55 'U'
    TxBuf[2] = '0';						//Module ID data   => 0x30 '0'                
    TxBuf[3] = 'R';                     //Data Type => Continues 'R'
                                        //TxBuf[4]~[31]
    TxBuf[28] = Set_OT;
    TxBuf[29] = ADJ_VGain;
    TxBuf[30] = ADJ_CGain;
    TxBuf[31] = CHG_Info;
    
    for(i=0 ; i<32 ; i++)
    {
        Checksum ^= TxBuf[i];
    }
    TxBuf[32] = ((Checksum & 0xF0) >> 4) + 0x30;        //Checksum High nibble
    TxBuf[33] = (Checksum & 0x0F) + 0x30;               //Checksum Low nibble
    TxBuf[34] = '>';                                    //End of Transmission Packet - EOT
    
    for(i=0; i<35; i++)                                 //Data Packet
    {
        U2TXREG = TxBuf[i];
        while(!U2STAbits.TRMT);
    }
}

//-----------------------------------------------------------//
//            Input Voltage Recieve for RS-232               //
//-----------------------------------------------------------//
void Input_Volt(void)
{
    int tmpvalue = 0;
    unsigned char a = 0, b = 0, c = 0, d = 0;
    unsigned char uctmp=0;
    // AC Input Voltage R
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
                    Input_Volt_R = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
                               
    // AC Input Voltage S
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
                    Input_Volt_S = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }
    }

    // AC Input Voltage T
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
                    Input_Volt_T = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }  
    
    // Spare1
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
                    Input_Spare1 = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
                               
    // Spare2
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
                    Input_Spare2 = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }
    }

    // Spare3//---------------------------------------------    
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
                    Input_Spare3 = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }
    //int RDVersion //c=RxBuf[26,b0b1],d =RxBuf[27]
    uctmp = c & 0x03;// 0000-0011//
    RDVersion = (unsigned int)((uctmp<<8)| d);
    // Module Status
    Module_Input = RxBuf[28];
    Input_OV = (Module_Input&0x80) >> 7;
    Input_UV = (Module_Input&0x40) >> 6;
    Input_Shutdown = (Module_Input&0x20) >> 5;
    Input_Spare = (Module_Input&0x10) >> 4;   
    
    //Input Info
    Module_Output = RxBuf[29];
    Module_OV = (Module_Output&0x80) >> 7;
    Module_UV = (Module_Output&0x40) >> 6;
    
    Module_Spare1 = RxBuf[30];
    BATT_A_FB = RxBuf[31];  
    
}
//                                              Module -> Module LCD Transmit-KHJ                                                           //
//==========================================================================================================================================//
//                                       Continuous Data Protocol Packet                                                                    //
//  '<' + 'U'+ '0' + 'C' + TxBuf[0~27](Vlotage[0~3] / Ampere[4~7]) / Temperature[8~11]                                     //   
//  Input_Volt_R[12~15] / Input_Volt_S[16~19] / Input_Volt_S[20~23] / STS[24] / SP1[25] / SP2[26] / BATT_A[27]) + Checksum[28~29] + '>'        //
//------------------------------------------------------------------------------------------------------------------------------------------//
//                                        Response Data Protocol Packet                                                                     //
// '<' + 'U'+ '0' + 'R' + TxBuf[0~27](Set_ARef[0~3] / Set_CL[4~7] / Set_VRef[8~11] / Set_UV[12~15] / Set_OV[16~19]         //
// / Set_OT[24] + ADJ_VGain[25] + ADJ_CGain[26] + CHG_Info[27]) + Checksum[28~29] + '>'                                                                             //
//==========================================================================================================================================//
void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch)
{
	unsigned int tmpval;
	unsigned char a,b,c,d,e;
	//==================================================================// 
	//TxBuf[0~23] ===> 'C' [0~3] / [4~7]									//
	//TxBuf[0~27] ===> 'R' [1~3] / [4~7] / [8~11] / [12~15] / [16~19]	//
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
                    TxBuf[10]= d + 0x30;                //Currentt_Out/10  => 10
                    TxBuf[11]= e + 0x30;                //Currentt_Out%1  => 1
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                //Temp/1000 => 1000
                    TxBuf[13]= c + 0x30;                //Temp/100 => 100
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
        
        case 'R':		//Tx Type - Response
            switch(pindex)
            {
                case 1:
                    TxBuf[4]= b + 0x30;                 //Set_ARef_FB/1000 => 1000
                    TxBuf[5]= c + 0x30;                 //Set_ARef_FB/100 => 100
                    TxBuf[6]= d + 0x30;                 //Set_ARef_FB/10  => 10
                    TxBuf[7]= e + 0x30;                 //Set_ARef_FB%1  => 1
                break;
                //
                case 2:
                    TxBuf[8]= b + 0x30;                 //Set_CL_FB/1000 => 1000
                    TxBuf[9]= c + 0x30;                 //Set_CL_FB/100 => 100
                    TxBuf[10]= d + 0x30;                //Set_CL_FB/10  => 10
                    TxBuf[11]= e + 0x30;                //Set_CL_FB%1  => 1
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                //Set_VRef_FB/1000 => 1000
                    TxBuf[13]= c + 0x30;                //Set_VRef_FB/100 => 100
                    TxBuf[14]= d + 0x30;                //Set_VRef_FB/10  => 10
                    TxBuf[15]= e + 0x30;                //Set_VRef_FB%1  => 1
                break;
                //
                case 4:
                    TxBuf[16]= b + 0x30;                //Set_UV_FB/1000 => 1000
                    TxBuf[17]= c + 0x30;                //Set_UV_FB/100 => 100
                    TxBuf[18]= d + 0x30;                //Set_UV_FB/10  => 10
                    TxBuf[19]= e + 0x30;                //Set_UV_FB%1  => 1
                break;
                
                case 5:
                    TxBuf[20]= b + 0x30;                //Set_OV_FB/1000 => 1000
                    TxBuf[21]= c + 0x30;                //Set_OV_FB/100 => 100
                    TxBuf[22]= d + 0x30;                //Set_OV_FB/10  => 10
                    TxBuf[23]= e + 0x30;                //Set_OV_FB%1  => 1
                break;
                
                case 6:
                    TxBuf[24]= b + 0x30;                //Set_BCL_FB/1000 => 1000
                    TxBuf[25]= c + 0x30;                //Set_BCL_FB/100 => 100
                    TxBuf[26]= d + 0x30;                //Set_BCL_FB/10  => 10
                    TxBuf[27]= e + 0x30;                //Set_BCL_FB%1  => 1
                break;
                               
                default:
                break;
                
            }
        break; 
     
        default:
        break;
    }

}


