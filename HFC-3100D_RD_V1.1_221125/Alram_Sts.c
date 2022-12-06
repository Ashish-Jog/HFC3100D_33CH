/* 
 * File:   Alram_Sts.h
 * Author: KHJ
 * goooo
 *
 * Created on 2018? 2? 13? (?), ?? 2:00
 */
#include    "mcc_generated_files/adc1.h"
//#include 	"eeprom.h"
#include "i2c_master_eeprom_24fc256.h"
#include    "Definition.h"
#include    "Initialize.h"
#include    "Alram_Sts.h"
#include    "RS232.h"

void Alram_Sts(void);
void CHG_CMD(void);
void CHG_Check(void);
void CHG_Time(void);
void hex16_3digt(unsigned char hex_data_8);
void hex16_4digt(unsigned int hex_data);

float Input_UV_Ref = 0;	
float Input_OV_Ref = 0;  	
float Input_V_Ref = 0;
float Input_Shutdown_Ref = 0; 

UINT8   CHG_Mode = 0;
UINT8   CHG_SEL = 0x00;
UINT8   CHG_EN = 0x00;
UINT8   Input_OV = 0x00;
UINT8   Input_UV = 0x00;
UINT8   Input_Shutdown = 0x00;

UINT8   Res_Year[10] = {0,}; 
UINT8   Res_Month[10] = {0,};
UINT8   Res_Day[10] = {0,};
UINT8   Res_Hour[10] = {0,}; 
UINT8   Res_Min[10] = {0,};
UINT8   Res_Sec[10] = {0,};

void Alram_Sts(void)
{
//    ACV_R_filter=381.1;
//    ACV_S_filter=382.1;
//    ACV_T_filter=383.1;
    //if(ACV_R_filter >= Set_Input_OV || ACV_S_filter >= Set_Input_OV || ACV_T_filter >= Set_Input_OV)
    if(ACV_R_filter >= 418.0 || ACV_S_filter >= 418.0  || ACV_T_filter >= 418.0 )
    {
        Input_OV = 0x80;//0x1000-0000//over voltage//
        //Input_OV = 0x00;
    }
    //else if((ACV_R_filter >= Input_Shutdown_Ref || ACV_S_filter >= Input_Shutdown_Ref || ACV_T_filter >= Input_Shutdown_Ref) && 
    //                            (ACV_R_filter <= Set_Input_UV || ACV_S_filter <= Set_Input_UV || ACV_T_filter <= Set_Input_UV))
    else if((ACV_R_filter >=Input_Shutdown_Ref || ACV_S_filter >= Input_Shutdown_Ref || ACV_T_filter >= Input_Shutdown_Ref) &&
                (ACV_R_filter <= Set_Input_UV || ACV_S_filter <= Set_Input_UV || ACV_T_filter <= Set_Input_UV))
    {
        Input_UV = 0x40;//0x0100-0000//under voltage//
        //Input_UV = 0x00;
    }
    //else if(ACV_R_filter <= Input_Shutdown_Ref || ACV_S_filter <= Input_Shutdown_Ref || ACV_T_filter <= Input_Shutdown_Ref)
    else if(ACV_R_filter <= 228.0 || ACV_S_filter <= 228.0 || ACV_T_filter <= 228.0)
    {
        Input_Shutdown = 0x20;//0x0010-0000//shutdown voltage//
        //Input_Shutdown = 0x00;
    }
    else
    {
        Input_OV = 0x00;
        Input_UV = 0x00;
        Input_Shutdown = 0x00;
    }    
    
}
    
void CHG_CMD(void)
{
    if(CHG_SEL == 0 && CHG_Mode == 0)
    {
        Set_VRef = ((battVoltSet[4] & 0x0f)*100)+((battVoltSet[5] & 0x0f)*10)+((battVoltSet[6] & 0x0f)*1)+((battVoltSet[7] & 0x0f)*0.1);
        CHG_Sts.b7 = 0;
        CHG_Sts.b6 = 0;
        Manual_LED = OFF;
        Float_LED = OFF;
        Equal_LED = ON;
    }
    else if(CHG_SEL == 0 && CHG_Mode == 1)
    {
        Set_VRef = ((battVoltSet[0] & 0x0f)*100)+((battVoltSet[1] & 0x0f)*10)+((battVoltSet[2] & 0x0f)*1)+((battVoltSet[3] & 0x0f)*0.1);
        CHG_Sts.b7 = 0;
        CHG_Sts.b6 = 1;
        Manual_LED = OFF;
        Float_LED = ON;
        Equal_LED = OFF;
    }
    else if(CHG_SEL == 1 && CHG_Mode == 0)
    {
        if(CHG_EN == 0)
        {
            Set_VRef = ((battVoltSet[4] & 0x0f)*100)+((battVoltSet[5] & 0x0f)*10)+((battVoltSet[6] & 0x0f)*1)+((battVoltSet[7] & 0x0f)*0.1);
        }
        else
        {
            Set_VRef = ((outSetNew[0]-0x30)*100)+((outSetNew[1]-0x30)*10)+((outSetNew[2]-0x30)*1)+((outSetNew[3]-0x30)*0.1);
        }
        CHG_Sts.b7 = 1;
        CHG_Sts.b6 = 0;
        Manual_LED = ON;
        Float_LED = OFF;
        Equal_LED = ON;
    }
    else if(CHG_SEL == 1 && CHG_Mode == 1)
    {
        if(CHG_EN == 0)
        {
            Set_VRef = ((battVoltSet[0] & 0x0f)*100)+((battVoltSet[1] & 0x0f)*10)+((battVoltSet[2] & 0x0f)*1)+((battVoltSet[3] & 0x0f)*0.1);
        }
        else
        {
            Set_VRef = ((outSetNew[0]-0x30)*100)+((outSetNew[1]-0x30)*10)+((outSetNew[2]-0x30)*1)+((outSetNew[3]-0x30)*0.1);
        }
        CHG_Sts.b7 = 1;
        CHG_Sts.b6 = 1;
        Manual_LED = ON;
        Float_LED = ON;
        Equal_LED = OFF;
    }
}

