
/**
 * HFC3100D RectDisplay - graphic lcd(128x64)
 * mcu : dspic33ch128mp506
 * filename : main.c
 * goooo
 * ===============================================
 * Section: Included Files
 * rev0 : 22.09.28. version 1.0  
 * ===============================================
 * 22.12.05 
 * #define RDVERSION 110 //v1.10 추가
 * 
 * 
*/
#include <libpic30.h>
//#include	<stdio.h>
#include    "mcc_generated_files/system.h"
#include    "mcc_generated_files/pin_manager.h"
#include    "i2c_master_eeprom_24fc256.h"

#include    "variables.h"
#include 	"sysinfo.h"
#include 	"Definition.h"
#include    "Initialize.h"
#include    "RS232.h"

#include    "mcc_generated_files/adc1.h"
#include    "Alram_Sts.h"
#include	"fnt3_cd.h"			

#define EEPROM_START_VALUE      0x55
#define EEPROM_START_ADDRESS        0
//lcd line//
#define line0 0xB8
#define line1 0xB9
#define line2 0xBA
#define line3 0xBB
#define line4 0xBC
#define line5 0xBD
#define line6 0xBE
#define line7 0xBF
//
#define X0    0x40//cursor position 0// X0(0x40) ~ X63(0x7F) 
//
  
//====================================================================================//
//                          Function Declaration                                      //
//====================================================================================//
void write_ds1302_byte(UINT8 cmd);
void write_ds1302(UINT8 cmd, UINT8 data) ;
UINT8 read_ds1302(UINT8 cmd) ;
void rtc_init(void);
UINT8 get_bcd(UINT8 data);
UINT8 rm_bcd(UINT8 data);
void rtc_set_datetime(UINT8 day, UINT8 mth, UINT8 year, UINT8 dow, UINT8 hour, UINT8 min);
void rtc_get_date();
void rtc_get_time();
//
void ParameterInitialize(void);
void Init_Interrupt(void);
void ledTest(void);
void dispInit(void);
void errBz_On(void);

void Delay_Cnt(UINT16 cnt);

void lcdcmd(unsigned char dd);
void lcdfill(unsigned char dd);
void lcdwrite(unsigned char dd);
void lcd_inst(unsigned int RW,unsigned int DI,unsigned char lcd_data);					// LCD instruction routine
void lcd_poss(unsigned char x_poss,unsigned char y_poss);						// LCD x-y loading function
void lcd_put_char(unsigned char dchar);								// LCD data put charictor load
void dispMenu(void);
void menuGain(void);
void MainProcess(void);
void delay500ms();
void delay200ms();
void delay100ms();
void Faultstate(void);
//
void dispLogo(void);
void digit_disp(void);

void Delay_Cnt(UINT16 cnt)
{
	UINT16 i;
    for(i=0;i<cnt;i++)
    {
        __asm__ volatile ("NOP");
    }
}
//

/* =================================================
        Main application
   ================================================= */
unsigned int mainCnt=0;
unsigned int mainFlag=0;
unsigned int iOldVal=0;
float fltOldValue = 9999.0;
float fltOldValue2 = 9999.0;
unsigned int iDebounce = 0;
unsigned int prevMenu = 999;


int main(void)
{
    SYSTEM_Initialize();
    //
    EEPROMInit();//I2CM_Init();
    //    
    rtc_init();    
    //
	initLCD(); //graphic lcd driver init//
    //    
	EEPROM_Initialize();
	ParameterInitialize();  
    BL=1;
    dispLogo();
    iMenuIndex=0;
	Set_Value();
	dispInit();//system start buzzer on/off//
	Set_Value();
	Set_OK = 1;
	Serial_Process();    
    tmrCnt[7]=0;
    
    while (1)
    {
        //TP2_Toggle();//instruction elapse time ~1us 50mhz/2 25mips//
        Serial_Process();        //
        //==========================================
        if(tmrCnt[0]>=20)//10ms
        {
            tmrCnt[0]=0;//clear timer
            //------------
			// ADC Processing
			//------------
            ACV_R_Raw = ADC_Conv(adcGetValue(ACV_R), ACV_R_Raw);
            ACV_S_Raw = ADC_Conv(adcGetValue(ACV_S), ACV_S_Raw);	
            ACV_T_Raw = ADC_Conv(adcGetValue(ACV_T), ACV_T_Raw);	
            //
            ACI_R_Raw = ADC_Conv(adcGetValue(ACI_R), ACI_R_Raw);				
            ACI_S_Raw = ADC_Conv(adcGetValue(ACI_S), ACI_S_Raw);				
            ACI_T_Raw = ADC_Conv(adcGetValue(ACI_T), ACI_T_Raw);	
            //-----------------------------------------------------
            ACV_R_filter = (ACV_R_Raw*ADC_Gain);          //Real Value
            ACV_S_filter = (ACV_S_Raw*ADC_Gain);          //Real Value	
            ACV_T_filter = (ACV_T_Raw*ADC_Gain);          //Real Value	
            //
            ACI_R_filter = (ACI_R_Raw*ADC_Gain);          //Real Value
            ACI_S_filter = (ACI_S_Raw*ADC_Gain);          //Real Value	
            ACI_T_filter = (ACI_T_Raw*ADC_Gain);          //Real Value		
            //------------------------------------------------------------
            Alram_Sts();			
        }//End if(tmrCnt[0]>=20)//10ms
        //==========================================
        if(tmrCnt[1] >= 100)//100ms
		{
            //Sp2_LED_Toggle();
            tmrCnt[1]=0;
            //------------------
			dispMenu();//Display Menu Background            
			Faultstate();			
            errBz_On();           
            CHG_Check();
		}//END if(tmrCnt[1] >= 100)//100ms
        //==========================================
		if(tmrCnt[2] >= 5)
		{
            tmrCnt[2] = 0;
            //------------------
                up=!UP;down=!DOWN;esc=!ESC;enter=!ENT;
            //    
			if(up||down||esc||enter)
			{
				if(fKey==1) 
                {
                    setKey=0;
                }    
				else
				{
					fKey=1;
					setKey=1;
					BL=1;
					tmr500ms=0;
				}
			}
			else fKey=0,setKey=0;
            //
        //-------------------------------------------
		key_oper();
        //-------------------------------------------
		digit_disp();
		}//End tmrCnt[2]>=5
        //===========================================
		if(tmr500ms>=60 && BL==1)
        //if(tmrCnt[3] >= 30000 && BL==1)//30second
		{
			tmr500ms=0;
            //tmrCnt[3]=0;
            //-------------------
			BL=0;
		} 
        //==========================================
        if(tmrCnt[4]>=20)
        {
            tmrCnt[4]=0;        //
        }
        //==========================================
        if(tmrCnt[5]>=300)
        {
            tmrCnt[5]=0;        //
        }
        //==========================================
        if(tmrCnt[6]>=500)
        {
            tmrCnt[6]=0;        //
        }
        //==========================================
//        if(tmrCnt[7]>=1000)
//        {
//            tmrCnt[7]=0;        //    Sp2_LED_Toggle();        
//        }
        //==========================================
        if(tmrCnt[8]>=2000)
        {
            tmrCnt[8]=0;//clear timer
            TP2_Toggle();
        }//End if(tmrCnt[8]>=500)
        //==========================================
//        if(tmrCnt[9]>=3000)
//        {
//            tmrCnt[9]=0;//clear timer
//            //
//            
//           //BL_Toggle();
//        }//End if(tmrCnt[9]>=1000)
    }
    return 1; 
}//End main()

//##############################################################################
void errBz_On()
{
	unsigned char data_8=0;
    
	data_8 = EEPROMRead(ALARM_STOP);
	if((fFaultState == 1) || (fAlarmState == 1))
	{
		if(data_8 == 'Y')
		{
			if(errBz100ms>=20)
			{
				BZ_OFF();
                //Fault_LED=OFF;
			}
			if(errBz100ms>=40)
			{
				errBz100ms=0;
				BZ_ON();
                //Fault_LED = ON;
			}
		}
		else
		{
            BZ_OFF();
//            if(errBz100ms>=20)
//			{
//                Fault_LED=OFF;
//			}
//			if(errBz100ms>=40)
//			{
//				errBz100ms=0;
//                Fault_LED = ON;
//			}
			
		}
		fFaultState=0;
		fAlarmState=0;
	}
	else BZ_OFF();
}

void delay500ms()
{
	unsigned char i;
	unsigned int j;
	for(i=0;i<50;i++)
	{
		for(j=0;j<15000;j++)		//15000 * 50 => 500ms
		{
			_NOP();
		}
	}
}
void dispInit() //LED Testing//
{    
    int i;
        tmrCnt[9]=0;
        while(tmrCnt[9]<400);
	Normal_LED=ON;      //PB9
        tmrCnt[9]=0;
        while(tmrCnt[9]<100);        
	Fault_LED=ON;       //PB10
        tmrCnt[9]=0;
        while(tmrCnt[9]<100);        
	Float_LED=ON;    //PB11
        tmrCnt[9]=0;
        while(tmrCnt[9]<100);        
	Equal_LED=ON;  //PB13
        tmrCnt[9]=0;
        while(tmrCnt[9]<100);        
	Manual_LED=ON;      //PB14
	    tmrCnt[9]=0;
        while(tmrCnt[9]<500);        
    Normal_LED=OFF;
    Fault_LED=OFF;
    Float_LED=OFF;
    Equal_LED=OFF;
    Manual_LED=OFF;    
        tmrCnt[9]=0;
        while(tmrCnt[9]<500);        
    for(i = 0; i<3; i++) 
    {
        Normal_LED=ON;
        Fault_LED=ON;
        Float_LED=ON;
        Equal_LED=ON;
        Manual_LED=ON;
            //
        tmrCnt[9]=0;
        while(tmrCnt[9]<400);        
            //  
        BZ_OFF();
        Normal_LED=OFF;
        Fault_LED=OFF;
        Float_LED=OFF;
        Equal_LED=OFF;
        Manual_LED=OFF;
        tmrCnt[9]=0;
        while(tmrCnt[9]<200);        
    }//End for
}
//=============//
// ds1302 RTC
//=============//
void Time_Set()
{
	rtc_set_datetime(day, mth, year, 0, hour, min); 
}

void write_ds1302_byte(UINT8 cmd) 
{
   UINT8 i;
   RTC_IO_DIR=0;//TRIS_RD11 set output//
   RTC_CLK=0;   //RD10
   for(i=0;i<=7;++i) 
   {
   		RTC_IO=(cmd & 0x01);
   		cmd>>=1;
     	RTC_CLK=1;
     	Delay_Cnt(2);
     	RTC_CLK=0;
     	Delay_Cnt(2);
   }
}

void write_ds1302(UINT8 cmd, UINT8 data) 
{
	RTC_RST=1;
	write_ds1302_byte(cmd);
	write_ds1302_byte(data);
	RTC_RST=0;//active low
}

UINT8 read_ds1302(UINT8 cmd) 
{
	unsigned char i,data;   
	RTC_RST=1;
    RTC_IO_DIR=0;//output//
	write_ds1302_byte(cmd);
	RTC_IO_DIR=1;//input//
	Delay_Cnt(2);
    //__delay_ms(1);
	data=0;
	for(i=0;i<=7;++i) 
	{
		data>>=1;
		if(RTC_IO_PIN)
		{
			data=data|0x80;
		}	
		else
		{
			data=data&0x7f;
		}	
		RTC_CLK=1;
		Delay_Cnt(2);
        //__delay_ms(1);
		RTC_CLK=0;
		Delay_Cnt(2);
        //__delay_ms(1);
	}
	RTC_RST=0;
	return(data);
}

void rtc_init(void) 
{
	unsigned char x;
	RTC_RST=0;//active low
    
	Delay_Cnt(2);	
    //__delay_ms(1);
	RTC_CLK=0;		
	write_ds1302(0x8e,0);//write protect
	write_ds1302(0x90,0xa5);//tcr reg
	//x=read_ds1302(0x81);
    x=read_ds1302(0x80); //clock halt
	if((x & 0x80)!=0)
	{	
		write_ds1302(0x80,0);//clock halt
	} 
}

UINT8 get_bcd(UINT8 data)
{
   UINT8 nibh;
   UINT8 nibl;
   nibh=data/10;
   nibl=data-(nibh*10);
   return((nibh<<4)|nibl);
}

UINT8 rm_bcd(UINT8 data)
{
   UINT8 i;	
   i=data;
   data=(i>>4)*10;
   data=data+(i&0x0f);
   return data;
}

void rtc_set_datetime(UINT8 day, UINT8 mth, UINT8 year, UINT8 dow, UINT8 hour, UINT8 min) 
{
   write_ds1302(0x86,get_bcd(day));	
   write_ds1302(0x88,get_bcd(mth));	
   write_ds1302(0x8c,get_bcd(year));
   write_ds1302(0x8a,get_bcd(dow));	
   write_ds1302(0x84,get_bcd(hour));
   write_ds1302(0x82,get_bcd(min));	
   write_ds1302(0x80,get_bcd(0));		
}

void rtc_get_date() 
{
   day = rm_bcd(read_ds1302(0x87));
   mth = rm_bcd(read_ds1302(0x89));
   year = rm_bcd(read_ds1302(0x8d));
   dow = rm_bcd(read_ds1302(0x8b));
}

void rtc_get_time()
{
   hour = rm_bcd(read_ds1302(0x85));
   min = rm_bcd(read_ds1302(0x83));
   sec = rm_bcd(read_ds1302(0x81));
}

//==============================================================================
// graphic lcd function//
//==============================================================================
//    LCD busy check  routine
void	lcdwrite(unsigned char dd) 
{
    D_I_SetHigh();
    R_W_SetLow();
    //
    db7 = CheckBit(dd,7);//(lcd_data & 0x80)>>7;
    db6 = CheckBit(dd,6);//(lcd_data & 0x40)>>6;
    db5 = CheckBit(dd,5);//(lcd_data & 0x20)>>5;
    db4 = CheckBit(dd,4);//(lcd_data & 0x10)>>4;
    db3 = CheckBit(dd,3);//(lcd_data & 0x08)>>3;
    db2 = CheckBit(dd,2);//(lcd_data & 0x04)>>2;
    db1 = CheckBit(dd,1);//(lcd_data & 0x02)>>1;
    db0 = CheckBit(dd,0);//(lcd_data & 0x01);

    if(cs_bit == 0)
    {
        CS1_SetHigh();
        CS2_SetLow();
    }        
    else 
    {
        CS1_SetLow();
        CS2_SetHigh();
    }
    EN_SetHigh();
    Delay_Cnt(10); //about 6us//
    EN_SetLow();
    Delay_Cnt(10); 
    R_W_SetHigh();   
 }//lcdwrite end

void lcdcmd(unsigned char dd)
{
    db7 = CheckBit(dd,7);//(lcd_data & 0x80)>>7;
    db6 = CheckBit(dd,6);//(lcd_data & 0x40)>>6;
    db5 = CheckBit(dd,5);//(lcd_data & 0x20)>>5;
    db4 = CheckBit(dd,4);//(lcd_data & 0x10)>>4;
    db3 = CheckBit(dd,3);//(lcd_data & 0x08)>>3;
    db2 = CheckBit(dd,2);//(lcd_data & 0x04)>>2;
    db1 = CheckBit(dd,1);//(lcd_data & 0x02)>>1;
    db0 = CheckBit(dd,0);//(lcd_data & 0x01);    
    
    D_I_SetLow();//instruction//
    R_W_SetLow();//write//    

    if(cs_bit == 0)
    {
        CS1_SetHigh();
        CS2_SetLow();
    }        
    else 
    {
        CS1_SetLow();
        CS2_SetHigh();
    }
    
    EN_SetHigh();
    Delay_Cnt(10); //about 6us//    
    EN_SetLow();
    Delay_Cnt(10); 
    //R_W_SetHigh();
    
}//lcdcmd end

void lcd_inst(unsigned int RW,unsigned int DI,unsigned char lcd_data) 
{
    db7 = CheckBit(lcd_data,7);//(lcd_data & 0x80)>>7;
    db6 = CheckBit(lcd_data,6);//(lcd_data & 0x40)>>6;
    db5 = CheckBit(lcd_data,5);//(lcd_data & 0x20)>>5;
    db4 = CheckBit(lcd_data,4);//(lcd_data & 0x10)>>4;
    db3 = CheckBit(lcd_data,3);//(lcd_data & 0x08)>>3;
    db2 = CheckBit(lcd_data,2);//(lcd_data & 0x04)>>2;
    db1 = CheckBit(lcd_data,1);//(lcd_data & 0x02)>>1;
    db0 = CheckBit(lcd_data,0);//(lcd_data & 0x01);
    //
    R_W_SetLow();    
    D_I = DI;    
    if(cs_bit == 0)
    {
        CS1_SetHigh();
        CS2_SetLow();
    }        
    else 
    {
        CS1_SetLow();
        CS2_SetHigh();
    }    
    EN_SetHigh();
    Delay_Cnt(10);
    EN_SetLow();
    Delay_Cnt(10);    
}//lcd_inst end	

void initLCD()
{
    cs_bit = 0;
	lcdcmd(0x3E);
	cs_bit = 1;
	lcdcmd(0x3E); //display off//
    //
    cs_bit = 0;
	lcdcmd(0x40);
	cs_bit = 1;
	lcdcmd(0x40); //set y address(column=0)
    //
    cs_bit = 0;
	lcdcmd(0xB8);
	cs_bit = 1;
	lcdcmd(0xB8); //set x address(page=0)
    //
//---------------------------
	cs_bit = 0;
	lcdcmd(0xC0);//BF
	cs_bit = 1;
	lcdcmd(0xC0);  // set z address(start line0)
    //
	cs_bit = 0;
	lcdcmd(0x3f);
	cs_bit = 1;
	lcdcmd(0x3f);   //display on
    //
	lcdfill(0);
}// lcdinit end

void MainProcess() //Not call
{
		switch(iMenuIndex)
		{
			case M0_LOGO:   //#1
				dispLogo();
				break;
				
			default: dispLogo();
				break;
		}	// END switch (iMenuIndex)
}

void	lcd_refresh() {
        LCD_RST = LO;
        _NOP();_NOP();_NOP();
        LCD_RST = HI;
        initLCD();										// LCD module initializing
        iMenuIndex=0;

        //dispMenu();
        dispLogo();
	}//lcd_refresh end

//LCD clear routine	
void	lcdfill(unsigned char dd) {
		unsigned char lcdx,lcdy;
		unsigned char cnt;
		delay(10);
		cs_bit = LO;CS1 = HI;CS2 = LO;
		for(cnt=0;cnt<2;cnt++) 
        {
			for(lcdy=0;lcdy<8;lcdy++) 
            {
				lcdcmd(lcdy|0xb8);          _NOP();_NOP();
				lcdcmd(0x40);               _NOP();_NOP();
				for(lcdx=0;lcdx<64;lcdx++) 
                {
                    //if(lcdx==0||lcdy==63)lcdwrite(0xff);//gooooo
					//else lcdwrite(dd);
                    lcdwrite(dd);
				    _NOP();_NOP();				
				}
			}
			cs_bit = HI;
			CS1 = LO;CS2 = HI;
		}
		cs_bit = LO;
		CS1 = LO;CS2 = LO;
	}//End lcdfill

void delay(unsigned int dly_time) {
		while(dly_time == 0) {
			dly_time--;			
		}
	}//delay end

