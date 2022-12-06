/*
 * File:   ds1302.c
 * Author: RN04
 *
 * Created on 2022? 6? 7? (?), ?? 3:35
 */
#ifndef FCY
    #define FCY     (_XTAL_FREQ/2)
#endif

#include <libpic30.h>
#include "ds1302.h"
#include "mcc_generated_files/system.h"
#include "var.h"
#include "mcc_generated_files/charlcd1.h"
//#include "mcc_generated_files/pin_manager.h"

#define TRUE    1
#define FALSE   0

// DS1302 Clock Halt Register & Bits
const uint8_t DS1302_REG_CH = 0x80; // bit in the seconds register
const uint8_t DS1302_CH     = 7;

// Write Protect Register & Bits
const uint8_t DS1302_REG_WP = 0x8E; 
const uint8_t DS1302_WP = 7;

//DS1302 Register Addresses
const uint8_t DS1302_REG_TIMEDATE   = 0x80;
const uint8_t DS1302_REG_TIMEDATE_BURST = 0xBE;
const uint8_t DS1302_REG_TCR    = 0x90;
const uint8_t DS1302_REG_RAM_BURST = 0xFE;
const uint8_t DS1302_REG_RAMSTART   = 0xc0;
const uint8_t DS1302_REG_RAMEND     = 0xfd;
// ram read and write addresses are interleaved
const uint8_t DS1302RamSize = 31;

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
//
void write_ds1302(unsigned char cmd, unsigned char data) 
{
	RTC_RST=1;
	write_ds1302_byte(cmd);
	write_ds1302_byte(data);
	RTC_RST=0;
}
//
void write_ds1302_byte(unsigned char cmd) 
{
   unsigned char i;
   RTC_IO_DIR=0;//output//
   RTC_CLK=0;
   for(i=0;i<=7;++i) 
   {
   		RTC_IO=(cmd & 0x01);
   		cmd>>=1;
     	RTC_CLK=1;
     	Delay_Cnt(2);
        //__delay_ms(1);
     	RTC_CLK=0;
     	Delay_Cnt(2);
        //__delay_ms(1);
   }
}
//
unsigned char read_ds1302(unsigned char cmd) 
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
//
unsigned char get_bcd(unsigned char data)
{
   unsigned char nibh;
   unsigned char nibl;
   nibh=data/10;
   nibl=data-(nibh*10);
   return((nibh<<4)|nibl);
}
//
unsigned char rm_bcd(unsigned char data)
{
   unsigned char i;	
   i=data;
   data=(i>>4)*10;
   data=data+(i&0x0f);
   return data;
}
//
void rtc_set_datetime(unsigned char day, unsigned char mth, unsigned char year, unsigned char dow, unsigned char hour, unsigned char min) 
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

void Delay_Cnt(unsigned int cnt)
{
	unsigned int i;
	for(i=0;i<cnt;i++)
	{
		__asm__ volatile ("NOP");
	}
	//__asm__ volatile ("CLRWDT");		
}

unsigned char RtcDateTime_IsValid(void)
{
    // this just tests the most basic validity of the value ranges
    // and valid leap years
    // It does not check any time zone or daylight savings time
    if ((mth > 0 && mth < 13) &&
        (day > 0 && day < 32) &&
        (hour < 24) &&
        (min < 60) &&
        (sec < 60))
    {
        // days in a month tests
        //
        if (mth == 2)
        {
            if (day > 29)
            {
                return 0;
            }
            else if (day > 28)
            {
                // leap day
                // check year to make sure its a leap year
                unsigned int iyear = year;

                if ((iyear % 4) != 0)
                {
                    return 0;
                }

                if ((iyear % 100) == 0 &&
                    (iyear % 400) != 0)
                {
                    return 0;
                }
            }
        }
        else if (day == 31)
        {
            if ((((mth - 1) % 7) % 2) == 1)
            {
                return 0;
            }
        }

        return 1;
    }
    return 0;
}