#include "mcc_generated_files/pin_manager.h"
#include	<stdio.h>
#include 	"sysinfo.h"
#include "i2c_master_eeprom_24fc256.h"
#include 	"Definition.h"
#include    "Initialize.h"
#include    "Alram_Sts.h"
#include    "RS232.h"


void ParameterInitialize(void)
{
	gPin[0] = EEPROMRead(PASSWD_0);
	gPin[1] = EEPROMRead(PASSWD_1);
	gPin[2] = EEPROMRead(PASSWD_2);
	gPin[3] = EEPROMRead(PASSWD_3);
	gPin[4] = EEPROMRead(PASSWD_4);
	gPin[5] = EEPROMRead(PASSWD_5);
	gPin[6] = EEPROMRead(PASSWD_6);
	gPin[7] = EEPROMRead(PASSWD_7);
	gPin[8] = EEPROMRead(PASSWD_8);
	gPin[9] = EEPROMRead(PASSWD_9);
    
    gPin2[0] = '1';
	gPin2[1] = '1';
	gPin2[2] = '1';
	gPin2[3] = '1';
	gPin2[4] = '1';
	gPin2[5] = '1';
	gPin2[6] = '1';
	gPin2[7] = '1';
	gPin2[8] = '1';
	gPin2[9] = '1';
    		
	fAlarmStop = EEPROMRead(ALARM_STOP);
    
    if(EEPROMRead(BATT_TYPE) == 0) 
    {
        CHG_Sts.b4 = 0;
        CHG_Sts.b3 = 0; 
    }        
    else if(EEPROMRead(BATT_TYPE) == 1)
    {
        CHG_Sts.b4 = 0;
        CHG_Sts.b3 = 1; 
    }
    else if(EEPROMRead(BATT_TYPE) == 2)
    {
        CHG_Sts.b4 = 1;
        CHG_Sts.b3 = 0; 
    }
    else //add goooo
    {
        EEPROMWrite(BATT_TYPE,0);
        CHG_Sts.b4 = 0;
        CHG_Sts.b3 = 0; 
    }
    
	goutValarm[0] = EEPROMRead(OUTVALARM_0);
	goutValarm[1] = EEPROMRead(OUTVALARM_1);
	goutValarm[2] = EEPROMRead(OUTVALARM_2);
	goutValarm[3] = EEPROMRead(OUTVALARM_3);
	goutValarm[4] = EEPROMRead(OUTVALARM_4);
	goutValarm[5] = EEPROMRead(OUTVALARM_5);
	goutValarm[6] = EEPROMRead(OUTVALARM_6);
	goutValarm[7] = EEPROMRead(OUTVALARM_7);
	goutValarm[8] = EEPROMRead(OUTVALARM_8);
	goutValarm[9] = EEPROMRead(OUTVALARM_9);
    //
	goutAalarm[0] = EEPROMRead(OUTAALARM_0);
	goutAalarm[1] = EEPROMRead(OUTAALARM_1);
	goutAalarm[2] = EEPROMRead(OUTAALARM_2);
	goutAalarm[3] = EEPROMRead(OUTAALARM_3);
	goutAalarm[4] = EEPROMRead(OUTAALARM_4);
	goutAalarm[5] = EEPROMRead(OUTAALARM_5);
	goutAalarm[6] = EEPROMRead(OUTAALARM_6);
	goutAalarm[7] = EEPROMRead(OUTAALARM_7);
	goutAalarm[8] = EEPROMRead(OUTAALARM_8);
	goutAalarm[9] = EEPROMRead(OUTAALARM_9);
	//
	battVoltSet[0] = EEPROMRead(BATTVOLT_0);
	battVoltSet[1] = EEPROMRead(BATTVOLT_1);
	battVoltSet[2] = EEPROMRead(BATTVOLT_2);
	battVoltSet[3] = EEPROMRead(BATTVOLT_3);
	battVoltSet[4] = EEPROMRead(BATTVOLT_4);
	battVoltSet[5] = EEPROMRead(BATTVOLT_5);
	battVoltSet[6] = EEPROMRead(BATTVOLT_6);
	battVoltSet[7] = EEPROMRead(BATTVOLT_7);
    //
    LoadAmpSet[0] = EEPROMRead(LOADAMP_0);
	LoadAmpSet[1] = EEPROMRead(LOADAMP_1);
	LoadAmpSet[2] = EEPROMRead(LOADAMP_2);
    LoadAmpSet[3] = EEPROMRead(LOADAMP_3);

	fbattVoltSet=1;
	battAmpSet[0] = EEPROMRead(BATTAMP_0);
	battAmpSet[1] = EEPROMRead(BATTAMP_1);
	battAmpSet[2] = EEPROMRead(BATTAMP_2);
	//input alarm
	ginValarm[0] = EEPROMRead(INVALARM_0);
	ginValarm[1] = EEPROMRead(INVALARM_1);
	ginValarm[2] = EEPROMRead(INVALARM_2);
	ginValarm[3] = EEPROMRead(INVALARM_3);
	ginValarm[4] = EEPROMRead(INVALARM_4);
	ginValarm[5] = EEPROMRead(INVALARM_5);
	ginValarm[6] = EEPROMRead(INVALARM_6);
	ginValarm[7] = EEPROMRead(INVALARM_7);
	ginValarm[8] = EEPROMRead(INVALARM_8);
	ginValarm[9] = EEPROMRead(INVALARM_9);
	//shutDownOutV
	shutDownOutV[0] = EEPROMRead(SHUTDOWNOUTV_0);
	shutDownOutV[1] = EEPROMRead(SHUTDOWNOUTV_1);
	shutDownOutV[2] = EEPROMRead(SHUTDOWNOUTV_2);
	shutDownOutV[3] = EEPROMRead(SHUTDOWNOUTV_3);
	shutDownOutV[4] = EEPROMRead(SHUTDOWNOUTV_4);
	shutDownOutV[5] = EEPROMRead(SHUTDOWNOUTV_5);
	shutDownOutV[6] = EEPROMRead(SHUTDOWNOUTV_6);
	shutDownOutV[7] = EEPROMRead(SHUTDOWNOUTV_7);
	shutDownOutV[8] = EEPROMRead(SHUTDOWNOUTV_8);
	shutDownOutV[9] = EEPROMRead(SHUTDOWNOUTV_9);
	//shutDownOutA
	shutDownOutA[0] = EEPROMRead(SHUTDOWNOUTA_0);
	shutDownOutA[1] = EEPROMRead(SHUTDOWNOUTA_1);
	shutDownOutA[2] = EEPROMRead(SHUTDOWNOUTA_2);
	shutDownOutA[3] = EEPROMRead(SHUTDOWNOUTA_3);
	shutDownOutA[4] = EEPROMRead(SHUTDOWNOUTA_4);
	shutDownOutA[5] = EEPROMRead(SHUTDOWNOUTA_5);
	shutDownOutA[6] = EEPROMRead(SHUTDOWNOUTA_6);
	shutDownOutA[7] = EEPROMRead(SHUTDOWNOUTA_7);
	shutDownOutA[8] = EEPROMRead(SHUTDOWNOUTA_8);
	shutDownOutA[9] = EEPROMRead(SHUTDOWNOUTA_9);
	//ADC Gain
	ADJ_VGain = EEPROMRead(ADJVGain_0);
	ADJ_CGain = EEPROMRead(ADJCGain_0);
    
    Set_time[0] = EEPROMRead(CHG_SET_NO) + 0x30;
    CHG_Cut_TIME = EEPROMRead(CHG_SET_TIME);
    CHG_Cut_CURR = EEPROMRead(CHG_SET_CURR);
    Set_COND[0] = (EEPROMRead(CHG_SET_TIME)/10) + 0x30;
    Set_COND[1] = (EEPROMRead(CHG_SET_TIME)%10) + 0x30;
    Set_COND[2] = (EEPROMRead(CHG_SET_CURR)/10) + 0x30;
    Set_COND[3] = (EEPROMRead(CHG_SET_CURR)%10) + 0x30; 
    
	lcd_refresh();
	//iMenuIndex=MENU_CHARGER_INFO;
	//iMenuIndex = MENU_ADCGAIN;	
	year = rm_bcd(read_ds1302(0x8d));
	if(!year)
	{	
		rtc_set_datetime(21,8,22,0,17,59);//(day,mth,year,dow,hour,min)
	}	
    if(EEPROMRead(INPUT_VOLT) == 0)
    {
        Input_UV_Ref = MinInputVoltage_380;	//18.06.18 -10% - KHJ
        Input_OV_Ref = MaxInputVoltage_380;  	//18.06.18 +10% - KHJ
        Input_V_Ref = RefInputVoltage_380;
        Input_Shutdown_Ref = ShutdownVoltage_380;   //18.06.18 -40% - Order by GJJ 
    }
    else
    {
        Input_UV_Ref = MinInputVoltage_220;	//18.06.18 -10% - KHJ
        Input_OV_Ref = MaxInputVoltage_220;  	//18.06.18 +10% - KHJ
        Input_V_Ref = RefInputVoltage_220;
        Input_Shutdown_Ref = ShutdownVoltage_220;   //18.06.18 -40% - Order by GJJ 
    }
    //Init Value
	Set_ARef = (((goutAalarm[0]-0x30)*100)+((goutAalarm[1]-0x30)*10)+((goutAalarm[2]-0x30)));
	Set_CL = ((LoadAmpSet[0]-0x30)*100)+((LoadAmpSet[1]-0x30)*10)+(LoadAmpSet[2]-0x30);
	Set_VRef = ((battVoltSet[4]-0x30)*100)+((battVoltSet[5]-0x30)*10)+((battVoltSet[6]-0x30)*1)+((battVoltSet[7]-0x30)*0.1);
    Set_OV = ((shutDownOutV[0]-0x30)*100)+((shutDownOutV[1]-0x30)*10)+((shutDownOutV[2]-0x30)*1)+((shutDownOutV[3]-0x30)*0.1);
	Set_UV = ((shutDownOutV[4]-0x30)*100)+((shutDownOutV[5]-0x30)*10)+((shutDownOutV[6]-0x30)*1)+((shutDownOutV[7]-0x30)*0.1);
	Set_Input_OV = ((ginValarm[0]-0x30)*100)+((ginValarm[1]-0x30)*10)+((ginValarm[2]-0x30)*1)+((ginValarm[3]-0x30)*0.1);
	Set_Input_UV = ((ginValarm[4]-0x30)*100)+((ginValarm[5]-0x30)*10)+((ginValarm[6]-0x30)*1)+((ginValarm[7]-0x30)*0.1);
	Set_Output_OV = ((goutValarm[0]-0x30)*100)+((goutValarm[1]-0x30)*10)+((goutValarm[2]-0x30)*1)+((goutValarm[3]-0x30)*0.1);
	Set_Output_UV = ((goutValarm[4]-0x30)*100)+((goutValarm[5]-0x30)*10)+((goutValarm[6]-0x30)*1)+((goutValarm[7]-0x30)*0.1);
	Set_OT = OverTempLimitmax;
	//BL = 0;
    Set_BattAmp=((battAmpSet[0] & 0x0F)*100) +
                ((battAmpSet[1] & 0x0F)*10)+ 
                ((battAmpSet[2] & 0x0F)*1)+
                ((battAmpSet[3] & 0x0F)*0.1);
}

