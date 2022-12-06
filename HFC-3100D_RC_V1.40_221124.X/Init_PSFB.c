/*
*************************************************************************
* REVISION HISTORY: Draft                                               *
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
* Author : Martin Kim,   Date : 18.04.10,  revision: V1.0               *
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
* Seoul Electric Power System co.,Ltd First release of source					*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *
** ADDITIONAL NOTES:        										    *
*************************************************************************
 */
#include "p33FJ64GS606.h"
#include "dsp.h"
#include "Define_PSFB.h"
#include "Variables_PSFB.h"

void init_Conf(void)
{
    	unsigned char i = 0;
	//---------------------------------------------------------------		
	/* Configure Oscillator to operate the device at 40Mhz*/
	/* Fosc= Fin*M/(N1*N2), Fcy=Fosc/2	*/
	/* Fosc= 7.37M*43(2*2)=80Mhz for 7.37M input clock*/ //<=79.2275
	//---------------------------------------------------------------
    PLLFBD = 41;                    // M=43
    CLKDIVbits.PLLPOST = 0;			// N2=2
    CLKDIVbits.PLLPRE = 0;			// N1=2
    OSCTUN = 0;						// Tune FRC oscillator, if FRC is used
	//--------------------------
	/* Disable Watch Dog Timer*/
	//--------------------------
    RCONbits.SWDTEN = 0;
	//----------------------------------
	/* Clock switch to incorporate PLL*/
	//----------------------------------
    __builtin_write_OSCCONH(0x01);		// Initiate Clock Switch to FRC with PLL (NOSC=0b001)
    __builtin_write_OSCCONL(0x01);		// Start clock switching
    while (OSCCONbits.COSC != 0b001);	// Wait for Clock switch to occur	

	/* Wait for PLL to lock*/
    while(OSCCONbits.LOCK != 1) {};	   
	//-----------------------------------------------------------------------------------------------------	   
	/* Now setup the ADC and PWM clock for 120MHz; ((FRC * 16) / APSTSCLR ) = (7.37 * 16) / 1 = ~ 120MHz*/
	//-----------------------------------------------------------------------------------------------------
    ACLKCONbits.FRCSEL = 1;    			// FRC provides input for Auxiliary PLL (x16)
    ACLKCONbits.SELACLK = 1;   			// Auxiliary Oscillator provides clock source for PWM & ADC
    ACLKCONbits.APSTSCLR = 4;  			// Divide Auxiliary clock by 8
    ACLKCONbits.ENAPLL = 1;    			// Enable Auxiliary PLL 
    OSCTUNbits.TUN = 0x0005; 
    while(ACLKCONbits.APLLCK != 1);  	// Wait for Auxiliary PLL to Lock	
	//=================================================================================	 
	// SET PORT Direction // 1 is INPUT PORT//
	//=================================================================================
    TRISE = 0xFF0B;						//1111-1111-0000-1011 //bit6 output change 160615//
    TRISD = 0xF9FE;						//1111-1001-1111-1110 	
                                        //portD1,2,3,4,5,6,-dipsw	
                                        //D.8 igbt falut - Input port//
    TRISF = 0xFFBD;						//1111-1111-1011-1101  	//PortF.6 is Relay
        //
    TRISFbits.TRISF6 = 0;				//Output - Relay
    TRISGbits.TRISG9 = 0;				//Output - NFB_Trip
    TRISEbits.TRISE7 = 0;				//Output - Fan

	//---------------------------------------------------------------
    Nfb_Trip 	= 0;					//NFB Trip Output Off
    Relay 		= 0;					//Pre-charger Relay Off
	//---------------------------------------------------------------
    for(i=0;i<100;i++)
    {
        OutCurrent[i] = 0;
        BattCurrent[i] = 0;
    }	
}

