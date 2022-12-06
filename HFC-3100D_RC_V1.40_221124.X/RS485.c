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

void Serial_Process_485(void);					//for 485 communication // 
void Response_Serial_485(void);                   // Set Data Response 
void Continuous_FB485(void);
void BittoByte485(void);
void Delay( unsigned int delay_count);
void Send_Serial_485(unsigned int ival,unsigned char pindex,unsigned char ch);

bool8 Alram_Sts485={0,0,0,0,0,0,0,0};
bool8 Warning_Sts485={0,0,0,0,0,0,0,0};
//-----------------------------------------------------------//
//            Recieve for RS-485 Data Processing             //
//-----------------------------------------------------------//
void Serial_Process_485(void)					//for 485 communication // 
{  
	//------------------------------------------------------------------------//
	//  0 1 2 3 4 5 
	//  [ U 1 S 1 ]
	//------------------------------------------------------------------------//    
    if(f_SData485 && f_EData485)
    {
        if(tmrtick2 >= RS485_Period)// Minimum Stand by time - 100ms delay
        {
            tmrtick2 = RS485_Period;
            Response_Serial_485();
        }
    }
}	

//-----------------------------------------------------------//
//            Recieve for RS-485 Data Response               //
//-----------------------------------------------------------//
void Response_Serial_485(void)                   // Set Data Response 
{
    switch(RxBuf485[1])                         //Unit Select
	{
        case 'U':
        {
            switch(RxBuf485[3])             
            { 
                case 'C':                                       //Continuous Data
                {
                    if(RxBuf485[2] == ID485 + 0x30)             //ID Select
                    {
                        tmrtick6 = 0;//comm.time out timer clear//
                        Continuous_FB485();                     
                    }
                }
                break;

                case 'S':                   //Set Data
                {
                    OP_MODE = RxBuf485[4] - 0x30;
                    OP_CMD = RxBuf485[5] - 0x30;
                    OP_Type = RxBuf485[6] - 0x30;
                    if(OP_Type == 1)// Load-Share Type              
                    {
                        SystemSet = System_ON;
                    }
                    else//Stand-Alone Type                        
                    {
                        if(OP_MODE == 0)  //Auto Mode
                        {
                            if(OP_CMD == ID485)
                            {
                                SystemSet = System_ON;
                            } 
                            else
                            {
                                SystemSet = System_OFF;
                            }
                        }                            
                        else if(OP_MODE == 1) //Manual Mode
                        {
                            if(OP_CMD == ID485)
                            {
                                SystemSet = System_ON;
                            } 
                            else
                            {
                                SystemSet = System_OFF;
                            }
                        }
                    }  
                }
                break;
                    
                default:
                break;
            }
        }
        break;
        
        default:
        break;   
    }
    //End Comm_Process, Clear flag //    
    f_SData485=0;                   //Start Flag
	f_EData485=0;                   //End Flag
    EOT_Flag485 = 0;
}

