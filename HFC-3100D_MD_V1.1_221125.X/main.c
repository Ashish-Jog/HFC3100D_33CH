
/*
    Target device : dspic33ch128mp506
    mplabx ide Ver 6.0
    Compiler : XC16 V2.0
    Product PCB : HFC3100D-Main Lcd Display 22-04-25 Rev2.0//
    support tools : pickit3, pickit4 
    version 1.0 : 2022.08.17
*/
 /*
  * Special Feature
  * 1. MCP121_300(voltage supervisor) populate(power on timer = 120ms delay)
  
  */
//------------------------------------------------
#ifndef FCY
    #define FCY     (_XTAL_FREQ/2)
#endif
#include <libpic30.h>
#include <math.h>
/**
  Section: Included Files
*/
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/serial1.h"
#include "mcc_generated_files/serial2.h"
#include "mcc_generated_files/charlcd1.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/uart1.h"
//
#include "var.h"
#include "definition.h"
#include "function.h"
#include "i2c_master_eeprom_24fc256.h"
#include "ds1302.h"
#include "mcc_generated_files/m1_nfb_led.h"
#include "mcc_generated_files/m2_nfb_led.h"
#include "mcc_generated_files/dc1_nfb_led.h"
#include "mcc_generated_files/dc2_nfb_led.h"
#include "mcc_generated_files/m1_line_led.h"
#include "mcc_generated_files/m2_line_led.h"
#include "mcc_generated_files/out_line_led.h"
#include "mcc_generated_files/batt_nfb_led.h"
#include "mcc_generated_files/load_nfb_led.h"
#include "mcc_generated_files/normal_led.h"
#include "mcc_generated_files/fail_led.h"
#include "mcc_generated_files/auto_led.h"
#include "mcc_generated_files/manual_led.h"
#include "mcc_generated_files/no1_led.h"
#include "mcc_generated_files/no2_led.h"
/*
                         Main application
 */
//
//--------------------------------
#define serialDebug     0       //
#define Debug           0   //1 enable, 0 is off//
//--------------------------------
void lcdInit(void);
void lcdUpdate(void);
void lcdSetValue(void);
void lcdSystemStart(void);
//
void sendData(void);
void softClock(void);
//
void DelayNop(unsigned int delaycnt);
void eepromTest(void);
//
void EEPROM_Initialize(void);
void ParameterInit(void);
void InitSetValue(void);

void SystemInitialize(void);

//====================================
                 //"0123456789abcdef"  
