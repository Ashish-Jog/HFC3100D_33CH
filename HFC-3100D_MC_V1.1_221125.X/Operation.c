/*
**********************************************************
    Target device : dspic33ch128mp506
    mplabx ide Ver 6.0
    Compiler : XC16 V2.0
    Product PCB : HFC3100D-MC22-04-25 Rev2.0//
    support tools : pickit3, pickit4
    version 1.0 : 2022.08.17    
* 
********************************************************** */
#include    "typedef.h"
#include    "Operation.h"
#include    "funcData.h"
#include    "main.h"
//
#include    "mcc_generated_files/pin_manager.h"

UINT8   RST_cnt = 0;            // Restart Count During Recovery condition
UINT8   RST_Done = 0;
UINT16   RST_Timer = 0;          // Recovery Flag Timer During Recovery condition
UINT8   Recovery_Flag = 0;      // Recovery Flag During Recovery condition
UINT8   Set_OK = 0;             // OP_CMD => '1' : Module1 / '2' : Module2
volatile UINT16 tmrOpLogic=0;

//////////////////////////////////////////////////////////////////////////////// 
//   Operation Logic Process(but, Manual Mode code is Added to SPI_RS485.C    //
//                         Health Monitoring Logic                            //
//             Please Refer to the Technical Documentation for Detail         //
////////////////////////////////////////////////////////////////////////////////
void OP_Logic(void)     
{   
    //-------------------------- Normal Condition ----------------------------//
    if( OP_CMD == 1)
    {
        if(SYS_Type==1)//single type//
        {
            if( Alarm_Sts[1].b7 == 1 && //#1_Module Run//
                Alarm_Sts[1].b4 == 0 && //#1_Module AC OK//
                Alarm_Sts[1].b3 == 0 && //#1_out high volt//            
                Alarm_Sts[1].b2 == 0 && //#1_out low volt//            
                LED_Module1_NFB == 1 && //#1_Module NFB ON//
                LED_DC1_NFB     == 1 && //#1_Module DC Line NFB ON//
                tm8 < 5000 )
            {
                RST_cnt=0;
                RST_Timer = 0;              // Recovery Flag Timer Reset
                Recovery_Flag = 0;          // Recovery_Flag Reset(Recovery Mode Clear)
                Set_OK = 0;                 // Forcing Change Normal Condition RS485 Bus 
                tmrOpLogic = 0;             //tmr2_2
                //
                FND0_SetLow();
            }
            
        }
        //----------------------------------------------------------------------
        if( Alarm_Sts[1].b7 == 1 && //#1_Module Run//
            //Alarm_Sts[2].b7 == 0 && //#2_Module standby//
            Alarm_Sts[1].b4 == 0 && //#1_Module AC OK//
            //Alarm_Sts[2].b4 == 0 && //#2_Module AC OK//
            Alarm_Sts[1].b3 == 0 && //#1_out high volt//            
            Alarm_Sts[1].b2 == 0 && //#1_out low volt//            
            LED_Module1_NFB == 1 && //#1_Module NFB ON//
            //LED_Module2_NFB == 1 && //#2_Module NFB ON//
            LED_DC1_NFB     == 1 && //#1_Module DC Line NFB ON//
            //LED_DC2_NFB     == 1 && 
            tm8 < 5000           )
        {               
            //---------------//
            //#1 Normal state//
            //---------------//
            RST_cnt=0;
            RST_Timer = 0;              // Recovery Flag Timer Reset
            Recovery_Flag = 0;          // Recovery_Flag Reset(Recovery Mode Clear)
            Set_OK = 0;                 // Forcing Change Normal Condition RS485 Bus 
            tmrOpLogic = 0;             //tmr2_2
            //
            FND0_SetLow();
        }
        else if(
                (Alarm_Sts[1].b4    == 1) || //#1 ac fail
                (Alarm_Sts[1].b7    == 0) || //Standby state//
                (LED_Module1_NFB    == 0) || //nfb off
                (LED_DC1_NFB        == 0) || //nfb off//
                (Alarm_Sts[1].b3    == 1) || //#1_out high volt//
                (Alarm_Sts[1].b2    == 1) ||
                (tm8 > 5000) //comm disconnect//
            )
        {
            //---------------//
            //#1 Fault state //
            //---------------//
            FND0_SetHigh();
            // Bypass Mode Entering
            if( Recovery_Flag == 0 && RST_cnt == 0)//event occur //
            {
                Set_OK = 1;                 // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                OP_CMD = 2;                 // Forcing Command Change Operation Command Module2
                TX_EN_SetHigh();    
                tmr485Send = 0;
                Request_Send();    //3      // Forcing Operation Command Send to Modules
                TX_EN_SetLow();    
                Alarm_Sts[1].b7 = 0;        // Forcing OFF Status Change to Module1(Not real, Because, Transmission Delay)
                Alarm_Sts[2].b7 = 1;        // Forcing ON Status Change to Module2(Not real, Because, Transmission Delay), Feed-back Time
                tmrOpLogic = 0;
                RST_Timer = 0;              // Recovery Flag Timer Reset
                Recovery_Flag = 1;          // Recovery_Flag Set(Recovery Mode)
                OP_CMD = 1;                 // Forcing Operation Command Module1 for Recovery Mode Memory, Recovery 3rd
                RST_cnt = 1;                //explicit count//
            }                
            //else//Recovery_Flag==1// Recovery Mode Entering
            //{
            else if(Recovery_Flag == 1 && RST_cnt < 4) //1,2,3//
            {
                if(tm8<5000)// && Alarm_Sts[1].b7 == 0)//comm ok and standBy
                {
                    if((int)RST_TIME < 1 )    
                    {
//                            if(RST_Timer > 10)
//                            {
                        Recovery_Flag = 0;//clear Recovery flag//
                        RST_Timer = 0;
                        OP_CMD =2;//set #2 main//
                        RST_cnt=4;                         
                    }    
                    else if(RST_Timer >= (RST_TIME * 60))//RST_TIME(Minute Unit)* 60(Sec) max = 60*60=3600second//                        
                    {
                        OP_CMD = 1;//add goooo//
                        Set_OK = 1;                 // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                        TX_EN_SetHigh();
                        tmr485Send = 0;
                        Request_Send();   //4          // Forcing Operation Command Send to Modules
                        TX_EN_SetLow();
                        Alarm_Sts[1].b7 = 1;        // Forcing OFF Status Change to Module1(Not real, Because, Transmission Delay)
                        Alarm_Sts[2].b7 = 0;       
                        tmrOpLogic = 0;
                        RST_Timer = 0;              // Recovery Flag Timer Reset
                        //----------------
                        fSendTwice = 1;
                        sendTimer=0;
                        sendCmd=OP_CMD;
                        //-----------------
                        RST_cnt++;                      // Restart Count Increase
                        if(RST_cnt>=4)
                        {
                            Recovery_Flag = 0;
                            RST_Timer = 0;
                            OP_CMD =2;
                            RST_cnt=4;    
                        }
                        //
                        FND2_Toggle();
                    }//End else if(RST_Timer > (RST_TIME * 60))              
                }//if(tm8<5000 && Alarm_Sts[1].b7 == 0)
                else
                {
                    if(tm8>5000)RST_Timer = 0;
                }
            }//End if(RST_cnt < 4)
            else
            {
                Recovery_Flag = 0;
                RST_Timer = 0;
                OP_CMD =2;
                RST_cnt=4;    
            }
           // }//End else(Recovery_Flag==1)
        }//End #1 module fault//
    }//End if( OP_CMD == 1)
    //==========================================================================
    else if(OP_CMD == 2)
    {
        if(
            //(Alarm_Sts[1].b7 == 0) && 
            (Alarm_Sts[2].b7 == 1) && 
            //(Alarm_Sts[1].b4 == 0) && 
            (Alarm_Sts[2].b4 == 0) && 
            (Alarm_Sts[2].b3 == 0) && //#2_out high volt//
            (Alarm_Sts[2].b2 == 0) && //#2_out Low volt//
            //(LED_Module1_NFB == 1) && 
            (LED_Module2_NFB == 1) && 
            //(LED_DC1_NFB     == 1) && 
            (LED_DC2_NFB     == 1) &&
            (tm9 < 5000)  //comm. connected//
                )
        {//module #2 normal state//
            Recovery_Flag = 0;          // Recovery_Flag Reset(Recovery Mode Clear)
            RST_cnt=0;
            RST_Timer = 0;              // Recovery Flag Timer Reset
            Set_OK = 0;                 // Forcing Change Normal Condition RS485 Bus 
            tmrOpLogic = 0;             //tmr2_2
            //
            FND0_SetLow();
        }
        else if(//#2 fault//
            (Alarm_Sts[2].b4    == 1) || //#2 ac fail
            (Alarm_Sts[2].b7    == 0) || //Standby state//
            (LED_Module2_NFB    == 0) || //nfb off//
            (LED_DC2_NFB        == 0) || //nfb off//
            (Alarm_Sts[2].b3    == 1) || //#2_out high volt//
            (Alarm_Sts[2].b2    == 1) ||
            (tm9 > 5000)                 //comm disconnect//
            )
        {
            FND0_SetHigh();
            // Bypass Mode Entering
            if( Recovery_Flag == 0 && RST_cnt == 0)//event occur //
            {
                Recovery_Flag = 1;          // Recovery_Flag Set(Recovery Mode)
                RST_cnt = 1;                //explicit count//
                Set_OK = 1;                 // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                OP_CMD = 1;                 // Forcing Command Change Operation Command Module2
                TX_EN_SetHigh();    
                tmr485Send = 0;
                Request_Send();    //3      // Forcing Operation Command Send to Modules
                TX_EN_SetLow();    
                Alarm_Sts[1].b7 = 1;        // Forcing OFF Status Change to Module1(Not real, Because, Transmission Delay)
                Alarm_Sts[2].b7 = 0;       
                tmrOpLogic = 0;
                RST_Timer = 0;              // Recovery Flag Timer Reset
                OP_CMD = 2;                 // Forcing Operation Command Module1 for Recovery Mode Memory, Recovery 3rd
            }                
            else//Recovery_Flag==1// Recovery Mode Entering
            {
                if(RST_cnt < 4) //1,2,3//
                {
                    if(tm9<5000)// && Alarm_Sts[2].b7 == 0)
                    {
                        if((int)RST_TIME < 1 )
                        {
//                            if(RST_Timer > 10)
//                            {

                                Recovery_Flag = 0;
                                RST_Timer = 0;
                                OP_CMD =1;
                                RST_cnt=4;
                        }    
                        else if(RST_Timer > (RST_TIME * 60)) //RST_TIME(Minute Unit)* 60(Sec) max = 60*60=3600second//
                      //  if(((int)RST_TIME >= 1) && (RST_Timer > (RST_TIME * 60)) )    
                        {
                            OP_CMD = 2;//add goooo//
                            Set_OK = 1;                 // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                            TX_EN_SetHigh();
                            tmr485Send = 0;
                            Request_Send();   //4          // Forcing Operation Command Send to Modules
                            TX_EN_SetLow();
                            Alarm_Sts[1].b7 = 0;        // Forcing OFF Status Change to Module1(Not real, Because, Transmission Delay)
                            Alarm_Sts[2].b7 = 1;       
                            tmrOpLogic = 0;
                            RST_Timer = 0;              // Recovery Flag Timer Reset
                            //----------------
                            fSendTwice = 1;
                            sendTimer=0;
                            sendCmd=OP_CMD;
                            //-----------------
                            RST_cnt++;                      // Restart Count Increase
                            FND2_Toggle();
                            if(RST_cnt>=4)
                            {
                                Recovery_Flag = 0;
                                RST_Timer = 0;
                                OP_CMD =1;
                                RST_cnt=4;    
                            }
                        }                
                    }//End if(tm9<5000 && Alarm_Sts[2].b7 == 0)
                    else
                    {
                        if(tm9>5000)RST_Timer = 0;
                    }
                }//End if(RST_cnt < 4) 
                else
                {
                    Recovery_Flag = 0;
                    RST_Timer = 0;
                    OP_CMD =1;
                    RST_cnt=4;    
                }
            }//#2recovery operation            
        }//#2fault//
    }//OP_CMD == 2//  
    /* ??????????????????
    else if( Alarm_Sts[1].b7 == 0 && //#1_Module Standby//
            Alarm_Sts[2].b7 == 1 && //#2_Module Run//
            Alarm_Sts[1].b4 == 0 && //#1_Module AC OK//
            Alarm_Sts[1].b3 == 0 && //#1_out high volt//            
            Alarm_Sts[1].b2 == 0 && //#1_out low volt//            
            LED_Module1_NFB == 1 && //#1_Module NFB ON//
            LED_DC1_NFB     == 1 && //#1_Module DC Line NFB ON//
            tm8 < 5000           )
    {//normal state//
        RST_cnt=0;
        RST_Timer = 0;              // Recovery Flag Timer Reset
        Recovery_Flag = 0;          // Recovery_Flag Reset(Recovery Mode Clear)
        Set_OK = 0;                 // Forcing Change Normal Condition RS485 Bus 
        tmrOpLogic = 0;             //tmr2_2
        //
        FND0_SetLow();
    }
    */
    // ac fail then 3 times recovery operation done  ----------------------------// 
    //after ac normal state then system on
    //=========//
    //Dual Type//
    //=========//
    //if( ((SYS_Type == 0) && (Recovery_Flag == 0)) ) //|| ((int)RST_TIME < 1) )
    //============================================================================
    
    
    if (SYS_Type == 0)
    {
        if( (Alarm_Sts[1].b7 == 0) && (Alarm_Sts[2].b7 == 0) )
        {
            if(Alarm_Sts[1].b4 == 0 && tm8 < 5000) //#1_Module AC OK//
            {
                if(fNormalState == 0)
                {
                    fNormalStateTimer=100;//start timer-5sec timer//
                    fNormalState = 1;
                }
                else
                {
                    if(fNormalStateTimer==0)
                    {
                        OP_CMD = 1;
                        Set_OK = 1;                     // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                        TX_EN_SetHigh();
                            tmr485Send = 0;
                            Request_Send();      //9           // Forcing Operation Command Send to Modules
                        TX_EN_SetLow();
                        Alarm_Sts[1].b7 = 1;        // Forcing ON Status Change to Module1(Not real, Because, Transmission Delay)
                        Alarm_Sts[2].b7 = 0;        // Forcing OFF Status Change to Module2(Not real, Be
                        fNormalStateTimer = 100;
                        //
                        Recovery_Flag = 0;
                    }
                }
            }
            else if( (Alarm_Sts[2].b4 == 0) && (tm9 < 5000) ) //#1_Module AC OK//
            {
                if(fNormalState == 0)
                {
                    fNormalStateTimer = 100;
                    fNormalState = 1;
                }
                else
                {
                    if(fNormalStateTimer==0)
                    {
                        OP_CMD = 2;
                        Set_OK = 1;                     // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                        TX_EN_SetHigh();
                            tmr485Send = 0;
                            Request_Send();      //9           // Forcing Operation Command Send to Modules
                        TX_EN_SetLow();
                        Alarm_Sts[1].b7 = 0;        // Forcing ON Status Change to Module1(Not real, Because, Transmission Delay)
                        Alarm_Sts[2].b7 = 1;        // Forcing OFF Status Change to Module2(Not real, Be
                        fNormalStateTimer = 100;
                        //
                        Recovery_Flag = 0;
                    }//End if(fNormalStateTimer==0)
                }
            }
            else
            {
                fNormalState = 0;
                fNormalStateTimer = 100;//100ms*100 = 10seconds//
            }
        }
        else
        {
            fNormalStateTimer=100;
        }
    }//End if(SYS_Type == 0)//dual//   
    //else if( (SYS_Type == 1) && (Recovery_Flag == 0) )////Single Type//
    else if (SYS_Type == 1)
    {
        //if( (tm8<5000) && (Alarm_Sts[1].b7 == 0) && (Alarm_Sts[1].b3 == 0) && (Alarm_Sts[1].b2 == 0))
        if( (tm8<5000) && (Alarm_Sts[1].b7 == 0)) // && (Alarm_Sts[1].b3 == 0) && (Alarm_Sts[1].b2 == 0))
        {
            if(Alarm_Sts[1].b4 == 0) //#1_Module AC OK//
            {
                if(fNormalState == 0)
                {
                    fNormalStateTimer = 100;
                    fNormalState = 1;
                }
                else
                {
                    if(fNormalStateTimer == 0)
                    {
                        OP_CMD=1;
                        Set_OK = 1;                     // Forcing Set, Change Bus Stop Condition of RS-485 before to Send Operation Command
                        TX_EN_SetHigh();
                            tmr485Send = 0;
                            Request_Send();      //9           // Forcing Operation Command Send to Modules
                        TX_EN_SetLow();
                        Alarm_Sts[1].b7 = 1;        // Forcing ON Status Change to Module1(Not real, Because, Transmission Delay)
                        Alarm_Sts[2].b7 = 0;        // Forcing OFF Status Change to Module2(Not real, Be
                        fNormalStateTimer = 100;
                        Recovery_Flag = 0;
                    }//End if(fNormalStateTimer == 0)
                }
            }//End if(Alarm_Sts[1].b4 == 0)
            else
            {
                fNormalState = 0;
                fNormalStateTimer = 100;//100ms*50 = 5seconds//
            }
        }
        else
        {
            fNormalStateTimer=100;
        }
    }//end else if (SYS_Type == 1)
     
       
    
} //End void OP_Logic(void) 