void init_PSFBDrive(void)
	{
		PTPER 	= PWMPeriod;			//PWMPeriod		((PWMClockFreq/PWMSwitchingFreq) - 8) 
										// 7858 - 8  =>7850
		PDC1 	= HALFPERIOD ;			//(int)(PWMPeriod/2)
		PDC2 	= HALFPERIOD ;			// 3925
		
	/*Initialization of the PWM for the left leg*/
		
		IOCON1bits.PENH = 1;			// PWM module controls PWMxH pin
		IOCON1bits.PENL = 1;			// PWM module controls PWMxL pin
		IOCON1bits.POLH = 0;			// PWM Output Polarity is high
		IOCON1bits.POLL = 0;			// PWM Output Polarity is high
		IOCON1bits.PMOD = 0;			// PWM I/O pin pair is in the Complementary Output mode

		PWMCON1bits.ITB = 0;			// PTPER register provides timing for this PWM generator
		PWMCON1bits.MDCS = 0;			// Master time base will provide duty cycle
		
		PWMCON1bits.DTC=0;				// page15		
		DTR1    = Deadtime; 			// Dead time setting 
	  	ALTDTR1 = ALTDeadtime; 			// Dead time setting
		//-----------------------------------------------------------------
		PWMCON2bits.TRGIEN = 1;			// trigger interrupt enable//
		TRGCON2bits.TRGDIV = 1;			//org. trigger # output divider  1 is 2nd trigger event-120k/2=~60khz
		//TRGCON2bits.TRGDIV = 0;			//º¯°æ
		TRIG2= trigger;					// PWMPeriod*0.95(95%)
		
	/*Initialization of the PWM for the right leg*/
		
		IOCON2bits.PENH = 1;			// PWM module controls PWMxH pin
		IOCON2bits.PENL = 1;			// PWM module controls PWMxL pin
		IOCON2bits.POLH = 0;			// PWM Output Polarity is high
		IOCON2bits.POLL = 0;			// PWM Output Polarity is high
		IOCON2bits.PMOD = 0;			// PWM I/O pin pair is in the Complementary Output mode		
	
		PWMCON2bits.ITB = 0;			// PTPER register provides timing for this PWM generator
		PWMCON2bits.MDCS = 0;			// Master time base will provide duty cycle		

		PWMCON1bits.DTC=0;				// page15
		DTR2    = Deadtime; 			// Deadtime setting 
	 	ALTDTR2 = ALTDeadtime; 			// Deadtime setting
	   	
    /* FLT1 associated with CMP2 and FLT2 associated with CMP4 */
	   	
	  	FCLCON1bits.FLTSRC = 1;			// Fault 2 has been selected for the Fault control signal source for PWM Generator1
		FCLCON2bits.FLTSRC = 1;			// Fault 2 has been selected for the Fault control signal source for PWM Generator2
		FCLCON3bits.FLTSRC = 1;			// Fault 2 has been selected for the Fault control signal source for PWM Generator3
		
		FCLCON1bits.CLSRC = 0;			// Fault 1 has been selected for current limit control signal source for PWM Generator1
		FCLCON2bits.CLSRC = 0;			// Fault 1 has been selected for current limit control signal source for PWM Generator2
		FCLCON3bits.CLSRC = 0;			// Fault 1 has been selected for current limit control signal source for PWM Generator3
				
// Over voltage
//		FCLCON1bits.CLMOD=1;			//  Cycle by cycle mode selected
//		FCLCON2bits.CLMOD=1;			//  Cycle by cycle mode selected
//		FCLCON3bits.CLMOD=1;			//  Cycle by cycle mode selected		
	   		
	  	FCLCON1bits.FLTMOD=3;			// Latch mode is enabled
		FCLCON2bits.FLTMOD=3;			// Latch mode is enabled
		FCLCON3bits.FLTMOD=3;			// Latch mode is enabled
		
		IOCON1bits.OVRDAT = 0;			// State for PWMxH and PWMxL Pins is low if Override is Enabled
		IOCON2bits.OVRDAT = 0;			// State for PWMxH and PWMxL Pins is low if Override is Enabled
		IOCON3bits.OVRDAT = 0;			// State for PWMxH and PWMxL Pins is low if Override is Enabled
	}