char lcdStr0[17] = "1. MC : V 1.10  ";//33ch128mp506
char lcdStr1[17] = "2. MD : V 1.10  ";//33ch128mp506
char lcdStr2[17] = "3. RC : V 1.40  ";//32fj64gs606
char lcdStr3[17] = "4. RD : V 1.10  ";//30f6012a-33ch128mp506
char lcdBuffer[17]="                ";
//
uint8_t mainToggle=0;
uint16_t mainVar=0;
//UINT8 iLcdBackLightTm=0;
//
int main(void)
{
    UINT16 i; //for iteration variable//
    //
    SYSTEM_Initialize();    
    CHARLCD1_Initialize();//when current position not initial lcd ???
                            //move below the init. code
    SERIAL2_Initialize();
    SERIAL1_Initialize();
    //
    CHARLCD1_Initialize();//char. lcd init(16x4)
    //----------------------
    rtc_init(); //ds1302N initial
    //-----------------------    
    iLcdBackLightTm = 4;
    BL_SetHigh();//always lcd bl on//
    //------------------------
    //LCD custom character loading//
    createChar(7, gauge_empty);   // middle empty gauge
    createChar(1, gauge_fill_1);  // filled gauge - 1 column
    createChar(2, gauge_fill_2);  // filled gauge - 2 columns
    createChar(3, gauge_fill_3);  // filled gauge - 3 columns
    createChar(4, gauge_fill_4);  // filled gauge - 4 columns  
    createChar(0, heart_icon); // warning icon - just because we have one more custom character that we could use  
    //-----------
    EEPROMInit();//i2c bus init(eeprom init)    
    //----------------------------------------------------
    //eepromTest();
    EEPROM_Initialize();//set system variable to EEPROM
    //------------
	ParameterInit();    //get system variable from EEPROM//
    //----------------------------------------------------
    rtc_init();         //ds1302 init//
    year = rm_bcd(read_ds1302(0x8d));
	if(!year)
	{	
		rtc_set_datetime(7,6,22,2,12,01);//(day,mth,year,thu,hour,min)
	}
    //----------------------------------------------------
    lcdInit(); //Display Version    
    //
    //1byte read(1.8ms)
    //10byte write(~1.5us)
    for(i = 0; i < 150; i++) //get alarm history - take time 2.97sec //     
	{
		gAlarmHistory[i][0] = EEPROMRead(ALARM_STORAGE + (i*10)    );
		gAlarmHistory[i][1] = EEPROMRead(ALARM_STORAGE + (i*10) + 1);
		gAlarmHistory[i][2] = EEPROMRead(ALARM_STORAGE + (i*10) + 2);
		gAlarmHistory[i][3] = EEPROMRead(ALARM_STORAGE + (i*10) + 3);
		gAlarmHistory[i][4] = EEPROMRead(ALARM_STORAGE + (i*10) + 4);
		gAlarmHistory[i][5] = EEPROMRead(ALARM_STORAGE + (i*10) + 5);
		gAlarmHistory[i][6] = EEPROMRead(ALARM_STORAGE + (i*10) + 6);
		gAlarmHistory[i][7] = EEPROMRead(ALARM_STORAGE + (i*10) + 7);
		gAlarmHistory[i][8] = EEPROMRead(ALARM_STORAGE + (i*10) + 8);
		gAlarmHistory[i][9] = EEPROMRead(ALARM_STORAGE + (i*10) + 9);        
		//gAlarmHistoryCnt =      EEPROMRead(ALARM_HISTORY_CNT);        
	}    
    //TP1_SetLow();
    gAlarmHistoryCnt =      EEPROMRead(ALARM_HISTORY_CNT);
    //------------------------------------------------------
    InitSetValue();//get battery hi/low value & restart time
    gSelectModule = 1;//default module #1 set//
    Set_OK=1;
    //
    Serial_Process();
    //----------------------------------------
    lcdSystemStart();//progress bar display.take Time 2.7sec//
    //----------------
    //fBzOn=1;
    BZ_SetHigh();    
    __delay_ms(100);
    //fBzOn=0;
    BZ_SetLow();
    //-----------------
    DispLogo();  //init. display 
    //
    //_SWDTEN = 1;//Enable watch dog?
    while (1)
    {
        Serial_Process();
        //--------------
        uc_key = getKeyData();//check key press for lcd backlight turn on/
        if(uc_key)
        {
            iLcdBackLightTm = 4; //5000ms*4 ~ 20s lcdbacklight off timer//
            BL_SetHigh();       //turn on lcd backlight//       
        }
        //===============================//  
        //Task0                          //
        //===============================//          
        if(HEARTTMR >= 5)//op.period : 1ms * 5 = 5ms
        {
            HEARTTMR = 0;      
            iMainCnt++;            
            if(iMainCnt>100)//
            {
                iMainCnt = 0;
                mainToggle ^= 1; 
            }                 
            //------------------
            KeyDetecting();    
            AutoManual();                
            SelectModule();
            //
        }
        //===============================//  
        //Task1                          //
        //===============================//  
        if(TM1>=100)
        {
            TM1 = 0;                        
            //------------           
            MainProcess();      //lcd menu display            
            //
            //lcdUpdate();        //lcd buffer ti lcd memory update
            //
            LedStatus(); //
            //
            ModuleStatus();     //AlarmOccurrenceApply(...)
            //
            BatteryStatus();
            //
            WarningStatus();
            //
			ModuleStartupStatus();
            //
            ChargeStatus();
            //
            dcGroundCheck();
            //
            bzOperation();
            //
            lcdUpdate();        //lcd buffer ti lcd memory update
        }//End TM1
        //===============================//  
        //Task2//
        //===============================//
        if(TM2>=200)
        {
            TM2 = 0;    
            TP2_Toggle();
            //-----------
           // bzOperation();
            
        }//End TM2
        //===============================//  
        //Task3//
        //===============================//
        if(TM3>=1000)
        {
            TM3 = 0;
            TP3_Toggle();           
            //-----------
            if(ErrCode==0)prev_alarm_name=ErrCode;
            tmr1s++;
            cursor1s++;
            key1s++;
        }//End TM3
        //===============================//  
        //Task4//
        //===============================//
        if(TM4>=2000)
        {
            TM4 = 0;
            TP4_Toggle();    
            //-----------            
        }//End TM4
        //===============================//  
        //Task5//
        //===============================//  
        if(TM5>=5000)
        {
            TM5 = 0;
            taskCnt++;
            TP5_Toggle();       
            //-------------        
            if(iLcdBackLightTm>0)
            {
                iLcdBackLightTm--;
                BL_SetHigh();//
            }
            else
            {
                BL_SetLow();//for always turn on backlight//
            }
        }//End TM5
        
        if(TM6>2000)
        {
//            TM6 = 0;
//            TP1_SetHigh();
//            ErrCode=51;
//			AlarmOccurrenceApply(gPacketsize);
//            TP1_SetLow();
        }    
            
        //---------
        uc_key = 0;        
        //---------
        HEART_BEAT_Toggle();//RD12bit toggle-for alive board//
        softClock();
        //--------------
    }//End while(1)-main()    
    return 1; 
}//End main()