void CHG_Check(void)
{
    float temp = 0.0;
    CHG_Time();
    // AUTO Equal Start
    if(CHG_SEL == 0 && Auto_Flag == 0)
    {
        Auto_Flag = 1;
        CHG_Mode = 0;//equal. Charging
        CHG_CMD();
        Set_OK = 1;  
        Set_cnt = 0;
    }
    //AUTO Equal
    else if(CHG_SEL == 0 && Auto_Flag == 1)
    {
        // Auto Equal Finish
        temp = ((battVoltSet[4] & 0x0f)*100)+
                ((battVoltSet[5] & 0x0f)*10)+
                ((battVoltSet[6] & 0x0f)*1)+
                ((battVoltSet[7] & 0x0f)*0.1); 
        if(Set_cnt == 10 && 
                (((BATT_A <= CHG_Cut_CURR)&&(Voltage_Out > temp*0.98)) || 
                (Cut_Timer >= (unsigned int)CHG_Cut_TIME)) )
        {
            Auto_Flag = 2;
            CHG_Mode = 1;   //Floating Charge//
            CHG_CMD();
            Set_OK = 1;
        }
        // Time delay Double check
        else
        {
            Set_cnt++;
            if(Set_cnt >= 10) Set_cnt = 10;
        }       
    }
    // Normal Condition
    else
    {
        Set_cnt = 0;
    }
}

void CHG_Time(void)
{ 
    int i = 0;
    for(i=0;i<10;i++)
    {
        if(CHG_SEL == 0)
        {
            Res_Year[i] = EEPROMRead(SET_YEAR_0 + (i*6));        
            if(Res_Year[i] == rm_bcd(read_ds1302(0x8d)) || Res_Year[i] == 0)
            {
                Res_Month[i] = EEPROMRead(SET_MONTH_0 + (i*6));
                if(Res_Month[i] == rm_bcd(read_ds1302(0x89)) || Res_Month[i] == 0)
                {
                    Res_Day[i] = EEPROMRead(SET_DAY_0 + (i*6));
                    if(Res_Day[i] == rm_bcd(read_ds1302(0x87)) || Res_Day[i] == 0)
                    {
                        Res_Hour[i] = EEPROMRead(SET_HOUR_0 + (i*6));
                        if(Res_Hour[i] == rm_bcd(read_ds1302(0x85)))
                        {
                            Res_Min[i] = EEPROMRead(SET_MIN_0 + (i*6));
                            if(Res_Min[i] == rm_bcd(read_ds1302(0x83)))
                            {
                                Res_Sec[i] = EEPROMRead(SET_SEC_0 + (i*6));
                                if(Res_Sec[i] == rm_bcd(read_ds1302(0x81)))
                                {
                                    Auto_Flag = 0;
                                    tmr1_60sec = 0;
                                    tmr1Min = 0;
                                    Cut_Timer = 0;
                                }
                            }
                        } 
                    }
                }
            }
        }
    }
    
}

void hex16_3digt(unsigned char hex_data_8)
{
	unsigned char buffer;
	buffer = hex_data_8;
//	WDT = HI;_NOP();_NOP();_NOP();WDT = LO;
	hund = ((buffer/100) | 0x30);buffer %= 100;
	ten = ((buffer/10) | 0x30);buffer %= 10;
	one = (buffer | 0x30);
//	WDT = HI;
}//hex16_3digt end

void hex16_4digt(unsigned int hex_data)
{
	unsigned int buffer;
	buffer = hex_data;
	//WDT = HI;_NOP();_NOP();_NOP();WDT = LO;
	thnd = ((buffer/1000) | 0x30);buffer %= 1000;
	hund = ((buffer/100) | 0x30);buffer %= 100;
	ten = ((buffer/10) | 0x30);buffer %= 10;
	one = (buffer | 0x30);
	//WDT = HI;
}//hex16_4digt end


