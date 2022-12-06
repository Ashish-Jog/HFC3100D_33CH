/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
//#ifndef __DS1302_H
//#define	__DS1302_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define RTC_RST     (_LATD11)
#define RTC_CLK     (_LATC9)
#define RTC_IO_DIR  (_TRISC8)
#define RTC_IO      (_LATC8)
#define RTC_IO_PIN  (_RC8)
void rtc_init(void); 
void write_ds1302(unsigned char cmd, unsigned char data);
void write_ds1302_byte(unsigned char cmd);
unsigned char read_ds1302(unsigned char cmd);
unsigned char get_bcd(unsigned char data);
unsigned char rm_bcd(unsigned char data);
void rtc_set_datetime(unsigned char day, unsigned char mth, unsigned char year, unsigned char dow, unsigned char hour, unsigned char min);
void rtc_get_date();
void rtc_get_time();
void Delay_Cnt(unsigned int cnt);
unsigned char RtcDateTime_IsValid(void);