void DelayNop(unsigned int delaycnt)
{
	unsigned int j;
	
	for(j=0;j < delaycnt;j++)
		Nop();
}

void lcdSystemStart(void)
{
    while(cpu_gauge<100)
    {
        float units_per_pixel = (gauge_size_chars*5.0)/100.0;        //  every character is 5px wide, we want to count from 0-100
        int value_in_pixels = round(cpu_gauge * units_per_pixel);    // cpu_gauge value converted to pixel width
        int tip_position = 0;      // 0= not set, 1=tip in first char, 2=tip in middle, 3=tip in last char

        if (value_in_pixels < 5) {tip_position = 1;}                            // tip is inside the first character
        else if (value_in_pixels > gauge_size_chars*5.0-5) {tip_position = 3;}  // tip is inside the last character
        else {tip_position = 2;}                                                // tip is somewhere in the middle

        move_offset = 4 - ((value_in_pixels-1) % 5);      // value for offseting the pixels for the smooth filling

        for (int i=0; i<8; i++) 
        {   // dynamically create left part of the gauge
            if (tip_position == 1) {gauge_left_dynamic[i] = (gauge_fill_5[i] << move_offset) | gauge_left[i];}  // tip on the first character
            else {gauge_left_dynamic[i] = gauge_fill_5[i];}                                                     // tip not on the first character

            gauge_left_dynamic[i] = gauge_left_dynamic[i] & gauge_mask_left[i];                                 // apply mask for rounded corners
        }

        for (int i=0; i<8; i++) 
        {   // dynamically create right part of the gauge
            if (tip_position == 3) {gauge_right_dynamic[i] = (gauge_fill_5[i] << move_offset) | gauge_right[i];}  // tip on the last character
            else {gauge_right_dynamic[i] = gauge_right[i];}                                                       // tip not on the last character

            gauge_right_dynamic[i] = gauge_right_dynamic[i] & gauge_mask_right[i];                                // apply mask for rounded corners
        }  

        createChar(5, gauge_left_dynamic);     // create custom character for the left part of the gauge
        createChar(6, gauge_right_dynamic);    // create custom character for the right part of the gauge

        for (int i=0; i<gauge_size_chars; i++) 
        {  // set all the characters for the gauge
            if (i==0) {gauge_string[i] = (BYTE)(5);}                        // first character = custom left piece
            else if (i==gauge_size_chars-1) {gauge_string[i] = (BYTE)(6);}  // last character = custom right piece
            else 
            {                                                        // character in the middle, could be empty, tip or fill
                if (value_in_pixels <= i*5) {gauge_string[i] = (BYTE)(7);}   // empty character
                else if (value_in_pixels > i*5 && value_in_pixels < (i+1)*5) {gauge_string[i] = (BYTE)(5-move_offset);} // tip
                else {gauge_string[i] = (BYTE)(255);}                        // filled character
            }
        }
        // gauge drawing
        setcurLCD(0,0);                         // move cursor to top left
                         //"0123456789abcedf"                   
        CHARLCD1_PutString("System Booting. ",17);
        setcurLCD(15,0); 
        customChar(0);//lcd.write(byte(0));                         // print warning character      
        //-----------------------------------------------------
        setcurLCD(0,1);              // move the cursor to the next line
                         //"0123456789abcedf"                       
        //CHARLCD1_PutString("                ",17);         // display the gauge        
        sprintf(buffer, "Step:%02d         ", cpu_gauge/19);    // set a string as CPU: XX%, with the number always taking at least 3 character
                       //"0123456789abcedf"                       
                       //"Step:01         "
        CHARLCD1_PutString(buffer,17);         // display the gauge        
        //------------------------------------------------------
        setcurLCD(0,2);              // move the cursor to the next line
        CHARLCD1_PutString(gauge_string,17);         // display the gauge
        //-------------------------------------------------------------------
        setcurLCD(0,3);              // move the cursor to the next line
                      //"0123456789abcedf"                       
        //CHARLCD1_PutString("                ",17);         // display the gauge
                      //"0123456789abcedf"                       
                      //"Progress : 099% " 
        sprintf(buffer, "Progress : %3d%% ", cpu_gauge);    // set a string as CPU: XX%, with the number always taking at least 3 character
        
        CHARLCD1_PutString(buffer,17);         // display the gauge
                         //"0123456789abcedf"                   
        // increase the CPU value, set between 0-100
        cpu_gauge = cpu_gauge +1;
        //
        if (cpu_gauge > 100) 
        {
            return;
            cpu_gauge = 0;        
        }

        //__delay_ms(1);  // wait for a while - 100ms = update the screen 10x in a second
        switch(cpu_gauge/19)
        {
            case 0:
                M1_NFB_LED_On();
                M2_NFB_LED_On();
                DC1_NFB_LED_On();
                DC2_NFB_LED_On();
                
                break;
            case 1:
                M1_NFB_LED_Off();
                M2_NFB_LED_Off();
                DC1_NFB_LED_Off();
                DC2_NFB_LED_Off();
                //
                M1_LINE_LED_On();
                M2_LINE_LED_On();
                OUT_LINE_LED_On();
                break;
            case 2:
                M1_LINE_LED_Off();
                M2_LINE_LED_Off();
                OUT_LINE_LED_Off();
                //
                BATT_NFB_LED_On();
                LOAD_NFB_LED_On();
                NORMAL_LED_On();
                FAIL_LED_On();
                break;
            case 3:
                BATT_NFB_LED_Off();
                LOAD_NFB_LED_Off();
                NORMAL_LED_Off();
                FAIL_LED_Off();
                //
                AUTO_LED_On();
                MANUAL_LED_On();
                break;
            case 4:
                AUTO_LED_Off();
                MANUAL_LED_Off();
                //
                NO1_LED_On();
                NO2_LED_On();
                break;
            default:
                NO1_LED_Off();
                NO2_LED_Off();                
                break;
        }
    }    
    
}