void init_ADC(void)   	
	{
/* AN1 is for output voltage measurment and AN0 is for PSFB PRY Current */
		
		ADSTATbits.P0RDY =  0;			// Pair 0 data ready bit cleared before use   
		ADSTATbits.P1RDY =  0;			// Pair 1 data ready bit cleared before use 	
				//
		ADPCFGbits.PCFG0 = 0;  		// configured as analog input, AN0 for the PSFBCurrent measurment
		ADPCFGbits.PCFG1 = 0; 		// configured as analog input, AN1 for output voltage measurment
				//
		ADPCFGbits.PCFG2 = 0;  		// configured as analog input, AN2 for temp. measurment 
		ADPCFGbits.PCFG3 = 0;  		// configured as analog input, AN3 for the batt.Current measurment
				//
		ADCPC0bits.IRQEN0 = 1; 			// IRQ is  generated
		ADCPC0bits.IRQEN1 = 1; 			// IRQ is  generated
				//
		ADCPC0bits.TRGSRC0 = 5; 		// Choose PWM2 as the trigger source
		ADCPC0bits.TRGSRC1 = 5; 		// Choose PWM2 as the trigger source
		
		//==========================================================================================
		ADSTATbits.P2RDY =  0;			// Pair 3//
		
		ADPCFGbits.PCFG4 = 0;  		// configured as analog input, AN4 for V_Bus
		ADPCFGbits.PCFG5 = 0;  		// configured as analog input, AN5 for LS_IN
		
		ADCPC1bits.IRQEN2 = 1; 			// Pair 3(an4,an5)IRQ is  generated
		
		ADCPC1bits.TRGSRC2 = 5; 		// Choose PWM2 as the trigger source
		//==========================================================================================
		
		ADSTATbits.P7RDY =  0;			// Pair 7//
				//
		ADPCFGbits.PCFG14 = 0;  		// configured as analog input, AN12 for 
		ADPCFGbits.PCFG15 = 0;  		// 
				//
		ADCPC3bits.IRQEN7 = 1; 			// Pair 6(an12,an13)IRQ is  generated
				//
		ADCPC3bits.TRGSRC7 = 5; 		// Choose PWM2 as the trigger source
		//==========================================================================================		
		// A/D Control Register
	   	ADCONbits.SLOWCLK = 1;			// ADC is clocked by the auxiliary PLL (ACLK)
	   	ADCONbits.FORM = 0;				// Data format is integer
	   	ADCONbits.EIE = 0;				// Interrupt is generated after both channels conversion is completed
	   	ADCONbits.ORDER = 1;			// Odd numbered analog input is converted first, followed by conversion of even numbered input
	   	ADCONbits.SEQSAMP = 0;			// Sequential Sample Enable
	   	ADCONbits.ASYNCSAMP = 0;  		// The dedicated S&H circuit starts sampling    			
										// 		when the trigger event is detected and completes the	      
										// 		sampling process in two ADC clock cycles 
		ADCONbits.ADCS = 4;				// FRC = 7.37MHz; ACLK = FRC * M/N     (N = APSTSCLR) = 7.37 *16 / 1 = 117.92Mhz
								      	// Sample and conversion time per pair=  2TAD + 14 TAD =16TAD; TAD = ADC clock period
								      	//  since two SARs are used 14MSPS is; 125ns(8Mhz) =>
		//=======================//
		/* Set up the Interrupts */
		//=======================//
	 	IPC3bits.ADIP = 7;					// Interrupt is priority 7 (highest priority interrupt)					      
     	//	IFS0bits.ADIF = 0;		    			// Clear ADC interrupt flag 					      
	 	//	IEC0bits.ADIE = 1;					// ADC Conversion global Interrupt request Enabled
	 
	   	_ADCP1IF = 0;					// ADC Pair 1 Conversion Done Interrupt Flag Status bit is cleared
	   	_ADCP1IE = 1;					// ADC Pair 1 Conversion Done Interrupt Enable bit is cleared  	
	   	
	   	//_ADCP3IF = 0;					// ADC Pair 1 Conversion Done Interrupt Flag Status bit is cleared
	   	//_ADCP3IE = 1;
	   	
	   	//_ADCP6IF = 0;					// ADC Pair 1 Conversion Done Interrupt Flag Status bit is cleared
	   	//_ADCP6IE = 1;   	
	}