void dispMenu()
{
	unsigned char x_cnt,y_cnt;//,data_8;
	unsigned int data_16;
    unsigned char data_8;
    //===================================================== 0,1,2line(Title)
    if(prevMenu != iMenuIndex || iMenuIndex == MENU_CHG_MODE_SET || iMenuIndex == MENU_ADCGAIN)
    {
        for(y_cnt=0;y_cnt<3;y_cnt++)
        {
            lcd_poss(0,y_cnt);        _NOP();//delay(10);

            for(x_cnt=0;x_cnt<64;x_cnt++)
            {
                data_16 = x_cnt+(y_cnt*128);                
                lcd_inst(0,1,MENUD[iMenuIndex][data_16]);                
                //
                _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
            }
            //---------------------------------------------
            lcd_poss(64,y_cnt);     _NOP();//delay(10);
            for(x_cnt=64;x_cnt<128;x_cnt++)        
            {
                data_16 = x_cnt+(y_cnt*128);                            
                lcd_inst(0,1,MENUD[iMenuIndex][data_16]);               
                //
                _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
            }
        }
        //========================================================    
        switch(iMenuIndex)  //3,4,5,6,7 line menu backgroud display
        {
            case(MENU_AC_INFO):
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                
                        lcd_inst(0,1,ACINFO[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                            
                        lcd_inst(0,1,ACINFO[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
            break;
            case(MENU_CHG_MODE_SET):
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                
                        lcd_inst(0,1,CHGMODESET[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                            
                        lcd_inst(0,1,CHGMODESET[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
                break;
            case(MENU_CHG_CONDITION_SET):
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                
                        lcd_inst(0,1,CHGCONDITIONSET[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                            
                        lcd_inst(0,1,CHGCONDITIONSET[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
                break;
            case(MENU_CHARGE_VOLT_SET_PB):           
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                
                        lcd_inst(0,1,CHGVOLTSET[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                            
                        lcd_inst(0,1,CHGVOLTSET[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
                break;
            case(MENU_CHARGE_VOLT_SET_LI):    
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                
                        lcd_inst(0,1,CHGVOLTSET_LI[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                            
                        lcd_inst(0,1,CHGVOLTSET_LI[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
                break;
            case(MENU_BATTYPE_ACVOLT):
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                
                        lcd_inst(0,1,BATTYPEACVOLT[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                            
                        lcd_inst(0,1,BATTYPEACVOLT[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
                break;
            case(MENU_TIME_SET):
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                
                        lcd_inst(0,1,TIMESET[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+((y_cnt-3)*128);                            
                        lcd_inst(0,1,TIMESET[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
                break;
            //------------------------------------------------------------------    
            default:
                for(y_cnt=3;y_cnt<8;y_cnt++)
                {
                    lcd_poss(0,y_cnt);        _NOP();//delay(10);

                    for(x_cnt=0;x_cnt<64;x_cnt++)
                    {
                        data_16 = x_cnt+(y_cnt*128);                
                        lcd_inst(0,1,bgFont[data_16]);                
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                    //---------------------------------------------
                    lcd_poss(64,y_cnt);     _NOP();//delay(10);
                    for(x_cnt=64;x_cnt<128;x_cnt++)        
                    {
                        data_16 = x_cnt+(y_cnt*128);                            
                        lcd_inst(0,1,bgFont[data_16]);               
                        //
                        _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                    }
                }
            break;    
        }    
        //######################################################
        switch(iMenuIndex) //item name or symbol or min max 
        {
            //===================//
            // Charger_Info
            //===================//
            case(MENU_CHARGER_INFO):
                lcd_poss(15,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//jun
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //jun?-26
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[19][x_cnt]);      //ap?-19          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[19][x_cnt]);    //ap?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //=======================================================
                lcd_poss(15,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//jun?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //jun?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);       //ryu?         
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);      //ryu?          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(106,4); 
                data_8 ='V';
                lcd_put_char(data_8);
                lcd_poss(106,6); 
                data_8 ='A';            
                lcd_put_char(data_8);
                break;
            //===================//
            // Menu AC Info
            //===================//
            case(MENU_AC_INFO):
                //
                break;
            case(MENU_TIME_DISP):           //
                lcd_poss(20,4);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<16;x_cnt++)
                //for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,~HFONT[49][x_cnt]);//clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(20,5);     _NOP();//delay(10);
                for(x_cnt=16;x_cnt<32;x_cnt++)
                //for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,~HFONT[49][x_cnt]); //clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                break;
            //===================//
            // Menu Temperature
            //===================//    
            case(MENU_TEMPERATURE):
                lcd_poss(20,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<16;x_cnt++)
                {
                    lcd_inst(0,1,HFONT[48][x_cnt]);//TEMPERATURE SYMBOL
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(20,4);     _NOP();//delay(10);
                for(x_cnt=16;x_cnt<32;x_cnt++)
                {
                    lcd_inst(0,1,HFONT[48][x_cnt]); //temperature symbol
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------------------
                lcd_poss(92,3);        _NOP();// 'C mark
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[52][x_cnt]);//Do-C
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(92,4);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[52][x_cnt]); //do-C
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                break;
            //===================//
            // Menu Login
            //===================//
            case(MENU_LOGIN):
                lcd_poss(20,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<16;x_cnt++)
                //for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[50][x_cnt]);//clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(20,4);     _NOP();//delay(10);
                for(x_cnt=16;x_cnt<32;x_cnt++)
                //for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[50][x_cnt]); //clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                break;
            //===============================//
            // Menu Charge volt set(Lituam)
            //===============================//
            case(MENU_CHARGE_VOLT_SET_LI):            
              //  
                break;
            //=====================================//
            // Menu Charge Volt Set PB battery
            //=====================================//
            case(MENU_CHARGE_VOLT_SET_PB):
                //
                break;
            //===================//
            // Menu Manual Set
            //===================//
            case(MENU_MANUAL_SET):
                lcd_poss(15,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//jun?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //?-26
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[19][x_cnt]);      //ap?-19          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[19][x_cnt]);    //ap?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //=======================================================
                lcd_poss(15,5);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//jun?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,6);     _NOP();//delay(10);
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,5);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);       //?         
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,6);     _NOP();//delay(10);
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);      //?          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(106,4); 
                data_8 ='V';
                lcd_put_char(data_8);
                lcd_poss(106,6); 
                data_8 ='A';            
                lcd_put_char(data_8);
                break;
            //========================//
            // Menu Current Limit Set
            //========================//        
            case(MENU_CURRENT_LIMIT):
                 lcd_poss(15,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);       //?         
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);      //?          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(106,4); 
                data_8 ='A';
                lcd_put_char(data_8);            
                break;    
            //====================================//
            // Menu Input Alarm Value Min Max Set
            //====================================//
            case(MENU_INPUT_ALARM_SET):
                lcd_poss(15,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]);//choi
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]); //choi?-53
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[54][x_cnt]);      //dae?-54          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[54][x_cnt]);    //dae 54?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //=======================================================
                lcd_poss(15,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]);//choi?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]); //choi?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[55][x_cnt]);       //so?         
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[55][x_cnt]);      //so?          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(106,4); 
                data_8 ='V';
                lcd_put_char(data_8);
                lcd_poss(106,6); 
                data_8 ='V';            
                lcd_put_char(data_8);
                break;
            //============================//
            // Menu output volt. alarm set
            //============================//
            case(MENU_OUTPUT_ALARM_SET):
                lcd_poss(15,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]);//choi
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]); //choi?-53
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[54][x_cnt]);      //dae?-54          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[54][x_cnt]);    //dae 54?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //=======================================================
                lcd_poss(15,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]);//choi?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]); //choi?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[55][x_cnt]);       //so?         
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[55][x_cnt]);      //so?          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(106,4); 
                data_8 ='V';
                lcd_put_char(data_8);
                lcd_poss(106,6); 
                data_8 ='V';            
                lcd_put_char(data_8);
                break;
            //===============================//
            // Menu Output Current Alarm Set
            //===============================//
            case(MENU_OUT_CUR_ALARM_SET):
                lcd_poss(15,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//jun
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //jun?-26
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);      //ap?-19          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);    //ap?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(106,4); 
                data_8 ='A';            
                lcd_put_char(data_8);
                break;
            //===================//
            //
            //===================//
            case(MENU_CUTOFF_VOLT_SET):
                lcd_poss(15,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]);//choi
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]); //choi?-53
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[54][x_cnt]);      //dae?-54          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[54][x_cnt]);    //dae 54?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //=======================================================
                lcd_poss(15,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]);//choi?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[53][x_cnt]); //choi?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[55][x_cnt]);       //so?         
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[55][x_cnt]);      //so?          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //
                lcd_poss(106,4); 
                data_8 ='V';
                lcd_put_char(data_8);
                lcd_poss(106,6); 
                data_8 ='V';            
                lcd_put_char(data_8);
                break;
            //===================//
            // Menu Buzzer on off
            //===================//    
            case(MENU_BUZZER_SET):
                lcd_poss(20,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<16;x_cnt++)
                //for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[51][x_cnt]);//clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(20,4);     _NOP();//delay(10);
                for(x_cnt=16;x_cnt<32;x_cnt++)
                //for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[51][x_cnt]); //clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                break;
            //===================//
            // Menu time setting
            //===================//
            case(MENU_TIME_SET):
                break;
            //===================//
            // Menu password setting
            //===================//
            case(MENU_PASS_SET):
                lcd_poss(10,4); 
                data_8 ='O';
                lcd_put_char(data_8);
                lcd_poss(18,4); 
                data_8 ='L';
                lcd_put_char(data_8);
                lcd_poss(26,4); 
                data_8 ='D';
                lcd_put_char(data_8);
                lcd_poss(34,4); 
                data_8 =':';
                lcd_put_char(data_8);
                //
                lcd_poss(10,6); 
                data_8 ='N';            
                lcd_put_char(data_8);
                lcd_poss(18,6); 
                data_8 ='E';
                lcd_put_char(data_8);
                lcd_poss(26,6); 
                data_8 ='W';
                lcd_put_char(data_8);
                lcd_poss(34,6); 
                data_8 =':';
                lcd_put_char(data_8);
                break;
            //=============================//
            // Menu System Initilize yes/no
            //=============================//
            case(MENU_SYS_INIT):
                //
                break;
            //=====================//
            // Menu_AdcGain setting
            //=====================//
            case(MENU_ADCGAIN):
                lcd_poss(15,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//jun
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //jun?-26
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[19][x_cnt]);      //ap?-19          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[19][x_cnt]);    //ap?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //=======================================================
                lcd_poss(15,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]);//jun?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[26][x_cnt]); //jun?
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,5);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);       //ryu?         
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,6);     _NOP();//delay(10);
                //for(x_cnt=16;x_cnt<32;x_cnt++)        
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[8][x_cnt]);      //ryu?          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                break;
            //===========================================//
            // Menu Charge Mode Setting(auto/float/Equal)
            //===========================================//
            case(MENU_CHG_MODE_SET):
                //
                break;
            //===============================//
            // Menu Equal. Charge Schedule
            //===============================//    
            case(MENU_CHG_TIME_SET):
                lcd_poss(24,3); 
                data_8 ='N';
                lcd_put_char(data_8);
                lcd_poss(32,3); 
                data_8 ='O';
                lcd_put_char(data_8);
                lcd_poss(40,3); 
                data_8 =':';
                lcd_put_char(data_8);
                //
                lcd_poss(8,5); 
                data_8 ='D';
                lcd_put_char(data_8);
                lcd_poss(16,5); 
                data_8 ='A';
                lcd_put_char(data_8);
                lcd_poss(24,5); 
                data_8 ='T';
                lcd_put_char(data_8);
                lcd_poss(32,5); 
                data_8 ='E';
                lcd_put_char(data_8);
                lcd_poss(40,5); 
                data_8 =':';
                lcd_put_char(data_8);
                //
                lcd_poss(8,6); 
                data_8 ='T';
                lcd_put_char(data_8);
                lcd_poss(16,6); 
                data_8 ='I';
                lcd_put_char(data_8);
                lcd_poss(24,6); 
                data_8 ='M';
                lcd_put_char(data_8);
                lcd_poss(32,6); 
                data_8 ='E';
                lcd_put_char(data_8);
                lcd_poss(40,6); 
                data_8 =':';
                lcd_put_char(data_8);
                break;
            //================================//
            // Menu Charge Condition Setting //
            //================================//
            case(MENU_CHG_CONDITION_SET):            
                break;
            //===================//
            // Menu_Debug login
            //===================//    
            case(MENU_DEBUG):
                lcd_poss(20,3);        _NOP();//delay(10);
                for(x_cnt=0;x_cnt<16;x_cnt++)
                {
                    lcd_inst(0,1,HFONT[50][x_cnt]);//clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(20,4);     _NOP();//delay(10);
                for(x_cnt=16;x_cnt<32;x_cnt++)
                {
                    lcd_inst(0,1,HFONT[50][x_cnt]); //clock
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                break;
            //========================//
            // Menu Manual Run Charger//
            //========================//    
            case(MENU_DEVICE_RUNNING):
                lcd_poss(15,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)
                {
                    lcd_inst(0,1,HFONT[57][x_cnt]);//jun
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(15,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[57][x_cnt]); //jun?-26
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------
                lcd_poss(30,3);        _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=0;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[58][x_cnt]);      //ap?-19          
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                //---------------------------------------------
                lcd_poss(30,4);     _NOP();//delay(10);
                //for(x_cnt=0;x_cnt<16;x_cnt++)
                for(x_cnt=1;x_cnt<32;x_cnt+=2)        
                {
                    lcd_inst(0,1,HFONT[58][x_cnt]);    //ap?            
                    _NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
                }
                break;
            //====================================================//
            // Menu Battery Type(PB1, PB2, Li) & AC Volt(220,380)
            //====================================================//    
            case(MENU_BATTYPE_ACVOLT):
                break;

            default:
                break;
        }
     prevMenu = iMenuIndex;   
}	
}


void dispLogo(void)
{
	unsigned int x_cnt,y_cnt,data_16;
	for(y_cnt=0;y_cnt<8;y_cnt++) 
    {
		lcd_poss(0,y_cnt);_NOP();_NOP();_NOP();//delay(10);
		for(x_cnt=0;x_cnt<64;x_cnt++) 
        {
			data_16 = x_cnt+(y_cnt*128);
			lcd_inst(LO,HI,logoFont[data_16]);
			_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
			_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
		}
		lcd_poss(64,y_cnt);_NOP();_NOP();_NOP();//delay(10);
		for(x_cnt=64;x_cnt<128;x_cnt++) 
        {
			data_16 = x_cnt+(y_cnt*128);
			lcd_inst(LO,HI,logoFont[data_16]);
			_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
			_NOP();_NOP();_NOP();_NOP();_NOP();_NOP();
		}
	}	
}


void Faultstate()
{
	unsigned char faultstate=0;
	if(Set_Output_UV > Voltage_Out)//check under volt
	{
		Fault_LED=ON;
		fAlarmState = 1;
		Output_UV=0x40;
	}
	else
	{
		if(fAlarmState == 1 || fFaultState == 1)	Fault_LED=ON;
		else	Fault_LED=OFF;
		Output_UV=0x00;
	}
    //-----------------------------
	if(Set_Output_OV < Voltage_Out)//check over volt
	{
		Fault_LED=ON;
		fAlarmState = 1;
		Output_OV=0x80;
	}
	else
	{
		if(fAlarmState == 1 || fFaultState == 1)	Fault_LED=ON;
		else	Fault_LED=OFF;
		Output_OV=0x00;
	}
    //----------------------------------------
	if(Set_ARef < Current_Out)
	{
		Fault_LED=ON;
		fAlarmState = 1;
	}
	else
	{
		if(fAlarmState == 1 || fFaultState == 1)	Fault_LED=ON;
		else	Fault_LED=OFF;
	}
    //---------------------------------------------------
	faultstate = Input_OV | Input_UV | Input_Shutdown;
	if(faultstate == 0x00 && (Rx_Char == 0x80 || Rx_Char == 0x00))
	{
		if(fAlarmState == 0)
		{
			Normal_LED=ON; //goooo remark
			Fault_LED=OFF;
		}
		else
		{
			Normal_LED=OFF; //goooo remark
			Fault_LED=ON;
		}			
	}		
    else
    {
        Normal_LED=OFF;         //goooo remark//
        Fault_LED=ON; 
        fFaultState = 1;
    }   
}

void key_oper()
{
	unsigned char	data_8,data_cal,i,cnt,buff_8;
	unsigned int	data_16,buff_16;
    if(iMenuIndex == MENU_MANUAL_SET)
    {
        if(fManVolt==0)fManVolt=1;
    }
    //
	if(setKey==1)
	{
		switch(iMenuIndex)
		{							//       down key < CoAc > up key   //
			case(MENU_CHARGER_INFO): {							// 4.system setup <0.logo> 1.input volt & amp
				if(up) iMenuIndex = MENU_LOGIN;
				else if(down) iMenuIndex = MENU_AC_INFO;
				key_clr();
			}break;
			case(MENU_AC_INFO): {
				if(up) iMenuIndex = MENU_CHARGER_INFO;
				else if(down) iMenuIndex = MENU_TIME_DISP;
				key_clr();
			}break;
			case(MENU_TIME_DISP): {
				if(up) iMenuIndex = MENU_AC_INFO;
				else if(down) iMenuIndex = MENU_TEMPERATURE;
				key_clr();
			}break;
			case(MENU_TEMPERATURE): {
				if(up) iMenuIndex = MENU_TIME_DISP;
				else if(down) iMenuIndex = MENU_LOGIN;
				key_clr();
			}break;
            
			case(MENU_LOGIN): {//system login//
				if(!modi)
				{
					if(up) iMenuIndex = MENU_TEMPERATURE;
					else
					{
						if(down) iMenuIndex = MENU_CHARGER_INFO;
						else if(enter)
						{		//press Enter key routine -> else is Enter key pressed routine
							modi = HI;
							poss_x = 0;
							for(cnt=0;cnt<6;cnt++) temp_buffer[cnt] = '0';
						}
					}
					key_clr();
				}
				else
				{	//system setup passwd display Enter Key
					if(up)
					{
						data_8 = temp_buffer[poss_x];
						if(data_8 == '9') data_8 = '0';
						else data_8++;
						temp_buffer[poss_x] = data_8;
					}
					else
					{
						if(down)
						{
							data_8 = temp_buffer[poss_x];
							if(data_8 == '0') data_8 = '9';
							else data_8--;
							temp_buffer[poss_x] = data_8;
						}
						else if(enter)
						{
							if(poss_x >= 5)
							{
								i = 0;modi = LO;
								for(cnt=0;cnt<6;cnt++)
								{
									data_8 = gPin[cnt];
									data_cal = temp_buffer[cnt];
									i += (data_8 ^ data_cal);
								}
								if(i == 0) 
                                {
                                    if(EEPROMRead(BATT_TYPE) == 2) iMenuIndex = MENU_CHARGE_VOLT_SET_LI;//LI-ion
                                    else iMenuIndex = MENU_CHARGE_VOLT_SET_PB;
                                }
								else
								{
									i = 0;
									for(cnt=0;cnt<6;cnt++)
									{
										data_8 = gPin[cnt];
										data_cal = temp_buffer[cnt];
										i += (data_8 ^ data_cal);
									}
									if(i == 0) 
                                    {
                                        if(EEPROMRead(BATT_TYPE) == 2) iMenuIndex = MENU_CHARGE_VOLT_SET_LI;
                                        else iMenuIndex = MENU_CHARGE_VOLT_SET_PB;
                                    }
								}
							}						
							else poss_x++;
						}
						if(esc) {
							modi = LO;
							iMenuIndex = MENU_LOGIN;		//pressed ESC key Return to System Setting Password Menu
						}
					}
					key_clr();
				}
			}break;
            
            case(MENU_CHARGE_VOLT_SET_LI): 
            {
				if(!modi)              //
				{
					if(up)iMenuIndex = MENU_DEBUG;//25;
					else 
					{
						if(down) iMenuIndex = MENU_MANUAL_SET;//11
						else if(enter) 
						{
							modi = HI;
                            Manual_LED=OFF;                            
							poss_x = poss_y = 0;
							for(cnt=0;cnt<8;cnt++) battVoltSet[cnt] = EEPROMRead(BATTVOLT_0+cnt);
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;
					key_clr();
				}
				else 		//later Enter key Menu in state// adjust volt. value//
				{
					if(up) 
					{
						data_8 = battVoltSet[poss_x];
						switch(poss_x) 	
						{
							case(0): //90 ~ 125 ~ 135V
                            {
                                if(data_8 >= '1') data_8 = '0';
								else data_8++;
                            }
							break;
								//
							case(1):		
								{
									if(battVoltSet[0] <= '0')
									{	 
										data_8 = '9';										
									}	
									else if(battVoltSet[0] >= '1')
									{
										if(data_8 >= '3') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '3';
										else data_8++;
									}
							}break;
								//
							case(2):
								{
									if((battVoltSet[0] <= '0') && (battVoltSet[1] <= '9'))
									{
										if(data_8 >= '9') data_8 = '0';
                                        else if(data_8 < '0') data_8 = '9';
										else data_8++;
									}
									else if((battVoltSet[0] >= '1') && (battVoltSet[1] >= '3'))
									{
										if(data_8 >= '5') data_8 = '0';
                                        else if(data_8 < '0') data_8 = '5';
										else data_8++;
									}
									else
									{
										if(data_8 >= '9') data_8 = '0';
										else data_8++;
									}
							}break;
							//
							case(3):
								{
									if((battVoltSet[0] >= '1') && (battVoltSet[1] >= '3') && (battVoltSet[2] >= '5'))
									{
										data_8 = '0';
									}								
									else
									{
										if(data_8 >= '9') data_8 = '0';
										else data_8++;
									}
							}break;
						}//end - switch(poss_x)
						battVoltSet[poss_x] = data_8;
					}
					else //down key pressed 
					{
						if(down) 						//case(n) => fl [0][01][2].[3]  eq [4][5][6].[7]
						{
							data_8 = battVoltSet[poss_x];
							switch(poss_x) 
							{
								case(0): 
								{
                                    if(data_8 <= '0') data_8 = '1';
                                    else data_8--;
                                }
								break;
								//
								case(1):
								{
									if(battVoltSet[0] <= '0')
									{	 
										data_8 = '9';
									}	
									else if(battVoltSet[0] >= '1')
									{
										if(data_8 <= '0') data_8 = '3';
                                        else if(data_8 > '3') data_8 = '0';
										else data_8--;
									}
                                    else
                                    {
                                        if(data_8 <= '0') data_8 = '9';
										else data_8--;
                                    }
								}
								break;
								//
								case(2): 								//xxN.x	// fl [0][01][2].[3]  eq [4][5][6].[7]
								{
									if((battVoltSet[0] <= '0') && (battVoltSet[1] <= '9'))
									{
										if(data_8 <= '0') data_8 = '9';
                                        else if(data_8 > '9') data_8 = '0';
										else data_8--;
									}
									else if((battVoltSet[0] >= '1') && (battVoltSet[1] >= '3'))
									{
										if(data_8 <= '0') data_8 = '5';
                                        else if(data_8 > '5') data_8 = '0';
										else data_8--;
									}
									else
									{
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
								}
								break;
								//
								case(3):
								{
									if((battVoltSet[0] >= '1') && (battVoltSet[1] >= '3') && (battVoltSet[2] >= '5'))
									{
										data_8 = '0';
									}								
									else
									{
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
								}break;				
							}//end - switch
							battVoltSet[poss_x]=data_8;
						}
						else if(enter) 
						{
							if(poss_x >= 3) 
							//if(poss_x >= 3) 	//goo//4,5,6,7 
							{
								modi = LO;
                                data_16 = 0;
                                data_16 += (battVoltSet[0]-0x30) * 1000;
                                data_16 += (battVoltSet[1]-0x30) * 100;
                                data_16 += (battVoltSet[2]-0x30) * 10;
                                data_16 += (battVoltSet[3]-0x30);
								if(data_16 > MaxOutputVoltage_LI*10)//135
								{
									battVoltSet[0] = ((MaxOutputVoltage_LI*10)/1000)+0x30;
									battVoltSet[1] = (((MaxOutputVoltage_LI*10)-((battVoltSet[0]-0x30)*1000))/100)+0x30;
									battVoltSet[2] = (((MaxOutputVoltage_LI*10)-((battVoltSet[0]-0x30)*1000)-((battVoltSet[1]-0x30)*100))/10)+0x30;
									battVoltSet[3] = ((MaxOutputVoltage_LI*10)-((battVoltSet[0]-0x30)*1000)-((battVoltSet[1]-0x30)*100)-((battVoltSet[2]-0x30)*10))+0x30;
								}
								else if(data_16 < MinOutputVoltage_LI*10)//90
								{
									battVoltSet[0] = ((MinOutputVoltage_LI*10)/1000)+0x30;
									battVoltSet[1] = (((MinOutputVoltage_LI*10)-((battVoltSet[0]-0x30)*1000))/100)+0x30;
									battVoltSet[2] = (((MinOutputVoltage_LI*10)-((battVoltSet[0]-0x30)*1000)-((battVoltSet[1]-0x30)*100))/10)+0x30;
									battVoltSet[3] = ((MinOutputVoltage_LI*10)-((battVoltSet[0]-0x30)*1000)-((battVoltSet[1]-0x30)*100)-((battVoltSet[2]-0x30)*10))+0x30;
								}
                                battVoltSet[4] = battVoltSet[0];
                                battVoltSet[5] = battVoltSet[1];
                                battVoltSet[6] = battVoltSet[2];
                                battVoltSet[7] = battVoltSet[3];
								for(cnt=0;cnt<8;cnt++) EEPROMWrite(BATTVOLT_0+cnt, battVoltSet[cnt]);
								Set_VRef = ((battVoltSet[0]-0x30)*100)+((battVoltSet[1]-0x30)*10)+((battVoltSet[2]-0x30)*1)+((battVoltSet[3]-0x30)*0.1);
								//Set_CL=((battVoltSet[4]-0x30)*100)+((battVoltSet[5]-0x30)*10)+(battVoltSet[6]-0x30);
								cal_error = LO;modi_end = HI;Set_OK=1;fEqFl=0;
							}
							else poss_x++;
							//============================================
						}
					}					
					if(esc) modi = LO;
					key_clr();
				}
            }break;
            
            
			case(MENU_CHARGE_VOLT_SET_PB): {//10
				if(!modi)              //
				{       
					if(up)
                    {
                        if(EEPROMRead(BATT_TYPE) == 2) iMenuIndex = MENU_CHARGE_VOLT_SET_LI;//9;
                        else iMenuIndex = MENU_DEBUG;//25;
                    }
					else 
					{
						if(down) iMenuIndex = MENU_MANUAL_SET;//11;
						else if(enter) 
						{
							modi = HI;
							poss_x = poss_y = 0;
							for(cnt=0;cnt<8;cnt++) battVoltSet[cnt] = EEPROMRead(BATTVOLT_0+cnt);
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else 						//later Enter key Menu in state// adjust volt. value//
				{
                    if(EEPROMRead(BATT_TYPE) == 0)//PB125V
                    {
                        if(up) 
                        {
                            data_8 = battVoltSet[poss_x];
                            switch(poss_x) 	
                            {
                                case(0)://max 124.7 ~141.0, min 127.7 ~ 147.0V 
                                    data_8 = '1';
                                break;
                                    //
                                case(1):		
                                    {
                                        if(data_8 >= '4') data_8 = '2';
                                        else if(data_8 < '2') data_8 = '4';
                                        else data_8++;
                                    }break;
                                    //
                                case(2):
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '2')
                                        {
                                            if(data_8 >= '9') data_8 = '4';
                                            else if(data_8 < '4') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '4')
                                        {
                                            if(data_8 >= '1') data_8 = '0';
                                            else if(data_8 < '0') data_8 = '1';
                                            else data_8++;
                                        }
                                        else 
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(3):
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '2' && battVoltSet[2] <= '4')
                                        {
                                           if(data_8 >= '9') data_8 = '7';
                                           else if(data_8 < '7') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '4' && battVoltSet[2] >= '1') data_8 = '0';
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                case(4)://
                                    {
                                        data_8 = '1';
                                    }break;
                                    //
                                case(5):		
                                    {
                                        if(data_8 >= '4') data_8 = '2';
                                        else data_8++;                                   
                                    }break;
                                    //
                                case(6):
                                    {
                                        if(battVoltSet[4] <= '1' && battVoltSet[5] <= '2')
                                        {
                                            if(data_8 >= '9') data_8 = '7';
                                            else if(data_8 < '7') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[4] >= '1' && battVoltSet[5] >= '4')
                                        {
                                            if(data_8 >= '7') data_8 = '0';
                                            else if(data_8 < '0') data_8 = '7';
                                            else data_8++;
                                        }
                                        else 
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(7):
                                    {
                                        if(battVoltSet[4] <= '1' && battVoltSet[5] <= '2' && battVoltSet[6] <= '7') 
                                        {
                                            if(data_8 >= '9') data_8 = '6';
                                            else if(data_8 < '6') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[4] >= '1' && battVoltSet[5] >= '4' && battVoltSet[6] >= '7') 
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;    
                            }//end - switch(poss_x)
                            battVoltSet[poss_x] = data_8;
                        }
                        else //down key pressed PB125V
                        {
                            if(down) 		//case(n) => fl [0][01][2].[3]  eq [4][5][6].[7]
                            {
                                data_8 = battVoltSet[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                    data_8 = '1';
                                    break;
                                    //
                                    case(1):
                                    {
                                        if(data_8 <= '2') data_8 = '4';
                                        else if(data_8 > '4') data_8 = '2';
                                        else data_8--;
                                    }
                                    break;
                                    //
                                    case(2): 			//xxN.x	// fl [0][01][2].[3]  eq [4][5][6].[7]
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '2')
                                        {
                                            if(data_8 <= '4') data_8 = '9';
                                            else if(data_8 > '9') data_8 = '4';
                                            else data_8--;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '4')
                                        {
                                            if(data_8 <= '0') data_8 = '1';
                                            else if(data_8 > '1') data_8 = '0';
                                            else data_8--;
                                        }
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }
                                    break;
                                    //
                                    case(3):
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '2' && battVoltSet[2] <= '4')
                                        {
                                            if(data_8 <= '7') data_8 = '9';
                                            else if(data_8 > '9') data_8 = '7';
                                            else data_8--;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '4' && battVoltSet[2] >= '1') data_8 = '0';
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else if(data_8 > '9') data_8 = '0';
                                            else data_8--;
                                        }
                                    }break;	
                                    case(4): 
                                    {
                                       data_8 = '1';
                                    }
                                    break;
                                    //
                                    case(5):
                                    {
                                        if(data_8 <= '2') data_8 = '4';
                                        else if(data_8 > '4') data_8 = '2';
                                        else data_8--;
                                    }
                                    break;
                                    //
                                    case(6): 								//xxN.x	// fl [0][01][2].[3]  eq [4][5][6].[7]
                                    {
                                        if(battVoltSet[4] <= '1' && battVoltSet[5] <= '2') 
                                        {
                                            if(data_8 <= '7') data_8 = '9';
                                            else data_8--;
                                        }
                                        else if(battVoltSet[4] >= '1' && battVoltSet[5] >= '4') 
                                        {
                                            if(data_8 <= '0') data_8 = '7';
                                            else data_8--;
                                        }
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }
                                    break;
                                    //
                                    case(7):
                                    {
                                        if(battVoltSet[4] <= '1' && battVoltSet[5] <= '2' && battVoltSet[6] <= '7')
                                        {
                                            if(data_8 <= '6') data_8 = '9';
                                            else data_8--;
                                        }
                                        else if(battVoltSet[4] >= '1' && battVoltSet[5] >= '4' && battVoltSet[6] >= '7') data_8 = '0';
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }break;	
                                }//end - switch
                                battVoltSet[poss_x]=data_8;
                            }
                            else if(enter) 
                            {
                                if(poss_x >= 7) 
                                {
                                    modi = LO;
                                    CHG_EN = 0;
                                    data_16 = 0;
                                    data_16 +=((battVoltSet[0] & 0x0f) * 1000);
                                    data_16 +=((battVoltSet[1] & 0x0f) * 100);
                                    data_16 +=((battVoltSet[2] & 0x0f) * 10);
                                    data_16 += (battVoltSet[3] & 0x0f);
                                    if(data_16 < (MinOutputVoltage_PB1*10))//pb125v float volt min 124.7V
                                    {
                                        battVoltSet[0] = ((MinOutputVoltage_PB1*10)/ 1000) + 0x30;
                                        battVoltSet[1] = (((MinOutputVoltage_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[2] = (((MinOutputVoltage_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[3] = ((MinOutputVoltage_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100) - ((battVoltSet[2] & 0x0f)*10)) + 0x30;
                                    }
                                    else if(data_16 > (MaxOutputVoltage_PB1*10))//pb125v float volt max 141.7V
                                    {
                                        battVoltSet[0] = ((MaxOutputVoltage_PB1*10)/ 1000) + 0x30;
                                        battVoltSet[1] = (((MaxOutputVoltage_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[2] = (((MaxOutputVoltage_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[3] = ((MaxOutputVoltage_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100) - ((battVoltSet[2] & 0x0f)*10)) + 0x30;
                                    }
                                    data_16 = 0;
                                    data_16 +=((battVoltSet[4] & 0x0f) * 1000);
                                    data_16 +=((battVoltSet[5] & 0x0f) * 100);
                                    data_16 +=((battVoltSet[6] & 0x0f) * 10);
                                    data_16 += (battVoltSet[7] & 0x0f);
                                    if(data_16 < (MinOutputVoltage_PB2*10))//equal charge volt min 127.6V
                                    {
                                        battVoltSet[4] = ((MinOutputVoltage_PB2*10)/ 1000) + 0x30;
                                        battVoltSet[5] = (((MinOutputVoltage_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[6] = (((MinOutputVoltage_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[7] = ((MinOutputVoltage_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100) - ((battVoltSet[6] & 0x0f)*10)) + 0x30;
                                    }
                                    else if(data_16 > (MaxOutputVoltage_PB2*10))//equal charge volt max 147V
                                    {
                                        battVoltSet[4] = ((MaxOutputVoltage_PB2*10)/ 1000) + 0x30;
                                        battVoltSet[5] = (((MaxOutputVoltage_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[6] = (((MaxOutputVoltage_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[7] = ((MaxOutputVoltage_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100) - ((battVoltSet[6] & 0x0f)*10)) + 0x30;       
                                    } 
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(BATTVOLT_0+cnt, battVoltSet[cnt]);                            
                                    CHG_SEL = 0;
                                    CHG_CMD();
                                    cal_error = LO;modi_end = HI;Set_OK=1;fEqFl=0; 
                                }
                                else poss_x++;
                                //============================================
                            }
                        }	
                    }
                    else if(EEPROMRead(BATT_TYPE) == 1)//PB110V batt.charge volt setting//
                    {
                        if(up) 
                        {
                            data_8 = battVoltSet[poss_x];
                            switch(poss_x) 	
                            {
                                case(0):
                                    data_8 = '1';
                                break;
                                    //
                                case(1):		
                                    {
                                        if(data_8 >= '3') data_8 = '1';
                                        else data_8++;
                                    }break;
                                    //
                                case(2):
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '1')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '3')
                                        {
                                            data_8 = '0';
                                        }
                                        else 
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(3):
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '1' && battVoltSet[2] <= '0')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '3' && battVoltSet[2] >= '0') data_8 = '0';
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                case(4):
                                    {
                                        data_8 = '1';
                                    }break;
                                    //
                                case(5):		
                                    {
                                        if(data_8 >= '5') data_8 = '1'; //110~150
                                        else data_8++;                                   
                                    }break;
                                    //
                                case(6):
                                    {
                                        if(battVoltSet[4] <= '1' && battVoltSet[5] <= '1')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[4] >= '1' && battVoltSet[5] >= '5')
                                        {
                                            data_8 = '0';
                                        }
                                        else 
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(7):
                                    {
                                        if(battVoltSet[4] <= '1' && battVoltSet[5] <= '1' && battVoltSet[6] <= '0') 
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                        else if(battVoltSet[4] >= '1' && battVoltSet[5] >= '5' && battVoltSet[6] >= '0') 
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;    
                            }//end - switch(poss_x)
                            battVoltSet[poss_x] = data_8;
                        }
                        else //down key pressed 
                        {
                            if(down) 						//case(n) => fl [0][01][2].[3]  eq [4][5][6].[7]
                            {
                                data_8 = battVoltSet[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                    data_8 = '1';
                                    break;
                                    //
                                    case(1):
                                    {
                                        if(data_8 <= '1') data_8 = '3';
                                        else data_8--;
                                    }
                                    break;
                                    //
                                    case(2): 								//xxN.x	// fl [0][01][2].[3]  eq [4][5][6].[7]
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '1')
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '3')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }
                                    break;
                                    //
                                    case(3):
                                    {
                                        if(battVoltSet[0] <= '1' && battVoltSet[1] <= '1' && battVoltSet[2] <= '0')
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                        else if(battVoltSet[0] >= '1' && battVoltSet[1] >= '3' && battVoltSet[2] >= '0') data_8 = '0';
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }break;	
                                    case(4): 
                                    {
                                       data_8 = '1';
                                    }
                                    break;
                                    //
                                    case(5):
                                    {
                                        if(data_8 <= '1') data_8 = '5';
                                        else data_8--;
                                    }
                                    break;
                                    //
                                    case(6): 								//xxN.x	// fl [0][01][2].[3]  eq [4][5][6].[7]
                                    {
                                        if(battVoltSet[4] <= '1' && battVoltSet[5] <= '1') 
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                        else if(battVoltSet[4] >= '1' && battVoltSet[5] >= '5') 
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }
                                    break;
                                    //
                                    case(7):
                                    {
                                        if(battVoltSet[4] >= '1' && battVoltSet[5] >= '5' && battVoltSet[6] >= '0') data_8 = '0';
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }break;	
                                }//end - switch
                                battVoltSet[poss_x]=data_8;
                            }
                            else if(enter) 
                            {
                                if(poss_x >= 7) 
                                {
                                    modi = LO;
                                    CHG_EN = 0;
                                    data_16 = 0;
                                    data_16 +=((battVoltSet[0] & 0x0f) * 1000);
                                    data_16 +=((battVoltSet[1] & 0x0f) * 100);
                                    data_16 +=((battVoltSet[2] & 0x0f) * 10);
                                    data_16 += (battVoltSet[3] & 0x0f);
                                    if(data_16 < (MinOutputVoltage2_PB1*10))// pb110v float volt min 90V
                                    {
                                        battVoltSet[0] = ((MinOutputVoltage2_PB1*10)/ 1000) + 0x30;
                                        battVoltSet[1] = (((MinOutputVoltage2_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[2] = (((MinOutputVoltage2_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[3] = ((MinOutputVoltage2_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100) - ((battVoltSet[2] & 0x0f)*10)) + 0x30;
                                    }
                                    else if(data_16 > (MaxOutputVoltage2_PB1*10))//130.0V
                                    {
                                        battVoltSet[0] = ((MaxOutputVoltage2_PB1*10)/ 1000) + 0x30;
                                        battVoltSet[1] = (((MaxOutputVoltage2_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[2] = (((MaxOutputVoltage2_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[3] = ((MaxOutputVoltage2_PB1*10) - ((battVoltSet[0] & 0x0f) * 1000) - ((battVoltSet[1] & 0x0f) * 100) - ((battVoltSet[2] & 0x0f)*10)) + 0x30;
                                    }   

                                    data_16 = 0;
                                    data_16 +=((battVoltSet[4] & 0x0f) * 1000);
                                    data_16 +=((battVoltSet[5] & 0x0f) * 100);
                                    data_16 +=((battVoltSet[6] & 0x0f) * 10);
                                    data_16 += (battVoltSet[7] & 0x0f);
                                    if(data_16 < (MinOutputVoltage2_PB2*10)) //pb110v eq charge volt min 90V
                                    {
                                        battVoltSet[4] = ((MinOutputVoltage2_PB2*10)/ 1000) + 0x30;
                                        battVoltSet[5] = (((MinOutputVoltage2_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[6] = (((MinOutputVoltage2_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[7] = ((MinOutputVoltage2_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100) - ((battVoltSet[6] & 0x0f)*10)) + 0x30;
                                    }
                                    else if(data_16 > (MaxOutputVoltage2_PB2*10)) //pb110V eq charge volt max 150V
                                    {
                                        battVoltSet[4] = ((MaxOutputVoltage2_PB2*10)/ 1000) + 0x30;
                                        battVoltSet[5] = (((MaxOutputVoltage2_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000))/100) + 0x30;
                                        battVoltSet[6] = (((MaxOutputVoltage2_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100))/10)+ 0x30;
                                        battVoltSet[7] = ((MaxOutputVoltage2_PB2*10) - ((battVoltSet[4] & 0x0f) * 1000) - ((battVoltSet[5] & 0x0f) * 100) - ((battVoltSet[6] & 0x0f)*10)) + 0x30;       
                                    } 
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(BATTVOLT_0+cnt, battVoltSet[cnt]);                            
                                    CHG_SEL = 0;
                                    CHG_CMD();
                                    cal_error = LO;modi_end = HI;Set_OK=1;fEqFl=0; 
                                }
                                else poss_x++;
                                //============================================
                            }
                        }	
                    }
					if(esc) modi = LO;
					key_clr();
				}
			}
            break;
			//
			case(MENU_MANUAL_SET)://11 key_oper()
			{//just view until pressed Enter key//
				if(modi==0){
                    //----------------------------------------------------------
                    if(up)
                    {
                        if(EEPROMRead(BATT_TYPE) == 2) iMenuIndex = MENU_CHARGE_VOLT_SET_LI;//9;
                        else iMenuIndex = MENU_CHARGE_VOLT_SET_PB;//10;
                    }
					else
					{
						if(down) iMenuIndex = MENU_CURRENT_LIMIT;//12;
						else if(enter) 
                        {
                            modi = HI; poss_x = 3;
                            CHG_EN = 1;
                            CHG_SEL = 1;
                            CHG_CMD();
						}
					}
                    //
					if(esc) iMenuIndex = MENU_LOGIN;//5;
                    //
					if(fEqFl == 0)  // Initial Start Equalizing Charge
					{
						//fEqFl=0;
                        for(cnt=0;cnt<4;cnt++) 
						{
                            if(CHG_Mode == 0)   // Equalizing Charge
                            {
                                outSetNew[cnt] = EEPROMRead(BATTVOLT_4+cnt);
                            }
                            else if(CHG_Mode == 1)  // Floating Charge
                            {
                                outSetNew[cnt] = EEPROMRead(BATTVOLT_0+cnt);
                            }   
							LoadAmpSet[cnt] = EEPROMRead(LOADAMP_0+cnt);
						}
						data_16 = ((outSetNew[0] & 0x0f)*1000);
						buff_16 = ((outSetNew[1] & 0x0f)*100);
						data_8 = ((outSetNew[2] & 0x0f)*10);
						buff_8 = (outSetNew[3] & 0x0f);
                        //
						hex1_16 = (data_16 + buff_16 + (unsigned int)data_8 + (unsigned int)buff_8);
//                        hex1_16 = 1310;
					}
					else 
					{
                        if(CHG_EN == 0)
                        {
                            for(cnt=0;cnt<4;cnt++) 
                            {
                                if(CHG_Mode == 0)   // Equalizing Charge
                                {
                                    outSetNew[cnt] = EEPROMRead(BATTVOLT_4+cnt);
                                }
                                else if(CHG_Mode == 1)  // Floating Charge
                                {
                                    outSetNew[cnt] = EEPROMRead(BATTVOLT_0+cnt);
                                } 
                            }
                        }
                        else
                        {
                        //no code
                        }
                        data_16 = ((outSetNew[0] & 0x0f)*1000);
						buff_16 = ((outSetNew[1] & 0x0f)*100);
						data_8 = ((outSetNew[2] & 0x0f)*10);
						buff_8 = (outSetNew[3] & 0x0f);
                        //
						hex1_16 = (data_16 + buff_16 + (unsigned int)data_8 + (unsigned int)buff_8);
					}
                    //
                    //verify voltage range//
                    if(EEPROMRead(BATT_TYPE) == 0)//pb0 125V
                    {
                        if((hex1_16 < MinOutputVoltage_PB1 * 10) && (hex1_16 > MaxOutputVoltage_PB * 10)) //124.7 - 150//
                                    hex1_16 = RefOutputVoltage_PB * 10;			// default set volt 131.0V
                    }
                    else if(EEPROMRead(BATT_TYPE) == 1) //
                    {
                        if((hex1_16 < MinOutputVoltage2_PB1 * 10) && (hex1_16 > MaxOutputVoltage2_PB2 * 10)) //90~150
                                    hex1_16 = RefOutputVoltage2_PB * 10;			// default set volt 124v//
                    }
                    else if(EEPROMRead(BATT_TYPE) == 2)
                    {
                        if((hex1_16 < MinOutputVoltage_LI * 10) && (hex1_16 > MaxOutputVoltage_LI * 10)) //90~135V
                                    hex1_16 = RefOutputVoltage_LI * 10;			// default set volt //125V
                    }                    
					key_clr();
				}//End if(!modi)
            else{//when modi is HIGH// 
					if(up) 
                    {
						data_8 = LoadAmpSet[poss_x-4];
						switch(poss_x) 
                        {
							case(0):case(1):case(2):case(3): Set_OK=1;break;
							case(4):case(5):case(6): break;
						}						
						switch(poss_x) 
                        {
							case(3): 
                            {	
                                if(EEPROMRead(BATT_TYPE) == 0)
                                {      
                                    // manual volt edit and increment
                                    if(hex1_16 >= MaxOutputVoltage_PB * 10) 
                                        hex1_16 = MaxOutputVoltage_PB * 10;
                                    else hex1_16 += 1;
                                } 
                                else if(EEPROMRead(BATT_TYPE) == 1)
                                {      
                                    // manual volt edit and increment
                                    if(hex1_16 >= MaxOutputVoltage2_PB2 * 10) 
                                        hex1_16 = MaxOutputVoltage2_PB2 * 10;
                                    else hex1_16 += 1;
                                } 
                                else if(EEPROMRead(BATT_TYPE) == 2)
                                {
                                    // manual volt edit and increment
                                    if(hex1_16 >= MaxOutputVoltage_LI * 10) 
                                        hex1_16 = MaxOutputVoltage_LI * 10;
                                    else hex1_16 += 1;
                                }
                                
							}break;
							//
							case(4): 
                            {
                                if(data_8 >= '1') data_8 = '0';
                                else data_8++;
                            }break;
							//
							case(5): 
                            {
							   if(LoadAmpSet[0] >= '1')
								{
									if(data_8 >= '2') data_8 = '0';
									else data_8++;
								}
                               else if(LoadAmpSet[0] <= '0')
								{
									if(data_8 >= '9') data_8 = '8';
                                    else if(data_8 < '8') data_8 = '8';
									else data_8++;
								}
								else 
								{
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
							}
							break;
							//
							case(6): 
                            {
								if(LoadAmpSet[0] >= '1' && LoadAmpSet[1] >= '2')
								{
									data_8 = '0';
								}
								else 
								{
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
							}break;
						}
                        LoadAmpSet[poss_x-4] = data_8;
						data_16 = hex1_16; 
                            thnd = ((data_16/1000)+0x30);
                        data_16 %= 1000; 
                            hund = ((data_16/100)+0x30);
                        data_16 %= 100;	
                            ten = ((data_16/10)+0x30);
                        data_16 %= 10; 
                            one = (data_16+0x30);
						outSetNew[0] = thnd; outSetNew[1] = hund; outSetNew[2] = ten; outSetNew[3] = one;
                        Set_VRef = ((outSetNew[0]-0x30)*100)+((outSetNew[1]-0x30)*10)+((outSetNew[2]-0x30)*1)+((outSetNew[3]-0x30)*0.1);
//                        WDT = HI;
//                        Set_OK=1;
					}
					else if(down)
                    {
							data_8 = LoadAmpSet[poss_x-4];
							switch(poss_x) 
                            {
								case(0):case(1):case(2):case(3): Set_OK=1;break;
								case(4):case(5):case(6): break;
							}
							switch(poss_x) 
                            {
								case(3): 
                                {
                                    if(EEPROMRead(BATT_TYPE) == 0)      
                                    {
                                        if(hex1_16 <= MinOutputVoltage_PB1 * 10) 
                                            hex1_16 = MinOutputVoltage_PB1 * 10;
                                        else hex1_16-= 1;
                                    }
                                    else if(EEPROMRead(BATT_TYPE) == 1)      
                                    {
                                        if(hex1_16 <= MinOutputVoltage2_PB1 * 10) 
                                            hex1_16 = MinOutputVoltage2_PB1 * 10;
                                        else hex1_16-= 1;
                                    }
                                    else if(EEPROMRead(BATT_TYPE) == 2)                                    
                                    {
                                        if(hex1_16 <= MinOutputVoltage_LI * 10) 
                                            hex1_16 = MinOutputVoltage_LI * 10;
                                        else hex1_16-= 1;
                                    }
								}break;
								//
								case(4):
								{
									if(data_8 <= '0') data_8 = '1';
									else data_8--;
								}
								break;
								//	
								case(5): 
								{
									if(LoadAmpSet[0] <= '0')
									{
										if(data_8 <= '8') data_8 = '9';
										else data_8--;
									}
                                    else if(LoadAmpSet[0] >= '1')
									{
										if(data_8 <= '0') data_8 = '2';
                                        else if(data_8 > '2') data_8 = '2';
										else data_8--;
									}
									else 
									{
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
								}
								break;
								//
								case(6):
								{
                                    if(LoadAmpSet[0] >= '1' && LoadAmpSet[1] >= '2')
                                    {
                                        data_8 = '0';
                                    }
                                    else 
                                    {
                                        if(data_8 <= '0') data_8 = '9';
                                        else data_8--;
                                    }
								}
							break;
							}
                            LoadAmpSet[poss_x-4] = data_8;
							data_16 = hex1_16;
							thnd = ((data_16/1000)+0x30);data_16 %= 1000;								
							hund = ((data_16/100)+0x30);data_16 %= 100;								
							ten = ((data_16/10)+0x30);data_16 %= 10;								
							one = (data_16+0x30);
							outSetNew[0] = thnd;outSetNew[1] = hund;outSetNew[2] = ten;outSetNew[3] = one;
							Set_VRef = ((outSetNew[0]-0x30)*100)+((outSetNew[1]-0x30)*10)+((outSetNew[2]-0x30)*1)+((outSetNew[3]-0x30)*0.1);                             
//							WDT = HI;
//                            Set_OK=1;
					}
                    else if(enter) 
                    {
                        //outSetNew[poss_x] = temp_buffer[poss_x];
                        if(poss_x >= 6) 
                        {
                            //outSetNew[poss_x] = temp_buffer[poss_x];
                            modi = LO;modi_end = HI;fEqFl=1;
                            data_16 = hex1_16;
                            thnd = ((data_16/1000)+0x30);data_16 %= 1000;								
                            hund = ((data_16/100)+0x30);data_16 %= 100;								
                            ten = ((data_16/10)+0x30);data_16 %= 10;								
                            one = (data_16+0x30);
                            outSetNew[0] = thnd; outSetNew[1] = hund; outSetNew[2] = ten; outSetNew[3] = one;
                            data_16 = 0;
                            data_16 +=((LoadAmpSet[0] & 0x0f) * 100);
                            data_16 +=((LoadAmpSet[1] & 0x0f) * 10);
                            data_16 += (LoadAmpSet[2] & 0x0f);
                            if(data_16 > OCL) //120.0A
                            {
                                LoadAmpSet[0] = (OCL / 100) + 0x30;
                                LoadAmpSet[1] = ((OCL - ((LoadAmpSet[0]-0x30) * 100))/10) + 0x30;
                                LoadAmpSet[2] = (OCL - ((LoadAmpSet[0]-0x30) * 100) - ((LoadAmpSet[1] - 0x30) * 10)) + 0x30;
                            }
                            else if(data_16 < UCL) //80.0A
                            {
                                LoadAmpSet[0] = (UCL / 100) + 0x30;
                                LoadAmpSet[1] = ((UCL - ((LoadAmpSet[0]-0x30) * 100))/10) + 0x30;
                                LoadAmpSet[2] = (UCL - ((LoadAmpSet[0]-0x30) * 100) - ((LoadAmpSet[1] - 0x30) * 10)) + 0x30;
                            }

                            for(cnt=0;cnt<4;cnt++) EEPROMWrite(LOADAMP_0+cnt, LoadAmpSet[cnt]);
                            Set_VRef = ((outSetNew[0] & 0x0F)*100)+((outSetNew[1] & 0x0F)*10)+((outSetNew[2] & 0x0F)*1)+((outSetNew[3] & 0x0F)*0.1);
                            Set_CL = ((LoadAmpSet[0] & 0x0F)*100)+((LoadAmpSet[1] & 0x0F)*10)+(LoadAmpSet[2] & 0x0F);
                            CHG_Check();
                            //WDT = HI;
                            Set_OK=1;
                        }
                        else 
                        {
                            if(poss_x==0) poss_x=3;
                            else poss_x++;
                        }
                    }
					else if(esc) {
						modi = LO;fEqFl=1;
						data_16 = hex1_16;
						thnd = ((data_16/1000)+0x30);data_16 %= 1000;								
						hund = ((data_16/100)+0x30);data_16 %= 100;								
						ten = ((data_16/10)+0x30);data_16 %= 10;								
						one = (data_16+0x30);
						outSetNew[0] = thnd;outSetNew[1] = hund;outSetNew[2] = ten;outSetNew[3] = one;
						Set_VRef = ((outSetNew[0] & 0x0F)*100)+((outSetNew[1] & 0x0F)*10)+((outSetNew[2] & 0x0F)*1)+((outSetNew[3] & 0x0F)*0.1);
                        Set_CL = ((LoadAmpSet[0] & 0x0F)*100)+((LoadAmpSet[1] & 0x0F)*10)+(LoadAmpSet[2] & 0x0F);
                        CHG_Check();
						//WDT = HI;
                        Set_OK=1;
					}
					key_clr();
                    //
				}
                
			}break;
			//
			case(MENU_CURRENT_LIMIT): {
				if(!modi)
				{
					if(up)iMenuIndex = MENU_MANUAL_SET;//11;
					else 
					{
						if(down) iMenuIndex = MENU_INPUT_ALARM_SET;//13;
						else if(enter)
						{
							modi = HI;
							poss_x = poss_y = 1;
							for(cnt=0;cnt<4;cnt++)
							{
								battAmpSet[cnt] = EEPROMRead(BATTAMP_0+cnt);
							}
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
					if(up) 
					{
						data_8 = battAmpSet[poss_x];
						switch(poss_x) 	
						{
							case(0):
								data_8 = '0';
								break;
								//
							case(1):
								{
									if(data_8 >= '4') data_8= '0';
									else data_8++;
								}break;
							//
							case(2):
								{
                                    if(battAmpSet[1] >= '4')
									{
										data_8 = '0';
									}
									else
									{
										if(data_8 >= '9') data_8 = '0';
										else data_8++;
									}
								}break;
							//
							case(3):
								data_8 = '0';
								break;
						}//end - switch(poss_x)
						battAmpSet[poss_x] = data_8;
					}
					else //down key pressed 
					{
						if(down)
						{
							data_8 = battAmpSet[poss_x];
							switch(poss_x) 
							{
								case(0): 
									data_8 = '0';
									break;
								//
								case(1):
									{
										if(data_8 <= '0') data_8 = '4';
										else data_8--;
									}break;
								//
								case(2):
									{
										if(battAmpSet[1] >= '4' )
										{
											data_8 = '0';
										}
										else
										{
											if(data_8 <= '0') data_8 = '9';
											else data_8--;
										}
									}break;
								//
								case(3):
									data_8 = '0';
									break;
							}//end - switch
							battAmpSet[poss_x] = data_8;
						}
						else if(enter) 
						{
							if(poss_x >= 2)
							{
								modi = LO;
                                data_16 = 0;
                                data_16 += (battAmpSet[0] - 0x30) * 1000;
                                data_16 += (battAmpSet[1] - 0x30) * 100;
                                data_16 += (battAmpSet[2] - 0x30) * 10;
                                data_16 += (battAmpSet[3] - 0x30) * 1;
                                if(data_16 > (MaxBattAmp * 10))//40.0A
                                {
                                    battAmpSet[0] = ((MaxBattAmp*10) / 1000) + 0x30;
                                    battAmpSet[1] = (((MaxBattAmp*10) - ((battAmpSet[0]-0x30)*1000)) / 100) + 0x30;
                                    battAmpSet[2] = (((MaxBattAmp*10) - ((battAmpSet[0]-0x30)*1000) - ((battAmpSet[1]-0x30)*100)) / 10) + 0x30;
                                    battAmpSet[3] = ((MaxBattAmp*10) - ((battAmpSet[0]-0x30)*1000) - ((battAmpSet[1]-0x30)*100) - ((battAmpSet[2]-0x30)*10)) + 0x30;
                                }
                                else if(data_16 < (MinBattAmp * 10))//1.0A
                                {
                                     battAmpSet[0] = ((MinBattAmp*10) / 1000) + 0x30;
                                    battAmpSet[1] = (((MinBattAmp*10) - ((battAmpSet[0]-0x30)*1000)) / 100) + 0x30;
                                    battAmpSet[2] = (((MinBattAmp*10) - ((battAmpSet[0]-0x30)*1000) - ((battAmpSet[1]-0x30)*100)) / 10) + 0x30;
                                    battAmpSet[3] = ((MinBattAmp*10) - ((battAmpSet[0]-0x30)*1000) - ((battAmpSet[1]-0x30)*100) - ((battAmpSet[2]-0x30)*10)) + 0x30;
                                }
								for(cnt=0;cnt<4;cnt++) EEPROMWrite(BATTAMP_0+cnt, battAmpSet[cnt]);
								Set_BattAmp=((battAmpSet[0] & 0x0F)*100)+((battAmpSet[1] & 0x0F)*10)+((battAmpSet[2] & 0x0F)*1)+((battAmpSet[3] & 0x0F)*0.1);
								cal_error = LO;modi_end = HI;Set_OK=1;
							}
							else poss_x++;
							//============================================
						}
					}					
					if(esc) modi = LO;
					key_clr();
				}
			}break;
            
			//Input Voltage Alram Set
			case(MENU_INPUT_ALARM_SET): 
            {
                if(EEPROMRead(INPUT_VOLT) == 0)
                {
                    if(!modi)
                    {
                        if(up) iMenuIndex = MENU_CURRENT_LIMIT;//12;
                        else 
                        {
                            if(down) iMenuIndex = MENU_OUTPUT_ALARM_SET;//14;
                            else if(enter) 
                            {
                                modi = HI;
                                poss_x = poss_y = 0;
                                for(cnt=0;cnt<8;cnt++)
                                {
                                    ginValarm[cnt] = EEPROMRead(INVALARM_0+cnt);
                                }
                            }
                        }
                        if(esc) iMenuIndex = MENU_LOGIN;//5;
                        key_clr();
                    }
                    else
                    {
                        if(up) 
                        {
                            data_8 = ginValarm[poss_x];
                            switch(poss_x) 	
                            {
                                case(0):
                                {
                                    data_8 = '4';
                                }
                                    break;
                                    //
                                case(1):
                                    {
                                        if(ginValarm[0] >= '4')
                                        {	 
                                            if(data_8>= '1') data_8 = '0';
                                            else data_8++;
                                        }	
                                        else if(ginValarm[0] <= '3')
                                        {
                                            if(data_8>= '9') data_8 = '8';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(2):
                                    {
                                        if(ginValarm[0] >= '4' && ginValarm[1] >= '1')
                                        {
                                            if(data_8 >= '8') data_8 = '0';
                                            else data_8++;
                                        }
                                        else if(ginValarm[0] <= '3' && ginValarm[1] <= '8')
                                        {
                                            if(data_8 >= '9') data_8 = '1';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(3):
                                    {
                                        if(ginValarm[0] >= '4' && ginValarm[1] >= '1' && ginValarm[2] >= '8')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(4):
                                    {
                                        data_8 = '3';
                                    }break;
                                //
                                case(5): 				
                                    {
                                        if(data_8>= '7') data_8 = '4';
                                        else data_8++;                                    
                                    }break;
                                //
                                case(6): 
                                    {
                                        if(ginValarm[5] <= '4')
                                        {
                                            if(data_8 >= '9') data_8 = '2';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(7): 
                                    {
                                        if(ginValarm[4] >= '3' && ginValarm[5] >= '7' && ginValarm[6] >= '9') 
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }		
                                    }break;
                            }//end - switch(poss_x)
                            ginValarm[poss_x]=data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = ginValarm[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                        data_8 = '3';
                                        break;
                                    //
                                    case(1):
                                        {
                                            if(ginValarm[0] <= '3')
                                            {
                                                if(data_8 <= '8') data_8 = '9';
                                                else data_8--;
                                            }
                                            else if(ginValarm[0] >= '4')
                                            {
                                                if(data_8 <= '0') data_8 = '1';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(ginValarm[0] <= '3' && ginValarm[1] <= '8') 
                                            {
                                                if(data_8 <= '1') data_8 = '9';
                                                else data_8--;
                                            }
                                            else if(ginValarm[0] >= '4' && ginValarm[1] >= '1') 
                                            {
                                                if(data_8 <= '0') data_8 = '8';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }											
                                        }break;
                                    //
                                    case(3):
                                    {
                                        if(ginValarm[0] >= '4' && ginValarm[1] >= '1' && ginValarm[2] >= '8') 
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }break;
                                    //
                                    case(4):
                                        data_8 = '3';
                                        break;
                                    //	
                                    case(5): 
                                        {
                                            if(data_8 <= '4') data_8 = '7';
                                            else data_8--;
                                        }break;
                                    //
                                    case(6):
                                        {
                                            if(ginValarm[5] <= '4')
                                            {
                                                if(data_8 <= '2') data_8 = '9';
                                                else data_8--;
                                            }
                                            else if(ginValarm[5] >= '8') data_8 = '0';
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(7):
                                    {
                                        if(ginValarm[4] >= '3' && ginValarm[5] >= '7' && ginValarm[6] >= '9') 
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 <= '0') data_8 = '9';
                                            else data_8--;
                                        }
                                    }break;									
                                }//end - switch
                                ginValarm[poss_x] = data_8;
                            }
                            else if(enter) 
                            {
                                if(poss_x >= 7)
                                {
                                    modi = LO;
                                    data_16 = (((ginValarm[0] & 0x0F)*1000)+((ginValarm[1] & 0x0F)*100)+((ginValarm[2] & 0x0F)*10)+(ginValarm[3] & 0x0F));

                                    if(data_16 > (Input_OV_Ref*10))
                                    {
                                        ginValarm[0] = ((MaxInputVoltage_380*10) / 1000) + 0x30;//418ACV
                                        ginValarm[1] = (((MaxInputVoltage_380*10) - ((ginValarm[0] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[2] = (((MaxInputVoltage_380*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[3] = ((MaxInputVoltage_380*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100) - ((ginValarm[2] - 0x30) * 10)) + 0x30;                                       
                                    }
                                    else if(data_16 < ((Input_V_Ref+1)*10))
                                    {
                                        ginValarm[0] = (((RefInputVoltage_380+1)*10) / 1000) + 0x30;//380ACV
                                        ginValarm[1] = ((((RefInputVoltage_380+1)*10) - ((ginValarm[0] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[2] = ((((RefInputVoltage_380+1)*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[3] = (((RefInputVoltage_380+1)*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100) - ((ginValarm[2] - 0x30) * 10)) + 0x30; 
                                    }

                                    data_16=(((ginValarm[4] & 0x0F)*1000)+((ginValarm[5] & 0x0F)*100)+((ginValarm[6] & 0x0F)*10)+(ginValarm[7] & 0x0F));
                                    if(data_16 < Input_UV_Ref*10)
                                    {
                                        ginValarm[4] = ((MinInputVoltage_380*10) / 1000) + 0x30;//342ACV
                                        ginValarm[5] = (((MinInputVoltage_380*10) - ((ginValarm[4] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[6] = (((MinInputVoltage_380*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[7] = ((MinInputVoltage_380*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100) - ((ginValarm[6] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 > ((Input_V_Ref-1)*10))
                                    {
                                        ginValarm[4] = (((RefInputVoltage_380-1)*10) / 1000) + 0x30;
                                        ginValarm[5] = ((((RefInputVoltage_380-1)*10) - ((ginValarm[4] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[6] = ((((RefInputVoltage_380-1)*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[7] = (((RefInputVoltage_380-1)*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100) - ((ginValarm[6] - 0x30) * 10)) + 0x30; 
                                    }
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(INVALARM_0+cnt, ginValarm[cnt]);
                                    Set_Input_OV=((ginValarm[0] & 0x0F)*100) + ((ginValarm[1] & 0x0F)*10)+ ((ginValarm[2] & 0x0F)*1) + ((ginValarm[3] & 0x0F)*0.1);
                                    Set_Input_UV=((ginValarm[4] & 0x0F)*100)+((ginValarm[5] & 0x0F)*10)+ ((ginValarm[6] & 0x0F)*1) + ((ginValarm[7] & 0x0F)*0.1);
                                    cal_error = LO;modi_end = HI;Set_OK=1;
                                }
                                else poss_x++;
                            }
                        }					
                        if(esc) modi = LO;
                        key_clr();
                    }
                }
                else                    
                {
                    if(!modi)
                    {
                        if(up) iMenuIndex = MENU_CURRENT_LIMIT;//12;
                        else 
                        {
                            if(down) iMenuIndex = MENU_OUTPUT_ALARM_SET;//14;
                            else if(enter) 
                            {
                                modi = HI;
                                poss_x = poss_y = 0;
                                for(cnt=0;cnt<8;cnt++)
                                {
                                    ginValarm[cnt] = EEPROMRead(INVALARM_0+cnt);
                                }
                            }
                        }
                        if(esc) iMenuIndex = MENU_LOGIN;//5;
                        key_clr();
                    }
                    else
                    {
                        if(up) 
                        {
                            data_8 = ginValarm[poss_x];
                            switch(poss_x)
                            {
                                case(0):
                                    {
                                        data_8 = '2';
                                    }
                                    break;
                                    //
                                case(1):
                                    {
                                        if(data_8>= '4') data_8 = '2';
                                        else data_8++;
                                    }break;
                                //
                                case(2):
                                    {
                                        if(ginValarm[1] == '4')
                                         {
                                            if(data_8 >= '2') data_8 = '0';
                                            else data_8++;
                                         }
                                         else if(ginValarm[1] == '2')
                                         {
                                            if(data_8 >= '9') data_8 = '1';
                                            else data_8++;
                                         }
                                         else 
                                         {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                         }
                                    }break;
                                //
                                case(3):
                                    {
                                        if(ginValarm[0] == '2' && ginValarm[1] == '4' && ginValarm[2] == '2')
                                        {
                                            data_8 = '0';
                                        }
                                        else 
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(4):
                                    {
                                        if(data_8 >= '2') data_8 = '1';
                                        else data_8++;
                                    }break;
                                //
                                case(5): 				
                                    {
                                        if(ginValarm[4] == '1')
                                        {
                                            data_8 = '9';
                                        }
                                        else if(ginValarm[4] == '2')
                                        {
                                            if(data_8 >= '1') data_8 = '0';
                                            else data_8++;
                                        }
                                        else 
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(6): 
                                    {
                                        if(ginValarm[4] == '1' && ginValarm[5] == '9')
                                        {
                                            if(data_8 >= '9') data_8 = '8';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(7): 
                                    {
                                        if(ginValarm[4] == '2' && ginValarm[5] == '1' && ginValarm[6] == '9')
                                        {
                                            data_8 = '0';
                                        }	
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                            }//end - switch(poss_x)
                            ginValarm[poss_x]=data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = ginValarm[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                    {
                                        data_8 = '2';
                                    }break;
                                    //
                                    case(1):
                                        {
                                            if(data_8 <= '2') data_8 = '4';
                                            else data_8--;
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(ginValarm[1] == '2') 
                                            {
                                                if(data_8 <= '1') data_8 = '9';
                                                else data_8--;
                                            }  
                                            else if(ginValarm[1] == '4')
                                            {
                                                if(data_8 <= '0') data_8 = '2';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }											
                                        }break;
                                    //
                                    case(3):
                                        {
                                            if(ginValarm[0] == '2' && ginValarm[1] == '4' && ginValarm[2] == '2') 
                                            {
                                                data_8 = '0';
                                            }  
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }
                                        break;
                                    //
                                    case(4):
                                        {
                                            if(data_8 <= '1') data_8 = '2';
                                            else data_8--;
                                    }
                                        break;
                                    //	
                                    case(5): 
                                        {
                                            if(ginValarm[4] == '1') 
                                            {
                                                data_8 = '9';
                                            }
                                            else if(ginValarm[4] == '2')
                                            {
                                                if(data_8 <= '0') data_8 = '1';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(6):
                                        {
                                            if(ginValarm[4] == '1' && ginValarm[5] == '9')
                                            {
                                                if(data_8 <= '8') data_8 = '9';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(7):
                                        {
                                            if(ginValarm[4] == '2' && ginValarm[5] == '1' && ginValarm[6] == '9')
                                            {
                                                data_8 = '0';                                                
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }
                                        break;									
                                }//end - switch
                                ginValarm[poss_x] = data_8;
                            }
                            else if(enter) 
                            {
                                if(poss_x >= 7)
                                {
                                    modi = LO;
                                    data_16=(((ginValarm[0] & 0x0F)*1000)+((ginValarm[1] & 0x0F)*100)+((ginValarm[2] & 0x0F)*10)+(ginValarm[3] & 0x0F));

                                    if(data_16 > (Input_OV_Ref * 10))
                                    {
                                        ginValarm[0] = ((MaxInputVoltage_220*10) / 1000) + 0x30;
                                        ginValarm[1] = (((MaxInputVoltage_220*10) - ((ginValarm[0] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[2] = (((MaxInputVoltage_220*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[3] = ((MaxInputVoltage_220*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100) - ((ginValarm[2] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 < ((Input_V_Ref + 1)*10))
                                    {
                                        ginValarm[0] = (((RefInputVoltage_220+1)*10) / 1000) + 0x30;
                                        ginValarm[1] = ((((RefInputVoltage_220+1)*10) - ((ginValarm[0] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[2] = ((((RefInputVoltage_220+1)*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[3] = (((RefInputVoltage_220+1)*10) - ((ginValarm[0] - 0x30) * 1000) - ((ginValarm[1] - 0x30) * 100) - ((ginValarm[2] - 0x30) * 10)) + 0x30;
                                    }

                                    data_16=(((ginValarm[4] & 0x0F)*1000)+((ginValarm[5] & 0x0F)*100)+((ginValarm[6] & 0x0F)*10)+(ginValarm[7] & 0x0F));
                                    if(data_16 < (Input_UV_Ref * 10))
                                    {
                                        ginValarm[4] = ((MinInputVoltage_220*10) / 1000) + 0x30;
                                        ginValarm[5] = (((MinInputVoltage_220*10) - ((ginValarm[4] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[6] = (((MinInputVoltage_220*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[7] = ((MinInputVoltage_220*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100) - ((ginValarm[6] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 > ((Input_V_Ref - 1)*10))
                                    {
                                        ginValarm[4] = (((RefInputVoltage_220-1)*10) / 1000) + 0x30;
                                        ginValarm[5] = ((((RefInputVoltage_220-1)*10) - ((ginValarm[4] - 0x30) * 1000)) / 100) + 0x30;
                                        ginValarm[6] = ((((RefInputVoltage_220-1)*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100))/10) + 0x30;
                                        ginValarm[7] = (((RefInputVoltage_220-1)*10) - ((ginValarm[4] - 0x30) * 1000) - ((ginValarm[5] - 0x30) * 100) - ((ginValarm[6] - 0x30) * 10)) + 0x30;
                                    }

                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(INVALARM_0+cnt, ginValarm[cnt]);
                                    Set_Input_OV=((ginValarm[0] & 0x0F)*100) + ((ginValarm[1] & 0x0F)*10)+ (ginValarm[2] & 0x0F) + ((ginValarm[3] & 0x0F)*0.1);
                                    Set_Input_UV=((ginValarm[4] & 0x0F)*100)+((ginValarm[5] & 0x0F)*10)+ (ginValarm[6] & 0x0F) + ((ginValarm[7] & 0x0F)*0.1);
                                    cal_error = LO;modi_end = HI;Set_OK=1;
                                }
                                else poss_x++;
                            }
                        }					
                        if(esc) modi = LO;
                        key_clr();
                    }
                }
                
			}break;
            
			//
			case(MENU_OUTPUT_ALARM_SET): {
				if(!modi)
				{
					if(up)iMenuIndex = MENU_INPUT_ALARM_SET;//13;
					else 
					{
						if(down) iMenuIndex = MENU_OUT_CUR_ALARM_SET;//15;
						else if(enter) 
						{
							modi = HI;
							poss_x = poss_y = 0;
							for(cnt=0;cnt<8;cnt++)
							{
								goutValarm[cnt] = EEPROMRead(OUTVALARM_0+cnt);
							}
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
                    if(EEPROMRead(BATT_TYPE) == 0)//pb125V
                    {
                        if(up) 
                        {
                            data_8 = goutValarm[poss_x];
                            switch(poss_x) 	
                            {
                                case(0):
                                {
                                    data_8 = '1';
                                }
                                break;
                                //
                                case(1):
                                {
                                    if (goutValarm[0] >= '1')
                                    {
                                        if (data_8 >= '5') data_8 = '2';
                                        else if (data_8 < '2') data_8 = '5';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(2):
                                {
                                    if (goutValarm[0] <= '1' && goutValarm[1] <= '2')
                                    {
                                        if (data_8 >= '9') data_8 = '7';
                                        else if (data_8 < '7') data_8 = '9';
                                        else data_8++;
                                    }                                    
                                    else if (goutValarm[0] >= '1' && goutValarm[1] >= '5')
                                    {
                                        data_8 = '0';
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(3):
                                {
                                    if (goutValarm[0] <= '1' && goutValarm[1] <= '2' && goutValarm[2] <= '7')
                                    {
                                        if (data_8 >= '9') data_8 = '6';
                                        else if (data_8 < '6') data_8 = '9';
                                        else data_8++;
                                    }
                                    else if (goutValarm[0] >= '1' && goutValarm[1] >= '5' && goutValarm[2] >= '0')
                                    {
                                        data_8 = '0';
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(4):
                                {
                                    if (data_8 >= '1') data_8 = '0';
                                    else data_8++;
                                }break;
                                //
                                case(5):
                                {
                                    if(goutValarm[4] <= '0')
                                    {
                                        data_8 = '9';
                                    }
                                    else if(goutValarm[4] >= '1')
                                    {
                                        if (data_8 >= '3') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '3';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(6):
                                {
                                    if(goutValarm[4] <= '0' && goutValarm[5] <= '9')
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else if(data_8 < '0') data_8 = '9';
                                        else data_8++;
                                    }
                                    else if(goutValarm[4] >= '1' && goutValarm[5] >= '3')
                                    {
                                        if(data_8 >= '2') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '2';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(7):
                                {
                                    if (goutValarm[4] <= '0' && goutValarm[5] <= '9' && goutValarm[6] <= '8')
                                    {
                                        if (data_8 >= '9') data_8 = '6';
                                        else if (data_8 < '6') data_8 = '9';
                                        else data_8++;
                                    }
                                    else if (goutValarm[4] >= '1' && goutValarm[5] >= '3' && goutValarm[6] >= '2')
                                    {
                                        data_8 = '0';
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                            }//end - switch(poss_x)
                            goutValarm[poss_x] = data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = goutValarm[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                        data_8 = '1';
                                        break;
                                    //
                                    case(1):
                                        {
                                            if(goutValarm[0] >= '1')
                                            {
                                                if(data_8 <= '2') data_8= '5';
                                                else if (data_8 > '5') data_8 = '2';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8= '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(goutValarm[0] <= '1' && goutValarm[1] <= '2')
                                            {
                                                if(data_8 <= '7') data_8= '9';
                                                else if (data_8 > '9') data_8 = '7';
                                                else data_8--;
                                            }
                                            else if(goutValarm[0] >= '1' && goutValarm[1] >= '5')
                                            {
                                                data_8= '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(3):
                                        {
                                            if(goutValarm[0] <= '1' && goutValarm[1] <= '2' && goutValarm[2] <= '7')
                                            {
                                                if(data_8 <= '6') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '6';
                                                else data_8--;
                                            }	
                                            else if(goutValarm[0] >= '1' && goutValarm[1] >= '5' && goutValarm[2] >= '0')
                                            {
                                                data_8 = '0';
                                            }	
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;									
                                            }
                                        }break;
                                    //
                                    case(4):
                                        {
                                            if(data_8 <= '0') data_8 = '1';
                                            else data_8--;
                                        }                                        
                                        break;
                                //	
                                    case(5):
                                        {
                                            if(goutValarm[4] <= '0')
                                            {
                                                data_8 = '9';
                                            }
                                            else if(goutValarm[4] >= '1')
                                            {
                                                if(data_8 <= '0') data_8 = '3';
                                                else if(data_8 > '3') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(6):
                                        {
                                            if(goutValarm[4] <= '0' && goutValarm[5] <= '9')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(goutValarm[4] >= '1' && goutValarm[5] >= '3')
                                            {
                                                if(data_8 <= '0') data_8 = '2';
                                                else if(data_8 > '2') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(7):
                                        {
//                                            if(goutValarm[4] <= '0' && goutValarm[5] <= '9' && goutValarm[6] <= '8')
//                                            {
//                                                if(data_8 <= '6') data_8 = '9';
//                                                else if (data_8 > '9') data_8 = '6';
//                                                else data_8--;
//                                            }
//                                            else if(goutValarm[4] >= '1' && goutValarm[5] >= '3' && goutValarm[6] >= '2')
                                            if(goutValarm[4] >= '1' && goutValarm[5] >= '3' && goutValarm[6] >= '2')
                                            {
                                                data_8 = '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }break;
                                }//end - switch
                                goutValarm[poss_x] = data_8;
                            }
                            else if(enter) 
                            {
                                if(poss_x >= 7) //150 ~127.6
                                {
                                    data_16=(((goutValarm[0] & 0x0F)*1000)+((goutValarm[1] & 0x0F)*100)+((goutValarm[2] & 0x0F)*10)+(goutValarm[3] & 0x0F));
                                    if(data_16 > (MaxOutputVoltage_PB * 10))//150v
                                    {
                                        goutValarm[0] = ((MaxOutputVoltage_PB*10) / 1000) + 0x30;
                                        goutValarm[1] = (((MaxOutputVoltage_PB*10) - ((goutValarm[0] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[2] = (((MaxOutputVoltage_PB*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[3] = ((MaxOutputVoltage_PB*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100) - ((goutValarm[2] & 0x0f)*10)) + 0x30;  
                                    }
                                    else if(data_16 < (MinOutputVoltage_PB2 * 10))//127.6
                                    {
                                        goutValarm[0] = ((MinOutputVoltage_PB2*10) / 1000) + 0x30;
                                        goutValarm[1] = (((MinOutputVoltage_PB2*10) - ((goutValarm[0] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[2] = (((MinOutputVoltage_PB2*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[3] = ((MinOutputVoltage_PB2*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100) - ((goutValarm[2] & 0x0f)*10)) + 0x30;  
                                    }
                                    //min (90~110V-132v)
                                    data_16=(((goutValarm[4] & 0x0F)*1000)+((goutValarm[5] & 0x0F)*100)+((goutValarm[6] & 0x0F)*10)+(goutValarm[7] & 0x0F));
                                    if(data_16 < (MinOutputVoltage_PB * 10))//90
                                    {
                                        goutValarm[4] = ((MinOutputVoltage_PB*10) / 1000) + 0x30;
                                        goutValarm[5] = (((MinOutputVoltage_PB*10) - ((goutValarm[4] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[6] = (((MinOutputVoltage_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[7] = ((MinOutputVoltage_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100) - ((goutValarm[6] & 0x0f)*10)) + 0x30;  
                                    }
                                    else if(data_16 > (MidOutputVoltage_PB * 10))//132.0V
                                    {
                                        goutValarm[4] = ((MidOutputVoltage_PB*10) / 1000) + 0x30;
                                        goutValarm[5] = (((MidOutputVoltage_PB*10) - ((goutValarm[4] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[6] = (((MidOutputVoltage_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[7] = ((MidOutputVoltage_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100) - ((goutValarm[6] & 0x0f)*10)) + 0x30;  
                                    }

                                    modi = LO;
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(OUTVALARM_0+cnt, goutValarm[cnt]);
                                    cal_error = LO;modi_end = HI;
                                    Set_Output_OV=((goutValarm[0]-0x30)*100)+((goutValarm[1]-0x30)*10)+((goutValarm[2]-0x30)*1)+((goutValarm[3]-0x30)*0.1);
                                    Set_Output_UV=((goutValarm[4]-0x30)*100)+((goutValarm[5]-0x30)*10)+((goutValarm[6]-0x30)*1)+((goutValarm[7]-0x30)*0.1);
                                }
                                else poss_x++;
                                //============================================	
                            }
                        }
                    }
                    else if(EEPROMRead(BATT_TYPE) == 1)//pb110V
                    {
                        if(up) 
                        {
                            data_8 = goutValarm[poss_x];
                            switch(poss_x) 	
                            {
                                case(0):
                                {
                                    data_8 = '1';
                                }
                                break;
                                //
                                case(1):
                                {
                                    if (goutValarm[0] >= '1')
                                    {
                                        if (data_8 >= '5') data_8 = '2';
                                        else if (data_8 < '2') data_8 = '5';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(2):
                                {
                                    if (goutValarm[0] <= '1' && goutValarm[1] <= '2')
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '9';
                                        else data_8++;
                                    }                                    
                                    else if (goutValarm[0] >= '1' && goutValarm[1] >= '5')
                                    {
                                        data_8 = '0';
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(3):
                                {
                                    if (goutValarm[0] <= '1' && goutValarm[1] <= '2' && goutValarm[2] <= '0')
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '9';
                                        else data_8++;
                                    }
                                    else if (goutValarm[0] >= '1' && goutValarm[1] >= '5' && goutValarm[2] >= '0')
                                    {
                                        data_8 = '0';
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(4):
                                {
                                    data_8 = '1';
                                }break;
                                //
                                case(5):
                                {
                                   if(goutValarm[4] >= '1')
                                    {
                                        if (data_8 >= '2') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '2';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(6):
                                {
                                    if(goutValarm[4] <= '1' && goutValarm[5] <= '0')
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '9';
                                        else data_8++;
                                    }
                                    else if(goutValarm[4] >= '1' && goutValarm[5] >= '2')
                                    {
                                        data_8 = '0';
                                    }
                                    else
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(7):
                                {
                                    if (goutValarm[4] <= '1' && goutValarm[5] <= '0' && goutValarm[6] <= '0')
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '9';
                                        else data_8++;
                                    }
                                    else if (goutValarm[4] >= '1' && goutValarm[5] >= '2' && goutValarm[6] >= '0')
                                    {
                                        data_8 = '0';
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                            }//end - switch(poss_x)
                            goutValarm[poss_x] = data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = goutValarm[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                        data_8 = '1';
                                        break;
                                    //
                                    case(1):
                                        {
                                            if(goutValarm[0] >= '1')
                                            {
                                                if(data_8 <= '2') data_8= '5';
                                                else if (data_8 > '5') data_8 = '2';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8= '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(goutValarm[0] <= '1' && goutValarm[1] <= '2')
                                            {
                                                if(data_8 <= '0') data_8= '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(goutValarm[0] >= '1' && goutValarm[1] >= '5')
                                            {
                                                data_8= '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(3):
                                        {
                                            if(goutValarm[0] <= '1' && goutValarm[1] <= '2' && goutValarm[2] <= '0')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }	
                                            else if(goutValarm[0] >= '1' && goutValarm[1] >= '5' && goutValarm[2] >= '0')
                                            {
                                                data_8 = '0';
                                            }	
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;									
                                            }
                                        }break;
                                    //
                                    case(4):
                                        {
                                            data_8 = '1';
                                        }                                        
                                        break;
                                //	
                                    case(5):
                                        {
                                            if(goutValarm[4] >= '1')
                                            {
                                                if(data_8 <= '0') data_8 = '2';
                                                //if(data_8 >= '2') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(6):
                                        {
//                                            if(goutValarm[4] <= '1' && goutValarm[5] <= '0')
//                                            {
//                                                 if(data_8 <= '5') data_8 = '9';
//                                                 else if (data_8 > '9') data_8 = '5';
//                                                else data_8--;
//                                            }
//                                            else if(goutValarm[4] >= '1' && goutValarm[5] >= '2')
                                            if(goutValarm[4] >= '1' && goutValarm[5] >= '2')
                                            {
                                                data_8 = '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(7):
                                        {
                                            if(goutValarm[4] <= '1' && goutValarm[5] <= '0' && goutValarm[6] <= '0')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                            else if(goutValarm[4] >= '1' && goutValarm[5] >= '2' && goutValarm[6] >= '0')
                                            {
                                                data_8 = '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }break;
                                }//end - switch
                                goutValarm[poss_x] = data_8;
                            }
                            else if(enter) 
                            {
                                if(poss_x >= 7)
                                {
                                    data_16=(((goutValarm[0] & 0x0F)*1000)+((goutValarm[1] & 0x0F)*100)+((goutValarm[2] & 0x0F)*10)+(goutValarm[3] & 0x0F));
                                    if(data_16 > (MaxOutputVoltage2_PB * 10))//150.0V
                                    {
                                        goutValarm[0] = ((MaxOutputVoltage2_PB*10) / 1000) + 0x30;
                                        goutValarm[1] = (((MaxOutputVoltage2_PB*10) - ((goutValarm[0] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[2] = (((MaxOutputVoltage2_PB*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[3] = ((MaxOutputVoltage2_PB*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100) - ((goutValarm[2] & 0x0f)*10)) + 0x30;  
                                    }
                                    else if(data_16 < (MinOutputVoltage2_PB2 * 10)) //90V
                                    {
                                        goutValarm[0] = ((MinOutputVoltage2_PB2*10) / 1000) + 0x30;
                                        goutValarm[1] = (((MinOutputVoltage2_PB2*10) - ((goutValarm[0] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[2] = (((MinOutputVoltage2_PB2*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[3] = ((MinOutputVoltage2_PB2*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100) - ((goutValarm[2] & 0x0f)*10)) + 0x30;  
                                    }

                                    data_16=(((goutValarm[4] & 0x0F)*1000)+((goutValarm[5] & 0x0F)*100)+((goutValarm[6] & 0x0F)*10)+(goutValarm[7] & 0x0F));
                                    if(data_16 < (MinOutputVoltage2_PB * 10)) //90.0V
                                    {
                                        goutValarm[4] = ((MinOutputVoltage2_PB*10) / 1000) + 0x30;
                                        goutValarm[5] = (((MinOutputVoltage2_PB*10) - ((goutValarm[4] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[6] = (((MinOutputVoltage2_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[7] = ((MinOutputVoltage2_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100) - ((goutValarm[6] & 0x0f)*10)) + 0x30;  
                                    }
                                    else if(data_16 > (MidOutputVoltage2_PB * 10))//120V
                                    {
                                        goutValarm[4] = ((MidOutputVoltage2_PB*10) / 1000) + 0x30;
                                        goutValarm[5] = (((MidOutputVoltage2_PB*10) - ((goutValarm[4] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[6] = (((MidOutputVoltage2_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[7] = ((MidOutputVoltage2_PB*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100) - ((goutValarm[6] & 0x0f)*10)) + 0x30;  
                                    }                                 
                                                                        
                                    modi = LO;
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(OUTVALARM_0+cnt, goutValarm[cnt]);
                                    cal_error = LO;modi_end = HI;
                                    Set_Output_OV=((goutValarm[0]-0x30)*100)+((goutValarm[1]-0x30)*10)+((goutValarm[2]-0x30)*1)+((goutValarm[3]-0x30)*0.1);
                                    Set_Output_UV=((goutValarm[4]-0x30)*100)+((goutValarm[5]-0x30)*10)+((goutValarm[6]-0x30)*1)+((goutValarm[7]-0x30)*0.1);
                                }
                                else poss_x++;
                                //============================================	
                            }
                        }
                    }
                    
                    else if(EEPROMRead(BATT_TYPE) == 2)
                    {
                        if(up)
                        {
                            data_8 = goutValarm[poss_x];
                            switch(poss_x)
                            {
                                case(0):
                                {
                                    data_8 = '1';
                                }
                                break;
                                //
                                case(1):
                                {
                                    if (data_8 >= '3') data_8 = '1';
                                    else if (data_8 < '1') data_8 = '3';
                                    else data_8++;
                                }break;
                                //
                                case(2):
                                {
                                    if (goutValarm[0] >= '1' && goutValarm[1] >= '3')
                                    {
                                        if (data_8 >= '5') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '5';
                                        else data_8++;
                                    }
                                    else if (goutValarm[0] <= '1' && goutValarm[1] <= '1')
                                    {
                                        if (data_8 >= '9') data_8 = '8';
                                        else if (data_8 < '8') data_8 = '9';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(3):
                                {
                                    if (goutValarm[0] >= '1' && goutValarm[1] >= '3' && goutValarm[2] >= '5')
                                    {
                                        data_8 = '0';
                                    }
                                    else if (goutValarm[0] <= '1' && goutValarm[1] <= '1' && goutValarm[2] <= '8')
                                    {
                                        if (data_8 >= '9') data_8 = '4';
                                        else if (data_8 < '4') data_8 = '9';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(4):
                                {
                                    if (data_8 >= '1') data_8 = '0';
                                    else if (data_8 < '0') data_8 = '1';
                                    else data_8++;
                                }break;
                                //
                                case(5):
                                {
                                    if(goutValarm[4] == '0')
                                    {
                                        data_8 = '9';
                                    }
                                    else if(goutValarm[4] >= '1')
                                    {
                                        if (data_8 >= '1') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '1';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(6):
                                {
                                    if(goutValarm[4] >= '1' && goutValarm[5] >= '1')
                                    {
                                        if (data_8 >= '8') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '8';
                                        else data_8++;
                                    }
                                    else if(goutValarm[4] <= '0' && goutValarm[5] <= '9')
                                    {
                                        if(data_8 >= '9') data_8 = '6';
                                        else if (data_8 < '6') data_8 = '9';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if(data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                                //
                                case(7):
                                {
                                    if (goutValarm[4] >= '1' && goutValarm[5] >= '1' && goutValarm[6] >= '8')
                                    {
                                        if (data_8 >= '4') data_8 = '0';
                                        else if (data_8 < '0') data_8 = '4';
                                        else data_8++;
                                    }
                                    else
                                    {
                                        if (data_8 >= '9') data_8 = '0';
                                        else data_8++;
                                    }
                                }break;
                            }//end - switch(poss_x)
                            goutValarm[poss_x] = data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = goutValarm[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                        data_8= '1';
                                        break;
                                    //
                                    case(1):
                                        {
                                            if(data_8 <= '1') data_8 = '3';
                                            else if (data_8 > '3') data_8 = '1';
                                            else data_8--;
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(goutValarm[0] <= '1' && goutValarm[1] <= '1')
                                            {
                                                if(data_8 <= '8') data_8= '9';
                                                else if (data_8 > '9') data_8 = '8';
                                                else data_8--;
                                            }
                                            else if(goutValarm[0] >= '1' && goutValarm[1] >= '3')
                                            {
                                                if(data_8 <= '0') data_8= '5';
                                                else if (data_8 > '5') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(3):
                                        {
                                            if(goutValarm[0] >= '1' && goutValarm[1] >= '3' && goutValarm[2] >= '5')
                                            {
                                                data_8 = '0';
                                            }	
                                            else if(goutValarm[0] <= '1' && goutValarm[1] <= '1' && goutValarm[2] <= '8')
                                            {
                                                if(data_8 <= '4') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '4';
                                                else data_8--;	
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;									
                                            }
                                        }break;
                                    //
                                    case(4):
                                    {
                                        if(data_8 <= '0') data_8 = '1';
                                        else data_8--;
                                    }
                                        break;
                                    //	
                                    case(5):
                                        {
                                            if(goutValarm[4] <= '0')
                                            {
                                                data_8 = '9';
                                            }
                                            else if(goutValarm[4] >= '1')
                                            {
                                                if(data_8 <= '0') data_8 = '1';
                                                else if (data_8 > '1') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(6):
                                        {
                                            if(goutValarm[4] >= '1' && goutValarm[5] >= '1')
                                            {
                                                if(data_8 <= '0') data_8 = '8';
                                                else if (data_8 > '8') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(goutValarm[4] <= '0' && goutValarm[5] <= '9')
                                            {
                                                if(data_8 <= '6') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '6';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(7):
                                        {
                                            if(goutValarm[4] >= '1' && goutValarm[5] >= '1' && goutValarm[6] >= '8')
                                            {
                                                if(data_8 <= '0') data_8 = '4';
                                                else if (data_8 > '4') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }break;
                                }//end - switch
                                goutValarm[poss_x] = data_8;
                            }
                            else if(enter) //lituam ion
                            {
                                if(poss_x >= 7)
                                {
                                    data_16=(((goutValarm[0] & 0x0F)*1000)+((goutValarm[1] & 0x0F)*100)+((goutValarm[2] & 0x0F)*10)+(goutValarm[3] & 0x0F));
                                    if(data_16 > (MaxOutputVoltage_LI * 10)) //135.0v
                                    {
                                        goutValarm[0] = ((MaxOutputVoltage_LI*10) / 1000) + 0x30;
                                        goutValarm[1] = (((MaxOutputVoltage_LI*10) - ((goutValarm[0] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[2] = (((MaxOutputVoltage_LI*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[3] = ((MaxOutputVoltage_LI*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100) - ((goutValarm[2] & 0x0f)*10)) + 0x30;  
                                    }
                                    else if(data_16 < (MidOutputVoltage_LI * 10))//118.4V
                                    {
                                        goutValarm[0] = ((MidOutputVoltage_LI*10) / 1000) + 0x30;
                                        goutValarm[1] = (((MidOutputVoltage_LI*10) - ((goutValarm[0] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[2] = (((MidOutputVoltage_LI*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[3] = ((MidOutputVoltage_LI*10) - ((goutValarm[0] & 0x0f) * 1000) - ((goutValarm[1] & 0x0f) * 100) - ((goutValarm[2] & 0x0f)*10)) + 0x30;  
                                    }

                                    data_16=(((goutValarm[4] & 0x0F)*1000)+((goutValarm[5] & 0x0F)*100)+((goutValarm[6] & 0x0F)*10)+(goutValarm[7] & 0x0F));
                                    if(data_16 < (MinOutputVoltage_LI * 10)) //90.0v
                                    {
                                        goutValarm[4] = ((MinOutputVoltage_LI*10) / 1000) + 0x30;
                                        goutValarm[5] = (((MinOutputVoltage_LI*10) - ((goutValarm[4] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[6] = (((MinOutputVoltage_LI*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[7] = ((MinOutputVoltage_LI*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100) - ((goutValarm[6] & 0x0f)*10)) + 0x30;  
                                    }
                                    else if(data_16 > (MidOutputVoltage_LI * 10))//118.4V
                                    {
                                        goutValarm[4] = ((MidOutputVoltage_LI*10) / 1000) + 0x30;
                                        goutValarm[5] = (((MidOutputVoltage_LI*10) - ((goutValarm[4] & 0x0f) * 1000))/100) + 0x30;
                                        goutValarm[6] = (((MidOutputVoltage_LI*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100))/10) + 0x30;
                                        goutValarm[7] = ((MidOutputVoltage_LI*10) - ((goutValarm[4] & 0x0f) * 1000) - ((goutValarm[5] & 0x0f) * 100) - ((goutValarm[6] & 0x0f)*10)) + 0x30;  
                                    }

                                    modi = LO;
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(OUTVALARM_0+cnt, goutValarm[cnt]);
                                    cal_error = LO;modi_end = HI;
                                    Set_Output_OV=((goutValarm[0]-0x30)*100)+((goutValarm[1]-0x30)*10)+((goutValarm[2]-0x30)*1)+((goutValarm[3]-0x30)*0.1);
                                    Set_Output_UV=((goutValarm[4]-0x30)*100)+((goutValarm[5]-0x30)*10)+((goutValarm[6]-0x30)*1)+((goutValarm[7]-0x30)*0.1);
                                }
                                else poss_x++;
                                //============================================	
                            }
                        }
                    }
                    
				
					if(esc) modi = LO;
					key_clr();
				}
			}break;
			//
			case(MENU_OUT_CUR_ALARM_SET): {
				if(!modi)
				{
					if(up)iMenuIndex = MENU_OUTPUT_ALARM_SET;//14;
					else 
					{
						if(down) iMenuIndex = MENU_CUTOFF_VOLT_SET;//16;
						else if(enter) 
						{
							modi = HI;
							poss_x = poss_y = 0;
							for(cnt=0;cnt<3;cnt++) goutAalarm[cnt] = EEPROMRead(OUTAALARM_0+cnt);//get float volt,Eq. Volt//
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
					if(up) 
					{
						data_8 = goutAalarm[poss_x];
						switch(poss_x) 	
						{
							case(0):
							{
								data_8 = '1';
							}break;
							//
							case(1): 				
							{
								if(goutAalarm[0] == '0')
								{
									if(data_8 >= '9' || data_8 < '8') data_8 = '8';
									else data_8++;
								}
								else 
								{
									if(data_8 >= '2') data_8 = '0';
									else data_8++;
								}
							}break;
							//
							case(2): 
							{
								if(goutAalarm[0] >= '1' && goutAalarm[1] <= '1')
								{
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
								else if((goutAalarm[0] <= '0') && (goutAalarm[1] >= '8') && (temp_buffer[2] <= '9'))
								{
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
								else data_8 = '0';
							}break;
							//
						}//end - switch(poss_x)
						goutAalarm[poss_x] = data_8;
					}
					else //down key pressed 
					{
						if(down) 						//case(n) => fl [0][01][2].[3]  eq [4][5][6].[7]
						{
							data_8 = goutAalarm[poss_x];
							switch(poss_x)
							{
								case(0):
									data_8 = '0';
								break;
							//	
								case(1): 
								{
									if(goutAalarm[0] == '0')
									{
										if(data_8 <= '8') data_8 = '9';
										else data_8--;
									}
									else 
									{
										if(data_8 <= '0') data_8 = '2';
										else data_8--;
									}
								}break;
								//
								case(2):
								{
								if(goutAalarm[0] >= '1' && goutAalarm[1] <= '1')
								{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								else if((goutAalarm[0] <= '0') && (goutAalarm[1] >= '8') && (goutAalarm[2] <= '9'))
								{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								else data_8 = '0';
								}break;
								//
							}//end - switch
							goutAalarm[poss_x] = data_8;
						}
						else if(enter) 
						{
							if(poss_x >= 2)
							{
								modi = LO;
								data_16=((goutAalarm[0]-0x30)*100)+((goutAalarm[1]-0x30)*10)+((goutAalarm[2]-0x30));
								if(data_16 > OCL)//120A
								{
									goutAalarm[0] = (OCL / 100) + 0x30;
									goutAalarm[1] = ((OCL - ((goutAalarm[0]-0x30) * 100)) / 10) + 0x30;
									goutAalarm[2] = (OCL - ((goutAalarm[0]-0x30) * 100) - ((goutAalarm[1]-0x30) * 10)) + 0x30;
								}
								if(data_16 < UCL)//80A
								{
									goutAalarm[0] = (UCL / 100) + 0x30;
									goutAalarm[1] = ((UCL - ((goutAalarm[0]-0x30) * 100)) / 10) + 0x30;
									goutAalarm[2] = (UCL - ((goutAalarm[0]-0x30) * 100) - ((goutAalarm[1]-0x30) * 10)) + 0x30;
								}
								for(cnt=0;cnt<4;cnt++) EEPROMWrite(OUTAALARM_0+cnt, goutAalarm[cnt]);
								Set_ARef=(((goutAalarm[0]-0x30)*100)+((goutAalarm[1]-0x30)*10)+((goutAalarm[2]-0x30)));
								Set_OT = 95;
								cal_error = LO;modi_end = HI;Set_OK=1;
							}
							else poss_x++;
							//============================================
							switch(poss_x)
							{
								case 2:
									if(temp_buffer[1]=='2')	temp_buffer[2]='0';
									break;					
							}	
						}
					}					
					if(esc) modi = LO;
					key_clr();
				}
			}break;
			//
			case(MENU_CUTOFF_VOLT_SET): {//16
				if(!modi)
				{
					if(up)iMenuIndex = MENU_OUT_CUR_ALARM_SET;//15;
					else 
					{
						if(down) iMenuIndex = MENU_BUZZER_SET;//17;
						else if(enter) 
						{
							modi = HI;
							poss_x = poss_y = 0;
							for(cnt=0;cnt<8;cnt++)
							{
								shutDownOutV[cnt] = EEPROMRead(SHUTDOWNOUTV_0+cnt);
							}
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
                    if(EEPROMRead(BATT_TYPE) == 0)//pb125V max 127.6 ~150v, min 90~132v
                    {
                        if(up) 
                        {
                            data_8 = shutDownOutV[poss_x];

                            switch(poss_x) 	
                            {
                                case(0):
                                    data_8 = '1';
                                    break;
                                    //
                                case(1):
                                    {
                                        if(data_8 >= '5') data_8= '2';
                                        else if (data_8 < '2') data_8 = '5';
                                        else data_8++;
                                    }break;
                                //
                                case(2):
                                    {
                                        if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2')
                                        {
                                            if(data_8 >= '9') data_8 = '7';
                                            else if (data_8 < '7') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(3):
                                    {
                                        if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2' && shutDownOutV[2] <= '7')
                                        {
                                            if(data_8 >= '9') data_8 = '6';
                                            else if (data_8 < '6') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5' && shutDownOutV[2] >= '0')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;									
                                        }
                                        //
                                    }break;
                                //
                                case(4):
                                    {
                                        if(data_8 >= '1') data_8 = '0';
                                        else data_8++;
                                    }
                                    break;
                                //
                                case(5):
                                    {
                                        if(shutDownOutV[4] <= '0')
                                        {
                                            data_8 = '9';
                                        }
                                        else if(shutDownOutV[4] >= '1')
                                        {
                                            if(data_8 >= '3') data_8 = '0';
                                            else if(data_8 < '0') data_8 = '3';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;									
                                        }                                        
                                    }
                                    break;
                                //
                                case(6): 
                                    {
                                        if(shutDownOutV[4] <= '0' && shutDownOutV[5] <= '9')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '3')
                                        {
                                            if(data_8 >= '2') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '2';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(7):
                                    {
                                        if(shutDownOutV[4] <= '0' && shutDownOutV[5] <= '9' && shutDownOutV[6] <= '8')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '3' && shutDownOutV[6] >= '2')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                }break;
                            }//end - switch(poss_x)
                            shutDownOutV[poss_x] = data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = shutDownOutV[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                        data_8 = '1';
                                        break;
                                    //
                                    case(1):
                                        {
                                            if(data_8 <= '2') data_8 = '5';
                                            else if (data_8 > '5') data_8 = '2';
                                            else data_8--;
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2')
                                            {
                                                if(data_8 <= '7') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '7';
                                                else data_8--;
                                            }
                                            else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5')
                                            {
                                                data_8 = '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(3):
                                        {
                                            if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2' && shutDownOutV[2] <= '7')
                                            {
                                                if(data_8 <= '6') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '6';
                                                else data_8--;			
                                            }		
                                            else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5' && shutDownOutV[2] >= '0')
                                            {
                                                data_8 = '0';
                                            }										
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;									
                                            }
                                        }break;
                                    //
                                    case(4):
                                        {
                                            if(data_8 <= '0') data_8 = '1';
                                            else data_8--;
                                        }
                                        break;
                                //	
                                    case(5):
                                        {
                                            if(shutDownOutV[4] <= '0')
                                            {
                                                data_8 = '9';
                                            }
                                            else if(shutDownOutV[4] >= '1')
                                            {
                                                if(data_8 <= '0') data_8 = '3';
                                                else if(data_8 > '3') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(6):
                                        {
                                            if(shutDownOutV[4] <= '0' && shutDownOutV[5] <= '9')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '3')
                                            {
                                                if(data_8 <= '0') data_8 = '2';
                                                else if(data_8 > '2') data_8 = '0';
                                                else data_8--;
                                            }                                            
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(7):
                                        {
                                            if(shutDownOutV[4] <= '0' && shutDownOutV[5] <= '9' && shutDownOutV[6] <= '8')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '3' && shutDownOutV[6] >= '2')
                                            {
                                                data_8 = '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }break;
                                }//end - switch
                                shutDownOutV[poss_x] = data_8;
                            }
                            else if(enter) 
                            {
                                if(poss_x >= 7)
                                {
                                    data_16=(((shutDownOutV[0]-0x30)*1000)+((shutDownOutV[1]-0x30)*100)+((shutDownOutV[2]-0x30)*10)+(shutDownOutV[3]-0x30));
                                    if(data_16 > (MaxOutputVoltage_PB * 10)) //150.0v  //127.6~150//
                                    {
                                        shutDownOutV[0] = ((MaxOutputVoltage_PB*10) / 1000) + 0x30;
                                        shutDownOutV[1] = (((MaxOutputVoltage_PB*10) - ((shutDownOutV[0] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[2] = (((MaxOutputVoltage_PB*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[3] = ((MaxOutputVoltage_PB*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100) - ((shutDownOutV[2] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 < (MinOutputVoltage_PB2 * 10))//127.6V
                                    {
                                        shutDownOutV[0] = ((MinOutputVoltage_PB2*10) / 1000) + 0x30;
                                        shutDownOutV[1] = (((MinOutputVoltage_PB2*10) - ((shutDownOutV[0] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[2] = (((MinOutputVoltage_PB2*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[3] = ((MinOutputVoltage_PB2*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100) - ((shutDownOutV[2] - 0x30) * 10)) + 0x30;
                                    }
                                    
                                    data_16=(((shutDownOutV[4]-0x30)*1000)+((shutDownOutV[5]-0x30)*100)+((shutDownOutV[6]-0x30)*10)+(shutDownOutV[7]-0x30));
                                    if(data_16 < (MinOutputVoltage_PB * 10))//90.0v
                                    {
                                        shutDownOutV[4] = ((MinOutputVoltage_PB*10) / 1000) + 0x30;
                                        shutDownOutV[5] = (((MinOutputVoltage_PB*10) - ((shutDownOutV[4] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[6] = (((MinOutputVoltage_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[7] = ((MinOutputVoltage_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100) - ((shutDownOutV[6] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 > (MidOutputVoltage_PB * 10))//132.0v
                                    {
                                        shutDownOutV[4] = ((MidOutputVoltage_PB*10) / 1000) + 0x30;
                                        shutDownOutV[5] = (((MidOutputVoltage_PB*10) - ((shutDownOutV[4] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[6] = (((MidOutputVoltage_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[7] = ((MidOutputVoltage_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100) - ((shutDownOutV[6] - 0x30) * 10)) + 0x30;
                                    }
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(SHUTDOWNOUTV_0+cnt, shutDownOutV[cnt]);
                                    modi = LO;
                                    Set_OV=((shutDownOutV[0]-0x30)*100)+((shutDownOutV[1]-0x30)*10)+((shutDownOutV[2]-0x30)*1)+((shutDownOutV[3]-0x30)*0.1);
                                    Set_UV=((shutDownOutV[4]-0x30)*100)+((shutDownOutV[5]-0x30)*10)+((shutDownOutV[6]-0x30)*1)+((shutDownOutV[7]-0x30)*0.1);
                                    cal_error = LO;modi_end = HI;Set_OK=1;
                                }
                                else poss_x++;
                                //============================================
                            }
                        }
                        if(esc) modi = LO;
                        key_clr();
                    }
                    
                    else if(EEPROMRead(BATT_TYPE) == 1)//pb110V 
                    {
                        if(up) 
                        {
                            data_8 = shutDownOutV[poss_x];

                            switch(poss_x) 	
                            {
                                case(0):
                                    data_8 = '1';
                                    break;
                                    //
                                case(1):
                                    {
                                        if(data_8 >= '5') data_8= '2';
                                        else if (data_8 < '2') data_8 = '5';
                                        else data_8++;
                                    }break;
                                //
                                case(2):
                                    {
                                        if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(3):
                                    {
                                        if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2' && shutDownOutV[2] <= '0')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5' && shutDownOutV[2] >= '0')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;									
                                        }
                                        //
                                    }break;
                                //------------------------min value (90-120v)
                                case(4):
                                    {
                                        if(data_8 == '1') data_8 = '0';
                                        else data_8 = '1';
                                    }
                                    break;
                                //
                                case(5):
                                    {
                                        if(shutDownOutV[4] >= '1')
                                        {
                                            if(data_8 >= '2') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '2';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;									
                                        }                                        
                                    }
                                    break;
                                //
                                case(6): 
                                    {
//                                        if(shutDownOutV[4] <= '1' && shutDownOutV[5] <= '0')//10n.x
//                                        {
//                                            if(data_8 >= '9') data_8 = '5';
//                                            else if (data_8 < '5') data_8 = '9';
//                                            else data_8++;
//                                        }
//                                        else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '2')
                                        if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '2')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(7):
                                    {
                                        if(shutDownOutV[4] <= '1' && shutDownOutV[5] <= '0' && shutDownOutV[6] <= '5')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '2' && shutDownOutV[6] >= '0')
                                        {
                                            data_8 = '0';
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                }break;
                            }//end - switch(poss_x)
                            shutDownOutV[poss_x] = data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = shutDownOutV[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                        data_8 = '1';
                                        break;
                                    //
                                    case(1):
                                        {
                                            if(data_8 <= '2') data_8 = '5';
                                            else if (data_8 > '5') data_8 = '2';
                                            else data_8--;
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5')
                                            {
                                                data_8 = '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(3):
                                        {
                                            if(shutDownOutV[0] <= '1' && shutDownOutV[1] <= '2' && shutDownOutV[2] <= '0')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;			
                                            }		
                                            else if(shutDownOutV[0] >= '1' && shutDownOutV[1] >= '5' && shutDownOutV[2] >= '0')
                                            {
                                                data_8 = '0';
                                            }										
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;									
                                            }
                                        }break;
                                    //
                                    case(4):
                                        {
                                            if(data_8 == '1')data_8 = '0';
                                            else data_8 = '1';
                                        }
                                        break;
                                //	
                                    case(5):
                                        {
                                            if(shutDownOutV[4] >= '1')
                                            {
                                                if(data_8 <= '0') data_8 = '2';
                                                else if (data_8 > '2') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(6):
                                        {
//                                            if(shutDownOutV[4] <= '1' && shutDownOutV[5] <= '0')
//                                            {
//                                                if(data_8 <= '5') data_8 = '9';
//                                                else if (data_8 > '9') data_8 = '5';
//                                                else data_8--;
//                                            }
//                                            else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '2')
                                            if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '2')
                                            {
                                                data_8 = '0';
                                            }                                            
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(7):
                                        {
                                            if(shutDownOutV[4] <= '1' && shutDownOutV[5] <= '0' && shutDownOutV[6] <= '5')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(shutDownOutV[4] >= '1' && shutDownOutV[5] >= '2' && shutDownOutV[6] >= '0')
                                            {
                                                data_8 = '0';
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }break;
                                }//end - switch
                                shutDownOutV[poss_x] = data_8;
                            }
                            else if(enter) //pb110V
                            {
                                if(poss_x >= 7)
                                {
                                    data_16=(((shutDownOutV[0]-0x30)*1000)+((shutDownOutV[1]-0x30)*100)+((shutDownOutV[2]-0x30)*10)+(shutDownOutV[3]-0x30));
                                    if(data_16 > (MaxOutputVoltage2_PB * 10))//150.0V
                                    {
                                        shutDownOutV[0] = ((MaxOutputVoltage2_PB*10) / 1000) + 0x30;
                                        shutDownOutV[1] = (((MaxOutputVoltage2_PB*10) - ((shutDownOutV[0] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[2] = (((MaxOutputVoltage2_PB*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[3] = ((MaxOutputVoltage2_PB*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100) - ((shutDownOutV[2] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 < (MinOutputVoltage2_PB2 * 10))//90.0V
                                    {
                                        shutDownOutV[0] = ((MinOutputVoltage2_PB2*10) / 1000) + 0x30;
                                        shutDownOutV[1] = (((MinOutputVoltage2_PB2*10) - ((shutDownOutV[0] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[2] = (((MinOutputVoltage2_PB2*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[3] = ((MinOutputVoltage2_PB2*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100) - ((shutDownOutV[2] - 0x30) * 10)) + 0x30;
                                    }
                                    
                                    data_16=(((shutDownOutV[4]-0x30)*1000)+((shutDownOutV[5]-0x30)*100)+((shutDownOutV[6]-0x30)*10)+(shutDownOutV[7]-0x30));
                                    if(data_16 < (MinOutputVoltage2_PB * 10))//90.0V
                                    {
                                        shutDownOutV[4] = ((MinOutputVoltage2_PB*10) / 1000) + 0x30;
                                        shutDownOutV[5] = (((MinOutputVoltage2_PB*10) - ((shutDownOutV[4] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[6] = (((MinOutputVoltage2_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[7] = ((MinOutputVoltage2_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100) - ((shutDownOutV[6] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 > (MidOutputVoltage2_PB * 10))//120.0V
                                    {
                                        shutDownOutV[4] = ((MidOutputVoltage2_PB*10) / 1000) + 0x30;
                                        shutDownOutV[5] = (((MidOutputVoltage2_PB*10) - ((shutDownOutV[4] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[6] = (((MidOutputVoltage2_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[7] = ((MidOutputVoltage2_PB*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100) - ((shutDownOutV[6] - 0x30) * 10)) + 0x30;
                                    }
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(SHUTDOWNOUTV_0+cnt, shutDownOutV[cnt]);
                                    modi = LO;
                                    Set_OV=((shutDownOutV[0]-0x30)*100)+((shutDownOutV[1]-0x30)*10)+((shutDownOutV[2]-0x30)*1)+((shutDownOutV[3]-0x30)*0.1);
                                    Set_UV=((shutDownOutV[4]-0x30)*100)+((shutDownOutV[5]-0x30)*10)+((shutDownOutV[6]-0x30)*1)+((shutDownOutV[7]-0x30)*0.1);
                                    cal_error = LO;modi_end = HI;Set_OK=1;
                                }
                                else poss_x++;
                                //============================================
                            }
                        }
                        if(esc) modi = LO;
                        key_clr();
                    }
                    else if(EEPROMRead(BATT_TYPE) == 2)//Lituam Ion
                    {
                        if(up) 
                        {
                            data_8 = shutDownOutV[poss_x];

                            switch(poss_x) 	
                            {
                                case(0):
                                    {
                                        data_8= '1';
                                    }break;
                                    //
                                case(1):
                                    {
                                        if(shutDownOutV[0] == '1')
                                        {
                                            if(data_8 >= '3') data_8= '1';
                                            else if (data_8 < '1') data_8 = '3';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8= '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(2):
                                    {
                                        if(shutDownOutV[0] == '1' && shutDownOutV[1] == '3')
                                        {
                                            if(data_8 >= '5') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '5';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[0] == '1' && shutDownOutV[1] == '1')
                                        {
                                            if(data_8 >= '9') data_8 = '8';
                                            else if (data_8 < '8') data_8 = '9';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(3):
                                    {
                                        if(shutDownOutV[1] == '3' && shutDownOutV[2] == '5')
                                        {
                                            data_8 = '0';
                                        }		
                                        else if(shutDownOutV[1] == '1' && shutDownOutV[2] == '8')
                                        {
                                            if(data_8 >= '9') data_8 = '4';
                                            else if (data_8 < '4') data_8 = '9';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;									
                                        }
                                        //
                                    }break;
                                //
                                case(4)://lituam ion shutdown min volt (min 90V)
                                    {
                                        if(data_8 >= '1') data_8 = '0';
                                        else data_8++;
                                    }
                                    break;
                                //
                                case(5):
                                    {
                                        if(shutDownOutV[4] <= '0')
                                        {
                                            data_8 = '9';
                                        }
                                        else
                                        {
                                            if(data_8 >= '1') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '1';
                                            else data_8++;
                                        }
                                    }
                                    break;
                                //
                                case(6): 
                                    {
                                        if(shutDownOutV[4] == '0' && shutDownOutV[5] == '9')
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '9';
                                            else data_8++;
                                        }
                                        else if(shutDownOutV[4] == '1' && shutDownOutV[5] == '1')
                                        {
                                            if(data_8 >= '8') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '8';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                    }break;
                                //
                                case(7):
                                    {
                                        if(shutDownOutV[4] == '1' && shutDownOutV[5] == '1' && shutDownOutV[6] == '8')
                                        {
                                            if(data_8 >= '4') data_8 = '0';
                                            else if (data_8 < '0') data_8 = '4';
                                            else data_8++;
                                        }
                                        else
                                        {
                                            if(data_8 >= '9') data_8 = '0';
                                            else data_8++;
                                        }
                                }break;
                            }//end - switch(poss_x)
                            shutDownOutV[poss_x] = data_8;
                        }
                        else //down key pressed 
                        {
                            if(down)
                            {
                                data_8 = shutDownOutV[poss_x];
                                switch(poss_x) 
                                {
                                    case(0): 
                                        data_8 = '1';
                                        break;
                                    //
                                    case(1):
                                        {
                                            if(data_8 <= '1') data_8= '3';
                                            else if (data_8 > '3') data_8 = '1';
                                            else data_8--;
                                        }break;
                                    //
                                    case(2):
                                        {
                                            if(shutDownOutV[1] == '1')
                                            {
                                                if(data_8 <= '8') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '8';
                                                else data_8--;
                                            }
                                            else if(shutDownOutV[1] == '3')
                                            {
                                                if(data_8 <= '0') data_8 = '5';
                                                else if (data_8 > '5') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(3):
                                        {
                                            if(shutDownOutV[1] == '3' && shutDownOutV[2] == '5')
                                            {
                                                data_8 = '0';
                                            }	
                                            else if(shutDownOutV[1] == '1' && shutDownOutV[2] == '8')
                                            {
                                                if(data_8 <= '4') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '4';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;									
                                            }
                                        }break;
                                    //
                                    case(4):
                                        {
                                                if(data_8 <= '0') data_8 = '1';
                                                else if (data_8 > '1') data_8 = '0';
                                                else data_8--;
                                        }break;
                                //	
                                    case(5):
                                        {
                                            if(shutDownOutV[4] == '0')
                                            {
                                                data_8 = '9';
                                            }
                                            else if(shutDownOutV[4] == '1')
                                            {
                                                if(data_8 <= '0') data_8 = '1';
                                                else if (data_8 > '1') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                        }break;
                                    //
                                    case(6):
                                        {
                                            if(shutDownOutV[4] == '0' && shutDownOutV[5] == '9')
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else if (data_8 > '9') data_8 = '0';
                                                else data_8--;
                                            }
                                            else if(shutDownOutV[4] == '1' && shutDownOutV[5] == '1')
                                            {
                                                if(data_8 <= '0') data_8 = '8';
                                                else if (data_8 > '8') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                            
                                        }break;
                                    //
                                    case(7):
                                        {
                                            if(shutDownOutV[4] == '1' && shutDownOutV[5] == '1' && shutDownOutV[6] == '8')
                                            {
                                                if(data_8 <= '0') data_8 = '4';
                                                else if (data_8 > '4') data_8 = '0';
                                                else data_8--;
                                            }
                                            else
                                            {
                                                if(data_8 <= '0') data_8 = '9';
                                                else data_8--;
                                            }
                                    }break;
                                }//end - switch
                                shutDownOutV[poss_x] = data_8;
                            }
                            else if(enter) //Lituam Ion
                            {
                                if(poss_x >= 7)
                                {
                                    data_16=(((shutDownOutV[0]-0x30)*1000)+((shutDownOutV[1]-0x30)*100)+((shutDownOutV[2]-0x30)*10)+(shutDownOutV[3]-0x30));
                                    if(data_16 > (MaxOutputVoltage_LI * 10))//135.0v
                                    {
                                        shutDownOutV[0] = ((MaxOutputVoltage_LI*10) / 1000) + 0x30;
                                        shutDownOutV[1] = (((MaxOutputVoltage_LI*10) - ((shutDownOutV[0] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[2] = (((MaxOutputVoltage_LI*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[3] = ((MaxOutputVoltage_LI*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100) - ((shutDownOutV[2] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 < ((MidOutputVoltage_LI) * 10))//118.4V
                                    {
                                        shutDownOutV[0] = ((MidOutputVoltage_LI*10) / 1000) + 0x30;
                                        shutDownOutV[1] = (((MidOutputVoltage_LI*10) - ((shutDownOutV[0] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[2] = (((MidOutputVoltage_LI*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[3] = ((MidOutputVoltage_LI*10) - ((shutDownOutV[0] - 0x30) * 1000) - ((shutDownOutV[1] - 0x30) * 100) - ((shutDownOutV[2] - 0x30) * 10)) + 0x30;
                                    }
                                    data_16=(((shutDownOutV[4]-0x30)*1000)+((shutDownOutV[5]-0x30)*100)+((shutDownOutV[6]-0x30)*10)+(shutDownOutV[7]-0x30));
                                    if(data_16 < (MinOutputVoltage_LI * 10))//90.0V
                                    {
                                        shutDownOutV[4] = ((MinOutputVoltage_LI*10) / 1000) + 0x30;
                                        shutDownOutV[5] = (((MinOutputVoltage_LI*10) - ((shutDownOutV[4] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[6] = (((MinOutputVoltage_LI*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[7] = ((MinOutputVoltage_LI*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100) - ((shutDownOutV[6] - 0x30) * 10)) + 0x30;
                                    }
                                    else if(data_16 > (MidOutputVoltage_LI * 10))//118.4V
                                    {
                                        shutDownOutV[4] = ((MidOutputVoltage_LI*10) / 1000) + 0x30;
                                        shutDownOutV[5] = (((MidOutputVoltage_LI*10) - ((shutDownOutV[4] - 0x30) * 1000)) / 100) + 0x30;
                                        shutDownOutV[6] = (((MidOutputVoltage_LI*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100))/10) + 0x30;
                                        shutDownOutV[7] = ((MidOutputVoltage_LI*10) - ((shutDownOutV[4] - 0x30) * 1000) - ((shutDownOutV[5] - 0x30) * 100) - ((shutDownOutV[6] - 0x30) * 10)) + 0x30;
                                    }

                                    modi = LO;
                                    Set_OV=((shutDownOutV[0]-0x30)*100)+((shutDownOutV[1]-0x30)*10)+((shutDownOutV[2]-0x30)*1)+((shutDownOutV[3]-0x30)*0.1);
                                    Set_UV=((shutDownOutV[4]-0x30)*100)+((shutDownOutV[5]-0x30)*10)+((shutDownOutV[6]-0x30)*1)+((shutDownOutV[7]-0x30)*0.1);
                                    for(cnt=0;cnt<8;cnt++) EEPROMWrite(SHUTDOWNOUTV_0+cnt, shutDownOutV[cnt]);
                                    cal_error = LO;modi_end = HI;Set_OK=1;
                                }
                                else poss_x++;
                                //============================================
                            }
                        }					
                        if(esc) modi = LO;
                        key_clr();
                    }
				}
			}break;
			//
			case(MENU_BUZZER_SET)://17
			{
				if(!modi)
				{
					if(up) iMenuIndex = MENU_CUTOFF_VOLT_SET;//16;
					else
					{
						if(down) iMenuIndex = MENU_TIME_SET;//18;
						else if(enter)
						{
							modi = HI;
							poss_x = poss_y = 0;
							fAlarmStop = EEPROMRead(ALARM_STOP);
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
					if(up)
					{
						data_8 = fAlarmStop;
						if(data_8 == 'N') data_8 = 'Y';
						else data_8 = 'N';
						fAlarmStop = data_8;
					}
					else
					{
						if(down)
						{
							data_8 = fAlarmStop;
							if(data_8 == 'Y') data_8 = 'N';
							else data_8 = 'Y';
							fAlarmStop = data_8;
						}
						else if(enter)
						{
                            EEPROMWrite(ALARM_STOP,fAlarmStop);
							modi = LO;
							err_menu = iMenuIndex;
							iMenuIndex = MENU_BUZZER_SET;//17;
							cal_error = HI;
							//modi_end = HI;
						}
					}
					if(esc) modi = LO;
					key_clr();
				}
			}break;
			//
			case(MENU_TIME_SET)://18
			{
				if(!modi)
				{
					if(up) iMenuIndex = MENU_BUZZER_SET;//17;
					else
					{
						if(down) iMenuIndex = MENU_PASS_SET;//19;
						else if(enter)
						{
							rtc_get_date();
							rtc_get_time();
							modi = HI;
							poss_x = 0;
							bar_poss=0;
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
					if(up)
					{
						data_8 = timeBuf[poss_x];
						switch(poss_x)
						{
							case 0: {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}
							timeBuf[0]=data_8;
							break;
							//
							case 1: {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}
							timeBuf[1]=data_8;
							break;
							case 2: {
								if(data_8 >= '1') data_8 = '0';
								else data_8++;
							}
							timeBuf[2]=data_8;
							break;
							//
							case 3: {												// 10month data check
								if(timeBuf[2] >= '1') {
									if(data_8 >= '2') data_8 = '0';
									else data_8++;
								}
								else {
									if(data_8 >= '9') data_8 = '1';
									else data_8++;
								}
							}
							timeBuf[3]=data_8;
							break;
							//
							case 4: {
								if(data_8 >= '3') data_8 = '0';
								else data_8++;
							}
							timeBuf[4]=data_8;break;
							//
							case 5: {												// 10hour data check
								if(timeBuf[4] != '3') {
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
								else {
									if(data_8 >= '1') data_8 = '0';
									else data_8++;
								}
							}
							timeBuf[5]=data_8;
							break;
							//
							case 6: {
								if(data_8 >= '2') data_8 = '0';
								else data_8++;
							}
							timeBuf[6]=data_8;
							break;
							//
							case 7: {
								if(timeBuf[6] != '2') {
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
								else {
									if(data_8 >= '3') data_8 = '0';
									else data_8++;
								}
							}
							timeBuf[7]=data_8;
							break;
							//
							case 8:	{
								if(data_8 >= '5') data_8 = '0';
								else data_8++;
							}
							timeBuf[8]=data_8;
							break;
							case 9: {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}
							timeBuf[9]=data_8;
							break;
							case 10: {
								if(data_8 >= '5') data_8 = '0';
								else data_8++;
							}
							timeBuf[10]=data_8;
							break;
							case 11: {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}
							timeBuf[11]=data_8;
							break;
						}
						//temp_buffer[poss_x] = data_8;
						year = ((timeBuf[0]-0x30)*10)+(timeBuf[1]-0x30);
						mth = ((timeBuf[2]-0x30)*10)+(timeBuf[3]-0x30);
						day = ((timeBuf[4]-0x30)*10)+(timeBuf[5]-0x30);
						hour = ((timeBuf[6]-0x30)*10)+(timeBuf[7]-0x30);
						min = ((timeBuf[8]-0x30)*10)+(timeBuf[9]-0x30);
						sec = ((timeBuf[10]-0x30)*10)+(timeBuf[11]-0x30);
					}
					else {
						if(down) {
							data_8 = timeBuf[poss_x];
							switch(poss_x) {
								case 0:	{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								timeBuf[0]=data_8;
								break;
								//
								case 1:	{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								timeBuf[1]=data_8;
								break;
								//
								case 2: {
									if(data_8 <= '0') data_8 = '1';
									else data_8--;
								}
								timeBuf[2]=data_8;
								break;
								//
								case 3: {
									if(timeBuf[2] <= '0') {						// 10month data check
										if(data_8 <= '1') data_8 = '9';
										else data_8--;
									}
									else if(timeBuf[2] >=1)
									{
										if(data_8 <= '0')	data_8 = '2';
										else data_8--;										
									}								
								}
								timeBuf[3]=data_8;
								break;
								//
								case 4: {
									if(data_8 <= '0') data_8 = '3';
									else data_8--;
								}
								timeBuf[4]=data_8;
								break;
								//
								case 5: {
									if(timeBuf[4] >= '3') {						// 10hour data check
										if(data_8 <= '0') data_8 = '1';
										else data_8--;
									}
									else {
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
								}
								timeBuf[5]=data_8;
								break;
								//
								case 6: {
									if(data_8 <= '0') data_8 = '2';
									else data_8--;
								}
								timeBuf[6]=data_8;
								break;
								//
								case 7: {
									if(timeBuf[6] >= '2') {
										if(data_8 <= '0') data_8 = '3';
										else data_8--;
									}
									else {
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
								}
								timeBuf[7]=data_8;
								break;
								//
								case 8: {
									if(data_8 <= '0') data_8 = '5';
									else data_8--;
								}
								timeBuf[8]=data_8;
								break;
								//
								case 9:	{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								timeBuf[9]=data_8;
								break;
								//
								case 10: {
									if(data_8 <= '0') data_8 = '5';
									else data_8--;
								}
								timeBuf[10]=data_8;
								break;
								//
								case 11:{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								timeBuf[11]=data_8;
								break;
							}
							//temp_buffer[poss_x] = data_8;
							year = ((timeBuf[0]-0x30)*10)+(timeBuf[1]-0x30);
							mth = ((timeBuf[2]-0x30)*10)+(timeBuf[3]-0x30);
							day = ((timeBuf[4]-0x30)*10)+(timeBuf[5]-0x30);
							hour = ((timeBuf[6]-0x30)*10)+(timeBuf[7]-0x30);
							min = ((timeBuf[8]-0x30)*10)+(timeBuf[9]-0x30);
							sec = ((timeBuf[10]-0x30)*10)+(timeBuf[11]-0x30);
						}
						else if(enter) {
							if(poss_x >= 11) {
								modi = LO;modi_end = HI;
								rtc_set_datetime(day, mth, year, 0, hour, min);
							}
							else poss_x++;
						}
					}
					switch(poss_x) {
						case 0: bar_poss = 0;break;					// 10year
						case 1: bar_poss = 1;break;					// 1year
						case 2: bar_poss = 3;break;					// 10month
						case 3: bar_poss = 4;break;					// 1month
						case 4: bar_poss = 6;break;					// 10day
						case 5: bar_poss = 7;break;					// 1day
						case 6: bar_poss = 8;break;					// 10hour
						case 7: bar_poss = 9;break;				// 1hour
						case 8: bar_poss = 11;break;				// 10minite
						case 9: bar_poss = 12;break;				// 1minite
						case 10: bar_poss = 14;break;				// 10sec
						case 11: bar_poss = 15;break;				// 1sec
					}
					if(esc) modi = LO;
					key_clr();
				}
			}break;
			//	
			case(MENU_PASS_SET): {//19
				if(!modi) {
					if(up) iMenuIndex=MENU_TIME_SET;//18;
					else {
						if(down) iMenuIndex = MENU_SYS_INIT;//20;
						else if(enter) {
							modi = HI;
							poss_x = 0;
							for(cnt=0;cnt<6;cnt++) gPin[cnt] = '0';
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else {
					if(up) {
						data_8 = gPin[poss_x];
						if(data_8 >= '9') data_8 = '0';							
						else data_8++;
						gPin[poss_x] = data_8;

					}
					else {
						if(down) {
							data_8 = gPin[poss_x];
							if(data_8 == '0') data_8 = '9';
							else data_8--;
							gPin[poss_x] = data_8;
						}
						else if(enter) {
							if(poss_x >= 5) {
								modi = LO;modi_end = HI;
								for(cnt=0;cnt<6;cnt++) EEPROMWrite((PASSWD_0+cnt),gPin[cnt]);
							}						
							else poss_x++;
						}
					}
					if(esc) modi = LO;
					key_clr();
				}
			}break;
			//
			case(MENU_SYS_INIT)://20
			{
				if(!modi)
				{
					if(up) iMenuIndex = MENU_PASS_SET;//19;
					else
					{
						if(down) iMenuIndex = MENU_ADCGAIN;//21;
						else if(enter)
						{
							modi = HI;
							poss_x = poss_y = 0;
							temp_buffer[poss_x]='N';
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
					if(up)
					{
						data_8 = temp_buffer[poss_x];
						if(data_8 == 'N') data_8 = 'Y';
						else data_8 = 'N';
						temp_buffer[poss_x] = data_8;
					}
					else
					{
						if(down)
						{
							data_8 = temp_buffer[poss_x];
							if(data_8 == 'Y') data_8 = 'N';
							else data_8 = 'Y';
							temp_buffer[poss_x] = data_8;
						}
						else if(enter)
						{
							if(temp_buffer[poss_x] == 'Y')
							{
								sysInit();
								ParameterInitialize();
                                CHG_SEL = 0;
                                CHG_Mode = 0;
                                Auto_Flag = 0;
                                CHG_CMD();
                                CHG_Check();
                                Set_OK = 1;
                                prevMenu=0;
                                iMenuIndex = MENU_SYS_INIT;//20;
							}
							modi = LO;
							err_menu = iMenuIndex;
							iMenuIndex = MENU_SYS_INIT;//20;
							cal_error = HI;
						}
					}
					if(esc) modi = LO;
					key_clr();
				}
			}break;
            
			case(MENU_ADCGAIN)://21
			{
				if(!modi)
				{
					if(up) iMenuIndex = MENU_SYS_INIT;//20;
					else
					{
						if(down)
                        {
                            if(EEPROMRead(BATT_TYPE) == 2) iMenuIndex = MENU_DEBUG;//25;
                            else iMenuIndex = MENU_CHG_MODE_SET;//22;
                        }                        
						else if(enter) 
                        {
                            modi = HI;
                            poss_x = 2;
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else {//when modi is HIGH//
					if(up) {
						vGain = ADJ_VGain;
						cGain = ADJ_CGain;
						switch(poss_x) {
							case(2): {
								if(vGain >=128) vGain+= 1;
								else if(vGain >= 127) vGain = 127;
								else vGain+= 1;
							}break;
							//
							case(5): {
								if(cGain >=128) cGain+= 1;
								else if(cGain >= 127) cGain = 127;
								else cGain+= 1;
							}break;
							//
						}
						ADJ_VGain = vGain;
						ADJ_CGain = cGain;
					}
					else {
						if(down) {
							vGain = ADJ_VGain;
							cGain = ADJ_CGain;
							switch(poss_x) {
								case(2): {
									if(vGain <=127) vGain-= 1;
									else if(vGain <= 128) vGain = 128;
									else vGain-= 1;
								}break;
								//
								case(5): {
									if(cGain <=127) cGain-= 1;
									else if(cGain <= 128) cGain = 128;
									else cGain-= 1;
								}
								break;
							}
							ADJ_VGain = vGain;
							ADJ_CGain = cGain;
						}
						else if(enter) {
							if(poss_x >= 5) {
								modi = LO;modi_end = HI;
								EEPROMWrite(ADJVGain_0, ADJ_VGain);
								EEPROMWrite(ADJCGain_0, ADJ_CGain);
								ADJ_VGain = ADJ_VGain;
								ADJ_CGain = ADJ_CGain;
								cal_error = LO;modi_end = HI;Set_OK=1;
							}
							else {
								if(poss_x==2) poss_x=5;
							}
						}
					}
					if(esc) {
						modi = LO;
					}
					key_clr();
				}
			}break;
            case(MENU_CHG_MODE_SET):       //CHG MODE
			{
				if(!modi)
				{                    
					if(up) iMenuIndex = MENU_ADCGAIN;//21;
					else
					{
						if(down){
                            iMenuIndex = MENU_CHG_TIME_SET;//23;
                            fEepromRead = 0;
                        } 
                            
						else if(enter)
						{
							modi = HI;
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
					if(up)
					{
                        if(poss_x >= 1) 
                        {
                            if(poss_y >= 1)
                            {
                                poss_x = 0;
                                poss_y = 0;
                            }
                            else
                            {
                                poss_y++;
                            }
                        }
                        else poss_x++;
  					}
					else
					{
						if(down)
						{
		                    if(poss_x <= 0) 
                            {
                                if(poss_y <= 0)
                                {
                                    poss_x = 1;
                                    poss_y = 1;
                                }
                                else
                                {
                                    poss_x--;
                                }
                            }
                            else if(poss_x >= 1 && poss_y <= 0)
                            {
                                poss_x = 0;
                            }
                            else poss_y--;
						}
						else if(enter)
						{
                           	modi = LO;
							err_menu = iMenuIndex;
							iMenuIndex = MENU_CHG_MODE_SET;//22;
							cal_error = HI;
                            CHG_SEL = poss_x;
                            CHG_Mode = poss_y;
                            EEPROMWrite(CHG_STS_0, CHG_SEL);
                            EEPROMWrite(CHG_STS_1, CHG_Mode);
                            if(CHG_SEL == 0 && CHG_Mode == 0)    Auto_Flag = 0;
                            else    Auto_Flag = 2;                                
                            CHG_EN = 0;
                            CHG_CMD();
                            Set_OK = 1;
						}
					}
					if(esc) modi = LO;
					key_clr();
				}
			}break;
            
            case(MENU_CHG_TIME_SET):           // CHG TIME
			{
				if(!modi)
				{   
					if(up) iMenuIndex = MENU_CHG_MODE_SET;//22;
					else
					{
						if(down) iMenuIndex = MENU_CHG_CONDITION_SET;//24;
						else if(enter)
						{
                            modi = HI;
							poss_x = 0; 
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
                    if(up)
					{
                        data_8 = Set_time[poss_x];
						switch(poss_x)
						{
                            case 0:
                            {           
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
                                fEepromRead=0;
							}break;
                            
							case 1:
                            {
                                if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}break;
							//
							case 2: 
                            {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}break;
                            
							case 3: 
                            {
								if(data_8 >= '1') data_8 = '0';
								else data_8++;
							}break;
							//
							case 4: 
                            {												// 10month data check
								if(Set_time[3] >= '1') 
                                {
									if(data_8 >= '2') data_8 = '0';
									else data_8++;
								}
								else 
                                {
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
							}break;
							//
							case 5: {
								if(data_8 >= '3') data_8 = '0';
								else data_8++;
							}break;
							
							//
							case 6: {												// 10hour data check
								if(Set_time[5] != '3') {
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
								else {
									if(data_8 >= '1') data_8 = '0';
									else data_8++;
								}
   							}
							
							break;
							//
							case 7: {
								if(data_8 >= '2') data_8 = '0';
								else data_8++;
							}
							
							break;
							//
							case 8: {
								if(Set_time[7] != '2') {
									if(data_8 >= '9') data_8 = '0';
									else data_8++;
								}
								else {
									if(data_8 >= '3') data_8 = '0';
									else data_8++;
								}
							}
							
							break;
							//
							case 9:	{
								if(data_8 >= '5') data_8 = '0';
								else data_8++;
							}
							
							break;
							case 10: {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}
							
							break;
							case 11: {
								if(data_8 >= '5') data_8 = '0';
								else data_8++;
							}
							
							break;
							case 12: {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}
							
							break;
						}
                        Set_time[poss_x] = data_8;
                        if(poss_x == 0)
                        {
                            if(fEepromRead==0)
                            {
                                fEepromRead = 1;
                                Set_time[1] = ( EEPROMRead( SET_YEAR_0 + ((Set_time[0]-0x30)*6))/10 ) + 0x30;
                                Set_time[2] = (EEPROMRead(SET_YEAR_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[3] = (EEPROMRead(SET_MONTH_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[4] = (EEPROMRead(SET_MONTH_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[5] = (EEPROMRead(SET_DAY_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[6] = (EEPROMRead(SET_DAY_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[7] = (EEPROMRead(SET_HOUR_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[8] = (EEPROMRead(SET_HOUR_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[9] = (EEPROMRead(SET_MIN_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[10] = (EEPROMRead(SET_MIN_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[11] = (EEPROMRead(SET_SEC_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[12] = (EEPROMRead(SET_SEC_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                            }
                            
                        }
					}
					else 
                    {
						if(down) 
                        {
							data_8 = Set_time[poss_x];
							switch(poss_x) 
                            {
                                case 0: 
                                {
                                    if(data_8 <= '0') data_8 = '9';
                                    else data_8--; 
                                    fEepromRead=0;
                                }                         
                                break;
								case 1:	
                                {
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								
								break;
								//
								case 2:	{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								
								break;
								//
								case 3: {
									if(data_8 <= '0') data_8 = '1';
									else data_8--;
								}
								break;
								//
								case 4: {
									if(Set_time[3] <= '0') {						// 10month data check
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
									else if(Set_time[3] >=1)
									{
										if(data_8 <= '0')	data_8 = '2';
										else data_8--;										
									}								
								}
								break;
								//
								case 5: {
									if(data_8 <= '0') data_8 = '3';
									else data_8--;
								}
								break;
								//
								case 6: {
									if(Set_time[5] >= '3') {						// 10hour data check
										if(data_8 <= '0') data_8 = '1';
										else data_8--;
									}
									else {
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
								}
								break;
								//
								case 7: {
									if(data_8 <= '0') data_8 = '2';
									else data_8--;
								}
								break;
								//
								case 8: {
									if(Set_time[7] >= '2') {
										if(data_8 <= '0') data_8 = '3';
										else data_8--;
									}
									else {
										if(data_8 <= '0') data_8 = '9';
										else data_8--;
									}
								}
								break;
								//
								case 9: {
									if(data_8 <= '0') data_8 = '5';
									else data_8--;
								}
								break;
								//
								case 10:	{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								break;
								//
								case 11: {
									if(data_8 <= '0') data_8 = '5';
									else data_8--;
								}
								break;
								//
								case 12:{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								break;
							}
                            
							Set_time[poss_x] = data_8;
                            if(poss_x == 0)
                            {
                                Set_time[1] = (EEPROMRead(SET_YEAR_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[2] = (EEPROMRead(SET_YEAR_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[3] = (EEPROMRead(SET_MONTH_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[4] = (EEPROMRead(SET_MONTH_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[5] = (EEPROMRead(SET_DAY_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[6] = (EEPROMRead(SET_DAY_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[7] = (EEPROMRead(SET_HOUR_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[8] = (EEPROMRead(SET_HOUR_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[9] = (EEPROMRead(SET_MIN_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[10] = (EEPROMRead(SET_MIN_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                                Set_time[11] = (EEPROMRead(SET_SEC_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                                Set_time[12] = (EEPROMRead(SET_SEC_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                            }
                                    
						}
						else if(enter) 
                        {
							if(poss_x >= 12) 
                            {
								modi = LO;modi_end = HI;
                                year_buf = ((Set_time[1]-0x30)*10)+(Set_time[2]-0x30);
                                mth_buf = ((Set_time[3]-0x30)*10)+(Set_time[4]-0x30);
                                day_buf = ((Set_time[5]-0x30)*10)+(Set_time[6]-0x30);
                                hour_buf = ((Set_time[7]-0x30)*10)+(Set_time[8]-0x30);
                                min_buf = ((Set_time[9]-0x30)*10)+(Set_time[10]-0x30);
                                sec_buf = ((Set_time[11]-0x30)*10)+(Set_time[12]-0x30);
                                EEPROMWrite(CHG_SET_NO, Set_time[0]-0x30);
								EEPROMWrite(SET_YEAR_0+((Set_time[0]-0x30)*6), year_buf);
                                EEPROMWrite(SET_MONTH_0+((Set_time[0]-0x30)*6), mth_buf);
                                EEPROMWrite(SET_DAY_0+((Set_time[0]-0x30)*6), day_buf);
                                EEPROMWrite(SET_HOUR_0+((Set_time[0]-0x30)*6), hour_buf);
                                EEPROMWrite(SET_MIN_0+((Set_time[0]-0x30)*6), min_buf);
                                EEPROMWrite(SET_SEC_0+((Set_time[0]-0x30)*6), sec_buf);
                            }
							else poss_x++;
						} 
                    }
                    
                    if(esc) modi = LO;
                    key_clr();
                    
				}
                
                
            }break;
            
            case(MENU_CHG_CONDITION_SET):           //CHG Condition
			{
				if(!modi)
				{   
					if(up)
                    {
                        iMenuIndex = MENU_CHG_TIME_SET;//23;
                        fEepromRead=0;
                    }
					else
					{
						if(down) iMenuIndex = MENU_DEBUG;//25;
						else if(enter)
						{
                            modi = HI;
							poss_x = 0; 
						}
					}
					if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{
                    if(up)
					{
                        data_8 = Set_COND[poss_x];
						switch(poss_x)
						{
                            case 0:
                            {           
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}break;
                            
							case 1:
                            {
                                if(Set_COND[0] == '0')
                                {
                                    if(data_8 >= '9') data_8 = '1';
                                    else data_8++;
                                }
                                else
                                {
                                    if(data_8 >= '9') data_8 = '0';
                                    else data_8++;
                                }
                                
							}break;
							//
							case 2: 
                            {
								if(data_8 >= '9') data_8 = '0';
								else data_8++;
							}break;
                            
							case 3: 
                            {
                                if(Set_COND[2] == '0')
                                {
                                    if(data_8 >= '9') data_8 = '1';
                                    else data_8++;
                                }
                                else
                                {
                                    if(data_8 >= '9') data_8 = '0';
                                    else data_8++;
                                }
							}break;
                        }
                        Set_COND[poss_x] = data_8;
                    }
                    else 
                    {
						if(down) 
                        {
							data_8 = Set_COND[poss_x];
							switch(poss_x) 
                            {
                                case 0: 
                                {
                                    if(data_8 <= '0') data_8 = '9';
                                    else data_8--; 
                                }                         
                                break;
								case 1:	
                                {
                                    if(Set_COND[0] == '0')
                                    {
                                        if(data_8 <= '1') data_8 = '9';
                                        else data_8--;
                                    }
                                    else
                                    {
                                        if(data_8 <= '0') data_8 = '9';
                                        else data_8--;
                                    }
								}
								
								break;
								//
								case 2:	{
									if(data_8 <= '0') data_8 = '9';
									else data_8--;
								}
								
								break;
								//
								case 3: 
                                {
									if(Set_COND[2] == '0')
                                    {
                                        if(data_8 <= '1') data_8 = '9';
                                        else data_8--;
                                    }
                                    else
                                    {
                                        if(data_8 <= '0') data_8 = '9';
                                        else data_8--;
                                    }
								}
								break;
							}                            
							Set_COND[poss_x] = data_8;
						}
						else if(enter) 
                        {
							if(poss_x >= 3) 
                            {
								modi = LO;modi_end = HI;
                                CHG_Cut_TIME = ((Set_COND[0]-0x30)*10)+(Set_COND[1]-0x30);
                                CHG_Cut_CURR = ((Set_COND[2]-0x30)*10)+(Set_COND[3]-0x30);
                                if(CHG_Cut_TIME == 0)
                                {
                                    Set_COND[0] = '0';
                                    Set_COND[1] = '1';
                                    CHG_Cut_TIME = ((Set_COND[0]-0x30)*10)+(Set_COND[1]-0x30);
                                }
                                if(CHG_Cut_CURR == 0)
                                {
                                    Set_COND[2] = '0';
                                    Set_COND[3] = '1';
                                    CHG_Cut_CURR = ((Set_COND[2]-0x30)*10)+(Set_COND[3]-0x30);
                                }
                                EEPROMWrite(CHG_SET_TIME, CHG_Cut_TIME);
                                EEPROMWrite(CHG_SET_CURR, CHG_Cut_CURR);
                            }
							else poss_x++;
						} 
                    }
                    if(esc) modi = LO;
                    key_clr();
				}
            }break;
            
            case(MENU_DEBUG): {
				if(!modi)
				{
					if(up) 
                    {
                        if(EEPROMRead(BATT_TYPE) == 2) iMenuIndex = MENU_ADCGAIN;//21;
                        else iMenuIndex = MENU_CHG_CONDITION_SET;//24;
                    }
					else
					{
						if(down) 
                        {
                            if(EEPROMRead(BATT_TYPE) == 2) iMenuIndex = MENU_CHARGE_VOLT_SET_LI;//9;
                            else iMenuIndex = MENU_CHARGE_VOLT_SET_PB;//10;
                        }
						else if(enter)
						{		//press Enter key routine -> else is Enter key pressed routine
							modi = HI;
							poss_x = 0;
							for(cnt=0;cnt<6;cnt++) temp_buffer[cnt] = '0';
						}
                        
					}
                    if(esc) iMenuIndex = MENU_LOGIN;//5;
					key_clr();
				}
				else
				{	//system setup passwd display Enter Key
					if(up)
					{
						data_8 = temp_buffer[poss_x];
						if(data_8 == '9') data_8 = '0';
						else data_8++;
						temp_buffer[poss_x] = data_8;
					}
					else
					{
						if(down)
						{
							data_8 = temp_buffer[poss_x];
							if(data_8 == '0') data_8 = '9';
							else data_8--;
							temp_buffer[poss_x] = data_8;
						}
						else if(enter)
						{
							if(poss_x >= 5)
							{
								i = 0;modi = LO;
								for(cnt=0;cnt<6;cnt++)
								{
									data_8 = gPin2[cnt];
									data_cal = temp_buffer[cnt];
									i += (data_8 ^ data_cal);
								}
								if(i == 0) iMenuIndex = MENU_DEVICE_RUNNING;//26;
								else
								{
									i = 0;
									for(cnt=0;cnt<6;cnt++)
									{
										data_8 = gPin2[cnt];
										data_cal = temp_buffer[cnt];
										i += (data_8 ^ data_cal);
									}
									if(i == 0) iMenuIndex = MENU_DEVICE_RUNNING;//26;
								}
							}						
							else poss_x++;
						}
						if(esc) {
							modi = LO;
							iMenuIndex = MENU_LOGIN;//5;		//pressed ESC key Return to System Setting Password Menu
						}
					}
					key_clr();
				}
			}break;            
                           
            case(MENU_DEVICE_RUNNING)://26
			{
				if(!modi)
				{
					if(up) iMenuIndex = MENU_BATTYPE_ACVOLT;//27;
					else
					{
						if(down) iMenuIndex = MENU_BATTYPE_ACVOLT;//27;
						else if(enter)
						{
							modi = HI;
							poss_x = poss_y = 0;
							temp_buffer[0] = EEPROMRead(OP_MODE);
						}
					}
					if(esc) iMenuIndex = MENU_DEBUG;//25;
					key_clr();
				}
				else
				{
					if(up)
					{
						data_8 = temp_buffer[0];
						if(data_8 == 'N') data_8 = 'Y';
						else data_8 = 'N';
						temp_buffer[0] = data_8;
					}
					else
					{
						if(down)
						{
							data_8 = temp_buffer[0];
							if(data_8 == 'Y') data_8 = 'N';
							else data_8 = 'Y';
							temp_buffer[0] = data_8;
						}
						else if(enter)
						{
							modi = LO;
							err_menu = iMenuIndex;
							iMenuIndex = MENU_DEVICE_RUNNING;//26
							cal_error = HI;
							EEPROMWrite((OP_MODE),temp_buffer[0]);
                            if(EEPROMRead(OP_MODE) == 'Y') CHG_Sts.b5 = 1;
                            else    CHG_Sts.b5 = 0;
                            Set_OK=1;
						}
					}
					if(esc) modi = LO;
					key_clr();
				}
			}break;           
            
            case(MENU_BATTYPE_ACVOLT):
			{
				if(!modi)
				{
					if(up) iMenuIndex = MENU_DEVICE_RUNNING;//26;
					else
					{
						if(down) iMenuIndex = MENU_DEVICE_RUNNING;//26;
						else if(enter)
						{
							modi = HI;
							poss_x = poss_y = 0;
                            temp_buffer[0] = EEPROMRead(INPUT_VOLT);
							temp_buffer[1] = EEPROMRead(BATT_TYPE);  
						}
					}
					if(esc) iMenuIndex = MENU_DEBUG;//25;
					key_clr();
				}
               
				else
				{
					if(up)
					{
                        switch(poss_x) 
                        {
                            case 0:
                            {           
								data_8 = temp_buffer[0];
                                if(data_8 == 0) data_8 = 1;
                                else data_8 = 0;
                                temp_buffer[0] = data_8;
							}break;
                            
                            case 1:
                            {    
                                data_8 = temp_buffer[1];
                                if(data_8 >= 2) data_8 = 0;
                                else data_8++;
                                temp_buffer[1] = data_8;
                            }break;
                        }
					}
					else
					{
						if(down)
						{
                            switch(poss_x) 
                            {
                                case 0:
                                { 
                           			data_8 = temp_buffer[0];
                                    if(data_8 == 1) data_8 = 0;
                                    else data_8 = 1;
                                    temp_buffer[0] = data_8;
                                }break;
                                
                                case 1:
                                { 
                           			data_8 = temp_buffer[1];
                                    if(data_8 <= 0) data_8 = 2;
                                    else data_8--;
                                    temp_buffer[1] = data_8;
                                }break;
                            }
						}
						else if(enter)
						{
                            if(poss_x >= 1) 
                            {
                                modi = LO;modi_end = HI;
                                err_menu = iMenuIndex;
                                iMenuIndex = MENU_BATTYPE_ACVOLT;
                                cal_error = HI;
                                EEPROMWrite((INPUT_VOLT),temp_buffer[0]);
                                EEPROMWrite((BATT_TYPE),temp_buffer[1]);  
                                                                
                                if(EEPROMRead(INPUT_VOLT) == 0)//380ACV
                                {
                                    Input_UV_Ref = MinInputVoltage_380;	//18.06.18 -10% - KHJ
                                    Input_OV_Ref = MaxInputVoltage_380;  	//18.06.18 +10% - KHJ
                                    Input_V_Ref = RefInputVoltage_380;
                                    Input_Shutdown_Ref = ShutdownVoltage_380;   //18.06.18 -40% - Order by GJJ 
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
                                else
                                {
                                    Input_UV_Ref = MinInputVoltage_220;	//18.06.18 -10% - KHJ
                                    Input_OV_Ref = MaxInputVoltage_220;  	//18.06.18 +10% - KHJ
                                    Input_V_Ref = RefInputVoltage_220;
                                    Input_Shutdown_Ref = ShutdownVoltage_220;   //18.06.18 -40% - Order by GJJ 
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
                                
                                for(cnt=0;cnt<8;cnt++)
                                {
                                    ginValarm[cnt] = EEPROMRead(INVALARM_0+cnt);
                                }
                                Set_Input_OV = ((ginValarm[0]-0x30)*100)+((ginValarm[1]-0x30)*10)+((ginValarm[2]-0x30)*1)+((ginValarm[3]-0x30)*0.1);
                                Set_Input_UV = ((ginValarm[4]-0x30)*100)+((ginValarm[5]-0x30)*10)+((ginValarm[6]-0x30)*1)+((ginValarm[7]-0x30)*0.1);
                                
                                if(EEPROMRead(BATT_TYPE) == 0)//PB125V
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
                  
                                    EEPROMWrite(OUTVALARM_0,		'1');
                                    EEPROMWrite(OUTVALARM_1,		'5');
                                    EEPROMWrite(OUTVALARM_2,		'0');
                                    EEPROMWrite(OUTVALARM_3,		'0');
                                    EEPROMWrite(OUTVALARM_4,		'1');
                                    EEPROMWrite(OUTVALARM_5,		'1');
                                    EEPROMWrite(OUTVALARM_6,		'0');
                                    EEPROMWrite(OUTVALARM_7,		'0');
                                    CHG_Sts.b4 = 0;
                                    CHG_Sts.b3 = 0;
                                }
                                else if(EEPROMRead(BATT_TYPE) == 1)
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
                                    EEPROMWrite(BATTVOLT_1,        '2');
                                    EEPROMWrite(BATTVOLT_2,        '4');
                                    EEPROMWrite(BATTVOLT_3,        '0');
                                    EEPROMWrite(BATTVOLT_4,        '1');
                                    EEPROMWrite(BATTVOLT_5,        '2');
                                    EEPROMWrite(BATTVOLT_6,        '4');
                                    EEPROMWrite(BATTVOLT_7,        '0');
                  
                                    EEPROMWrite(OUTVALARM_0,		'1');
                                    EEPROMWrite(OUTVALARM_1,		'5');
                                    EEPROMWrite(OUTVALARM_2,		'0');
                                    EEPROMWrite(OUTVALARM_3,		'0');
                                    EEPROMWrite(OUTVALARM_4,		'1');
                                    EEPROMWrite(OUTVALARM_5,		'0');
                                    EEPROMWrite(OUTVALARM_6,		'5');
                                    EEPROMWrite(OUTVALARM_7,		'0');
                                    CHG_Sts.b4 = 0;
                                    CHG_Sts.b3 = 1;
                                }
                                else if(EEPROMRead(BATT_TYPE) == 2)
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
                                    CHG_Sts.b4 = 1;
                                    CHG_Sts.b3 = 0;
                                }
                                for(cnt=0;cnt<8;cnt++)
                                {
                                    shutDownOutV[cnt] = EEPROMRead(SHUTDOWNOUTV_0+cnt);
                                    goutValarm[cnt] = EEPROMRead(OUTVALARM_0+cnt);
                                    battVoltSet[cnt] = EEPROMRead(BATTVOLT_0+cnt);
                                    outSetNew[cnt] = EEPROMRead(BATTVOLT_0+cnt);
                                }
                                Set_Input_OV = ((ginValarm[0]-0x30)*100)+((ginValarm[1]-0x30)*10)+((ginValarm[2]-0x30)*1)+((ginValarm[3]-0x30)*0.1);
                                Set_Input_UV = ((ginValarm[4]-0x30)*100)+((ginValarm[5]-0x30)*10)+((ginValarm[6]-0x30)*1)+((ginValarm[7]-0x30)*0.1);
                                
                                Set_OV = ((shutDownOutV[0]-0x30)*100)+((shutDownOutV[1]-0x30)*10)+((shutDownOutV[2]-0x30)*1)+((shutDownOutV[3]-0x30)*0.1);
                                Set_UV = ((shutDownOutV[4]-0x30)*100)+((shutDownOutV[5]-0x30)*10)+((shutDownOutV[6]-0x30)*1)+((shutDownOutV[7]-0x30)*0.1);
                                Set_Output_OV = ((goutValarm[0]-0x30)*100)+((goutValarm[1]-0x30)*10)+((goutValarm[2]-0x30)*1)+((goutValarm[3]-0x30)*0.1);
                                Set_Output_UV = ((goutValarm[4]-0x30)*100)+((goutValarm[5]-0x30)*10)+((goutValarm[6]-0x30)*1)+((goutValarm[7]-0x30)*0.1);
                                Set_VRef = ((battVoltSet[0]-0x30)*100)+((battVoltSet[1]-0x30)*10)+((battVoltSet[2]-0x30)*1)+((battVoltSet[3]-0x30)*0.1);
                                                                
                                Set_OK = 1;
                            }
                            else poss_x++;
						}
					}
                    if(esc) modi = LO;
					key_clr();
				}
			}break; 
			default:
			break;
		}
	}
}

void digit_disp()
{
	unsigned char data_8=0;    
    unsigned char x_cnt=0;
    unsigned char data_cal,y_cnt=0;
	unsigned char cnt1,cnt2;//,/cnt3;
	unsigned int dg[12];//,dg10,dg100,dg1000;
    //unsigned char	data_8,data_cal,i,cnt,buff_8;
	unsigned int	data_16,buff_16;
	
    //Voltage_Out = 125.1;
    //Current_Out = 100.1; //goooo
	switch(iMenuIndex)
	{
		case(MENU_CHARGER_INFO):{
			dg[0] = ((Voltage_Out*10)/1000);
			dg[1] = (((Voltage_Out*10)-(dg[0]*1000))/100);
			dg[2] = (((Voltage_Out*10)-(dg[0]*1000))-(dg[1]*100))/10;
			dg[3] = ((Voltage_Out*10)-(dg[0]*1000))-(dg[1]*100)-(dg[2]*10);
			dg[4] = ((Current_Out*10)/1000);
			dg[5] = (((Current_Out*10)-(dg[4]*1000))/100);
			dg[6] = (((Current_Out*10)-(dg[4]*1000))-(dg[5]*100))/10;
			dg[7] = ((Current_Out*10)-(dg[4]*1000))-(dg[5]*100)-(dg[6]*10);
			for(x_cnt=0;x_cnt<8;x_cnt++)
			{								
				if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
				else lcd_poss((x_cnt+9)*8,4);
				//
				data_cal = dg[x_cnt]+0x30;
				for(cnt1=0;cnt1<4;cnt1++)												
				{	
					data_8 = CHAR1[(data_cal-0x20)][cnt1];
					if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60;
					lcd_inst(0,1,data_8);
				}
				for(cnt2=4;cnt2<8;cnt2++)												
				{	
					data_8 = CHAR1[(data_cal-0x20)][cnt2];
					if((x_cnt == 3) && (cnt2 == 0)) data_8 |= 0x60;
					lcd_inst(0,1,data_8);
				}
			}
		}break;
		case(MENU_AC_INFO):{
			dg[0] = ((ACV_R_filter)/100);
			dg[1] = (((ACV_R_filter)-(dg[0]*100))/10);
			dg[2] = (((ACV_R_filter)-(dg[0]*100))-(dg[1]*10));
			dg[3] = ((ACV_S_filter)/100);
			dg[4] = (((ACV_S_filter)-(dg[3]*100))/10);
			dg[5] = (((ACV_S_filter)-(dg[3]*100))-(dg[4]*10));
			dg[6] = ((ACV_T_filter)/100);
			dg[7] = (((ACV_T_filter)-(dg[6]*100))/10);
			dg[8] = (((ACV_T_filter)-(dg[6]*100))-(dg[7]*10));
			
			for(cnt2=0;cnt2<3;cnt2++) {			// R-phase volt
				lcd_poss((cnt2+4)*8,4);
				data_cal = dg[cnt2]+0x30;					
				lcd_put_char(data_cal);
			}
			//-----------------------------------------
			for(cnt2=0;cnt2<3;cnt2++) {			// S-phase volt
				lcd_poss((cnt2+8)*8,4);
				data_cal = dg[3+cnt2]+0x30;
				lcd_put_char(data_cal);
			}
			//-----------------------------------------
			for(cnt2=0;cnt2<3;cnt2++) {			// T-phase volt
				lcd_poss((cnt2+12)*8,4);
				data_cal = dg[6+cnt2]+0x30;
				lcd_put_char(data_cal);
			}
		}break;
		case(MENU_TIME_DISP):case(MENU_TIME_SET):{//display time 
			if(!modi) {	
				rtc_get_date();
				rtc_get_time();
                // 
				timeBuf[0]=((year)/10+0x30);
				timeBuf[1]=(year%10)+0x30;
				timeBuf[2]=((mth)/10+0x30);
				timeBuf[3]=(mth%10)+0x30;
				timeBuf[4]=((day)/10+0x30);
				timeBuf[5]=(day%10)+0x30;
				timeBuf[6]=((hour)/10+0x30);
				timeBuf[7]=(hour%10)+0x30;
				timeBuf[8]=((min)/10+0x30);
				timeBuf[9]=(min%10)+0x30;
				timeBuf[10]=((sec)/10+0x30);
				timeBuf[11]=(sec%10)+0x30;
				//
				for(cnt1=0;cnt1<16;cnt1++)
				{
					switch(cnt1)
					{
                        // year
						case 0:	x_cnt = 6;y_cnt = 4;data_8 = timeBuf[0];break;
						case 1: x_cnt = 7;y_cnt = 4;data_8 = timeBuf[1];break;
						//
						case 2:	x_cnt = 8;y_cnt = 4;data_8 = '-';break;
						// 12-month
						case 3: x_cnt = 9;y_cnt = 4;data_8 = timeBuf[2];break;
						case 4: x_cnt = 10;y_cnt = 4;data_8 = timeBuf[3];break;
						// 
						case 5: x_cnt = 11;y_cnt = 4;data_8 = '-';break;
						// 31-day
						case 6:	x_cnt = 12;y_cnt = 4;data_8 = timeBuf[4];break;
						case 7: x_cnt = 13;y_cnt = 4;data_8 = timeBuf[5];break;
						// 23-hour
						case 8:	x_cnt = 6;y_cnt = 6;data_8 = timeBuf[6];break;
						case 9: x_cnt = 7;y_cnt = 6;data_8 = timeBuf[7];break;
						//
						case 10: x_cnt = 8;y_cnt = 6;data_8 = ':';break;
						// 17-minute
						case 11:	x_cnt = 9;y_cnt = 6;data_8 = timeBuf[8];break;
						case 12: x_cnt = 10;y_cnt = 6;data_8 = timeBuf[9];break;
						// 
						case 13: x_cnt = 11;y_cnt = 6;data_8 = ':';break;
						// 30-sec
						case 14:	x_cnt = 12;y_cnt = 6;data_8 = timeBuf[10];break;
						case 15: x_cnt = 13;y_cnt = 6;data_8 = timeBuf[11];break;
					}
					lcd_poss(x_cnt*8,y_cnt);
					lcd_put_char(data_8);
				}
			}
			else {
				for(cnt1=0;cnt1<16;cnt1++) {
					switch(cnt1) {
						// year
						case 0: x_cnt = 6;y_cnt = 4;data_8 = timeBuf[0];break;
						case 1: x_cnt = 7;y_cnt = 4;data_8 = timeBuf[1];break;
						//
						case 2: x_cnt = 8;y_cnt = 4;data_8 = '-';break;
						// month
						case 3: x_cnt = 9;y_cnt = 4;data_8 = timeBuf[2];break;
						case 4: x_cnt = 10;y_cnt = 4;data_8 = timeBuf[3];break;
						// 
						case 5: x_cnt = 11;y_cnt = 4;data_8 = '-';break;
						// day
						case 6: x_cnt = 12;y_cnt = 4;data_8 = timeBuf[4];break;
						case 7: x_cnt = 13;y_cnt = 4;data_8 = timeBuf[5];break;
						// hour
						case 8: x_cnt = 6;y_cnt = 6;data_8 = timeBuf[6];break;
						case 9: x_cnt = 7;y_cnt = 6;data_8 = timeBuf[7];break;
						//
						case 10: x_cnt = 8;y_cnt = 6;data_8 = ':';break;
						// minute
						case 11: x_cnt = 9;y_cnt = 6;data_8 = timeBuf[8];break;
						case 12: x_cnt = 10;y_cnt = 6;data_8 = timeBuf[9];break;
						// 
						case 13: x_cnt = 11;y_cnt = 6;data_8 = ':';break;
						// sec
						case 14: x_cnt = 12;y_cnt = 6;data_8 = timeBuf[10];break;
						case 15: x_cnt = 13;y_cnt = 6;data_8 = timeBuf[11];break;
					}
					lcd_poss(x_cnt*8,y_cnt);
					for(cnt2=0;cnt2<8;cnt2++) {
						data_cal = CHAR1[(data_8-0x20)][cnt2];
						if(cnt1 == bar_poss) data_cal |= 0x80;
						else  data_cal &= 0x7f;
						lcd_inst(LO,HI,data_cal);
					}
				}
			}
		}break;
		//
		case(MENU_TEMPERATURE): {
			dg[0]=((Temperature*10)/1000);
			dg[1]=(((Temperature*10)-(dg[0]*1000))/100);
			dg[2]=(((Temperature*10)-(dg[0]*1000))-(dg[1]*100))/10;
			dg[3]=((Temperature*10)-(dg[0]*1000))-(dg[1]*100)-(dg[2]*10);
            //if(iDebounce++>10)
            //{
                iDebounce=0;
                for(x_cnt=0;x_cnt<4;x_cnt++)
                {								
                    lcd_poss((x_cnt+7)*8,4);//72-80
                    data_cal = dg[x_cnt]+0x30;
                    for(cnt1=0;cnt1<8;cnt1++)												
                    {	
                        data_8 = CHAR1[(data_cal-0x20)][cnt1];
                        if((x_cnt == 3) && (cnt1 == 0)) data_8 |= 0x60;
                        lcd_inst(LO,HI,data_8);
                    }
                }                
            //}    
			
		}break;
		//
		case(MENU_LOGIN): {
				if(!modi) {
					for(x_cnt=5;x_cnt<11;x_cnt++) {
						lcd_poss(x_cnt*8,4);
						lcd_put_char('*');
					}
				}
				else {
					for(x_cnt=5;x_cnt<11;x_cnt++) {
						lcd_poss(x_cnt*8,4);
						data_cal = temp_buffer[x_cnt-5];
						data_cal -= 0x20;
						if((x_cnt-5) == poss_x) {
							for(cnt1=0;cnt1<8;cnt1++) {
								data_8 = CHAR1[data_cal][cnt1];
								if(cnt1 == 0) data_8 &= 0x7f;
								else data_8 |= 0x80;
								lcd_inst(LO,HI,data_8);
							}
						}
						else {
							for(cnt1=0;cnt1<8;cnt1++) {
								data_8 = CHAR1[data_cal][cnt1];
								lcd_inst(LO,HI,data_8);
							}
						}						
					}
				}
		}break;
		        
        case(MENU_CHARGE_VOLT_SET_LI): {
                if(!modi) {	//Just View Display				
					for(x_cnt=0;x_cnt<4;x_cnt++)
					{								
						lcd_poss((x_cnt+9)*8,4);
						battVoltSet[x_cnt] = EEPROMRead(BATTVOLT_0+x_cnt);
						data_cal = battVoltSet[x_cnt];				
						//
						for(cnt1=0;cnt1<8;cnt1++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt1];									  
							if((x_cnt == 3)  && (cnt1 == 0)) data_8 |= 0x60;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
				else { 		//modifing Display//
					for(x_cnt=0;x_cnt<4;x_cnt++)
					{
						lcd_poss((x_cnt+9)*8,4);
						data_cal = battVoltSet[x_cnt];						
						for(cnt1=0;cnt1<8;cnt1++)
						{
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(x_cnt == poss_x) data_8 |= 0x80;
							else data_8 &= 0x7f;
							if((x_cnt == 3) && (cnt1 == 0)) data_8 |= 0x60;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
		}break; 
        
		case(MENU_CHARGE_VOLT_SET_PB): 
        {
                if(!modi) 
                {	//Just View Display				
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{								
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						battVoltSet[x_cnt] = EEPROMRead(BATTVOLT_0 + x_cnt);
						//data_cal = '1';//battVoltSet[x_cnt];
                        data_cal = battVoltSet[x_cnt];
						//
						for(cnt1=0;cnt1<4;cnt1++)//float volt//										
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60; //0x0110-0000//				
							lcd_inst(LO,HI,data_8);
						}
						for(cnt2=4;cnt2<8;cnt2++)//equ. volt//
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt2];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt2 == 0)) data_8 |= 0x60; //0x0110-0000//
							lcd_inst(LO,HI,data_8);
						}
					}
				}
				else 
                {
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						data_cal = battVoltSet[x_cnt];						
						for(cnt1=0;cnt1<8;cnt1++)
						{
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(x_cnt == poss_x) data_8 |= 0x80;
							else data_8 &= 0x7f;
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
		}break;
		//
        case(MENU_MANUAL_SET): {//digit_disp()//
            if(!modi)
            {
                if(tmrCnt[7]>500)
                {
                    tmrCnt[7]=0;
                    Sp2_LED_Toggle();
                }
                //hex1_16 =1310;//test goooo//
                //-------------------------------------------------------------
            if(fManVolt==1)
            {
                for(cnt1=0;cnt1<4;cnt1++) 
                {
                    if(CHG_Mode == 0)   // Equalizing Charge
                    {
                        outSetNew[cnt1] = EEPROMRead(BATTVOLT_4+cnt1);
                    }
                    else if(CHG_Mode == 1)  // Floating Charge
                    {
                        outSetNew[cnt1] = EEPROMRead(BATTVOLT_0+cnt1);
                    }   
                    LoadAmpSet[cnt1] = EEPROMRead(LOADAMP_0+cnt1);
                }
                fManVolt = 2;//system boot after only 1 excute//
            }
                data_16 = ((outSetNew[0] & 0x0f)*1000);
                buff_16 = ((outSetNew[1] & 0x0f)*100);
                data_8 = ((outSetNew[2] & 0x0f)*10);
                buff_8 = (outSetNew[3] & 0x0f);
                hex1_16 = (data_16 + buff_16 + (unsigned int)data_8 + (unsigned int)buff_8);
                //
            
                 
                //--------------------------------------------------------------------------
               // if(hex1_16 !=1310 && hex1_16 != 1410)hex1_16=1311;
                hex16_4digt(hex1_16);
                outSetNew[0] = thnd;outSetNew[1] = hund;
                outSetNew[2] = ten;outSetNew[3] = one;
                for(cnt1=0;cnt1<4;cnt1++) 
                {
                    lcd_poss((cnt1+9)*8,4);
                    data_cal = outSetNew[cnt1];
                    for(cnt2=0;cnt2<8;cnt2++) 
                    {
                        data_8 = CHAR1[(data_cal-0x20)][cnt2];
                        if((cnt1 == 3) && (cnt2 == 0)) data_8 |= 0x60;
                        lcd_inst(LO,HI,data_8);
                        //
                    }
                }
                //
                for(cnt1=4;cnt1<7;cnt1++) 
                {
                    lcd_poss((cnt1+9-3)*8,6);
                    LoadAmpSet[cnt1-4] = EEPROMRead(LOADAMP_0+cnt1-4);
                    data_8 = LoadAmpSet[cnt1-4];
                    lcd_put_char(data_8);
                }
            }//End if(!modi)
            else 
            {
                hex16_4digt(hex1_16);
                outSetNew[0] = thnd;outSetNew[1] = hund;
                outSetNew[2] = ten;outSetNew[3] = one;
                for(cnt1=0;cnt1<4;cnt1++) 
                {
                    lcd_poss((cnt1+9)*8,4);
                    data_cal = outSetNew[cnt1];
                    for(cnt2=0;cnt2<8;cnt2++) 
                    {
                        data_8 = CHAR1[(data_cal-0x20)][cnt2];
                        if(cnt1 == poss_x) data_8 |= 0x80;
                        else  data_8 &= 0x7f;
                        if((cnt1 == 3) && (cnt2 == 0)) data_8 |= 0x60;
                        lcd_inst(LO,HI,data_8);
                    }
                }
                //
                for(cnt1=4;cnt1<7;cnt1++) 
                {
                    lcd_poss((cnt1+9-3)*8,6);
                    data_cal = LoadAmpSet[cnt1-4];

                    for(cnt2=0;cnt2<8;cnt2++) {
                        data_8 = CHAR1[(data_cal-0x20)][cnt2];
                        if(cnt1 == poss_x) data_8 |= 0x80;
                        else  data_8 &= 0x7f;
                        lcd_inst(LO,HI,data_8);
                    }
                }
            }
		}break;
		//
		case(MENU_CURRENT_LIMIT): {
				if(!modi) {
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+9+1)*8,4);
						battAmpSet[x_cnt] = EEPROMRead(BATTAMP_0+x_cnt);
						data_cal = battAmpSet[x_cnt];
						for(cnt1=0;cnt1<8;cnt1++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							lcd_inst(LO,HI,data_8);
						}
					}
				}
				else {																					    
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+9+1)*8,4);
						data_cal = battAmpSet[x_cnt];
						for(cnt1=0;cnt1<8;cnt1++) {
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(x_cnt == poss_x) data_8 |= 0x80;
							else  data_8 &= 0x7f;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
		}break;
		//
		case(MENU_INPUT_ALARM_SET): {
				if(!modi) {	//Just View Display				
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{								
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						ginValarm[x_cnt] = EEPROMRead(INVALARM_0+x_cnt);
						data_cal = ginValarm[x_cnt];				
						//
						for(cnt1=0;cnt1<4;cnt1++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60; //0x0110-0000//				
							lcd_inst(LO,HI,data_8);
						}
						for(cnt2=4;cnt2<8;cnt2++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt2];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt2 == 0)) data_8 |= 0x60; //0x0110-0000//
							lcd_inst(LO,HI,data_8);
						}
					}
				}
				else {
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						data_cal = ginValarm[x_cnt];						
						for(cnt1=0;cnt1<8;cnt1++)
						{
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(x_cnt == poss_x) data_8 |= 0x80;
							else data_8 &= 0x7f;
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
		}break;
		//
		case(MENU_OUTPUT_ALARM_SET): {
				if(!modi) {	//Just View Display				
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{								
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						goutValarm[x_cnt] = EEPROMRead(OUTVALARM_0+x_cnt);
						data_cal = goutValarm[x_cnt];				
						//
						for(cnt1=0;cnt1<4;cnt1++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60; //0x0110-0000//				
							lcd_inst(LO,HI,data_8);
						}
						for(cnt2=4;cnt2<8;cnt2++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt2];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt2 == 0)) data_8 |= 0x60; //0x0110-0000//
							lcd_inst(LO,HI,data_8);
						}
					}
				}
				else {
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						data_cal = goutValarm[x_cnt];						
						for(cnt1=0;cnt1<8;cnt1++)
						{
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(x_cnt == poss_x) data_8 |= 0x80;
							else data_8 &= 0x7f;
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
		}break;
		//
		case(MENU_OUT_CUR_ALARM_SET): {
				if(!modi) {	//Just View Display				
					for(x_cnt=0;x_cnt<3;x_cnt++)
					{								
						lcd_poss((x_cnt+9+1)*8,4);
						goutAalarm[x_cnt] = EEPROMRead(OUTAALARM_0+x_cnt);
						data_cal = goutAalarm[x_cnt];				
						//
						for(cnt1=0;cnt1<8;cnt1++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							lcd_inst(LO,HI,data_8);
						}
					}
				}
				else {
					for(x_cnt=0;x_cnt<3;x_cnt++)
					{
						lcd_poss((x_cnt+9+1)*8,4);
						data_cal = goutAalarm[x_cnt];						
						for(cnt1=0;cnt1<8;cnt1++)
						{
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(x_cnt == poss_x) data_8 |= 0x80;
							else data_8 &= 0x7f;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
		}break;
		//
		case(MENU_CUTOFF_VOLT_SET): {
				if(!modi) {	//Just View Display				
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{								
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						shutDownOutV[x_cnt] = EEPROMRead(SHUTDOWNOUTV_0+x_cnt);
						data_cal = shutDownOutV[x_cnt];				
						//
						for(cnt1=0;cnt1<4;cnt1++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60; //0x0110-0000//				
							lcd_inst(LO,HI,data_8);
						}
						for(cnt2=4;cnt2<8;cnt2++)												
						{	
							data_8 = CHAR1[(data_cal-0x20)][cnt2];
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt2 == 0)) data_8 |= 0x60; //0x0110-0000//
							lcd_inst(LO,HI,data_8);
						}
					}
				}
				else {
					for(x_cnt=0;x_cnt<8;x_cnt++)
					{
						if(x_cnt >= 4) lcd_poss(((x_cnt+9)-4)*8,6);
						else lcd_poss((x_cnt+9)*8,4);
						data_cal = shutDownOutV[x_cnt];						
						for(cnt1=0;cnt1<8;cnt1++)
						{
							data_8 = CHAR1[(data_cal-0x20)][cnt1];
							if(x_cnt == poss_x) data_8 |= 0x80;
							else data_8 &= 0x7f;
							if(((x_cnt == 3) || (x_cnt == 7)) && (cnt1 == 0)) data_8 |= 0x60;
							lcd_inst(LO,HI,data_8);
						}
					}
				}
		}break;
		//
		case(MENU_BUZZER_SET): {
			if(!modi) {
				data_8 = EEPROMRead(ALARM_STOP);
				if(data_8 == 'Y') {										// buzzer en/disable display
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+9)*8,4);
						data_8 = BUZZER[0][x_cnt];
						lcd_put_char(data_8);
					}
				}
				else {
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+9)*8,4);
						data_8 = BUZZER[1][x_cnt];
						lcd_put_char(data_8);
					}
				}
			}
			else {
				for(x_cnt=0;x_cnt<3;x_cnt++) {
					lcd_poss((x_cnt+9)*8,4);
					if(fAlarmStop == 'Y') data_8 = BUZZER[0][x_cnt];
					else data_8 = BUZZER[1][x_cnt];
					for(cnt1=0;cnt1<8;cnt1++) {
						data_cal = CHAR1[(data_8-0x20)][cnt1];
						if(poss_x == 0) data_cal |= 0x80;								
						lcd_inst(LO,HI,data_cal);
					}
				}
					}
		}break;
		//
		case(MENU_PASS_SET): {
				if(!modi) {
					for(cnt1=0;cnt1<6;cnt1++) {
						lcd_poss((cnt1+6)*8,4);
						data_8 = EEPROMRead(PASSWD_0+cnt1);
						lcd_put_char(data_8);
						lcd_poss((cnt1+6)*8,6);
						lcd_put_char('0');
					}
				}
				else {
					for(cnt1=0;cnt1<6;cnt1++) {
						lcd_poss((cnt1+6)*8,4);
						data_8 = EEPROMRead(PASSWD_0+cnt1);
						lcd_put_char(data_8);
						lcd_poss((cnt1+6)*8,6);
						data_8 = gPin[cnt1];
						for(cnt2=0;cnt2<8;cnt2++) {
							data_cal = CHAR1[(data_8-0x20)][cnt2];
							if(cnt1 == poss_x) data_cal |= 0x80;
							else data_cal &= 0x7f;
							lcd_inst(LO,HI,data_cal);
						}
					}
				}
			}break;
			//
		case(MENU_SYS_INIT): {//system init(yes/no)
				if(!modi) {
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+7)*8,4);
						data_8 = INIT_SYS[0][x_cnt];
						lcd_put_char(data_8);
					}
				}
				else {
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+7)*8,4);
						if(temp_buffer[poss_x] == 'Y') data_8 = INIT_SYS[1][x_cnt];
						else data_8 = INIT_SYS[0][x_cnt];
						for(cnt1=0;cnt1<8;cnt1++) {
							data_cal = CHAR1[(data_8-0x20)][cnt1];
							if(poss_x == 0) data_cal |= 0x80;								
							lcd_inst(LO,HI,data_cal);
						}
					}
				}
			}break;
			
        case(MENU_ADCGAIN): 
        {
            if(!modi) 
            {
                ADJ_VGain = EEPROMRead(ADJVGain_0);
                ADJ_CGain = EEPROMRead(ADJCGain_0);
                if(ADJ_VGain >= 0x80)
                {
                    lcd_poss(72,4);                    
                    data_8 = '-';
                    lcd_put_char(data_8);
                    hex1_8=256-ADJ_VGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[0] = hund;
                    gain_buffer[1] = ten;
                    gain_buffer[2] = one;
                    for(cnt1=0;cnt1<3;cnt1++) 
                    {
                        lcd_poss((cnt1+10)*8,4);                        
                        data_8 = gain_buffer[cnt1];
                        lcd_put_char(data_8);
                    }
                }
                else
                {
                    hex1_8=ADJ_VGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[0] = hund;
                    gain_buffer[1] = ten;
                    gain_buffer[2] = one;
                    for(cnt1=0;cnt1<3;cnt1++) 
                    {
                        lcd_poss((cnt1+10)*8,4);                        
                        data_8 = gain_buffer[cnt1];
                        lcd_put_char(data_8);
                    }
                }
                //
                if(ADJ_CGain >= 0x80)
                {
                    lcd_poss(72,6);                    
                    data_8 = '-';
                    lcd_put_char(data_8);
                    hex1_8=256-ADJ_CGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[3] = hund;
                    gain_buffer[4] = ten;
                    gain_buffer[5] = one;																    
                    for(cnt1=3;cnt1<6;cnt1++) 
                    {
                        lcd_poss((cnt1+7)*8,6);                        
                        data_8 = gain_buffer[cnt1];						
                        lcd_put_char(data_8);
                    }
                }
                else
                {
                    hex1_8=ADJ_CGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[3] = hund;
                    gain_buffer[4] = ten;
                    gain_buffer[5] = one;																    
                    for(cnt1=3;cnt1<6;cnt1++) 
                    {
                        lcd_poss((cnt1+7)*8,6);                        
                        data_8 = gain_buffer[cnt1];						
                        lcd_put_char(data_8);
                    }
                }
            }
            else 
            {
                if(ADJ_VGain >= 0x80)
                {
                    hex1_8=256-ADJ_VGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[0] = hund;
                    gain_buffer[1] = ten;
                    gain_buffer[2] = one;
                    for(cnt1=0;cnt1<3;cnt1++) {
                        lcd_poss((cnt1+10)*8,4);                        
                        data_cal = gain_buffer[cnt1];
                        for(cnt2=0;cnt2<8;cnt2++) {
                            data_8 = CHAR1[(data_cal-0x20)][cnt2];
                            if(cnt1 == poss_x) data_8 |= 0x80;
                            else  data_8 &= 0x7f;
                            lcd_inst(LO,HI,data_8);
                        }
                    }
                    lcd_poss(72,4);                    
                    data_cal = '-';
                    for(cnt2=0;cnt2<8;cnt2++) {
                        data_8 = CHAR1[(data_cal-0x20)][cnt2];
                        if(poss_x == 0) data_cal |= 0x80;
                        lcd_inst(LO,HI,data_8);
                    }
                }
                else
                {
                    hex1_8=ADJ_VGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[0] = hund;
                    gain_buffer[1] = ten;
                    gain_buffer[2] = one;
                    for(cnt1=0;cnt1<3;cnt1++) {
                        lcd_poss((cnt1+10)*8,4);                        
                        data_cal = gain_buffer[cnt1];
                        for(cnt2=0;cnt2<8;cnt2++) {
                            data_8 = CHAR1[(data_cal-0x20)][cnt2];
                            if(cnt1 == poss_x) data_8 |= 0x80;
                            else  data_8 &= 0x7f;
                            lcd_inst(LO,HI,data_8);
                        }
                    }
                }
                //
                if(ADJ_CGain >= 0x80)
                {
                    hex1_8=256-ADJ_CGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[3] = hund;
                    gain_buffer[4] = ten;
                    gain_buffer[5] = one;																    
                    for(cnt1=3;cnt1<6;cnt1++) {
                        lcd_poss((cnt1+7)*8,6);                        
                        data_cal = gain_buffer[cnt1];						
                        for(cnt2=0;cnt2<8;cnt2++) {
                            data_8 = CHAR1[(data_cal-0x20)][cnt2];
                            if(cnt1 == poss_x) data_8 |= 0x80;
                            else  data_8 &= 0x7f;
                            lcd_inst(LO,HI,data_8);
                        }
                    }
                    lcd_poss(72,6);                    
                    data_cal = '-';
                    for(cnt2=0;cnt2<8;cnt2++) {
                        data_8 = CHAR1[(data_cal-0x20)][cnt2];
                        if(poss_x == 0) data_cal |= 0x80;
                        lcd_inst(LO,HI,data_8);
                    }
                }
                else
                {
                    hex1_8=ADJ_CGain;
                    hex16_3digt(hex1_8);
                    gain_buffer[3] = hund;
                    gain_buffer[4] = ten;
                    gain_buffer[5] = one;																    
                    for(cnt1=3;cnt1<6;cnt1++) {
                        lcd_poss((cnt1+7)*8,6);
                        
                        data_cal = gain_buffer[cnt1];						
                        for(cnt2=0;cnt2<8;cnt2++) {
                            data_8 = CHAR1[(data_cal-0x20)][cnt2];
                            if(cnt1 == poss_x) data_8 |= 0x80;
                            else  data_8 &= 0x7f;
                            lcd_inst(LO,HI,data_8);
                        }
                    }
                }
            }
        }break;
        case(MENU_CHG_MODE_SET): //#22
        {
            if(!modi) 
            {
                poss_x = EEPROMRead(CHG_STS_0);
                poss_y = EEPROMRead(CHG_STS_1);
                for(x_cnt=0;x_cnt<4;x_cnt++) 
                {
                    if(poss_x == 0)
                    {
                        lcd_poss((x_cnt+1)*8,5);//auto
                    }
                    else if(poss_x == 1 && poss_y == 1)
                    {
                        lcd_poss((x_cnt+6)*8,5);//float
                    }
                    else if(poss_x == 1 && poss_y == 0)
                    {
                        lcd_poss((x_cnt+11)*8,5);//equal
                    }
                    else
                    {
                        lcd_poss((x_cnt+11)*8,6);
                    }
                    data_8 = MODE_SEL[0];
                    lcd_put_char(data_8);
                }
            }//end if(!modi)
            else 
            {
                for(x_cnt=0;x_cnt<4;x_cnt++) 
                {
                    if(poss_x == 0)
                    {
                        lcd_poss((x_cnt+1)*8,5);
                    }
                    else if(poss_x == 1 && poss_y == 1)
                    {
                        lcd_poss((x_cnt+6)*8,5);
                    }
                    else if(poss_x == 1 && poss_y == 0)
                    {
                        lcd_poss((x_cnt+11)*8,5);
                    }
                    else
                    {
                        lcd_poss((x_cnt+11)*8,6);
                    }
                    data_8 = MODE_SEL[0];
                    for(cnt1=0;cnt1<8;cnt1++) 
                    {
                        data_cal = CHAR1[(data_8-0x20)][cnt1];
                        data_cal |= 0x80;								
                        lcd_inst(LO,HI,data_cal);
                    }
                }
            }
        }break;

        case(MENU_CHG_TIME_SET):
        {
            //CHGTIME_SET
            /*
            for(x_cnt=0;x_cnt<5;x_cnt++)
            {
                lcd_poss((x_cnt+1)*8,3);            
                data_8 = CHGTIME_SET[0][x_cnt]; //"  NO:"
                lcd_put_char(data_8);
                //
                lcd_poss((x_cnt+1)*8,5);
                data_8 = CHGTIME_SET[1][x_cnt];//"DATE:"
                lcd_put_char(data_8);
                //
                lcd_poss((x_cnt+1)*8,6);
                data_8 = CHGTIME_SET[2][x_cnt];//"TIME:"
                lcd_put_char(data_8);
            }
*/
            if(!modi) 
            {
                if(fEepromRead==0)
                {
                    fEepromRead = 1;
                    Set_time[0] = EEPROMRead(CHG_SET_NO) + 0x30;
                    Set_time[1] = (EEPROMRead(SET_YEAR_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                    Set_time[2] = (EEPROMRead(SET_YEAR_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                    Set_time[3] = (EEPROMRead(SET_MONTH_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                    Set_time[4] = (EEPROMRead(SET_MONTH_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                    Set_time[5] = (EEPROMRead(SET_DAY_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                    Set_time[6] = (EEPROMRead(SET_DAY_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                    Set_time[7] = (EEPROMRead(SET_HOUR_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                    Set_time[8] = (EEPROMRead(SET_HOUR_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                    Set_time[9] = (EEPROMRead(SET_MIN_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                    Set_time[10] = (EEPROMRead(SET_MIN_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                    Set_time[11] = (EEPROMRead(SET_SEC_0 + ((Set_time[0]-0x30)*6))/10) + 0x30;
                    Set_time[12] = (EEPROMRead(SET_SEC_0 + ((Set_time[0]-0x30)*6))%10) + 0x30;
                }
                for(cnt1=0;cnt1<13;cnt1++)
                {
                    switch(cnt1)
                    {
                        case 0: x_cnt = 6;y_cnt = 3;data_8 = Set_time[0];break;  

                        // year
                        case 1: x_cnt = 6;y_cnt = 5;data_8 = Set_time[1];break;
                        case 2: x_cnt = 7;y_cnt = 5;data_8 = Set_time[2];break;

                        // 12-month
                        case 3: x_cnt = 9;y_cnt = 5;data_8 = Set_time[3];break;
                        case 4: x_cnt = 10;y_cnt = 5;data_8 = Set_time[4];break;

                        // 31-day
                        case 5: x_cnt = 12;y_cnt = 5;data_8 = Set_time[5];break;
                        case 6: x_cnt = 13;y_cnt = 5;data_8 = Set_time[6];break;

                        // 23-hour
                        case 7: x_cnt = 6;y_cnt = 6;data_8 = Set_time[7];break;
                        case 8: x_cnt = 7;y_cnt = 6;data_8 = Set_time[8];break;

                        // 17-minute
                        case 9: x_cnt = 9;y_cnt = 6;data_8 = Set_time[9];break;
                        case 10: x_cnt = 10;y_cnt = 6;data_8 = Set_time[10];break;

                        // 30-sec
                        case 11: x_cnt = 12;y_cnt = 6;data_8 = Set_time[11];break;
                        case 12: x_cnt = 13;y_cnt = 6;data_8 = Set_time[12];break;   
                    }
                    lcd_poss(x_cnt*8,y_cnt);
                    lcd_put_char(data_8);
                }
            }       
            else 
            {
                for(cnt1=0;cnt1<13;cnt1++)
                {
                    switch(cnt1)
                    {
                        case 0: 
                        {
                            x_cnt = 6;
                            y_cnt = 3;
                            data_8 = Set_time[0]; 
                        }break; 
                        // year
                        case 1:
                        {
                            x_cnt = 6;
                            y_cnt = 5;
                            data_8 = Set_time[1];
                        }
                        break;
                        case 2:
                        {
                            x_cnt = 7;
                            y_cnt = 5;
                            data_8 = Set_time[2];
                        }
                        break;

                        // 12-month
                        case 3:
                        {
                            x_cnt = 9;
                            y_cnt = 5;
                            data_8 = Set_time[3];
                        }break;

                        case 4:
                        {
                            x_cnt = 10;
                            y_cnt = 5;
                            data_8 = Set_time[4];
                        }break;

                        // 31-day
                        case 5:
                        {
                            x_cnt = 12;
                            y_cnt = 5;
                            data_8 = Set_time[5];
                        }break;
                        case 6:
                        {
                            x_cnt = 13;
                            y_cnt = 5;
                            data_8 = Set_time[6];
                        }break;

                        // 23-hour
                        case 7: 
                        {
                            x_cnt = 6;
                            y_cnt = 6;
                            data_8 = Set_time[7];
                        }break;
                        case 8:
                        {
                            x_cnt = 7;
                            y_cnt = 6;
                            data_8 = Set_time[8];
                        }break;

                        // 17-minute
                        case 9:
                        {
                            x_cnt = 9;
                            y_cnt = 6;
                            data_8 = Set_time[9];
                        }break;
                        case 10:
                        {
                            x_cnt = 10;
                            y_cnt = 6;
                            data_8 = Set_time[10];
                        }break;

                        // 30-sec
                        case 11:
                        {
                            x_cnt = 12;
                            y_cnt = 6;
                            data_8 = Set_time[11];
                        }break;

                        case 12:
                        {
                            x_cnt = 13;
                            y_cnt = 6;
                            data_8 = Set_time[12];
                        }break;   
                    }
                    lcd_poss(x_cnt*8,y_cnt);
                    for(cnt2=0;cnt2<8;cnt2++) 
                    {
                        data_cal = CHAR1[(data_8-0x20)][cnt2];
                        if(cnt1 == poss_x) data_cal |= 0x80;
                        else  data_cal &= 0x7f;
                        lcd_inst(LO,HI,data_cal);
                    }
                }
            }         
        }break;	

        case(MENU_CHG_CONDITION_SET): 
        {
            if(!modi) 
            {	//Just View Display				
                Set_COND[0] = EEPROMRead(CHG_SET_TIME)/10 + 0x30;
                Set_COND[1] = EEPROMRead(CHG_SET_TIME)%10 + 0x30;
                Set_COND[2] = EEPROMRead(CHG_SET_CURR)/10 + 0x30;
                Set_COND[3] = EEPROMRead(CHG_SET_CURR)%10 + 0x30;

                for(cnt1=0;cnt1<4;cnt1++)
                {
                    switch(cnt1)
                    {
                        // Charge Time
                        case 0: x_cnt = 11;y_cnt = 4;data_8 = Set_COND[0];break;  
                        case 1: x_cnt = 12;y_cnt = 4;data_8 = Set_COND[1];break;
                        // Chrage Current
                        case 2: x_cnt = 11;y_cnt = 6;data_8 = Set_COND[2];break;
                        case 3: x_cnt = 12;y_cnt = 6;data_8 = Set_COND[3];break;
                    }
                    lcd_poss(x_cnt*8,y_cnt);
                    lcd_put_char(data_8);
                }
            }
            else 
            {
                for(cnt1=0;cnt1<4;cnt1++)
                {
                    switch(cnt1)
                    {
                        case 0: 
                        {
                            x_cnt = 11;
                            y_cnt = 4;
                            data_8 = Set_COND[0]; 
                        }break; 
                        // year
                        case 1:
                        {
                            x_cnt = 12;
                            y_cnt = 4;
                            data_8 = Set_COND[1];
                        }
                        break;
                        case 2:
                        {
                            x_cnt = 11;
                            y_cnt = 6;
                            data_8 = Set_COND[2];
                        }
                        break;
                        case 3:
                        {
                            x_cnt = 12;
                            y_cnt = 6;
                            data_8 = Set_COND[3];
                        }break;
                    }
                    lcd_poss(x_cnt*8,y_cnt);
                    for(cnt2=0;cnt2<8;cnt2++) 
                    {
                        data_cal = CHAR1[(data_8-0x20)][cnt2];
                        if(cnt1 == poss_x) data_cal |= 0x80;
                        else  data_cal &= 0x7f;
                        lcd_inst(LO,HI,data_cal);
                    }
                }
            }
        }break;     
        
        //
        case(MENU_DEBUG): {
                 /*
            for(x_cnt=0;x_cnt<4;x_cnt++)//goooo
            {
                lcd_poss((x_cnt+1)*8,3);            
                data_8 = CHGMODE_SEL[0][x_cnt]; //"AUTO"
                lcd_put_char(data_8);
                //
                lcd_poss((x_cnt+7)*8,3);
                data_8 = CHGMODE_SEL[1][x_cnt];//"FLT ""
                lcd_put_char(data_8);
                //
                lcd_poss((x_cnt+12)*8,3);
                data_8 = CHGMODE_SEL[2][x_cnt];//"EQ  "
                lcd_put_char(data_8);
            }
            */
				if(!modi) {
					for(x_cnt=5;x_cnt<11;x_cnt++) {
						lcd_poss(x_cnt*8,4);
						lcd_put_char('*');
					}
				}
				else {
					for(x_cnt=5;x_cnt<11;x_cnt++) {
						lcd_poss(x_cnt*8,4);
						data_cal = temp_buffer[x_cnt-5];
						data_cal -= 0x20;
						if((x_cnt-5) == poss_x) {
							for(cnt1=0;cnt1<8;cnt1++) {
								data_8 = CHAR1[data_cal][cnt1];
								if(cnt1 == 0) data_8 &= 0x7f;
								else data_8 |= 0x80;
								lcd_inst(LO,HI,data_8);
							}
						}
						else {
							for(cnt1=0;cnt1<8;cnt1++) {
								data_8 = CHAR1[data_cal][cnt1];
								lcd_inst(LO,HI,data_8);
							}
						}						
					}
				}
		}break;
          
        case(MENU_DEVICE_RUNNING): 
        {
			if(!modi) 
            {
				data_8 = EEPROMRead(OP_MODE);
				if(data_8 == 'Y') 
                {										
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+9)*8,4);
						data_8 = OP_M[0][x_cnt];
						lcd_put_char(data_8);
					}
				}
				else 
                {
					for(x_cnt=0;x_cnt<3;x_cnt++) {
						lcd_poss((x_cnt+9)*8,4);
						data_8 = OP_M[1][x_cnt];
						lcd_put_char(data_8);
					}
				}
			}
			else 
            {
                for(x_cnt=0;x_cnt<3;x_cnt++) 
                {
					lcd_poss((x_cnt+9)*8,4);
					if(temp_buffer[0] == 'Y') data_8 = OP_M[0][x_cnt];
					else data_8 = OP_M[1][x_cnt];
					for(cnt1=0;cnt1<8;cnt1++) 
                    {
						data_cal = CHAR1[(data_8-0x20)][cnt1];
						if(poss_x == 0) data_cal |= 0x80;								
						lcd_inst(LO,HI,data_cal);
					}
				}
            }
		}break;
        
        case(MENU_BATTYPE_ACVOLT): 
        {
            // waiting Display
			if(!modi) 
            {
                //Input Voltage Select display 
                for(x_cnt=0;x_cnt<3;x_cnt++) 
                {
                    data_8 = EEPROMRead(INPUT_VOLT);
                    if(data_8 == 0) 
                    {										
                        data_8 = IN_VOLT[0][x_cnt];
                        lcd_poss((x_cnt+9)*8,4);
                        lcd_put_char(data_8);
                    }
                    else 
                    {
                        data_8 = IN_VOLT[1][x_cnt];
                        lcd_poss((x_cnt+9)*8,4);                                    
                        lcd_put_char(data_8);
                    }
                }
                for(x_cnt=0;x_cnt<3;x_cnt++) 
                {
                    // Batt type select display
                    data_8 = EEPROMRead(BATT_TYPE);
                    if(data_8 == 0) 
                    {										
                        data_8 = BATT_T[0][x_cnt];
                        lcd_poss((x_cnt+9)*8,6);                                    
                        lcd_put_char(data_8);
                    }
                    else if(data_8 == 1) 
                    {
                        data_8 = BATT_T[1][x_cnt];
                        lcd_poss((x_cnt+9)*8,6);                                    
                        lcd_put_char(data_8);
                    } 
                    else if(data_8 == 2) 
                    {
                        data_8 = BATT_T[2][x_cnt];
                        lcd_poss((x_cnt+9)*8,6);                                    
                        lcd_put_char(data_8);
                    }
                }
			}
			else 
            {
                for(x_cnt=0;x_cnt<3;x_cnt++) 
                {
                    //Input Voltage Select display  
                    lcd_poss((x_cnt+9)*8,4);
                    if(temp_buffer[0] == 0) data_8 = IN_VOLT[0][x_cnt];
                    else data_8 = IN_VOLT[1][x_cnt];
                    for(cnt1=0;cnt1<8;cnt1++) 
                    {
                        data_cal = CHAR1[(data_8-0x20)][cnt1];
                        if(poss_x == 0) data_cal |= 0x80;								
                        lcd_inst(LO,HI,data_cal);
                    }

                    //Battery Type Select display
                    lcd_poss((x_cnt+9)*8,6);
                    if(temp_buffer[1] == 0) data_8 = BATT_T[0][x_cnt];
                    else if(temp_buffer[1] == 1) data_8 = BATT_T[1][x_cnt];
                    else if(temp_buffer[1] == 2) data_8 = BATT_T[2][x_cnt];
                    for(cnt1=0;cnt1<8;cnt1++) 
                    {
                        data_cal = CHAR1[(data_8-0x20)][cnt1];
                        if(poss_x == 1) data_cal |= 0x80;								
                        lcd_inst(LO,HI,data_cal);
                    }
                }
            }
        }break;
        default:
        break;
	}
}

void key_clr()
{
	up = down = esc = enter = LO;
}

// LCD x-y loading function
void lcd_poss(unsigned char x_poss,unsigned char y_poss) 
{
    if(x_poss >= 63) {				
        //CS1 = LO;        
        //CS2 = HI;cs_bit = HI;                
        CS1_SetLow();
        CS2_SetHigh();
        cs_bit=1;
    }
    else {				        
//        CS1 = HI;cs_bit = LO;//PC14
//        CS2 = LO;            //PC13
        CS1_SetHigh();
        CS2_SetLow();
        cs_bit=0;
    }
    lcd_inst(0,0,(Y_LINE | y_poss));
    lcd_inst(0,0,(X_LINE | x_poss));
}//lcd_poss end

void lcd_put_char(unsigned char dchar)
{
	unsigned char	j,c;
	for(j=0;j<=7;j++) {
		
		c = CHAR1[dchar-0x20][j];
		lcd_inst(0,1,c);
	}
}//lcd_put_char end


//============================================================================


/**
 End of File Main.c
*/