//============================================================================//
//                    Continuous Data Protocol Packet                         //
//  '<' + 'M'+ #(1~2) + 'C' + TxBuf[0~31](Vlot_Fnd[0~3] /            //
//  Amp_Val[4~7] / Temp_Val[8~11] /                                           //
//  Input_Volt_R[12~15] / Input_Volt_S[16~19] / Input_Volt_S[20~23]) /        //
//  STS[24] / SP1[25] / SP2[26] / SP2[27]) + Checksum[28~29] + '>'            // 
//============================================================================//
void Continuous_FB485(void)
{
    unsigned char i = 0;
    unsigned int Checksum = 0;
    int tmpvalue = 0;
    unsigned char hibyte=0, lobyte=0;
    unsigned char imsi;
    static char turnOrder = 0;
    //----------------------------------------------------------//
    //              Metering & Alram Data Transmit-KHJ          //
    //----------------------------------------------------------//  

    TxBuf485[0] = '<';                      //Start of Transmission Packet - SOT => 0x3C '<' 
    TxBuf485[1] = 'M';                      //Main Controller      => 0x4D 'M'
    TxBuf485[2] = ID485 + 0x30;             //Module ID data   => N(1~8)                
    TxBuf485[3] = 'C';                      //Data Type => Continuous 'C'

    ///////////////////////////////////////////
    //          Metering data(24byte)        //
    ///////////////////////////////////////////   
                            
    tmpvalue = VoltFnd_Avg*10;              //Volt_out_Bus              
    Send_Serial_485(tmpvalue,1,'C');

    tmpvalue = AmpFnd_Avg*10;               //Current_out
    Send_Serial_485(tmpvalue,2,'C');

    tmpvalue = Temp_Val*10;                 //Temperature
    Send_Serial_485(tmpvalue,3,'C');

    tmpvalue = Input_Volt_R * 10;           //Input_Volt_R
    Send_Serial_485(tmpvalue,4,'C');

    tmpvalue = Input_Volt_S * 10;           //Input_Volt_S
    Send_Serial_485(tmpvalue,5,'C');

    tmpvalue = Input_Volt_T * 10;           //Input_Volt_T
    Send_Serial_485(tmpvalue,6,'C');
    
    BittoByte485();                         //Status Conversion
    
    TxBuf485[28] = Tx_Char485;              //Alram Status Packet
    TxBuf485[29] = Tx_Char485_2;            //Warning Status Packet      
    TxBuf485[30] = CHG_Info;                //SP2
    TxBuf485[31] = '0';                     //SP3
    //========================================
    //Setting & Send RD, RC version//
    //========================================
    //if(ID485==1)//xxxx-00vv
    //{
        if(turnOrder == 1)
        {
            //imsi = 0x00;//xxxx-00vv            
            hibyte = (unsigned char)(RCVersion>>8); //
            //hibyte |= imsi; //
            lobyte = (unsigned char)(RCVersion & 0x00ff); //
            TxBuf485[30] |= hibyte;
            TxBuf485[31] = lobyte;
        }
        else
        {
            imsi = 0x08;//xxxx-10vv
            
            hibyte = (unsigned char)(RDVersion>>8); //
            hibyte |= imsi; //
            lobyte = (unsigned char)(RDVersion & 0x00ff); //
            TxBuf485[30] |= hibyte;
            TxBuf485[31] = lobyte;
        }
    //}
    turnOrder ^= 0x01;// toggle//
    //===============================================
    // Checksum
    for(i=0 ; i<32 ; i++)
    {
        Checksum ^= TxBuf485[i];
    }

    TxBuf485[32] = ((Checksum & 0xF0) >> 4) + 0x30;     //Checksum High nibble
    TxBuf485[33] = (Checksum & 0x0F) + 0x30;            //Checksum Low Nibble
    
    TxBuf485[34] = '>';                                 //End of Transmission Packet - EOT
    
    //RS-485 Module
    RS485_CTRL = 1;                     //Enable driver set

    for(i=0; i<35; i++)                 //Data Packet Send
    {
        U1TXREG = TxBuf485[i];
        while(!U1STAbits.TRMT);
    }

    while(!U1STAbits.TRMT);            //CTRL_Register set delay        
    RS485_CTRL = 0;                    //Enable driver set
    
}

//------------------------------------------------------------------------//
//                   Bit to Byte Convert for RS-485                       //
//  MSB ------------------------------------------------------------- LSB //
//  0    |     0    |     0    |        0       |  0   |  0  |  0   |  0  //
//  OP   | Input_OV | Input_UV | Input_Shutdown |  OV  | UV  |  OC  |  OT //
//------------------------------------------------------------------------//
void BittoByte485(void)
{
    //////////////////////////////////////////
    //          Alram data(1byte : 8bit)    //
    //////////////////////////////////////////
    Alram_Sts485.b7 = SystemState;             //Operation
    Alram_Sts485.b6 = Input_OV;//1;            //Input_OV
    Alram_Sts485.b5 = Input_UV;//1;            //Input_UV
    Alram_Sts485.b4 = Input_Shutdown;//1;      //Input_Shutdown
    Alram_Sts485.b3 = OverVoltageStatus;       //OV
    Alram_Sts485.b2 = UnderVoltageStatus;      //UV
    Alram_Sts485.b1 = OverCurrentStatus;       //OC
    Alram_Sts485.b0 = OverTemperatureStatus;   //OT
    
    //////////////////////////////////////////
    //      Alram Status Byte Package       //
    //////////////////////////////////////////
    Tx_Char485 = Alram_Sts485.b7;             
    Tx_Char485 = (Tx_Char485 << 1) | Alram_Sts485.b6;
    Tx_Char485 = (Tx_Char485 << 1) | Alram_Sts485.b5;
    Tx_Char485 = (Tx_Char485 << 1) | Alram_Sts485.b4;
    Tx_Char485 = (Tx_Char485 << 1) | Alram_Sts485.b3;
    Tx_Char485 = (Tx_Char485 << 1) | Alram_Sts485.b2;
    Tx_Char485 = (Tx_Char485 << 1) | Alram_Sts485.b1;
    Tx_Char485 = (Tx_Char485 << 1) | Alram_Sts485.b0;
    
    //////////////////////////////////////////
    //       Warning data(1byte : 8bit)     //
    //////////////////////////////////////////
    Warning_Sts485.b7 = Module_OV;
    Warning_Sts485.b6 = Module_UV;
    Warning_Sts485.b5 = 0;
    Warning_Sts485.b4 = 0;
    Warning_Sts485.b3 = 0;
    Warning_Sts485.b2 = 0;
    Warning_Sts485.b1 = 0;
    Warning_Sts485.b0 = 0;
    
        //////////////////////////////////////////
    //      Alram Status Byte Package       //
    //////////////////////////////////////////
    Tx_Char485_2 = Warning_Sts485.b7;             
    Tx_Char485_2 = (Tx_Char485_2 << 1) | Warning_Sts485.b6;
    Tx_Char485_2 = (Tx_Char485_2 << 1) | Warning_Sts485.b5;
    Tx_Char485_2 = (Tx_Char485_2 << 1) | Warning_Sts485.b4;
    Tx_Char485_2 = (Tx_Char485_2 << 1) | Warning_Sts485.b3;
    Tx_Char485_2 = (Tx_Char485_2 << 1) | Warning_Sts485.b2;
    Tx_Char485_2 = (Tx_Char485_2 << 1) | Warning_Sts485.b1;
    Tx_Char485_2 = (Tx_Char485_2 << 1) | Warning_Sts485.b0;
}