void init_Timer1_2(void)	
{
	//timer 1//	
	T1CON = 0x0000;
	//PR1=400;						//prev value 792
	PR1 = 780;						//40MHZ / 780 = 51kHz (19.6us)
	_T1IP = 4;
	_T1IF = 0;
	_T1IE = 1;
	T1CONbits.TON = 1;
	
	//timer 2//=====================================================================
	T2CON = 0x0000;
	PR2 = 40000;						//1ms	
	_T2IP = 4;
	_T2IF = 0;
	_T2IE = 1;
	T2CONbits.TON = 1;    
}

void init_Uart1(void)		//RS485//
{
	TRISFbits.TRISF2 = 1;		// I/O Setting UART RX 
    TRISFbits.TRISF3 = 0;		// I/O Setting UART TX 	
	TRISDbits.TRISD11 = 0;		// I/O Setting UART RE/DE Control
	
	U1MODEbits.STSEL=0;	//1 stop bit
	U1MODEbits.PDSEL=0;	//no parity, 8 data bits
	U1MODEbits.ABAUD=0;	//disabled Auto-Baud
	U1MODEbits.BRGH=1;	//High-Speed Mode -KHJ//Low-speed mode
	
	U1BRG = BRGVAL;
	
	U1STAbits.URXISEL=0;	//Interrupt after one RX character is received;
	U1STAbits.UTXISEL0=0;
	U1STAbits.UTXISEL1=0;
	//IEC1bits.U2TXIE=1;		//Enable Transmit Interrupt
	IEC0bits.U1RXIE=1;			//Enable Receive Interrupt
    IFS0bits.U1RXIF=0;			//Clear Receive flag 
	
	U1MODEbits.UARTEN=1;	//Enable UART
	U1STAbits.UTXEN=1;		//Enable Transmit
    RS485_CTRL = 0;
}

void init_Uart2(void)		//RS232//
{
	TRISFbits.TRISF5 = 0;		// I/O Setting UART TX
	TRISFbits.TRISF4 = 1;		// I/O Setting UART RX
	
	U2MODEbits.STSEL=0;	//1 stop bit
	U2MODEbits.PDSEL=0;	//no parity, 8 data bits
	U2MODEbits.ABAUD=0;	//disabled Auto-Baud
	U2MODEbits.BRGH =1;	//High-Speed Mode -KHJ//Low-speed mode
	
	U2BRG = BRGVAL;       //BRGVAL
	
	U2STAbits.URXISEL=0;	//Interrupt after one RX character is received;
	U2STAbits.UTXISEL0=0;
	U2STAbits.UTXISEL1=0;
	//IEC1bits.U2TXIE=1;		//Enable Transmit Interrupt
	IEC1bits.U2RXIE=1;			//Enable Receive Interrupt
    IFS1bits.U2RXIF=0;			//Clear Receive flag 
	
	U2MODEbits.UARTEN=1;	//Enable UART
	U2STAbits.UTXEN=1;		//Enable Transmit
}

//-----------------------------------------------------------//
//            ID Indent For Initializing - read dip sw       //
//-----------------------------------------------------------//
void GetDipSW()
{
	unsigned char swvalue = 0, swvalue2 = 0; 	//active high//
	if(_RD1==0)swvalue = 1;		
	if(_RD2==0)swvalue |= 2;
	if(_RD3==0)swvalue |= 4;
	if(_RD4==0)swvalue |= 8;
	//		
	if(_RD1==0)swvalue2 = 1;
	if(_RD2==0)swvalue2 |= 2;
	if(_RD3==0)swvalue2 |= 4;
	if(_RD4==0)swvalue2 |= 8;
	//
	if(swvalue == swvalue2)
	{
		if(swvalue>0 && swvalue<9)
		{
			ID485 = swvalue;
		}
		else	ID485 = 0;
				
	}
	else
	{
		ID485 = 9;
	}		
}