void lcdInit(void)
{
    setcurLCD(0,0);
    CHARLCD1_PutString(lcdStr0, 17);        
    setcurLCD(0,1);
    CHARLCD1_PutString(lcdStr1, 17);        
    setcurLCD(0,2);
    CHARLCD1_PutString(lcdStr2, 17);        
    setcurLCD(0,3);
    CHARLCD1_PutString(lcdStr3, 17);        
}
//
//void lcdUpdate(unsigned char flag, unsigned char x, unsigned char y)
void lcdUpdate(void)
{
    //---------------------------------line#1
    setcurLCD(0,0);
#if(Debug == 1) 
    if(StatusBuf[6]==0)//StatusBuf[13]
    {
        lcdStr0[0]='O';
    }
    else
    {
        lcdStr0[0]='X';
    }
#endif    
    CHARLCD1_PutString(lcdStr0, 17);
    if(mainToggle)
    {
        setcurLCD(15,0);        
        customChar(0);//heart symbol//
    }
    //---------------------------------line#2
    setcurLCD(0,1);    
//    if(Module1_Sts[3]==1) lcdStr1[0] = 'X';
//    else lcdStr1[0] = 'O';
//    if(Module2_Sts[3]==1) lcdStr1[1] = 'X';
//    else lcdStr1[1] = 'O';
    //
    CHARLCD1_PutString(lcdStr1, 17);        
    //---------------------------------line#3
    setcurLCD(0,2);
    CHARLCD1_PutString(lcdStr2, 17);        
    //---------------------------------line#4
    setcurLCD(0,3);
    CHARLCD1_PutString(lcdStr3, 17);                
}