//                                              Module -> Module LCD Transmit-KHJ                                    //
//===================================================================================================================//
//                                       Continuous Data Protocol Packet                                             //
//  '<' + 'U'+ #(1~2) + 'C' + TxBuf[4~31](Vlotage[4~7] / Ampere[8~11]) / Temperature[8~11]             //   
//  Input_Volt_R[16~19] / Input_Volt_S[20~23] / Input_Volt_S[24~27]) / STS[28] / SP1[29] / SP2[30] / SP2[31] + '>'   //
//--------------------------------------------------------------------------------------------------------------------------------------------//
//                                        Response Data Protocol Packet                                                                       //
//============================================================================================================================================//
void Send_Serial_485(unsigned int ival,unsigned char pindex,unsigned char ch)
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
                    TxBuf485[4]= b + 0x30;                 //Volt_Out_BUS/1000 => 1000
                    TxBuf485[5]= c + 0x30;                 //Volt_Out_BUS/100 => 100
                    TxBuf485[6]= d + 0x30;                 //Volt_Out_BUS/10  => 10
                    TxBuf485[7]= e + 0x30;                 //Volt_Out_BUS%1  => 1
                break;
                        //
                case 2:
                    TxBuf485[8]= b + 0x30;                 //Currentt_Out/1000 => 1000
                    TxBuf485[9]= c + 0x30;                 //Currentt_Out/100 => 100
                    TxBuf485[10]= d + 0x30;                 //Currentt_Out/10  => 10
                    TxBuf485[11]= e + 0x30;                 //Currentt_Out%1  => 1
                break;
                
                case 3:
                    TxBuf485[12]= b + 0x30;                 //Temp/1000 => 1000
                    TxBuf485[13]= c + 0x30;                 //Temp/100 => 100
                    TxBuf485[14]= d + 0x30;                //Temp/10  => 10
                    TxBuf485[15]= e + 0x30;                //Temp%1  => 1
                break;
                
                case 4:
                    TxBuf485[16]= b + 0x30;                 //Input_Volt_R/1000 => 1000
                    TxBuf485[17]= c + 0x30;                 //Input_Volt_R/100 => 100
                    TxBuf485[18]= d + 0x30;                 //Input_Volt_R/10  => 10
                    TxBuf485[19]= e + 0x30;                 //Input_Volt_R%1  => 1
                break;
                
                case 5:
                    TxBuf485[20]= b + 0x30;                 //Input_Volt_S/1000 => 1000
                    TxBuf485[21]= c + 0x30;                 //Input_Volt_S/100 => 100
                    TxBuf485[22]= d + 0x30;                 //Input_Volt_S/10  => 10
                    TxBuf485[23]= e + 0x30;                 //Input_Volt_S%1  => 1
                break;
                
                case 6:
                    TxBuf485[24]= b + 0x30;                 //Input_Volt_T/1000 => 1000
                    TxBuf485[25]= c + 0x30;                 //Input_Volt_T/100 => 100
                    TxBuf485[26]= d + 0x30;                 //Input_Volt_T/10  => 10
                    TxBuf485[27]= e + 0x30;                 //Input_Volt_T%1  => 1
                break;

                default:
                break;
            }
        break; 
        
        
        default:
        break;
    }

}

