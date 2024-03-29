/*********************************************************************
 *
 *                    Simple Delay Routines 
 *
 *********************************************************************
 * FileName:        delay.c
 * Dependencies:    delay.h
 * Processor:       dsPIC33F
 * Complier:        MPLAB C30 v2.01.00 or higher
 *
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the �Company�) for its dsPIC30F Microcontroller is intended 
 * and supplied to you, the Company�s customer, for use solely and
 * exclusively on Microchip's dsPIC30F Microcontroller products. 
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Martin Kim          18/04/10     Draft
 ********************************************************************/

#include "delay.h"

unsigned int temp_count;

#if defined(__dsPIC33F__)

void Delay( unsigned int delay_count ) 
{
	temp_count = delay_count +1;
	asm volatile("outer: dec _temp_count");	
	asm volatile("cp0 _temp_count");
	asm volatile("bra z, done");
	asm volatile("do #3200, inner" );	
	asm volatile("nop");
	asm volatile("inner: nop");
	asm volatile("bra outer");
	asm volatile("done:");
}
	

void Delay_Us( unsigned int delayUs_count )
{
	temp_count = delayUs_count +1;
	asm volatile("outer1: dec _temp_count");	
	asm volatile("cp0 _temp_count");
	asm volatile("bra z, done1");
	asm volatile("do #1500, inner1" );	
	asm volatile("nop");
	asm volatile("inner1: nop");
	asm volatile("bra outer1");
	asm volatile("done1:");
}		
#elif defined(__PIC24H__)
void Delay( unsigned int delay_count ) 
{
	temp_count = delay_count +1;
	asm volatile("outer: dec _temp_count");	
	asm volatile("cp0 _temp_count");
	asm volatile("bra z, done");
	asm volatile("repeat #3200" );	
	asm volatile("nop");
	asm volatile("repeat #3200" );	
	asm volatile("nop");
	asm volatile("bra outer");
	asm volatile("done:");
}
	

void Delay_Us( unsigned int delayUs_count )
{
	temp_count = delayUs_count +1;
	asm volatile("outer1: dec _temp_count");	
	asm volatile("cp0 _temp_count");
	asm volatile("bra z, done1");
	asm volatile("repeat #1500" );	
	asm volatile("nop");
	asm volatile("repeat #1500" );	
	asm volatile("nop");
	asm volatile("bra outer1");
	asm volatile("done1:");
}

#endif