/******************************************************************************
*                                                                             
*    Function:			initCAN
*    Description:       Initialises the ECAN module                                                        
*                                                                             
*    Arguments:			none 
*	 Author:            Jatinder Gharoo                                                      
*	                                                                 
*                                                                              
******************************************************************************/
void initECAN (void)
{
	//unsigned long temp;
	//unsigned int tempint;
	
	/* put the module in configuration mode */
	C1CTRL1bits.REQOP=4;		//4
	while(C1CTRL1bits.OPMODE != 4);//
			
	/* FCAN is selected to be FCY
    FCAN = FCY = 40MHz */
	//C1CTRL1bits.CANCKS = 0x1;

	/*
	Bit Time = (Sync Segment + Propagation Delay + Phase Segment 1 + Phase Segment 2)=20*TQ
	Phase Segment 1 = 8TQ
	Phase Segment 2 = 6Tq
	Propagation Delay = 5Tq
	Sync Segment = 1TQ
	CiCFG1<BRP> =(FCAN /(2 ??BAUD))?1
	BIT RATE OF 1Mbps
	*/	
	C1CFG1bits.BRP = BRP_VAL;
	/* Synchronization Jump Width set to 4 TQ */
	C1CFG1bits.SJW = 0x3;
	/* Phase Segment 1 time is 8 TQ */
	C1CFG2bits.SEG1PH=0x7;
	/* Phase Segment 2 time is set to be programmable */
	C1CFG2bits.SEG2PHTS = 0x1;
	/* Phase Segment 2 time is 6 TQ */
	C1CFG2bits.SEG2PH = 0x5;
	/* Propagation Segment time is 5 TQ */
	C1CFG2bits.PRSEG = 0x4;
	/* Bus line is sampled three times at the sample point */
	C1CFG2bits.SAM = 0x1;
	
	/* 4 CAN Messages to be buffered in DMA RAM */	
	C1FCTRLbits.DMABS=0b000;
//-----------------------------------------------------------------------
	/* Filter configuration */
	/* enable window to access the filter configuration registers */
	C1CTRL1bits.WIN=0b1;
	//====================================================

	/* select acceptance mask 0 filter 0 buffer 1 */
	/* configure accpetence mask 0 - match the id in filter 0 
	setup the mask to check every bit of the standard message, 
	the macro when called as CAN_FILTERMASK2REG_SID(0x7FF) will 
	write the register C1RXM0SID to include every bit in filter comparison 
	*/ 	
			C1FMSKSEL1bits.F0MSK=0; //C1FMSKSEL1bit.F0~F7,C2FMSKSEL1bit.F8~F15)
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);		//C1RXMnSID - n is 0~2)
			C1RXF0SID=CAN_FILTERMASK2REG_SID(0x001);		//C1RXFnSID - n is 0~15)
	//-----------------------------------------------------------------------
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);				//C1RXMnSID - n is 0~2)
			C1RXF0SID=CAN_FILTERSTD(C1RXF0SID);				//C1RXFnSID - n is 0~15)
	/* acceptance filter to use buffer 1 for incoming messages */
			C1BUFPNT1bits.F0BP=0b0000;						//C1BUFPNT1bits.FnBP - n is 0~4)
	/* enable filter 0 */
			C1FEN1bits.FLTEN0=1;							//(FLTENn - n is 0~15)
//======================================================================	
			C1FMSKSEL1bits.F1MSK=0b01;	//C1FMSKSEL1bit.F0~F7,C2FMSKSEL1bit.F8~F15)
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);	//C1RXMnSID - n is 0~2)
			C1RXF1SID=CAN_FILTERMASK2REG_SID(0x002);
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);
			C1RXF1SID=CAN_FILTERSTD(C1RXF1SID);	
			C1BUFPNT1bits.F1BP=0b0001;
	/* enable filter 1 */
				C1FEN1bits.FLTEN1=1;
//=============================================================================	
	/* select acceptance mask 1 filter 2 and buffer 3 */
			C1FMSKSEL1bits.F2MSK=0b01;	//C1FMSKSEL1bit.F0~F7,C2FMSKSEL1bit.F8~F15)
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);	//C1RXMnSID - n is 0~2)
			C1RXF2SID=CAN_FILTERMASK2REG_SID(0x003); //C1RXF
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);
			C1RXF2SID=CAN_FILTERSTD(C1RXF2SID);	
	C1BUFPNT1bits.F2BP=0b0010;
	/* enable filter 2 */
				C1FEN1bits.FLTEN2=1; //(FLTENx - x - is 0~15)