void sysInit()
{
    EEPROMWrite(ALARM_STOP,		'Y');
    
	EEPROMWrite(ALARM_TEMP_0,		'0');
	EEPROMWrite(ALARM_TEMP_1,		'9');
	EEPROMWrite(ALARM_TEMP_2,		'5');
	EEPROMWrite(ALARM_TEMP_3,		'0');
    
	//change volt//
    if(EEPROMRead(BATT_TYPE) == 0)//pb125V
    {
        EEPROMWrite(SHUTDOWNOUTV_0,    '1');
        EEPROMWrite(SHUTDOWNOUTV_1,    '5');
        EEPROMWrite(SHUTDOWNOUTV_2,    '0');
        EEPROMWrite(SHUTDOWNOUTV_3,    '0');
        EEPROMWrite(SHUTDOWNOUTV_4,    '0');    
        EEPROMWrite(SHUTDOWNOUTV_5,    '9');
        EEPROMWrite(SHUTDOWNOUTV_6,    '0');
        EEPROMWrite(SHUTDOWNOUTV_7,    '0');

        EEPROMWrite(BATTVOLT_0,        '1');
        EEPROMWrite(BATTVOLT_1,        '3');
        EEPROMWrite(BATTVOLT_2,        '1');
        EEPROMWrite(BATTVOLT_3,        '0');
        EEPROMWrite(BATTVOLT_4,        '1');
        EEPROMWrite(BATTVOLT_5,        '3');
        EEPROMWrite(BATTVOLT_6,        '1');
        EEPROMWrite(BATTVOLT_7,        '0');
        //output volt alarm(hi/low)
        EEPROMWrite(OUTVALARM_0,		'1');
        EEPROMWrite(OUTVALARM_1,		'5');
        EEPROMWrite(OUTVALARM_2,		'0');
        EEPROMWrite(OUTVALARM_3,		'0');
        EEPROMWrite(OUTVALARM_4,		'1');
        EEPROMWrite(OUTVALARM_5,		'1');
        EEPROMWrite(OUTVALARM_6,		'0');
        EEPROMWrite(OUTVALARM_7,		'0');
    }
    else if(EEPROMRead(BATT_TYPE) == 1) //PB110V
    {   //150.0 ~ 90.0V
        EEPROMWrite(SHUTDOWNOUTV_0,    '1');
        EEPROMWrite(SHUTDOWNOUTV_1,    '5');
        EEPROMWrite(SHUTDOWNOUTV_2,    '0');
        EEPROMWrite(SHUTDOWNOUTV_3,    '0');
        EEPROMWrite(SHUTDOWNOUTV_4,    '0');    
        EEPROMWrite(SHUTDOWNOUTV_5,    '9');
        EEPROMWrite(SHUTDOWNOUTV_6,    '0');
        EEPROMWrite(SHUTDOWNOUTV_7,    '0');
        //charge volt 124,124//
        EEPROMWrite(BATTVOLT_0,        '1');
        EEPROMWrite(BATTVOLT_1,        '2');
        EEPROMWrite(BATTVOLT_2,        '4');
        EEPROMWrite(BATTVOLT_3,        '0');
        EEPROMWrite(BATTVOLT_4,        '1');
        EEPROMWrite(BATTVOLT_5,        '2');
        EEPROMWrite(BATTVOLT_6,        '4');
        EEPROMWrite(BATTVOLT_7,        '0');
        //out volt alarm 150,100//
        EEPROMWrite(OUTVALARM_0,		'1');
        EEPROMWrite(OUTVALARM_1,		'5');
        EEPROMWrite(OUTVALARM_2,		'0');
        EEPROMWrite(OUTVALARM_3,		'0');
        EEPROMWrite(OUTVALARM_4,		'1');
        EEPROMWrite(OUTVALARM_5,		'0');
        EEPROMWrite(OUTVALARM_6,		'0');
        EEPROMWrite(OUTVALARM_7,		'0');
    }
    else if(EEPROMRead(BATT_TYPE) == 2) //Li ion
    {
        EEPROMWrite(SHUTDOWNOUTV_0,    '1');
        EEPROMWrite(SHUTDOWNOUTV_1,    '3');
        EEPROMWrite(SHUTDOWNOUTV_2,    '5');
        EEPROMWrite(SHUTDOWNOUTV_3,    '0');
        EEPROMWrite(SHUTDOWNOUTV_4,    '0');    
        EEPROMWrite(SHUTDOWNOUTV_5,    '9');
        EEPROMWrite(SHUTDOWNOUTV_6,    '6');
        EEPROMWrite(SHUTDOWNOUTV_7,    '0');                                   

        EEPROMWrite(BATTVOLT_0,        '1');
        EEPROMWrite(BATTVOLT_1,        '2');
        EEPROMWrite(BATTVOLT_2,        '5');
        EEPROMWrite(BATTVOLT_3,        '0');
        EEPROMWrite(BATTVOLT_4,        '1');
        EEPROMWrite(BATTVOLT_5,        '2');
        EEPROMWrite(BATTVOLT_6,        '5');
        EEPROMWrite(BATTVOLT_7,        '0');

        EEPROMWrite(OUTVALARM_0,		'1');
        EEPROMWrite(OUTVALARM_1,		'3');
        EEPROMWrite(OUTVALARM_2,		'5');
        EEPROMWrite(OUTVALARM_3,		'0');
        EEPROMWrite(OUTVALARM_4,		'0');
        EEPROMWrite(OUTVALARM_5,		'9');
        EEPROMWrite(OUTVALARM_6,		'6');
        EEPROMWrite(OUTVALARM_7,		'0');
    }
    else//add goooo
    {
        EEPROMWrite(BATT_TYPE,0);
        EEPROMWrite(SHUTDOWNOUTV_0,    '1');
        EEPROMWrite(SHUTDOWNOUTV_1,    '5');
        EEPROMWrite(SHUTDOWNOUTV_2,    '0');
        EEPROMWrite(SHUTDOWNOUTV_3,    '0');
        EEPROMWrite(SHUTDOWNOUTV_4,    '0');    
        EEPROMWrite(SHUTDOWNOUTV_5,    '9');
        EEPROMWrite(SHUTDOWNOUTV_6,    '0');
        EEPROMWrite(SHUTDOWNOUTV_7,    '0');

        EEPROMWrite(BATTVOLT_0,        '1');
        EEPROMWrite(BATTVOLT_1,        '3');
        EEPROMWrite(BATTVOLT_2,        '1');
        EEPROMWrite(BATTVOLT_3,        '0');
        EEPROMWrite(BATTVOLT_4,        '1');
        EEPROMWrite(BATTVOLT_5,        '3');
        EEPROMWrite(BATTVOLT_6,        '1');
        EEPROMWrite(BATTVOLT_7,        '0');

        EEPROMWrite(OUTVALARM_0,		'1');
        EEPROMWrite(OUTVALARM_1,		'5');
        EEPROMWrite(OUTVALARM_2,		'0');
        EEPROMWrite(OUTVALARM_3,		'0');
        EEPROMWrite(OUTVALARM_4,		'1');
        EEPROMWrite(OUTVALARM_5,		'1');
        EEPROMWrite(OUTVALARM_6,		'0');
        EEPROMWrite(OUTVALARM_7,		'0');
    }
       
	//Out Current alarm
	EEPROMWrite(OUTAALARM_0,		'1');
	EEPROMWrite(OUTAALARM_1,		'1');
	EEPROMWrite(OUTAALARM_2,		'0');
	EEPROMWrite(OUTAALARM_3,		'0');
	EEPROMWrite(OUTAALARM_4,		'0');
	EEPROMWrite(OUTAALARM_5,		'8');
	EEPROMWrite(OUTAALARM_6,		'0');
	EEPROMWrite(OUTAALARM_7,		'0');
	EEPROMWrite(OUTAALARM_8,		'0');
	EEPROMWrite(OUTAALARM_9,		'0');

    //LoadAmpSet add 
	EEPROMWrite(LOADAMP_0,   	 	'1');
	EEPROMWrite(LOADAMP_1,			'2');
	EEPROMWrite(LOADAMP_2,			'0');
	EEPROMWrite(LOADAMP_3,			'0');
      
	//BattAmpSet add JSS 2018.06.11
	EEPROMWrite(BATTAMP_0,   	 	'0');
	EEPROMWrite(BATTAMP_1,			'3');
	EEPROMWrite(BATTAMP_2,			'0');
	EEPROMWrite(BATTAMP_3,			'0');
    
	//Input Voltage
    if(EEPROMRead(INPUT_VOLT) == 0)     //0 is 380v, 1 is 220v
    {
        EEPROMWrite(INVALARM_0,    	'4');
        EEPROMWrite(INVALARM_1,		'1');
        EEPROMWrite(INVALARM_2,		'8');
        EEPROMWrite(INVALARM_3,		'0');
        EEPROMWrite(INVALARM_4,		'3');
        EEPROMWrite(INVALARM_5,		'4');
        EEPROMWrite(INVALARM_6,		'2');
        EEPROMWrite(INVALARM_7,		'0');
        EEPROMWrite(INVALARM_8,		'0');
        EEPROMWrite(INVALARM_9,		'0');
    }
    else if(EEPROMRead(INPUT_VOLT) == 1)  //1 phase 220v
    {
        EEPROMWrite(INVALARM_0,    	'2');
        EEPROMWrite(INVALARM_1,		'4');
        EEPROMWrite(INVALARM_2,		'2');
        EEPROMWrite(INVALARM_3,		'0');
        EEPROMWrite(INVALARM_4,		'1');
        EEPROMWrite(INVALARM_5,		'9');
        EEPROMWrite(INVALARM_6,		'8');
        EEPROMWrite(INVALARM_7,		'0');
        EEPROMWrite(INVALARM_8,		'0');
        EEPROMWrite(INVALARM_9,		'0');
    }
    else//add goooo
    {
        EEPROMWrite(INPUT_VOLT,     0x00);
        EEPROMWrite(INVALARM_0,    	'4');
        EEPROMWrite(INVALARM_1,		'1');
        EEPROMWrite(INVALARM_2,		'8');
        EEPROMWrite(INVALARM_3,		'0');
        EEPROMWrite(INVALARM_4,		'3');
        EEPROMWrite(INVALARM_5,		'4');
        EEPROMWrite(INVALARM_6,		'2');
        EEPROMWrite(INVALARM_7,		'0');
        EEPROMWrite(INVALARM_8,		'0');
        EEPROMWrite(INVALARM_9,		'0');
    }
	//overload shutdown -120A//
	EEPROMWrite(SHUTDOWNOUTA_0, '1');
	EEPROMWrite(SHUTDOWNOUTA_1,	'2');
	EEPROMWrite(SHUTDOWNOUTA_2,	'0');
	EEPROMWrite(SHUTDOWNOUTA_3,	'0');
	EEPROMWrite(SHUTDOWNOUTA_4,	'0');
	EEPROMWrite(SHUTDOWNOUTA_5,	'0');
	EEPROMWrite(SHUTDOWNOUTA_6,	'0');
	EEPROMWrite(SHUTDOWNOUTA_7,	'0');
	EEPROMWrite(SHUTDOWNOUTA_8,	'0');
	EEPROMWrite(SHUTDOWNOUTA_9,	'0');
    //adc gain//    
    EEPROMWrite(ADJVGain_0,	 	0x00);
	EEPROMWrite(ADJCGain_0,		0x00);
    //
    EEPROMWrite(CHG_STS_0,         0x00);
	EEPROMWrite(CHG_STS_1,         0x00);
    //
    EEPROMWrite(CHG_SET_NO,        0x00);
    EEPROMWrite(CHG_SET_TIME,      0x01); //1 Hour//
    EEPROMWrite(CHG_SET_CURR,      0x05); //5 Ampere//
    //
    EEPROMWrite(SET_YEAR_0,        0x01);
    EEPROMWrite(SET_MONTH_0,       0x01);
    EEPROMWrite(SET_DAY_0,         0x01);
    EEPROMWrite(SET_HOUR_0,        0x00);
    EEPROMWrite(SET_MIN_0,         0x00);
    EEPROMWrite(SET_SEC_0,         0x00);
    
    EEPROMWrite(SET_YEAR_1,        0x01);
    EEPROMWrite(SET_MONTH_1,       0x01);
    EEPROMWrite(SET_DAY_1,         0x01);
    EEPROMWrite(SET_HOUR_1,        0x00);
    EEPROMWrite(SET_MIN_1,         0x00);
    EEPROMWrite(SET_SEC_1,         0x00);
    
    EEPROMWrite(SET_YEAR_2,        0x01);
    EEPROMWrite(SET_MONTH_2,       0x01);
    EEPROMWrite(SET_DAY_2,         0x01);
    EEPROMWrite(SET_HOUR_2,        0x00);
    EEPROMWrite(SET_MIN_2,         0x00);
    EEPROMWrite(SET_SEC_2,         0x00);
    
    EEPROMWrite(SET_YEAR_3,        0x01);
    EEPROMWrite(SET_MONTH_3,       0x01);
    EEPROMWrite(SET_DAY_3,         0x01);
    EEPROMWrite(SET_HOUR_3,        0x00);
    EEPROMWrite(SET_MIN_3,         0x00);
    EEPROMWrite(SET_SEC_3,         0x00);
    
    EEPROMWrite(SET_YEAR_4,        0x01);
    EEPROMWrite(SET_MONTH_4,       0x01);
    EEPROMWrite(SET_DAY_4,         0x01);
    EEPROMWrite(SET_HOUR_4,        0x00);
    EEPROMWrite(SET_MIN_4,         0x00);
    EEPROMWrite(SET_SEC_4,         0x00);
    
    EEPROMWrite(SET_YEAR_5,        0x01);
    EEPROMWrite(SET_MONTH_5,       0x01);
    EEPROMWrite(SET_DAY_5,         0x01);
    EEPROMWrite(SET_HOUR_5,        0x00);
    EEPROMWrite(SET_MIN_5,         0x00);
    EEPROMWrite(SET_SEC_5,         0x00);
    
    EEPROMWrite(SET_YEAR_6,        0x01);
    EEPROMWrite(SET_MONTH_6,       0x01);
    EEPROMWrite(SET_DAY_6,         0x01);
    EEPROMWrite(SET_HOUR_6,        0x00);
    EEPROMWrite(SET_MIN_6,         0x00);
    EEPROMWrite(SET_SEC_6,         0x00);
    
    EEPROMWrite(SET_YEAR_7,        0x01);
    EEPROMWrite(SET_MONTH_7,       0x01);
    EEPROMWrite(SET_DAY_7,         0x01);
    EEPROMWrite(SET_HOUR_7,        0x00);
    EEPROMWrite(SET_MIN_7,         0x00);
    EEPROMWrite(SET_SEC_7,         0x00);
    
    EEPROMWrite(SET_YEAR_8,        0x01);
    EEPROMWrite(SET_MONTH_8,       0x01);
    EEPROMWrite(SET_DAY_8,         0x01);
    EEPROMWrite(SET_HOUR_8,        0x00);
    EEPROMWrite(SET_MIN_8,         0x00);
    EEPROMWrite(SET_SEC_8,         0x00);
    
    EEPROMWrite(SET_YEAR_9,        0x01);
    EEPROMWrite(SET_MONTH_9,       0x01);
    EEPROMWrite(SET_DAY_9,         0x01);
    EEPROMWrite(SET_HOUR_9,        0x00);
    EEPROMWrite(SET_MIN_9,         0x00);
    EEPROMWrite(SET_SEC_9,         0x00);
    
}

void EEPROM_Initialize(void)
{
	if(EEPROMRead(INIT_STATE) != 0xAA)    // Only one after download
	{
		EEPROMWrite(INIT_STATE, 		0xAA);
		EEPROMWrite(PASSWD_0,			'0');
		EEPROMWrite(PASSWD_1,			'0');
		EEPROMWrite(PASSWD_2,		    '0');
		EEPROMWrite(PASSWD_3,		    '0');
		EEPROMWrite(PASSWD_4,		    '0');
		EEPROMWrite(PASSWD_5,		    '0');
		EEPROMWrite(PASSWD_6,		    '0');
		EEPROMWrite(PASSWD_7,		    '0');
		EEPROMWrite(PASSWD_8,		    '0');
		EEPROMWrite(PASSWD_9,		    '0');
        EEPROMWrite(INPUT_VOLT,0x00);   // add goooo
        sysInit();		
	}
    else
    {   
        EEPROMWrite(OP_MODE,   	 	'N');
        EEPROMWrite(CHG_STS_0,   	 	0x00);
        EEPROMWrite(CHG_STS_1,         0x00);
    }
}

