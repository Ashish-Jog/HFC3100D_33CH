/*
    Target device : dspic33ch128mp506
    mplabx ide Ver 6.0
    Compiler : XC16 V2.0
    Product PCB : HFC3100D-MC22-04-25 Rev2.0//
    support tools : pickit3, pickit4
    version 1.0 : 2022.08.17    
    //
 * ========================================================================
   [Special Feature]
  1. MCP121_300(voltage supervisor) populate(power on timer = 120ms delay) 
  2. Recovery Time('0' ~ '60'min) '0' min setting --> 10 second) - @MD board
     -> dspic30f6012a(setting value /2 ) ex) 1min -> 30second 
  3. MC <-> RC RS485commm. interval 500ms -> 1000ms
  4. MC -> RC SET Command(run/stop) send 2times 
  5. DSPIC33CH128MP506(dual core) - used only master core//
  ==========================================================================
  (22.08.29) - goooo
  110V/40A new mcu(dspic33ch128mp506)
 1.MainControl: 
        -Scada contact. adjust. (totalAlarm, AC FAil, Bat.low voltage) - main.c - getContact()//
        -TotalAlarm Led  :LED_M1_FAIL
        -AC_FAil Led     :LED_M2_FAIL
        -Battery voltage Low led     :LED_Bat_low
 2. DC125V or DC110V setting 
   --> typeDef.h  #define CHG125V  1 //1 is 125V, 0 is 110V
*/
#ifndef FCY
    #define FCY     (_XTAL_FREQ/2) //for _delay_ms() 
#endif
//
#include <libpic30.h>
#include <stdio.h>
#include "typeDef.h"
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
//
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/uart1.h"
#include "mcc_generated_files/uart2.h"
//
#include "funcData.h"
#include "Operation.h"
#include "main.h"
#include "i2c_master_eeprom_24fc256.h"

#define EEPROM_START_VALUE          0x55
#define EEPROM_START_ADDRESS        0

#define MCVERSION 110
unsigned int rc1Version=0;
unsigned int rc2Version=0;
unsigned int rd1Version=0;
unsigned int rd2Version=0;


unsigned int mainCnt=0;
unsigned int mainToggle=0;
unsigned char mainByte=0;
unsigned int mainValue[10]={0,};
//
//unsigned int fSkip=1;
//
//---------------
unsigned int sendCmd = 0;//OP_CMD;
unsigned int sendTimer = 0;
unsigned int fSendTwice = 0;
unsigned int normalCnt = 0;
//---------------
/*
                         Main application
 */
int main(void)
{    
    SYSTEM_Initialize();// initialize the device
    //
    EEPROMInit();       //I2C Init    
    //
    EEPROMSet();
    //---------------    
    TX_EN_SetLow();//rs485 tx disable    
    //----------------
    while (1)
    {
        if(tm0 > 1) //2ms second//
        {
            tm0 = 0;   
            //----------       
            if(OP_Type == 1)      // Dual Type - OP_Type(0:Stand-Alone, 1: Dual)
            {
                if(OP_MODE != 0 && OP_CMD != 0) //Op_Mode --> 0:Manual, 1:Auto
                {
                    OP_MODE = 0;
                    OP_CMD = 0;//id = 0????
                    //
                    Set_OK = 1;
                    //      
                    TX_EN_SetHigh();
                    tmr485Send = 0;
                    Request_Send();//rs1-Forcing Operation Command Send to Modules                    
                    TX_EN_SetLow();                    
                }
            }                
            else    // Stand-Alone Type                        
            {              
                OP_Logic();             // Operation Logic//~2ms period//                
            }
            Serial_Process();           // RS232 Processing
            //
            Serial_Process_485();       // RS485 Processing//~2ms period//      
        }//end if(tm0)
        //======================================================================
        if(tm1 >= 1000) //ms
        {
            tm1 = 0;
        //---------- 
            RST_Timer++;
        }
        //======================================================================
        if(tm2 >= 30)
        {
            tm2 = 0;
        //----------                        
            Adc_Sensing();          // ADC Processing            
            GetNfbState1();         // NFB Status Processing
            //GetNfbState2();         //  not used//NFB Status Processing
            GetContact();           // Contact Status Processing
        }
        //======================================================================
        if(tm3 >= 100)//100ms interval//
        {
            tm3 = 0;    //clear//
        //----------   
            sendTimer++;//clear when fSendTwice Set//
            if(fSendTwice==1)
            {
                if(sendTimer > 50)//100ms * 50 = 5000ms//
                {
                    fSendTwice=0;
                   // if(tm8>5000)Alarm_Sts[1].b7 = 0;
                   // if(tm9>5000)Alarm_Sts[2].b7 = 0;
                    //
                    if( 
                        (OP_CMD == 1 && Alarm_Sts[1].b7 == 0)  //ac fail// 
                       )       //module1 dc1 nfb on state//
                    {
                        Set_OK = 1;                 // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                        OP_CMD = 2;                 // Forcing Command Change Operation Command Module2
                        TX_EN_SetHigh();
                        tmr485Send = 0;
                        Request_Send();    //5      // Forcing Operation Command Send to Modules
                        TX_EN_SetLow();
                        Alarm_Sts[1].b7 = 0;        // Forcing OFF Status Change to Module1(Not real, Because, Transmission Delay)
                        Alarm_Sts[2].b7 = 1;        // Forcing ON Status Change to Module2(Not real, Because, Transmission Delay)
                        OP_CMD = 1;                 // Forcing Operation Command Module1 for Recovery Mode Memory 
                        if(RST_cnt>=4)
                        {
                            RST_cnt = 4;
                            Recovery_Flag = 0;
                            OP_CMD = 2;
                        }
                             
                    }//==============================================================
                    else if (
                        //(OP_CMD == 2 && Alarm_Sts[2].b4 != 0) || //ac fail// 
                        (OP_CMD == 2 && Alarm_Sts[2].b7 == 0 ) //|| //standby mode
                        //(OP_CMD == 2 && LED_Module2_NFB != 1) || //module1 nfb on state//
                        //(OP_CMD == 2 && LED_DC2_NFB != 1) ||
                        //(Alarm_Sts[2].b3 == 1)            || //#2_out high volt//
                        //(Alarm_Sts[2].b2 == 1)                             
                    ) 
                    {
                        Set_OK = 1;                 // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                        OP_CMD = 1;                 // Forcing Command Change Operation Command Module2
                        TX_EN_SetHigh();
                        tmr485Send = 0;
                        Request_Send();    //5      // Forcing Operation Command Send to Modules
                        TX_EN_SetLow();
                        Alarm_Sts[1].b7 = 0;        // Forcing OFF Status Change to Module1(Not real, Because, Transmission Delay)
                        Alarm_Sts[2].b7 = 1;        // Forcing ON Status Change to Module2(Not real, Because, Transmission Delay)
                        OP_CMD = 2;                 // Forcing Operation Command Module1 for Recovery Mode Memory 
                        if(RST_cnt>=4)
                        {
                            RST_cnt = 4;
                            Recovery_Flag = 0;
                            OP_CMD = 1;
                        }
                    }
                }
                
            }
        }
        
        if(tm4 >= 2000)
        {
            tm4 = 0;
        //----------
        }
        //======================================================================
        if(tm5 >= 300)
        {
            tm5 = 0;
        //----------
        #if(SerialDebug ==1)  
        //
        #endif
           mainByte++;
           if(mainByte>10)
           {
               mainByte=0;
               mainCnt++;               
           }
        }
        //======================================================================
        //Check RS485 communication timeout//
        if(tm8 >= 15000)//30second
        {
            tm8=15000;
        //----------
            Alarm_Sts[1].b7=0;
        }
        //======================================================================
        if(tm9 >= 15000)//30second
        {
            tm9=15000;
        //----------
            Alarm_Sts[2].b7=0;
        }
    }
    return 1; 
}
/**
 End of File(main())
*/
//****************************************************************************//
void EEPROMSet(void)
{
    if(EEPROMRead(EEPROM_START_ADDRESS) != 0x55)
    {
        EEPROMWrite(EEPROM_START_ADDRESS,EEPROM_START_VALUE);
        EEPROMWrite(EEPROM_START_ADDRESS+1,SYS_Type);        
    }
    else
    {
        SYS_Type = EEPROMRead(EEPROM_START_ADDRESS + 1);//
    }
}
void Serial_Process(void)
{
    // Fully Protocol Packet Data Receive Complete Start to END    
    if(f_SData && f_EData)
    {
        if(tmr232 >= RS232_RX_Delay)//RS232_RX_Delay is 100(ms)
        {
            Response_Serial();
        }  
    }
    if(f_SData_Debug && f_EData_Debug)
    {
        f_SData_Debug = 0;                // Start Character Flag Clear 
        f_EData_Debug = 0;                // End Character Flag Clear
        EOT_Flag_Debug = 0;
    }
}