//void lcdSetValue(void)
//{
    /*
        //------------------------------------------------------------
                      //"0123456789abcdef"
                      //" V_OUT : 125[V] "
                      //" V_OUT :3.29[V] " //rxDataIndex   
                      //" A_OUT : 000[A] " //rxDataIndex   
        sprintf(lcdStr1," V_OUT :%1.2f[V] ",convVolt);
        sprintf(lcdStr2," A_OUT : %03d[A] ",rxDataIndex);
        sprintf(lcdStr3,"[%02d-%02d-%02d][%04d]",iHr,iMin,iSec,iMainCnt);        
        lcdUpdate();        
    }//if(iSecOld != iSec)    
    */
//}

void softClock(void)//1000ms(1second)
{
    if(iSec > 59)
    {        
        iSec = 0;
        if(iMin<59)iMin++;
        else
        {
            iMin=0;
            if(iHr<99)iHr++;
            else
            {
                iHr=0;
            }
        }//else
    }
}
//
void EEPROM_Initialize(void)
{
	if(EEPROMRead(INIT_STATE) != 0xAA)
	{
		EEPROMWrite(INIT_STATE,     0xAA); //first Time//
		//
        EEPROMWrite(ALARM_STOP,     1);//1 is buzzer on//
        EEPROMWrite(SYS_TYPE,       0);
        EEPROMWrite(BATT_TYPE,      0);
        EEPROMWrite(OP_MODE,        0);
        //    
		EEPROMWrite(PASSWD_0,       '0');
		EEPROMWrite(PASSWD_1,		'0');
		EEPROMWrite(PASSWD_2,		'0');
		EEPROMWrite(PASSWD_3,		'0');
		EEPROMWrite(PASSWD_4,		'0');
		EEPROMWrite(PASSWD_5,		'0');
		EEPROMWrite(PASSWD_6,		'0');
		EEPROMWrite(PASSWD_7,		'0');
		EEPROMWrite(PASSWD_8,		'0');
		EEPROMWrite(PASSWD_9,		'0');
		//
        if(EEPROMRead(BATT_TYPE) == 0)  //0 PB125v, 1 PB110, 2 LiIon
        {  //batt.volage 150/105v 
            EEPROMWrite(BATTVOLT_0,	'1');
            EEPROMWrite(BATTVOLT_1,	'5');
            EEPROMWrite(BATTVOLT_2,	'0');
            EEPROMWrite(BATTVOLT_3,	'0');
            EEPROMWrite(BATTVOLT_4,	'1');
            EEPROMWrite(BATTVOLT_5,	'0');
            EEPROMWrite(BATTVOLT_6,	'5');
            EEPROMWrite(BATTVOLT_7,	'0');
            EEPROMWrite(BATTVOLT_8,	'0');
            EEPROMWrite(BATTVOLT_9,	'0');
        }
        else if(EEPROMRead(BATT_TYPE)==1) //PB110V
        {//batt.voltage 150/100V
            EEPROMWrite(BATTVOLT_0,	'1');
            EEPROMWrite(BATTVOLT_1,	'5');
            EEPROMWrite(BATTVOLT_2,	'0');
            EEPROMWrite(BATTVOLT_3,	'0');
            EEPROMWrite(BATTVOLT_4,	'1');
            EEPROMWrite(BATTVOLT_5,	'0');
            EEPROMWrite(BATTVOLT_6,	'0');
            EEPROMWrite(BATTVOLT_7,	'0');
            EEPROMWrite(BATTVOLT_8,	'0');
            EEPROMWrite(BATTVOLT_9,	'0');
        }
        else
        {           //LituamIon - batt.voltage 135/96V
            EEPROMWrite(BATTVOLT_0,	'1');
            EEPROMWrite(BATTVOLT_1,	'3');
            EEPROMWrite(BATTVOLT_2,	'5');
            EEPROMWrite(BATTVOLT_3,	'0');
            EEPROMWrite(BATTVOLT_4,	'0');
            EEPROMWrite(BATTVOLT_5,	'9');
            EEPROMWrite(BATTVOLT_6,	'6');
            EEPROMWrite(BATTVOLT_7,	'0');
            EEPROMWrite(BATTVOLT_8,	'0');
            EEPROMWrite(BATTVOLT_9,	'0');
        }
		//
		EEPROMWrite(RESTARTTIME_0,			0);//1 to 0
		EEPROMWrite(RESTARTTIME_1,			0);
		//
	}
}
//
void ParameterInit(void)//read 
{
	gPass[0] = EEPROMRead(PASSWD_0);
	gPass[1] = EEPROMRead(PASSWD_1);
	gPass[2] = EEPROMRead(PASSWD_2);
	gPass[3] = EEPROMRead(PASSWD_3);
	gPass[4] = EEPROMRead(PASSWD_4);
	gPass[5] = EEPROMRead(PASSWD_5);
	gPass[6] = EEPROMRead(PASSWD_6);
	gPass[7] = EEPROMRead(PASSWD_7);
	gPass[8] = EEPROMRead(PASSWD_8);
	gPass[9] = EEPROMRead(PASSWD_9);
    
	fAlarmStop      = EEPROMRead(ALARM_STOP);
    gSysTypeMode    = EEPROMRead(SYS_TYPE);
    gBattTypeMode   = EEPROMRead(BATT_TYPE);
    gOperationMode  = EEPROMRead(OP_MODE);
	//
	gBattVoltSet[0] = EEPROMRead(BATTVOLT_0);
	gBattVoltSet[1] = EEPROMRead(BATTVOLT_1);
	gBattVoltSet[2] = EEPROMRead(BATTVOLT_2);
	gBattVoltSet[3] = EEPROMRead(BATTVOLT_3);
	gBattVoltSet[4] = EEPROMRead(BATTVOLT_4);
	gBattVoltSet[5] = EEPROMRead(BATTVOLT_5);
	gBattVoltSet[6] = EEPROMRead(BATTVOLT_6);
	gBattVoltSet[7] = EEPROMRead(BATTVOLT_7);
	gBattVoltSet[8] = EEPROMRead(BATTVOLT_8);
	gBattVoltSet[9] = EEPROMRead(BATTVOLT_9);
	//
	gRestartTime[0] = EEPROMRead(RESTARTTIME_0);
	gRestartTime[1] = EEPROMRead(RESTARTTIME_1);
}
//
void SystemInitialize(void)
{	
	EEPROMWrite(ALARM_STOP,            1); //1 is buzzer on
    EEPROMWrite(SYS_TYPE,              0);
    EEPROMWrite(BATT_TYPE,             0);
    EEPROMWrite(OP_MODE,               0);
    
    gSysTypeMode = EEPROMRead(SYS_TYPE);
    gBattTypeMode = EEPROMRead(BATT_TYPE);
    gOperationMode = EEPROMRead(OP_MODE);
    
    if(EEPROMRead(BATT_TYPE) == 0)  //0 PB125v, 1 PB110, 2 LiIon
        {  //batt.volage 150/105v 
            EEPROMWrite(BATTVOLT_0,	'1');
            EEPROMWrite(BATTVOLT_1,	'5');
            EEPROMWrite(BATTVOLT_2,	'0');
            EEPROMWrite(BATTVOLT_3,	'0');
            EEPROMWrite(BATTVOLT_4,	'1');
            EEPROMWrite(BATTVOLT_5,	'0');
            EEPROMWrite(BATTVOLT_6,	'5');
            EEPROMWrite(BATTVOLT_7,	'0');
            EEPROMWrite(BATTVOLT_8,	'0');
            EEPROMWrite(BATTVOLT_9,	'0');
        }
        else if(EEPROMRead(BATT_TYPE)==1) //PB110V
        {//batt.voltage 150/100V
            EEPROMWrite(BATTVOLT_0,	'1');
            EEPROMWrite(BATTVOLT_1,	'5');
            EEPROMWrite(BATTVOLT_2,	'0');
            EEPROMWrite(BATTVOLT_3,	'0');
            EEPROMWrite(BATTVOLT_4,	'1');
            EEPROMWrite(BATTVOLT_5,	'0');
            EEPROMWrite(BATTVOLT_6,	'0');
            EEPROMWrite(BATTVOLT_7,	'0');
            EEPROMWrite(BATTVOLT_8,	'0');
            EEPROMWrite(BATTVOLT_9,	'0');
        }
        else
        {           //LituamIon - batt.voltage 135/96V
            EEPROMWrite(BATTVOLT_0,	'1');
            EEPROMWrite(BATTVOLT_1,	'3');
            EEPROMWrite(BATTVOLT_2,	'5');
            EEPROMWrite(BATTVOLT_3,	'0');
            EEPROMWrite(BATTVOLT_4,	'0');
            EEPROMWrite(BATTVOLT_5,	'9');
            EEPROMWrite(BATTVOLT_6,	'6');
            EEPROMWrite(BATTVOLT_7,	'0');
            EEPROMWrite(BATTVOLT_8,	'0');
            EEPROMWrite(BATTVOLT_9,	'0');
        }
	//
	gBattVoltSet[0] = EEPROMRead(BATTVOLT_0);
	gBattVoltSet[1] = EEPROMRead(BATTVOLT_1);
	gBattVoltSet[2] = EEPROMRead(BATTVOLT_2);
	gBattVoltSet[3] = EEPROMRead(BATTVOLT_3);
	gBattVoltSet[4] = EEPROMRead(BATTVOLT_4);
	gBattVoltSet[5] = EEPROMRead(BATTVOLT_5);
	gBattVoltSet[6] = EEPROMRead(BATTVOLT_6);
	gBattVoltSet[7] = EEPROMRead(BATTVOLT_7);
	gBattVoltSet[8] = EEPROMRead(BATTVOLT_8);
	gBattVoltSet[9] = EEPROMRead(BATTVOLT_9);
	//
	EEPROMWrite(RESTARTTIME_0,			0);//0 min
	EEPROMWrite(RESTARTTIME_1,			0);
	//
	gRestartTime[0] = EEPROMRead(RESTARTTIME_0);
	gRestartTime[1] = EEPROMRead(RESTARTTIME_1);
}