//=============================================================================
	/* select acceptance mask 1 filter 1 and buffer 2 */
			C1FMSKSEL1bits.F3MSK=0b01;	//C1FMSKSEL1bit.F0~F7,C2FMSKSEL1bit.F8~F15)
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);		//C1RXMnSID - n is 0~2)
			C1RXF3SID=CAN_FILTERMASK2REG_SID(0x004);		//C1RXFnSID - n is 0~15)
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);				//C1RXMnSID - n is 0~2)
			C1RXF3SID=CAN_FILTERSTD(C1RXF3SID);				//C1RXFnSID - n is 0~15)
	/* acceptance filter to use buffer 1 for incoming messages */
	C1BUFPNT1bits.F3BP=0b0011;						//C1BUFPNT1bits.FnBP - n is 0~3)
	/* enable filter 0 */
				C1FEN1bits.FLTEN3=1;							//(FLTENn - n is 0~15)
//=============================================================================	         
/* select acceptance mask 1 filter 3 and buffer 3 */
			C1FMSKSEL1bits.F4MSK=0b01;	
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);		//C1RXMnSID - n is 0~2)
			C1RXF4SID=CAN_FILTERMASK2REG_SID(0x005);		//C1RXFnSID - n is 0~15)
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);				//C1RXMnSID - n is 0~2)
			C1RXF4SID=CAN_FILTERSTD(C1RXF4SID);				
	C1BUFPNT2bits.F4BP=0b0100;						//C1BUFPNT2bits.FnBP - n is 4~7)	
	/* enable filter 2 */
				C1FEN1bits.FLTEN4=1;
//=============================================================================	         
/* select acceptance mask 1 filter 3 and buffer 3 */
			C1FMSKSEL1bits.F5MSK=0b01;	
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);		//C1RXMnSID - n is 0~2)
			C1RXF5SID=CAN_FILTERMASK2REG_SID(0x006);		//C1RXFnSID - n is 0~15)
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);				//C1RXMnSID - n is 0~2)
			C1RXF5SID=CAN_FILTERSTD(C1RXF5SID);				
	C1BUFPNT2bits.F4BP=0b0101;						//C1BUFPNT2bits.FnBP - n is 4~7)	
	/* enable filter 2 */
				C1FEN1bits.FLTEN5=1;
//=============================================================================	         
/* select acceptance mask 1 filter 3 and buffer 3 */
			C1FMSKSEL1bits.F6MSK=0b01;	
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);		//C1RXMnSID - n is 0~2)
			C1RXF6SID=CAN_FILTERMASK2REG_SID(0x007);		//C1RXFnSID - n is 0~15)
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);				//C1RXMnSID - n is 0~2)
			C1RXF6SID=CAN_FILTERSTD(C1RXF6SID);				
	C1BUFPNT2bits.F4BP=0b0110;						//C1BUFPNT2bits.FnBP - n is 4~7)	
	/* enable filter 2 */
				C1FEN1bits.FLTEN6=1;
//=============================================================================	         
/* select acceptance mask 1 filter 3 and buffer 3 */
			C1FMSKSEL1bits.F7MSK=0b01;	
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);		//C1RXMnSID - n is 0~2)
			C1RXF7SID=CAN_FILTERMASK2REG_SID(0x008);		//C1RXFnSID - n is 0~15)
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);				//C1RXMnSID - n is 0~2)
			C1RXF7SID=CAN_FILTERSTD(C1RXF7SID);				
	C1BUFPNT2bits.F4BP=0b0111;						//C1BUFPNT2bits.FnBP - n is 4~7)	
	/* enable filter 2 */
				C1FEN1bits.FLTEN7=1;