void Serial_Process_485(void)
{       
    if(f_SData_485 && f_EData_485)
    {        
        Response_Serial_485();//get metering data//~518ms period//        
    }
    //else //Control or Data Request Command Send//2~4ms period//
    //{
    if(Set_OK == 1)
    {
        tmr485=0;
        tmr485Send = 0;
        Request_Send();                  // Set Command Data Sent to Modules
        if(OP_CMD == 1)                  // Operation CMD : => Module1 ON
        {
            Alarm_Sts[1].b7 = 1;         // Forcing ON Status Change to Module1(Not real, Because, Transmission Delay)
            Alarm_Sts[2].b7 = 0;         // Forcing OFF Status Change to Module2(Not real,Because, Transmission Delay)
        }
        else if(OP_CMD == 2)
        {
            Alarm_Sts[1].b7 = 0;        // Forcing OFF Status Change to Module1(Not real, Because, Transmission Delay)
            Alarm_Sts[2].b7 = 1;        // Forcing ON Status Change to Module2(Not real, Because, Transmission Delay)
        }
        RST_cnt = 0;                    // Restart Count Clear
        Recovery_Flag=0;
    }
    else
    {   
        if(tmr485 >= RS485_Period)//1000ms interval//
        {
            tmr485 = 0;    
            if(ID_485 == 1)
            {
                ID_485 = 2;
            }//Set ID 485 : Module2
            else if(ID_485 == 2)
            {
                ID_485 = 1;
            }//Set ID 485 : Module1                
            Continuous_Send(); //Continuous Data Send to Module1(with Module2)            
            tmr485Send = 0;//restart timer req. metering data//
        }
    }
        //----------------------------------------------------------------------
    //}
}


//                     Module <-> Module LCD-KHJ                             //
//============================================================================//
//                    Request Data Protocol Packet                            //
// '[' + 'D'+ '0' + 'S' + TxBuf[0~19](Set_ARef[0~3] / Set_CL[4~7] /           // 
//  Set_VRef[8~11] / Set_UV[12~15] / Set_OV[16~19] / Set_OT[20~23]) + ']'     //
//----------------------------------------------------------------------------// 
//                   Response Data Protocol Packet                            //
// '<' + 'U'+ '0' + 'R' + TxBuf[0~19](Set_ARef[0~3] / Set_CL[4~7] /  //
// Set_VRef[8~11] / Set_UV[12~15] / Set_OV[16~19] / Set_OT[20~23]) + '>'      //
//============================================================================//
// Data Response Select
//rs232 to Maindisplay//
void Response_Serial(void)
{
	switch(RxBuf[1])                            //Unit Select
	{
        case 'D':                               // Display
            switch(RxBuf[2])                    //Module ID Select          
            {                
                ////////////////////////////////////////////////////////////////
                //------------------------- Module ---------------------------//               
                //////////////////////////////////////////////////////////////// 
                case '0':                       //Module 1
                    switch(RxBuf[3])            //Ctrl Code Select
                    {
                        case 'S':               //Setting Data 
                        Response_Data();        //Module Response Data Recieve
                        Response_FB();          //Module Response Data Feedback Send
                        Set_OK = 1;             //Set, Change Bus Stop Condition of RS-485 before to Send Response Feedback
                        break;                  //Setting Data Set Start END
                        
                        case 'C':               //Continuous Data       9600bps = ~29.16ms / 115200 = ~2.43ms
                        Continuous_FB();        //Module Continuous Data Recieve
                        Set_OK = 0;
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
  	f_SData = 0;                // Start Character Flag Clear 
    f_EData = 0;                // End Character Flag Clear
    EOT_Flag = 0;
}

//                     Main Controller <-> Module - KHJ                       //
//============================================================================//
//                       Request Data Protocol Packet                         //
//   '[' + 'U'+ '1'/'2' + 'C' + Spare1[4] + Spare2[5] + Checksum[6~7] + ']'   //
// -------------------------------------------------------------------------- //                                                                                                                                                 //
//                      Response Data Protocol Packet                         //
// '<' + 'M'+ '1'/'2' + 'C' + TxBuf[4~31](Vlotage[4~7] / Ampere[8~11])//
//  / Temperature[12~15] / Input_Volt_R[16~19] / Input_Volt_S[20~23] /        //
//  Input_Volt_S[24~27]) / STS[28] / SP1[29] / SP2[30] / SP2[31]) + '>'       //
//============================================================================//

void Response_Serial_485(void)
{
	switch(RxBuf_485[1])                            //Unit Select
	{
        case 'M':
            switch(RxBuf_485[2])                    //Module ID Select          
            {                
                case '1':                           //Module #1
                    switch(RxBuf_485[3])            //Ctrl Code Select
                    {
                        case 'C':
                            tm8 = 0;                //clear time out timer//
                            Receive_Data_485(1);    //Continues Data Recieve
                        break;
                      
                        default:
                        break;
                    }                               
                break;                              
                //---------------------------------------------
                case '2':                           //Module #2
                    switch(RxBuf_485[3])            //Ctrl Code Select
                    {
                        case 'C':
                            tm9=0;                  //clear timeOut timer//
                            Receive_Data_485(2);    //LCD Continues Data Recieve
                            break;
                      
                        default:
                        break;
                    }                               
                break;                              
                //
                default:
                break;
            }
        break;
        
        default:
        break;
	}
  	f_SData_485 = 0;                                // Start Character Flag Clear 
    f_EData_485 = 0;                                // End Character Flag Clear
    EOT_Flag485 = 0;
    tmr485Send = 0;
}

//=======//
// rs485 //
//=======//
void Continuous_Send(void)
{   
    unsigned char i = 0;
    unsigned int Checksum = 0;
    //
    TxBuf_485[0] = '[';                 //Start of Transmission Packet - SOT => 0x5B '[' 
    TxBuf_485[1] = 'U';                 //Display Unit => 0x55 'U'
    TxBuf_485[2] = ID_485 + 0x30;       //CODE
    TxBuf_485[3] = 'C';                 //Data Type => Continuous => 0x43 'C'
    TxBuf_485[4] = '0';                 //Spare
    TxBuf_485[5] = '0';                 //Spare
    TxBuf_485[6] = '0';                 //Spare    
    //
    for(i=0 ; i<7 ; i++)
    {
        Checksum ^= TxBuf_485[i];
    }
    TxBuf_485[7] = ((Checksum & 0xF0) >> 4) + 0x30;        //Checksum High nibble
    TxBuf_485[8] = (Checksum & 0x0F) + 0x30;               //Checksum Low nibble
    TxBuf_485[9] =']';                  //End of Transmission Packet - EOT => 0x5D ']'
#if serialDebug
    //
#endif
    //--------------------------------------------------------------------------
    TX_EN_SetHigh();
        for(i=0 ; i<10 ; i++)
        {       
            UART2_Write(TxBuf_485[i]);
        }
        while(UART2_IsTxDone() != 1){};
    TX_EN_SetLow();
    //tmr485Send=0;//start counter 140ms//
}   

void Request_Send(void)
{   
    
    unsigned char i = 0;
    unsigned int Checksum = 0;
    
    TxBuf_485[0] = '[';                 //Start of Transmission Packet - SOT => 0x5B '[' 
    TxBuf_485[1] = 'U';                 //Display Unit => 0x55 'U'
    TxBuf_485[2] = '0';                 //CODE
    TxBuf_485[3] = 'S';                 //Setting => 0x53 'S'
                                        //-------------------        
    TxBuf_485[4] = OP_MODE + 0x30;      //0x30 '0' : Manual 
                                        //0x31 '1' : Auto
                                        //------------------
    TxBuf_485[5] = OP_CMD + 0x30;       //0x31 '1' : Module#1 ON, Module#2 OFF 
                                        //0x32 '2' :Module#1 OFF, Module#2 ON
                                        //------------------
    TxBuf_485[6] = OP_Type + 0x30;      //0x30 '0' : Stand-alone 
    //                                  //0x31 '1' : Load-sharing    
    for(i=0 ; i<7 ; i++)
    {
        Checksum ^= TxBuf_485[i];
    }
    //
    TxBuf_485[7] = ((Checksum & 0xF0) >> 4) + 0x30;        //Checksum High nibble
    TxBuf_485[8] = (Checksum & 0x0F) + 0x30;               //Checksum Low nibble
    TxBuf_485[9] =']';                  //End of Trans. Packet - EOT => 0x5D ']'    
    //
    //------------------------
    while(tmr485Send<500){};//request metering data and then start tmr485Send timer// 
    tmr485Send=0;
    //------------------------
    TX_EN_SetHigh();        
//        for(i=0 ; i<10 ; i++)
//        {       
//            UART2_Write(TxBuf_485[i]);
//        }
//        while(UART2_IsTxDone() != 1){};
        //-------------------------------
        __delay_ms(5);
        for(i=0 ; i<10 ; i++)
        {       
            UART2_Write(TxBuf_485[i]);
        }
        while(UART2_IsTxDone() != 1){};
         __delay_ms(5);
        for(i=0 ; i<10 ; i++)
        {       
            UART2_Write(TxBuf_485[i]);
        }
        while(UART2_IsTxDone() != 1){};
    TX_EN_SetLow();
    //
    Set_OK = 0;
    tmr485= 0;
}//End Request_Send()

void Set_Send(void)
{   
    
    unsigned char i = 0;
    unsigned int Checksum = 0;
    
    TxBuf_485[0] = '[';                 //Start of Transmission Packet - SOT => 0x5B '[' 
    TxBuf_485[1] = 'U';                 //Display Unit => 0x55 'U'
    TxBuf_485[2] = '0';                 //CODE
    TxBuf_485[3] = 'S';                 //Setting => 0x53 'S'
                                        //-------------------        
    TxBuf_485[4] = OP_MODE + 0x30;      //0x30 '0' : Manual 
                                        //0x31 '1' : Auto
                                        //------------------
    TxBuf_485[5] = sendCmd + 0x30;       //0x31 '1' : Module#1 ON, Module#2 OFF 
                                        //0x32 '2' :Module#1 OFF, Module#2 ON
                                        //------------------
    TxBuf_485[6] = OP_Type + 0x30;      //0x30 '0' : Stand-alone 
    //                                  //0x31 '1' : Load-sharing    
    for(i=0 ; i<7 ; i++)
    {
        Checksum ^= TxBuf_485[i];
    }
    //
    TxBuf_485[7] = ((Checksum & 0xF0) >> 4) + 0x30;        //Checksum High nibble
    TxBuf_485[8] = (Checksum & 0x0F) + 0x30;               //Checksum Low nibble
    TxBuf_485[9] =']';                  //End of Trans. Packet - EOT => 0x5D ']'    
    //
    //------------------------
    while(tmr485Send<500){};//request metering data and then start tmr485Send timer// 
    tmr485Send=0;
    //------------------------
    TX_EN_SetHigh();        
//        for(i=0 ; i<10 ; i++)
//        {       
//            UART2_Write(TxBuf_485[i]);
//        }
//        while(UART2_IsTxDone() != 1){};
        //-------------------------------
        __delay_ms(5);
        for(i=0 ; i<10 ; i++)
        {       
            UART2_Write(TxBuf_485[i]);
        }
        while(UART2_IsTxDone() != 1){};
       
    TX_EN_SetLow();
    //
    Set_OK = 0;
    tmr485= 0;
}//End Set_Send()


void Receive_Data_485(UINT8 ID)
{
    int tmpvalue = 0;    
    unsigned char a = 0, b = 0, c = 0, d = 0;        
    // Voltage Output
    a = RxBuf_485[4];
    b = RxBuf_485[5];
    c = RxBuf_485[6];
    d = RxBuf_485[7];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    Voltage_Out[ID] = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
    // Current Output
    a = RxBuf_485[8];
    b = RxBuf_485[9];
    c = RxBuf_485[10];
    d = RxBuf_485[11];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Current_Out[ID] = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }
    }
    // Temperature
    a = RxBuf_485[12];
    b = RxBuf_485[13];
    c = RxBuf_485[14];
    d = RxBuf_485[15];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Temperature[ID] = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }  
    // Input Voltage AC_R
    a = RxBuf_485[16];
    b = RxBuf_485[17];
    c = RxBuf_485[18];
    d = RxBuf_485[19];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    Input_Volt_R[ID] = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
    // Input Voltage AC_S
    a = RxBuf_485[20];
    b = RxBuf_485[21];
    c = RxBuf_485[22];
    d = RxBuf_485[23];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Input_Volt_S[ID] = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }
    }
    // Input Voltage AC_T
    a = RxBuf_485[24];
    b = RxBuf_485[25];
    c = RxBuf_485[26];
    d = RxBuf_485[27];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    Input_Volt_T[ID] = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }
    // Input Status
    Rx_Char[ID]   = RxBuf_485[28];// && 0xDf;//0x 1101-1111//
    Alarm_Sts[ID].b0 =  Rx_Char[ID] & 0x01;         //over temp
    Alarm_Sts[ID].b1 = (Rx_Char[ID] & 0x02) >> 1;   //over load
    Alarm_Sts[ID].b2 = (Rx_Char[ID] & 0x04) >> 2;   //low out volt 
    Alarm_Sts[ID].b3 = (Rx_Char[ID] & 0x08) >> 3;   //over out volt
    Alarm_Sts[ID].b4 = (Rx_Char[ID] & 0x10) >> 4;   //ac volt fail
    Alarm_Sts[ID].b5 = (Rx_Char[ID] & 0x20) >> 5;   //ac volt low
    Alarm_Sts[ID].b6 = (Rx_Char[ID] & 0x40) >> 6;   //ac volt high
    Alarm_Sts[ID].b7 = (Rx_Char[ID] & 0x80) >> 7;   //run/stop    
    //
    Rx_Char2[ID] = RxBuf_485[29];
    Warning_Sts[ID].b0 =  Rx_Char2[ID] & 0x01;
    Warning_Sts[ID].b1 = (Rx_Char2[ID] & 0x02) >> 1;
    Warning_Sts[ID].b2 = (Rx_Char2[ID] & 0x04) >> 2;
    Warning_Sts[ID].b3 = (Rx_Char2[ID] & 0x08) >> 3;
    Warning_Sts[ID].b4 = (Rx_Char2[ID] & 0x10) >> 4;
    Warning_Sts[ID].b5 = (Rx_Char2[ID] & 0x20) >> 5;
    Warning_Sts[ID].b6 = (Rx_Char2[ID] & 0x40) >> 6;
    Warning_Sts[ID].b7 = (Rx_Char2[ID] & 0x80) >> 7;    
    //
    Rx_Char3[ID] = RxBuf_485[30];
    CHG_Info[ID].b0 = Rx_Char3[ID] & 0x01;
    CHG_Info[ID].b1 = (Rx_Char3[ID] & 0x02) >> 1;
    CHG_Info[ID].b2 = (Rx_Char3[ID] & 0x04) >> 2;
    CHG_Info[ID].b3 = (Rx_Char3[ID] & 0x08) >> 3;
    //----------------------------------------------
    CHG_Info[ID].b4 = (Rx_Char3[ID] & 0x10) >> 4;
    CHG_Info[ID].b5 = (Rx_Char3[ID] & 0x20) >> 5;
    CHG_Info[ID].b6 = (Rx_Char3[ID] & 0x40) >> 6;
    CHG_Info[ID].b7 = (Rx_Char3[ID] & 0x80) >> 7;
    //
    Rx_Spare1[ID] = RxBuf_485[31];   
    //==============================================
    // rc --(rs485)--> mc receive rc,rd version//
    a = Rx_Char3[ID];  //high byte
    b = Rx_Spare1[ID]; //lowbyte
    if(ID == 0x01) //[30]xxxx-
    {
        if( (Rx_Char3[ID] & 0x0C) == 0x10 ) //0000-1100 RD//
            rd1Version = ( (a & 0x03)<<8 | b );
        else rc1Version = ( (a & 0x03)<<8 | b );
    }
    else if(ID == 0x02) //[30]xxxx-
    {
        if( (Rx_Char3[ID] & 0x0C) == 0x10 ) //0000-1100 RD//
            rd2Version = ( (a & 0x03)<<8 | b );
        else rc2Version = ( (a & 0x03)<<8 | b );
    }
    //----------------------------------------------------
}