//------------------------------------------------------------------------------
void keyTest()//Not use for debug//
{
    iTestVal = hKey();          
    if(iTestVal > 0)
    {   
        iMainTmp = iTestVal;
        switch(iTestVal)
        {                
            case keyKEY_UP://3              
                iMainTmp = 3;
                lcdStr1[0] = 'U';
                lcdStr1[1] = 'p';
                lcdStr1[2] = ' ';
                lcdStr1[3] = ' ';
                lcdStr1[4] = ' ';
                lcdStr1[5] = ' ';
                break;
            case keyKEY_DOWN://5                                        
                iMainTmp = 5;
                lcdStr1[0] = 'D';
                lcdStr1[1] = 'o';
                lcdStr1[2] = 'w';
                lcdStr1[3] = 'n';
                lcdStr1[4] = ' ';
                lcdStr1[5] = ' ';
                break;
            case keyKEY_LEFT://2
                iMainTmp = 2;
                lcdStr1[0] = 'L';
                lcdStr1[1] = 'e';
                lcdStr1[2] = 'f';
                lcdStr1[3] = 't';
                lcdStr1[4] = ' ';
                lcdStr1[5] = ' ';
                break;
            case keyKEY_RIGHT://4                    
                iMainTmp = 4;
                lcdStr1[0] = 'R';
                lcdStr1[1] = 'i';
                lcdStr1[2] = 'g';
                lcdStr1[3] = 'h';
                lcdStr1[4] = 't';
                lcdStr1[5] = ' ';
                break;
            case keyKEY_BZ_OFF://9
                iMainTmp = 9;
                break;
            case keyKEY_BZ_OFFfast://18
                iMainTmp = 18;
                break;
            case keyKEY_ENTERfast://10
                iMainTmp = 10;
                break;
            case keyKEY_ENTER://1
                iMainTmp = 1;
                lcdStr1[0] = 'E';
                lcdStr1[1] = 'n';
                lcdStr1[2] = 't';
                lcdStr1[3] = 'e';
                lcdStr1[4] = 'r';
                lcdStr1[5] = ' ';
                break;            
            case keyKEY_AUTOMANUAL://6
                lcdStr1[0] = 'A';
                lcdStr1[1] = 't';
                lcdStr1[2] = '/';
                lcdStr1[3] = 'M';
                lcdStr1[4] = 'n';
                lcdStr1[5] = ' ';
                iMainTmp = 6;
                break;
            case keyKEY_NO_SELECT://7
                iMainTmp = 7;
                lcdStr1[0] = 'N';
                lcdStr1[1] = 'o';
                lcdStr1[2] = 'S';
                lcdStr1[3] = 'e';
                lcdStr1[4] = 'l';
                lcdStr1[5] = ' ';
                break;    
            default:
                break;			
        }
    }
            
}


void eepromTest(void)
{   
    unsigned int i;
    for(i=0;i<2000;i++)
    {        
        EEPROMWrite(i, 0);//(unsigned int address,unsigned char value//) 0x40(@)
    }
}
/**
 End of File
*/