//=============================================================================	         
/* select acceptance mask 1 filter 3 and buffer 3 */
			C1FMSKSEL2bits.F8MSK=0b01;	
			C1RXM0SID=CAN_FILTERMASK2REG_SID(0x7FF);		//C1RXMnSID - n is 0~2)
			C1RXF8SID=CAN_FILTERMASK2REG_SID(0x00A);		//C1RXFnSID - n is 0~15)
	/* set filter to check for standard ID and accept standard id only */
			C1RXM0SID=CAN_SETMIDE(C1RXM0SID);				//C1RXMnSID - n is 0~2)
			C1RXF8SID=CAN_FILTERSTD(C1RXF8SID);				
	C1BUFPNT2bits.F4BP=0b1000;						//C1BUFPNT2bits.FnBP - n is 4~7)	
	/* enable filter 2 */
				C1FEN1bits.FLTEN8=1;
//=============================================================================	         

	/* clear window bit to access ECAN control registers */
	C1CTRL1bits.WIN=0;
		
	/* put the module in normal mode */
	C1CTRL1bits.REQOP=0;
	while(C1CTRL1bits.OPMODE != 0);	
	
	/* clear the buffer and overflow flags */
	C1RXFUL1=C1RXFUL2=C1RXOVF1=C1RXOVF2=0x0000;
	/* ECAN1, Buffer 0 is a Transmit Buffer */
	C1TR01CONbits.TXEN0=1;			
	/* ECAN1, Buffer 1 is a Receive Buffer */
	C1TR01CONbits.TXEN1=0;	
	/* ECAN1, Buffer 2 is a Receive Buffer */
	C1TR23CONbits.TXEN2=0;	
	/* ECAN1, Buffer 3 is a Receive Buffer */
	C1TR23CONbits.TXEN3=0;	
	/* Message Buffer 0 Priority Level */
	C1TR01CONbits.TX0PRI=0b11; 		
		
	/* configure the device to interrupt on the receive buffer full flag */
	/* clear the buffer full flags */
	C1RXFUL1=0;
	C1INTFbits.RBIF=0;
}

/******************************************************************************
*                                                                             
*    Function:			initDMAECAN
*    Description:       Initialises the DMA to be used with ECAN module                                                        
*                       Channel 0 of the DMA is configured to Tx ECAN messages
* 						of ECAN module 1. 
*						Channel 2 is uconfigured to Rx ECAN messages of module 1.                                                      
*    Arguments:			
*	 Author:            Jatinder Gharoo                                                      
*	                                                                 
*                                                                              
******************************************************************************/
void initDMAECAN(void)
{
	/* initialise the DMA channel 0 for ECAN Tx */
	/* clear the collission flags */
	DMACS0=0;	
    /* setup channel 0 for peripheral indirect addressing mode 
    normal operation, word operation and select as Tx to peripheral */
    DMA0CON=0x2020; 
    /* setup the address of the peripheral ECAN1 (C1TXD) */ 
	DMA0PAD=0x0642;		//
	/* Set the data block transfer size of 8 */
 	DMA0CNT=7;
 	/* automatic DMA Tx initiation by DMA request */
	DMA0REQ=0x0046;	
	/* DPSRAM atart adddress offset value */ 
	DMA0STA=__builtin_dmaoffset(&ecan1msgBuf);	
	/* enable the channel */
	DMA0CONbits.CHEN=1;
	
	/* initialise the DMA channel 2 for ECAN Rx */
	/* clear the collission flags */
	DMACS0=0;
    /* setup channel 2 for peripheral indirect addressing mode 
    normal operation, word operation and select as Rx to peripheral */
    DMA2CON=0x0020;
    /* setup the address of the peripheral ECAN1 (C1RXD) */ 
	DMA2PAD=0x0640;	
 	/* Set the data block transfer size of 8 */
 	DMA2CNT=7;
 	/* automatic DMA Rx initiation by DMA request */
	DMA2REQ=0x0022;	
	/* DPSRAM atart adddress offset value */ 
	DMA2STA=__builtin_dmaoffset(&ecan1msgBuf);	
	/* enable the channel */
	DMA2CONbits.CHEN=1;
}	 


//------------------------//
// initialize CAN modules //
//------------------------//	
void init_CAN(void)		
{
    initECAN();
    initDMAECAN();
    IEC2bits.C1IE=1;							/* Enable ECAN1 Interrupt */	
    C1INTEbits.TBIE=0;							/* disable Transmit interrupt */	
    C1INTEbits.RBIE=1;							/* Enable Receive interrupt */
}