void Response_Data(void)
{
    int tmpvalue = 0;
    unsigned char a = 0, b = 0, c = 0, d = 0;
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
                    Set_Batt_OV = tmpvalue * 0.1;  //  xxxx -> xxx.x
                    if(Set_Batt_OV > Batt_UV_Ref && Set_Batt_OV <= Batt_OV_Ref)
                    {
                        Set_Batt_OV_B = Set_Batt_OV;
                    }
                    else 
                    {
                        Set_Batt_OV = Set_Batt_OV_B;
                    }
                }//End if(d >= '0' && d <= '9')
            }//End if(c >= '0' && c <= '9')
        }//End if(b >= '0' && b <= '9')		
    }//End if(a >= '0' && a <= '9')                               
    //--------------------------------//
    // Current Limit Setting Feedback //
    //--------------------------------//
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
                    Set_Batt_UV = tmpvalue * 0.1;    //  xxxx -> xxx.x
                    if(Set_Batt_UV >= Batt_UV_Ref && Set_Batt_UV < Batt_OV_Ref)
                    {
                        Set_Batt_UV_B = Set_Batt_UV;
                    }
                    else 
                    {
                        Set_Batt_UV = Set_Batt_UV_B;
                    }
                }
            }
        }
    }
    //----------//
    // RST_TIME //
    //----------//
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
                    RST_TIME = tmpvalue * 0.1;      //  xxxx -> xxx.x
                    if(RST_TIME >= RST_Time_Min && RST_TIME <= RST_Time_Max)
                    {
                        RST_Time_B = RST_TIME;
                    }
                    else 
                    {
                        RST_TIME = RST_Time_B;
                    }
                }
            }
        }		
    }
    //Spare2
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
                    Spare2 = tmpvalue * 0.1;      //xxxx -> xxx.x
                }
            }
        }		
    }
    //Spare3
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
                    Spare3 = tmpvalue * 0.1;        //xxxx -> xxx.x
                }
            }
        }		
    }
    //Spare4
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
                    Spare4 = tmpvalue * 0.1;        //xxxx -> xxx.x
                }
            }
        }
    }   
    // Input Status
    OP_MODE = RxBuf[28] - 0x30;
    OP_CMD = RxBuf[29] - 0x30;
    OP_Type = RxBuf[30] - 0x30;
    SYS_Type = RxBuf[31] - 0x30;
    Mode_Check();
}

// RS-232 Module
// Set Data Feedback TX
void Response_FB(void)
{
    unsigned char i = 0;
    unsigned int Checksum = 0;
    int tmpvalue = 0;
    //
    TxBuf[0] = '<';                     //Start of Transmission Packet - SOT => 0x3C '<' 
    TxBuf[1] = 'U';                     //Chrger Unit      => 0x55 'U'
    TxBuf[2] = '0';						//Module ID data   => 0x30 '0'                
    TxBuf[3] = 'R';                     //Data Type => Continues 'R'
    //
    //TxBuf[4]~[27]
    tmpvalue = Set_Batt_OV * 10;
    Send_Serial(tmpvalue,1,'R');
    tmpvalue = Set_Batt_UV * 10;
    Send_Serial(tmpvalue,2,'R');
    tmpvalue = RST_TIME * 10;
    Send_Serial(tmpvalue,3,'R');
    tmpvalue = Spare2 * 10;
    Send_Serial(tmpvalue,4,'R');
    tmpvalue = Spare3 * 10;
    Send_Serial(tmpvalue,5,'R');
    tmpvalue = Spare4 * 10;
    Send_Serial(tmpvalue,6,'R');    
    //Status TxBuf[28]~[31]
    TxBuf[28] = OP_MODE + 0x30;
    TxBuf[29] = OP_CMD + 0x30;
    TxBuf[30] = OP_Type + 0x30;
    TxBuf[31] = SYS_Type + 0x30;
    //Checksum TxBuf[32]~[33]
    for(i=0 ; i<32 ; i++)
    {
        Checksum ^= TxBuf[i];
    }
    TxBuf[32] = ((Checksum & 0xF0) >> 4) + 0x30;    //Checksum High nibble
    TxBuf[33] = (Checksum & 0x0F) + 0x30;           //Checksum Low nibble    
    TxBuf[34] = '>';                                //End of Trans. Packet - EOT
    //
    for(i=0 ; i<35 ; i++)
    {       
        UART1_Write(TxBuf[i]);
        while(!U1STAbits.TRMT); 
    }
}

//============================================================================//
// Continuous Data Protocol Packet                                            //
// '<' + 'U'+ '0' + 'C' + TxBuf[0~6](Vlotage[0~3] / Ampere[4~7]) + '>'        // 
//============================================================================//
// RS-232 Module
// Continuous Data Feedback TX
void Continuous_FB(void)
{
    unsigned char i = 0;
    unsigned int Checksum = 0;
    int tmpvalue = 0;
    //unsigned char tmpbyte=0;
    //unsigned char min=0, sec=0;
    //----------------------------------------------------------//
    //              Metering & Alarm Data Transmit-KHJ          //
    //----------------------------------------------------------//                
    TxBuf[0] = '<';                     //Start of Transmission Packet - SOT => 0x3C '<' 
    TxBuf[1] = 'U';                     //Charger Unit      => 0x55 'U'
    TxBuf[2] = '0';						//Module ID data   => 0x30 '0'                
    TxBuf[3] = 'C';                     //Data Type => Continues 'C'
    //TxBuf[4]~[19]        
    tmpvalue = Batt_Volt * 10;
    Send_Serial(tmpvalue,1,'C');
    tmpvalue = Batt_Amp * 10;
    Send_Serial(tmpvalue,2,'C');
    tmpvalue = Load_Volt * 10;
    Send_Serial(tmpvalue,3,'C');
    tmpvalue = Load_Amp * 10;
    Send_Serial(tmpvalue,4,'C');            
    //TxBuf[20]~[31]
    BittoByte();
    for(i=0; i<12 ; i++)
    {
        TxBuf[i+20] = Tx_Char[i];//TxBuf[20....31]
    }
    //TxBuf[21].bit6 dcgnd//
    //-------------------------------------------
//    if(Recovery_Flag==1)
//    {//min/sec
//        tmpbyte = RST_cnt;
//        if(tmpbyte<=3)
//        {
//            tmpbyte = tmpbyte & 0x03; //0000-0011
//            //tmpbyte--;//1->0 ,2->1,
//        }
//        tmpbyte = tmpbyte<<6;
//        //-------------------------------------
//        tmpvalue = RST_Timer;//*60;//10min * 60 = 600//
//        min = (unsigned char)(tmpvalue/60);//min
//        //min = 59; //0011-1011
//        //sec = (unsigned char)((tmpvalue-(min*60))%60);
//        sec = (unsigned char)(tmpvalue%60);
//        //sec = 59;
//        TxBuf[26] = tmpbyte | min; //min 0000-0111
//        
//        //TxBuf[26] = 0x00 | 0x60;
//        
//        if(sec <= 59)//sec
//            TxBuf[27]=sec;
//        else
//            TxBuf[27]=0;
//        //TxBuf[27] = 30;
//    }
//    else
//    {
//        TxBuf[26]=0;
//        TxBuf[27]=0;
//    }
    
    //Checksum[32]~[33]
    for(i=0 ; i<32 ; i++)
    {
        Checksum ^= TxBuf[i];
    }
    TxBuf[32] = ((Checksum & 0xF0) >> 4) + 0x30;//Checksum High nibble
    TxBuf[33] = (Checksum & 0x0F) + 0x30;       //Checksum Low nibble
    TxBuf[34] = '>';                            //End of Trans. Packet - EOT
    //
    for(i=0 ; i<35 ; i++)
    {       
        UART1_Write(TxBuf[i]);
        while(!U1STAbits.TRMT); 
    }    
}

//                    Module -> Module LCD Transmit-KHJ                       //
//============================================================================//
//                    Continuous Data Protocol Packet                         //
//  '<' + 'U'+ '0' + 'C' + TxBuf[0~7](Vlotage[0~3] / Ampere[4~7]) /  //
//  Temperature[8~11] / Input_Volt_R[12~15] / Input_Volt_S[16~19] /           //
//  Input_Volt_S[20~23]) / STS[24] / SP1[25] / SP2[26] / SP2[27] + '>'        //
//----------------------------------------------------------------------------//
//                    Response Data Protocol Packet                           //
// '<' + 'U'+ '0' + 'R' + TxBuf[4~27](Batt_OV[4~7] / Batt_UV[8~11]   //
//  / RST_TIME[12~15] / Spare2[16~19] / Spare3[20~23] / Spare4[24~27])        //
//  + OP_MODE + OP_CMD + '>'                                                  //
//============================================================================//
void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch)
{
	unsigned int tmpval = 0;
	unsigned char a = 0, b = 0, c = 0, d = 0, e = 0;
	//==================================================================// 
	//TxBuf[0~7] ===> 'C' [0~3] / [4~7]									//
	//TxBuf[0~19] ===> 'R' [1~3] / [4~7] / [8~11] / [12~15] / [16~19]	//
	//==================================================================//
	tmpval = ival;
	a=__builtin_divud((long)tmpval,10000);
	b=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000)) ),1000);
	c=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000)) ),100);
	d=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000) + __builtin_muluu(c,100)) ),10);
	e= tmpval%10;
	
    switch(ch)		//KHJ - RS-232 & RS-485
    {
        case 'R':		//Tx Type - Response
            switch(pindex)
            {
                case 1:
                    TxBuf[4]= b + 0x30;                 //Batt_OV/1000 => 1000
                    TxBuf[5]= c + 0x30;                 //Batt_OV/100 => 100
                    TxBuf[6]= d + 0x30;                 //Batt_OV/10  => 10
                    TxBuf[7]= e + 0x30;                 //Batt_OV%1  => 1
                break;
                
                case 2:
                    TxBuf[8]= b + 0x30;                 //Batt_UV/1000 => 1000
                    TxBuf[9]= c + 0x30;                 //Batt_UV/100 => 100
                    TxBuf[10]= d + 0x30;                //Batt_UV/10  => 10
                    TxBuf[11]= e + 0x30;                //Batt_UV%1  => 1
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                //RST_TIME/1000 => 1000
                    TxBuf[13]= c + 0x30;                //RST_TIME/100 => 100
                    TxBuf[14]= d + 0x30;                //RST_TIME/10  => 10
                    TxBuf[15]= e + 0x30;                //RST_TIME%1  => 1
                break;
                
                case 4:
                    TxBuf[16]= b + 0x30;                //Spare2/1000 => 1000
                    TxBuf[17]= c + 0x30;                //Spare2/100 => 100
                    TxBuf[18]= d + 0x30;                //Spare2/10  => 10
                    TxBuf[19]= e + 0x30;                //Spare2%1  => 1
                break;
                
                case 5:
                    TxBuf[20]= b + 0x30;                //Spare3/1000 => 1000
                    TxBuf[21]= c + 0x30;                //Spare3/100 => 100
                    TxBuf[22]= d + 0x30;                //Spare3/10  => 10
                    TxBuf[23]= e + 0x30;                //Spare3%1  => 1
                break;
                
                case 6:
                    TxBuf[24]= b + 0x30;                //Spare4/1000 => 1000
                    TxBuf[25]= c + 0x30;                //Spare4/100 => 100
                    TxBuf[26]= d + 0x30;                //Spare4/10  => 10
                    TxBuf[27]= e + 0x30;                //Spare4%1  => 1
                break;
                                              
                default:
                break;
            }
        break;   
        
        case 'C':		//Tx Type - Continuous
            switch(pindex)
            {
                case 1:
                    TxBuf[4]= b + 0x30;                 //Batt_Volt/1000 => 1000
                    TxBuf[5]= c + 0x30;                 //Batt_Volt/100 => 100
                    TxBuf[6]= d + 0x30;                 //Batt_Volt/10  => 10
                    TxBuf[7]= e + 0x30;                 //Batt_Volt%1  => 1
                break;
                
                case 2:
                    TxBuf[8]= b + 0x30;                 //Batt_Amp/1000 => 1000
                    TxBuf[9]= c + 0x30;                 //Batt_Amp/100 => 100
                    TxBuf[10]= d + 0x30;                //Batt_Amp/10  => 10
                    TxBuf[11]= e + 0x30;                //Batt_Amp%1  => 1
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                //Load_Volt/1000 => 1000
                    TxBuf[13]= c + 0x30;                //Load_Volt/100 => 100
                    TxBuf[14]= d + 0x30;                //Load_Volt/10  => 10
                    TxBuf[15]= e + 0x30;                //Load_Volt%1  => 1
                break;
                
                case 4:
                    TxBuf[16]= b + 0x30;                //Load_Amp/1000 => 1000
                    TxBuf[17]= c + 0x30;                //Load_Amp/100 => 100
                    TxBuf[18]= d + 0x30;                //Load_Amp/10  => 10
                    TxBuf[19]= e + 0x30;                //Load_Amp%1  => 1
                break;
                                              
                default:
                break;
            }
        break;   
     
        default:
        break;
    }

}

// Converting Data Type Byte to Bit
void BittoByte(void)
{
    unsigned char hibyte=0;
    unsigned char lobyte=0;
    //unsigned int iTmpVal = 0;
    static unsigned int iSendTurn = 0;
	//////////////////////////////////////////
    //    LED Status1 data(1byte : 8bit)    //
    //////////////////////////////////////////
    LED_Module1_NFB = CheckBit(bNfbState1,7);
	LED_Module2_NFB = CheckBit(bNfbState1,6);
//	LED_Module3_NFB = CheckBit(bNfbState1,5);
//	LED_Module4_NFB = CheckBit(bNfbState1,4);
	LED_DC1_NFB = CheckBit(bNfbState1,5);
	LED_DC2_NFB = CheckBit(bNfbState1,4);
	LED_DC3_NFB = CheckBit(bNfbState1,1);//dc gnd rc9 1 is dc ground error//
//	LED_DC4_NFB = CheckBit(bNfbState2,7);
    
    LED_Sts1.b7 = LED_Module1_NFB;      //Module1_NFB
    LED_Sts1.b6 = LED_Module2_NFB;      //Module2_NFB
    LED_Sts1.b5 = LED_Module3_NFB;      //Module3_NFB
    LED_Sts1.b4 = LED_Module4_NFB;      //Module4_NFB
    LED_Sts1.b3 = LED_DC1_NFB;          //DC1_NFB
    LED_Sts1.b2 = LED_DC2_NFB;          //DC2_NFB
    LED_Sts1.b1 = LED_DC3_NFB;          //DC3_NFB-> dc gnd rc9//
    LED_Sts1.b0 = LED_DC4_NFB;          //DC4_NFB
    
    //////////////////////////////////////////
    //    LED Status2 data(1byte : 8bit)    //
    //////////////////////////////////////////
    LED_Battery_NFB = CheckBit(bNfbState1,3);
	//LED_Spare1_NFB = CheckBit(bNfbState2,5);
//	LED_Spare2_NFB = CheckBit(bNfbState2,4);
	LED_Load_NFB = CheckBit(bNfbState1,2);
//	LED_Spare3_NFB = CheckBit(bNfbState2,2);
//	LED_Spare4_NFB = CheckBit(bNfbState2,1);

    LED_Sts2.b7 = LED_Battery_NFB;      //Battery_NFB
    LED_Sts2.b6 = LED_Spare1_NFB;       //Spare1_NFB
    LED_Sts2.b5 = LED_Spare2_NFB;       //Spare2_NFB
    LED_Sts2.b4 = LED_Load_NFB;         //Load_NFB
    LED_Sts2.b3 = LED_Spare3_NFB;       //Spare3_NFB
    LED_Sts2.b2 = LED_Spare4_NFB;       //Spare4_NFB //dc Fgnd//
    LED_Sts2.b1 = LED_Spare5_NFB;       //Spare5_NFB
    LED_Sts2.b0 = LED_Spare6_NFB;       //Spare6_NFB
    
    //////////////////////////////////////////
    //    LED Status3 data(1byte : 8bit)    //
    //////////////////////////////////////////
    LED_Sts3.b7 = 0;        //Battery_NFB
    LED_Sts3.b6 = 0;        //Spare1_NFB
    LED_Sts3.b5 = 0;        //Spare2_NFB
    LED_Sts3.b4 = 0;        //Load_NFB
    LED_Sts3.b3 = 0;        //Spare3_NFB
    LED_Sts3.b2 = 0;        //Spare4_NFB
    LED_Sts3.b1 = 0;        //Spare5_NFB
    LED_Sts3.b0 = 0;        //Spare6_NFB
    
    //////////////////////////////////////////
    //    LED Status4 data(1byte : 8bit)    //
    //////////////////////////////////////////
    LED_Sts4.b7 = 0;        //Battery_NFB
    LED_Sts4.b6 = 0;        //Spare1_NFB
    LED_Sts4.b5 = 0;        //Spare2_NFB
    LED_Sts4.b4 = 0;        //Load_NFB
    LED_Sts4.b3 = 0;        //Spare3_NFB
    LED_Sts4.b2 = 0;        //Spare4_NFB
    LED_Sts4.b1 = 0;        //Spare5_NFB
    LED_Sts4.b0 = 0;        //Spare6_NFB
    
    //////////////////////////////////////////
    //   Module #1 Status1(1byte : 8bit)    //
    //////////////////////////////////////////
    SystemState_STS[1] = Alarm_Sts[1].b7;
    Input_OV_STS[1] = Alarm_Sts[1].b6;
    Input_UV_STS[1] = Alarm_Sts[1].b5;//AC under volt alarm//book
    Input_SD_STS[1] = Alarm_Sts[1].b4;
    Output_OV_STS[1] = Alarm_Sts[1].b3;
    Output_UV_STS[1] = Alarm_Sts[1].b2;
    Output_OC_STS[1] = Alarm_Sts[1].b1;
    Output_OT_STS[1] = Alarm_Sts[1].b0;
    //
    Module1_Sts.b7 = SystemState_STS[1];          //Operation
    Module1_Sts.b6 = Input_OV_STS[1];             //Input_OV
    Module1_Sts.b5 = Input_UV_STS[1];             //Input_UV
    Module1_Sts.b4 = Input_SD_STS[1];             //Input_Shutdown
    Module1_Sts.b3 = Output_OV_STS[1];            //Output_OV
    Module1_Sts.b2 = Output_UV_STS[1];            //Output_UV
    Module1_Sts.b1 = Output_OC_STS[1];            //Output_OC
    Module1_Sts.b0 = Output_OT_STS[1];            //Output_OT
    //////////////////////////////////////////
    //   Module #2 Status1(1byte : 8bit)    //
    //////////////////////////////////////////
    SystemState_STS[2]  = Alarm_Sts[2].b7;
    Input_OV_STS[2]     = Alarm_Sts[2].b6;
    Input_UV_STS[2]     = Alarm_Sts[2].b5;
    Input_SD_STS[2]     = Alarm_Sts[2].b4;
    Output_OV_STS[2]    = Alarm_Sts[2].b3;
    Output_UV_STS[2]    = Alarm_Sts[2].b2;
    Output_OC_STS[2]    = Alarm_Sts[2].b1;
    Output_OT_STS[2]    = Alarm_Sts[2].b0;
    //
    Module2_Sts.b7 = SystemState_STS[2];          //Operation
    Module2_Sts.b6 = Input_OV_STS[2];             //Input_OV
    Module2_Sts.b5 = Input_UV_STS[2];             //Input_UV
    Module2_Sts.b4 = Input_SD_STS[2];             //Input_Shutdown
    Module2_Sts.b3 = Output_OV_STS[2];            //Output_OV
    Module2_Sts.b2 = Output_UV_STS[2];            //Output_UV
    Module2_Sts.b1 = Output_OC_STS[2];            //Output_OC
    Module2_Sts.b0 = Output_OT_STS[2];            //Output_OT    
    
    //////////////////////////////////////////
    //   Module #3 Status1(1byte : 8bit)    //
    //////////////////////////////////////////
    SystemState_STS[3]  = Alarm_Sts[3].b7;
    Input_OV_STS[3]     = Alarm_Sts[3].b6;
    Input_UV_STS[3]     = Alarm_Sts[3].b5;
    Input_SD_STS[3]     = Alarm_Sts[3].b4;
    Output_OV_STS[3]    = Alarm_Sts[3].b3;
    Output_UV_STS[3]    = Alarm_Sts[3].b2;
    Output_OC_STS[3]    = Alarm_Sts[3].b1;
    Output_OT_STS[3]    = Alarm_Sts[3].b0;
    //
    Module3_Sts.b7 = SystemState_STS[3];          //Operation
    Module3_Sts.b6 = Input_OV_STS[3];             //Input_OV
    Module3_Sts.b5 = Input_UV_STS[3];             //Input_UV
    Module3_Sts.b4 = Input_SD_STS[3];             //Input_Shutdown
    Module3_Sts.b3 = Output_OV_STS[3];            //Output_OV
    Module3_Sts.b2 = Output_UV_STS[3];            //Output_UV
    Module3_Sts.b1 = Output_OC_STS[3];            //Output_OC
    Module3_Sts.b0 = Output_OT_STS[3];            //Output_OT            
    //////////////////////////////////////////
    //   Module #4 Status1(1byte : 8bit)    //
    //////////////////////////////////////////
    SystemState_STS[4]  = Alarm_Sts[4].b7;
    Input_OV_STS[4]     = Alarm_Sts[4].b6;
    Input_UV_STS[4]     = Alarm_Sts[4].b5;
    Input_SD_STS[4]     = Alarm_Sts[4].b4;
    Output_OV_STS[4]    = Alarm_Sts[4].b3;
    Output_UV_STS[4]    = Alarm_Sts[4].b2;
    Output_OC_STS[4]    = Alarm_Sts[4].b1;
    Output_OT_STS[4]    = Alarm_Sts[4].b0;
    //
    Module4_Sts.b7 = SystemState_STS[4];          //Operation
    Module4_Sts.b6 = Input_OV_STS[4];             //Input_OV
    Module4_Sts.b5 = Input_UV_STS[4];             //Input_UV
    Module4_Sts.b4 = Input_SD_STS[4];             //Input_Shutdown
    Module4_Sts.b3 = Output_OV_STS[4];            //Output_OV
    Module4_Sts.b2 = Output_UV_STS[4];            //Output_UV
    Module4_Sts.b1 = Output_OC_STS[4];            //Output_OC
    Module4_Sts.b0 = Output_OT_STS[4];            //Output_OT    
    //////////////////////////////////////////
    //   Battery Status(1byte : 8bit)       //
    //////////////////////////////////////////    
    Batt_Sts.b7 = Batt_OV_STS;              // Battery Under voltage Status
    Batt_Sts.b6 = Batt_UV_STS;              // Battery Under voltage Status
    Batt_Sts.b5 = Batt_Mode_STS;            // Battery Charge : 0 / Discharge : 1 
    Batt_Sts.b4 = 0;                        //
    Batt_Sts.b3 = 0;                        //
    Batt_Sts.b2 = 0;                        //
    Batt_Sts.b1 = 0;                        //
    Batt_Sts.b0 = 0;                        //    
    //////////////////////////////////////////
    //     Module Status(1byte : 8bit)      //
    //////////////////////////////////////////
    Module_OV_STS[1] = Warning_Sts[1].b7;
    Module_UV_STS[1] = Warning_Sts[1].b6;
    Module_OV_STS[2] = Warning_Sts[2].b7;
    Module_UV_STS[2] = Warning_Sts[2].b6;
    Module_OV_STS[3] = Warning_Sts[3].b7;
    Module_UV_STS[3] = Warning_Sts[3].b6;
    Module_OV_STS[4] = Warning_Sts[4].b7;
    Module_UV_STS[4] = Warning_Sts[4].b6;
    //
    Module_Sts.b7 = Module_OV_STS[1];       
    Module_Sts.b6 = Module_UV_STS[1];       
    Module_Sts.b5 = Module_OV_STS[2];       
    Module_Sts.b4 = Module_UV_STS[2];       
    Module_Sts.b3 = Module_OV_STS[3];       
    Module_Sts.b2 = Module_UV_STS[3];       
    Module_Sts.b1 = Module_OV_STS[4];       
    Module_Sts.b0 = Module_UV_STS[4];           
    //////////////////////////////////////////
    //      Spare1 Status(1byte : 8bit)     //
    //////////////////////////////////////////
    CHG_STS[1] = CHG_Info[1].b7;
    CHG_STS[2] = CHG_Info[1].b6;
    CHG_STS[3] = CHG_Info[2].b7;
    CHG_STS[4] = CHG_Info[2].b6;
    CHG_STS[5] = CHG_Info[3].b7;
    CHG_STS[6] = CHG_Info[3].b6;
    CHG_STS[7] = CHG_Info[4].b7;
    CHG_STS[8] = CHG_Info[4].b6;
    //    
    CHG_Sts.b7 = CHG_STS[1];
    CHG_Sts.b6 = CHG_STS[2];
    CHG_Sts.b5 = CHG_STS[3];
    CHG_Sts.b4 = CHG_STS[4];
    CHG_Sts.b3 = CHG_STS[5];
    CHG_Sts.b2 = CHG_STS[6];
    CHG_Sts.b1 = CHG_STS[7];
    CHG_Sts.b0 = CHG_STS[8];    
    //////////////////////////////////////////
    //      Spare2 Status(1byte : 8bit)     //
    //////////////////////////////////////////
    Spare_Sts1.b7 = 0;                        
    Spare_Sts1.b6 = 0;                        
    Spare_Sts1.b5 = 0;                        
    Spare_Sts1.b4 = 0;                        
    Spare_Sts1.b3 = 0;                        
    Spare_Sts1.b2 = 0;                        
    Spare_Sts1.b1 = 0;                        
    Spare_Sts1.b0 = 0;                        
    //////////////////////////////////////////
    //      LED Status1 Byte Package        //
    //////////////////////////////////////////
    Tx_Char[0] = LED_Sts1.b7;             
    Tx_Char[0] = (Tx_Char[0] << 1) | LED_Sts1.b6;
    Tx_Char[0] = (Tx_Char[0] << 1) | LED_Sts1.b5;
    Tx_Char[0] = (Tx_Char[0] << 1) | LED_Sts1.b4;
    Tx_Char[0] = (Tx_Char[0] << 1) | LED_Sts1.b3;
    Tx_Char[0] = (Tx_Char[0] << 1) | LED_Sts1.b2;
    Tx_Char[0] = (Tx_Char[0] << 1) | LED_Sts1.b1;//dc gnd//
    Tx_Char[0] = (Tx_Char[0] << 1) | LED_Sts1.b0;    
    //////////////////////////////////////////
    //      LED Status2 Byte Package       //
    //////////////////////////////////////////
    Tx_Char[1] =                     LED_Sts2.b7;             
    Tx_Char[1] = (Tx_Char[1] << 1) | LED_Sts2.b6;
    Tx_Char[1] = (Tx_Char[1] << 1) | LED_Sts2.b5;
    Tx_Char[1] = (Tx_Char[1] << 1) | LED_Sts2.b4;
    Tx_Char[1] = (Tx_Char[1] << 1) | LED_Sts2.b3;
    Tx_Char[1] = (Tx_Char[1] << 1) | LED_Sts2.b2;
    Tx_Char[1] = (Tx_Char[1] << 1) | LED_Sts2.b1;
    Tx_Char[1] = (Tx_Char[1] << 1) | LED_Sts2.b0;    
    //////////////////////////////////////////
    //      LED Status3 Byte Package        //
    //////////////////////////////////////////
    Tx_Char[2] =                     LED_Sts3.b7;             
    Tx_Char[2] = (Tx_Char[2] << 1) | LED_Sts3.b6;
    Tx_Char[2] = (Tx_Char[2] << 1) | LED_Sts3.b5;
    Tx_Char[2] = (Tx_Char[2] << 1) | LED_Sts3.b4;
    Tx_Char[2] = (Tx_Char[2] << 1) | LED_Sts3.b3;
    Tx_Char[2] = (Tx_Char[2] << 1) | LED_Sts3.b2;
    Tx_Char[2] = (Tx_Char[2] << 1) | LED_Sts3.b1;
    Tx_Char[2] = (Tx_Char[2] << 1) | LED_Sts3.b0;    
    //////////////////////////////////////////
    //      LED Status4 Byte Package        //
    //////////////////////////////////////////
    Tx_Char[3] = LED_Sts4.b7;             
    Tx_Char[3] = (Tx_Char[3] << 1) | LED_Sts4.b6;
    Tx_Char[3] = (Tx_Char[3] << 1) | LED_Sts4.b5;
    Tx_Char[3] = (Tx_Char[3] << 1) | LED_Sts4.b4;
    Tx_Char[3] = (Tx_Char[3] << 1) | LED_Sts4.b3;
    Tx_Char[3] = (Tx_Char[3] << 1) | LED_Sts4.b2;
    Tx_Char[3] = (Tx_Char[3] << 1) | LED_Sts4.b1;
    Tx_Char[3] = (Tx_Char[3] << 1) | LED_Sts4.b0;    
    //////////////////////////////////////////
    //      Module1_Status Byte Package     //
    //////////////////////////////////////////    
    Tx_Char[4] =                     Module1_Sts.b7;              
    Tx_Char[4] = (Tx_Char[4] << 1) | Module1_Sts.b6;//0x40
    Tx_Char[4] = (Tx_Char[4] << 1) | Module1_Sts.b5;//0x20 //ac under voltage//
    Tx_Char[4] = (Tx_Char[4] << 1) | Module1_Sts.b4;//0x10 //ac fail//
    Tx_Char[4] = (Tx_Char[4] << 1) | Module1_Sts.b3;//0x08
    Tx_Char[4] = (Tx_Char[4] << 1) | Module1_Sts.b2;//0x04
    Tx_Char[4] = (Tx_Char[4] << 1) | Module1_Sts.b1;//0x02
    Tx_Char[4] = (Tx_Char[4] << 1) | Module1_Sts.b0;//0x01   
    //
    //Tx_Char[4] = 0x10;
    //////////////////////////////////////////
    //      Module2_Status Byte Package     //
    //////////////////////////////////////////
    Tx_Char[5] = Module2_Sts.b7;             
    Tx_Char[5] = (Tx_Char[5] << 1) | Module2_Sts.b6;
    Tx_Char[5] = (Tx_Char[5] << 1) | Module2_Sts.b5;
    Tx_Char[5] = (Tx_Char[5] << 1) | Module2_Sts.b4;
    Tx_Char[5] = (Tx_Char[5] << 1) | Module2_Sts.b3;
    Tx_Char[5] = (Tx_Char[5] << 1) | Module2_Sts.b2;
    Tx_Char[5] = (Tx_Char[5] << 1) | Module2_Sts.b1;
    Tx_Char[5] = (Tx_Char[5] << 1) | Module2_Sts.b0;    
    //////////////////////////////////////////
    //      Module3_Status Byte Package     //
    //////////////////////////////////////////
    Tx_Char[6] = Module3_Sts.b7;             
    Tx_Char[6] = (Tx_Char[6] << 1) | Module3_Sts.b6;
    Tx_Char[6] = (Tx_Char[6] << 1) | Module3_Sts.b5;
    Tx_Char[6] = (Tx_Char[6] << 1) | Module3_Sts.b4;
    Tx_Char[6] = (Tx_Char[6] << 1) | Module3_Sts.b3;
    Tx_Char[6] = (Tx_Char[6] << 1) | Module3_Sts.b2;
    Tx_Char[6] = (Tx_Char[6] << 1) | Module3_Sts.b1;
    Tx_Char[6] = (Tx_Char[6] << 1) | Module3_Sts.b0;    
    //////////////////////////////////////////
    //      Module4_Status Byte Package     //
    //////////////////////////////////////////
    Tx_Char[7] =                     Module4_Sts.b7;             
    Tx_Char[7] = (Tx_Char[7] << 1) | Module4_Sts.b6;
    Tx_Char[7] = (Tx_Char[7] << 1) | Module4_Sts.b5;
    Tx_Char[7] = (Tx_Char[7] << 1) | Module4_Sts.b4;
    Tx_Char[7] = (Tx_Char[7] << 1) | Module4_Sts.b3;
    Tx_Char[7] = (Tx_Char[7] << 1) | Module4_Sts.b2;
    Tx_Char[7] = (Tx_Char[7] << 1) | Module4_Sts.b1;
    Tx_Char[7] = (Tx_Char[7] << 1) | Module4_Sts.b0;
    
    //////////////////////////////////////////
    //      Batt_Status Byte Package        //[28]
    //////////////////////////////////////////
    Tx_Char[8] =                     Batt_Sts.b7;             
    Tx_Char[8] = (Tx_Char[8] << 1) | Batt_Sts.b6;
    Tx_Char[8] = (Tx_Char[8] << 1) | Batt_Sts.b5;
    Tx_Char[8] = (Tx_Char[8] << 1) | Batt_Sts.b4;
    Tx_Char[8] = (Tx_Char[8] << 1) | Batt_Sts.b3;
    Tx_Char[8] = (Tx_Char[8] << 1) | Batt_Sts.b2;
    Tx_Char[8] = (Tx_Char[8] << 1) | Batt_Sts.b1;
    Tx_Char[8] = (Tx_Char[8] << 1) | Batt_Sts.b0;    
    //////////////////////////////////////////
    //      Module_Sts Byte Package       //
    //////////////////////////////////////////
    Tx_Char[9] =                     Module_Sts.b7;             
    Tx_Char[9] = (Tx_Char[9] << 1) | Module_Sts.b6;
    Tx_Char[9] = (Tx_Char[9] << 1) | Module_Sts.b5;
    Tx_Char[9] = (Tx_Char[9] << 1) | Module_Sts.b4;
    Tx_Char[9] = (Tx_Char[9] << 1) | Module_Sts.b3;
    Tx_Char[9] = (Tx_Char[9] << 1) | Module_Sts.b2;
    Tx_Char[9] = (Tx_Char[9] << 1) | Module_Sts.b1;
    Tx_Char[9] = (Tx_Char[9] << 1) | Module_Sts.b0;    
    //////////////////////////////////////////
    //           CHG_Sts Byte Package       //
    //////////////////////////////////////////
    Tx_Char[10] =                      CHG_Sts.b7;             
    Tx_Char[10] = (Tx_Char[10] << 1) | CHG_Sts.b6;
    Tx_Char[10] = (Tx_Char[10] << 1) | CHG_Sts.b5;
    Tx_Char[10] = (Tx_Char[10] << 1) | CHG_Sts.b4;
    Tx_Char[10] = (Tx_Char[10] << 1) | CHG_Sts.b3;
    Tx_Char[10] = (Tx_Char[10] << 1) | CHG_Sts.b2;
    Tx_Char[10] = (Tx_Char[10] << 1) | CHG_Sts.b1;
    Tx_Char[10] = (Tx_Char[10] << 1) | CHG_Sts.b0;
    //////////////////////////////////////////
    //      Spare_Sts1 Byte Package       //
    //////////////////////////////////////////
    Tx_Char[11] =                      Spare_Sts1.b7;             
    Tx_Char[11] = (Tx_Char[11] << 1) | Spare_Sts1.b6;
    Tx_Char[11] = (Tx_Char[11] << 1) | Spare_Sts1.b5;
    Tx_Char[11] = (Tx_Char[11] << 1) | Spare_Sts1.b4;
    Tx_Char[11] = (Tx_Char[11] << 1) | Spare_Sts1.b3;
    Tx_Char[11] = (Tx_Char[11] << 1) | Spare_Sts1.b2;
    Tx_Char[11] = (Tx_Char[11] << 1) | Spare_Sts1.b1;
    Tx_Char[11] = (Tx_Char[11] << 1) | Spare_Sts1.b0; 
    //-----------------------------------------------//
    // setting version value(RC,RD,MC), for send to MainDisplay
    //md1Version,mc1Version,
    //md2Version,mc2Version,
    //MCVERSION
    //-----------------------------------------------//
    // MC --(rs232)--> MD
	// Batt.Sts[28,bit0~4],Spare1[31] : 3(구분자) + 10 bit사용)
	// 최상위 3비트 
    //              (MC - bbb1-00vv, //0x10 
    //              rc1 - bbb0-00vv, //0x00
    //              rd1 - bbb0-10vv, //0x08
    //              rc2 - bbb0-01vv, //0x04
    //              rd2 - bbb0-11vv) //0x0c
	// 하위 10비트 (max 1023 => version 10.23)
    //-----------------------------------------------//    
    switch(iSendTurn%5)//0,1,2,3,4//
    {
        case 0://mc //bbb1-00vv
            hibyte = (unsigned char)((MCVERSION>>8)) | 0x10;
            lobyte = (unsigned char)(MCVERSION & 0x00FF);
            break;
        case 1://rc1 //bbb0-00vv
            hibyte = (unsigned char)((rc1Version>>8)) | 0x00;
            lobyte = (unsigned char)(rc1Version & 0x00FF);
            break;
        case 2://rd1 //bbb0-10vv
            hibyte = (unsigned char)((rd1Version>>8)) | 0x08;
            lobyte = (unsigned char)(rd1Version & 0x00FF);
            break;
        case 3://rc2 //bbb0-01vv
            hibyte = (unsigned char)((rc2Version>>8)) | 0x04;
            lobyte = (unsigned char)(rc2Version & 0x00FF);
            break;
        case 4://rd2 //bbb0-11vv
            hibyte = (unsigned char)((rd2Version>>8)) | 0x0c;
            lobyte = (unsigned char)(rd2Version & 0x00FF);
            break;
        default://rc1
            iSendTurn =0;
            break;                    
    }
    iSendTurn++;

    Tx_Char[8] |= hibyte;//0xxxxx-nnvv//
    Tx_Char[11] = lobyte;

}

void GetNfbState1(void)
{
	static	UINT8 NewInput =0xff;
	static	UINT8 OldInput =0xff;
    //
MODULE_STATE_EN_SetHigh();	
NFB_STATE_EN_SetLow();//active low    
	__asm__ volatile ("NOP");
	__asm__ volatile ("NOP");    
  //NewInput = (!_RD4<<7|!_RD3<<6|!_RD2<<5|!_RD1<<4|!_RC14<<3|!_RC13<<2|!_RD0<<1|0x01);//6012a  
    NewInput = (!_RB5<<7|!_RD5<<6|!_RD10<<5|!_RD11<<4|!_RD8<<3|!_RD9<<2|!_RC9<<1|0x01);//Module4
	//		   (M1_NFB  | M2_nfb | DC1_nfb | dc2_Nfb |batt_NFB|load_nfb| spare  |x   ) 	
	bNfbState1 &= NewInput ^ OldInput;
	bNfbState1 |= NewInput & OldInput;
	OldInput 	= NewInput;				
    //	
	if(bNfbState1 != bNfbState1Old)//changed Nfb state
	{	
		bNfbState1Old = bNfbState1;
	}		
    
//NFB_STATE_EN_SetHigh();//active low    

    //
//    if(!CheckBit(bNfbState1,7))
//    {
//        Alarm_Sts[1].b4 = 1;
//    }
//    if(!CheckBit(bNfbState1,6))
//    {
//        Alarm_Sts[2].b4 = 1;
//    }
    // Input Voltage NFB Check
    if(_RB5 == 1)
    {
        Alarm_Sts[1].b4 = 1;        
    }
    if(_RD5 == 1)
    {
        Alarm_Sts[2].b4 = 1;       
    }    
}

void GetNfbState2(void)
{
	static	UINT8 NewInput =0xff;
	static	UINT8 OldInput =0xff;
NFB_STATE_EN_SetHigh();
MODULE_STATE_EN_SetLow();//active low		    
	__asm__ volatile ("NOP");
	__asm__ volatile ("NOP");
	//NewInput = (!_RD4<<7|!_RD3<<6|!_RD2<<5|!_RD1<<4|!_RC14<<3|!_RC13<<2|!_RD0<<1|0x01);//6012a
    NewInput = (!_RB5<<7|!_RD5<<6|!_RD10<<5|!_RD11<<4|!_RD8<<3|!_RD9<<2|!_RC9<<1|0x01);   //Module4
	//			    x   | module7| module6 | module5 | module4| module3| module2| module1| 
	bNfbState2 &= NewInput ^ OldInput;
	bNfbState2 |= NewInput & OldInput;
	OldInput = NewInput;				
	//
	if(bNfbState2 != bNfbState2Old)
	{	
		bNfbState2Old = bNfbState2;
	}		
MODULE_STATE_EN_SetHigh();//active low		
}

void GetContact(void)
{
    UINT8 contactState=0;
	contactState =                       Alarm_Sts[1].b6;
    contactState = (contactState << 1) | Alarm_Sts[1].b5;
    contactState = (contactState << 1) | Alarm_Sts[1].b4;
    contactState = (contactState << 1) | Alarm_Sts[1].b3;
    contactState = (contactState << 1) | Alarm_Sts[1].b2;
    contactState = (contactState << 1) | Alarm_Sts[1].b1;
    contactState = (contactState << 1) | Alarm_Sts[1].b0;
    //
	if(contactState || 
            LED_Module1_NFB == 0 || 
                LED_DC1_NFB == 0 || 
                    LED_Battery_NFB == 0 || 
                        LED_Load_NFB == 0 || 
                            Module_OV_STS[1] == 1 || 
                                Module_UV_STS[1] == 1)
	{
        if(tmrContact1 >= 3000)
        {
            tmrContact1 = 3000;
            LED_M1_FAIL_SetHigh();//active high//
        }	
	}
	else 
    {
        tmrContact1 =0;
        LED_M1_FAIL_SetLow();       //active high//        
    }
    //
	contactState =                       Alarm_Sts[2].b6;
    contactState = (contactState << 1) | Alarm_Sts[2].b5;
    contactState = (contactState << 1) | Alarm_Sts[2].b4;
    contactState = (contactState << 1) | Alarm_Sts[2].b3;
    contactState = (contactState << 1) | Alarm_Sts[2].b2;
    contactState = (contactState << 1) | Alarm_Sts[2].b1;
    contactState = (contactState << 1) | Alarm_Sts[2].b0;
    //
	if(contactState || 
            LED_Module2_NFB == 0 || 
                LED_DC2_NFB == 0 || 
                    Module_OV_STS[2] == 1 || 
                        Module_UV_STS[2] == 1)
	{        
        if(tmrContact2 >= 3000)
        {
            tmrContact2 = 3000;
            if(SYS_Type == 0)   LED_M2_FAIL_SetHigh();//On//
            else                LED_M2_FAIL_SetLow(); //OFF//
        }
	}
	else        
    {
        tmrContact2=0;
        LED_M2_FAIL_SetLow();
    }
	//
	if(Batt_UV_STS)
	{
        if(tmrContact3 >=3000)
        {
            tmrContact3=0;
            LED_BATT_LOW_SetHigh();            
        }    
	}
	else
    {
        tmrContact3=0;
        LED_BATT_LOW_SetLow();
    }
    /*
	UINT8 contactState=0;
    UINT8 totalAlarm = 0;
    UINT8 Warning_Temp = 0;
	contactState =                       Alarm_Sts[1].b6;
    contactState = (contactState << 1) | Alarm_Sts[1].b5;
    contactState = (contactState << 1) | Alarm_Sts[1].b4;
    contactState = (contactState << 1) | Alarm_Sts[1].b3;
    contactState = (contactState << 1) | Alarm_Sts[1].b2;
    contactState = (contactState << 1) | Alarm_Sts[1].b1;
    contactState = (contactState << 1) | Alarm_Sts[1].b0;
    //
    if(contactState || 
            LED_Module1_NFB == 0 || 
                LED_DC1_NFB == 0 || 
                    LED_Battery_NFB == 0 || 
                        LED_Load_NFB == 0 || 
                            Module_OV_STS[1] == 1 || 
                                Module_UV_STS[1] == 1)
    {
        totalAlarm |= 0x01;
    }
    else totalAlarm &= 0xFE;//1111-1110;// 
    //---------------------------------------------------
	contactState =                       Alarm_Sts[2].b6;
    contactState = (contactState << 1) | Alarm_Sts[2].b5;
    contactState = (contactState << 1) | Alarm_Sts[2].b4;
    contactState = (contactState << 1) | Alarm_Sts[2].b3;
    contactState = (contactState << 1) | Alarm_Sts[2].b2;
    contactState = (contactState << 1) | Alarm_Sts[2].b1;
    contactState = (contactState << 1) | Alarm_Sts[2].b0;
    
    if(contactState || 
            LED_Module2_NFB == 0 || 
                LED_DC2_NFB == 0 || 
                    Module_OV_STS[2] == 1 || 
                        Module_UV_STS[2] == 1)
    {
        totalAlarm |= 0x02;
    }else totalAlarm &= 0xFD;//1111-1101// 
    //---------------------------------------------------------------------------
    if(SYS_Type == 0)   
        Warning_Temp = ( (Alarm_Sts[1].b6 | Alarm_Sts[1].b5 | Alarm_Sts[1].b4) != 0 )|| 
                            ((Alarm_Sts[2].b6 | Alarm_Sts[2].b5 | Alarm_Sts[2].b4) != 0) ;
    else                
        Warning_Temp = (Alarm_Sts[1].b6 | Alarm_Sts[1].b5 | Alarm_Sts[1].b4) != 0;       
    if(Warning_Temp) //check ac fail//
	{ 
        if(tmrContact2 >= 3000)
        {
            tmrContact2 = 3000;
            LED_M2_FAIL_SetHigh();//AC_FAIL_ON();
            totalAlarm |=0x04;
        }    
	}
	else
    {
        tmrContact2 = 0;
        LED_M2_FAIL_SetLow();   //AC_FAIL_OFF();
        totalAlarm &= 0xFB;//1111-1011
    }      
	//-------------------------------------------------------------------
	if(Batt_UV_STS)
	{
        if(tmrContact3 >=3000)
        {
            tmrContact3=0;
            LED_BATT_LOW_SetHigh();
            totalAlarm |= 0x08;
        }    
	}
	else
    {
        tmrContact3=0;
        LED_BATT_LOW_SetLow();
        totalAlarm &= 0xF7;//1111-0111// 
    }
    //--------------------------------------------------------------------
    if(LED_Sts1.b1)
    {
        totalAlarm |= 0x10;
    }
    else
    {
        totalAlarm &= 0xEF;//1110-1111//
    }
   // totalAlarm
    if(totalAlarm>0)
    {
        if(tmrContact1 >= 3000)
        {
            tmrContact1 = 3000;
            LED_M1_FAIL_SetHigh();//total Alarm//
        }
    }
    else
    {
        tmrContact1 = 0;
        LED_M1_FAIL_SetLow();//total Alarm//
    }
  */ 
}