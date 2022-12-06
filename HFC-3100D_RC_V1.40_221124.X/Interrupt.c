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

extern void Fault_Check(void);                          //Checking Fault
extern void Control_Loop(void);
extern long mapping(long x, long inMin, long inMax, long outMin, long outMax);
extern void ModuleOnOff(void);                         //ON/OFF

////////////////////////////////////////////////////////////////////////////////
//--------------------- IGBT Fault : Cause DESAT -----------------------------//
////////////////////////////////////////////////////////////////////////////////
void __attribute__((__interrupt__, no_auto_psv)) _INT1Interrupt()  // 
{
	//===========   
	_INT1IF = 0 ;               // Clear interrupt flag
	//=========== 
//    TDI_count++;
//    if(TDI_count >= 100)
    {
        FLT_Flag = 1;            // Fault Status
        TDI_count = 100;
    }
	INTCON2bits.INT1EP = 0;		// External Interrupt 1 Edge Detect Polarity Select bit; 0 positive, 1 negative	
}

//=========================================================================//
// 	Timer1 interrupt to take care of the live volatge variations 		   //	
//	    to improve the transient response and 							   //
//	    also compensate the transformer current for the voltage variations //
//=========================================================================//
void __attribute__((__interrupt__, no_auto_psv)) _T1Interrupt()  
// This is to get the transient response same at all the line voltages.
{	
	//interval is 20us //19.6us//
	if(softstart == 0)	 									// during the normal operation of the converter accumulate the PIOutput
	{
		SumPIOutput = SumPIOutput + PIOutput ;              // to get the average value of the PI output
		Sumcount++;	
	}

	if(Sumcount > 128)                                      // 	Averaging the PIoutput for the variation of inputs 	
	{
        AveragePIOutput = (SumPIOutput>>7);                 // 	divide by 512 to get the average; averaging after every 10mSec
		Sumcount = 0;
		SumPIOutput = 0;

		if(AveragePIOutput  < Q15(0.501) )                  //  saturating the Average PI output 
		AveragePIOutput = Q15(0.501);
		modifier = __builtin_divsd(((long)32767<<14),AveragePIOutput);		// Max value of modifier will be 2Xaverage PI output	
	}
	//--------
	_T1IF = 0;
	//--------
} 

////////////////////////////////////////////////////////////////////////////////
// 	Timer2 interrupt                                                          //	
//	Interval 10us                                                            //
////////////////////////////////////////////////////////////////////////////////
void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt()  
{	
    tmrtick1++;
    tmrtick2++;	
    tmrtick3++;	
    tmrtick4++;	//@used main loop
    
    // Start Timer
    if(softstart == 0)
    {
       tmrtick5++; 
    }
    else
    {
        tmrtick5 = 0;
    }
    
    tmrtick6++;
    tmrtick7++;
    
    
    // This is to get the transient response same at all the line voltages.
    if(tmrtick1 > 200)
    {
        tmrtick1 = 200;
    }
    if(tmrtick2 > 200)
    {
        tmrtick2 = 200;
    }    
    if(tmrtick3 > 200)
    {
        tmrtick3 = 200;
    }   
    if(tmrtick5 > 61000)
    {
        tmrtick5 = 61000;
    }  
    if(tmrtick6 > 60000)
    {
        tmrtick6 = 60000;
    }  
    if(tmrtick7 > 100)
    {
        tmrtick7 = 100;
    }  
	_T2IF = 0;						//100ms //clear Interrupt flag//
}//_T2Interrupt


//=====================================================================================
/* ADC Interrupt Service Routine to execute the PI compensator and faults execution */
//=====================================================================================
void __attribute__((__interrupt__, no_auto_psv)) _ADCP1Interrupt()  
{ //소요시간 42us-주기 10.0khz ?? 100us
	long 	ltmp = 0;	
	int		tmpvalue = 0;       //Output Voltage ADC
	int 	tmpvalue2 = 0;      //Bus Voltage ADC
	int		tmpvalue3 = 0;      //VR Voltage ADC
	int		tmpvalue4 = 0;      //VR Current ADC
	int		tmpvalue5 = 0;      //Output Current
    int		tmpvalue6 = 0;      //Temperature
    int     tmpvalue7 = 0;      //Batt Current
	int 	OutAmpTmp = 0;
    int     BattAmpTmp = 0;

	//============================
	// Output Gain Select	
	//============================
	if( (gain_OutV>1500)|| (gain_OutV<500) )gain_OutV=1000;
	if( (gain_OutI>1500)|| (gain_OutI<500) )gain_OutI=1000;
	if( (gain_VBus>1500)|| (gain_VBus<500) )gain_VBus=1000;
    if( (gain_Temp>1500)|| (gain_Temp<500) )gain_Temp=1000;
    if( (gain_BattI>1500)|| (gain_BattI<500) )gain_BattI=1000;
    
	//============================
	// Output Voltage ADC
	//============================	
	if(OutputVoltage>1020)
	{
		tmpvalue=1000;
	}
	else tmpvalue = sdiv((long)OutputVoltage*gain_OutV,1000);
//    tmpvalue = (Cutoff_Freq * Pre_tmpvalue + tmpvalue) / (Cutoff_Freq + 1);
//    Pre_tmpvalue = tmpvalue;
    
    //============================
	// Bus Voltage ADC
	//============================	
	if(VBus>1020)
	{
		tmpvalue2=1000;
	}		
	else tmpvalue2= sdiv((long)VBus*gain_VBus,1000);
//    tmpvalue2 = (Cutoff_Freq * Pre_tmpvalue2 + tmpvalue2) / (Cutoff_Freq + 1);
//    Pre_tmpvalue2 = tmpvalue2;
    
    //============================
	// VR Voltage ADC
	//============================	
	tmpvalue3 = VAdj;
    if(VAdj>1020)
	{
		tmpvalue3=1000;
	}		
	else tmpvalue3 = sdiv((long)VAdj*gain_VBus,1000);
//    tmpvalue3 = (Cutoff_Freq * Pre_tmpvalue3 + tmpvalue3) / (Cutoff_Freq + 1);
//    Pre_tmpvalue3 = tmpvalue3;
    
    //============================
	// VR Current ADC
	//============================	
	tmpvalue4 = AAdj;
    if(AAdj>1020)
	{
		tmpvalue4=1000;
	}		
	else tmpvalue4 = sdiv((long)AAdj*gain_VBus,1000);
//    tmpvalue4 = (Cutoff_Freq * Pre_tmpvalue4 + tmpvalue4) / (Cutoff_Freq + 1);
//    Pre_tmpvalue4 = tmpvalue4;
    
	//============================		
	//Output Current
	//============================
	OutAmpTmp=PSFBCurrent;	
	//
	if(OutAmpTmp>1020)
	{
		OutAmpTmp=1000;
	}							
	tmpvalue5=sdiv((long)OutAmpTmp * gain_OutI,1000);//OutCurrent index is 0~99 //
    
    //============================		
    //Temperature
    //============================
    if(Temperature>1020)
    {
        Temperature=1000;
    }							
    else tmpvalue6=sdiv((long)Temperature * gain_Temp,1000);//Temperature
	
    //============================		
	//Batt Current
	//============================
	BattAmpTmp=BatteryCurrent;	
	//
	if(BattAmpTmp>1020)
	{
		BattAmpTmp=1000;
	}							
	tmpvalue7=sdiv((long)BattAmpTmp * gain_BattI,1000);//BattCurrent index is 0~99 //
    
	//-----------------------------------------------------			
	if(fSaveDone == 0)		//first save buffer//
	{
		OutVolt[ovCnt]		= tmpvalue;
		VBusVolt[ovCnt]		= tmpvalue2;
		VAdjVolt[ovCnt]		= tmpvalue3;
		AAdjVolt[ovCnt]		= tmpvalue4;
		OutCurrent[ovCnt]	= tmpvalue5;
        Temp[ovCnt]			= tmpvalue6;
        BattCurrent[ovCnt]	= tmpvalue7;
		//
		sumOutVolt 	=	sumOutVolt		+ 	OutVolt[ovCnt];
		sumVBusVolt	=	sumVBusVolt		+ 	VBusVolt[ovCnt];
		sumVAdjVolt	=	sumVAdjVolt		+	VAdjVolt[ovCnt];
		sumAAdjVolt	=	sumAAdjVolt		+	AAdjVolt[ovCnt];
		sumOutCurrent = sumOutCurrent 	+ 	OutCurrent[ovCnt];
        sumTemp     =   sumTemp         +   Temp[ovCnt];
        sumBattCurrent = sumBattCurrent	+ 	BattCurrent[ovCnt];
		if(ovCnt >= 31)
		{			
			VoltFnd 	= (int)(sumOutVolt>>5);                 // fnd display//
			VBusFnd 	= (int)(sumVBusVolt>>5);
			VAdjFnd 	= (int)(sumVAdjVolt>>5);
			AAdjFnd 	= (int)(sumAAdjVolt>>5);				
			AmpFnd		= (int)(sumOutCurrent>>5);
            TempFnd		= (int)(sumTemp>>5);
            BattAmpFnd	= (int)(sumBattCurrent>>5);
			fSaveDone 	= 1;
		}
	}
	else if(fSaveDone == 1)
	{
		sumOutVolt 	=	sumOutVolt	-	OutVolt[ovCnt]	+ tmpvalue;
			OutVolt[ovCnt]=tmpvalue;
				
		sumVBusVolt	=	sumVBusVolt	-	VBusVolt[ovCnt]	+ tmpvalue2;
			VBusVolt[ovCnt]=tmpvalue2;
				
		sumVAdjVolt	=	sumVAdjVolt	-	VAdjVolt[ovCnt]	+ tmpvalue3;
			VAdjVolt[ovCnt]=tmpvalue3;
				
		sumAAdjVolt	=	sumAAdjVolt	-	AAdjVolt[ovCnt]	+ tmpvalue4;
			AAdjVolt[ovCnt]=tmpvalue4;						
				
		sumOutCurrent =	sumOutCurrent -	OutCurrent[ovCnt]	+ tmpvalue5;
			OutCurrent[ovCnt]=tmpvalue5;
            
        sumTemp = sumTemp - Temp[ovCnt]	+ tmpvalue6;
			Temp[ovCnt]=tmpvalue6;
            
        sumBattCurrent =	sumBattCurrent - BattCurrent[ovCnt]	+ tmpvalue7;
        BattCurrent[ovCnt]=tmpvalue7;
			//
		VoltFnd = (int)(sumOutVolt>>5);             // fnd display//
		VBusFnd = (int)(sumVBusVolt>>5);
		VAdjFnd = (int)(sumVAdjVolt>>5);
		AAdjFnd = (int)(sumAAdjVolt>>5);			//2016-04-22 ?? 11:54:49			
		AmpFnd = (int)(sumOutCurrent>>5);
        TempFnd = (int)(sumTemp>>5);
        BattAmpFnd = (int)(sumBattCurrent>>5);
	}
	//
	ovCnt++;
	//
	if(ovCnt >= (31))
	{
		fSaveDone=1;//refirm//
		ovCnt=0;	//reset index//
	}	
	//---------------------------------------------------	
	ltmp = mapping(AAdjFnd,0,600,0,40);             //0~600 => 0~1200
	VRCurrRef = (int)(ltmp*468);
	if(VRCurrRef>19000)VRCurrRef=19000;
	//	
	VRVoltRef= mapping(VAdjFnd,0,1000,0,6000);      //0~600 => 0~1200		
	//------------------------------------------------------------
	VDcQ15 = VoltFnd << 5;							// convert Q15 format//	
	VBusQ15 = VBusFnd << 5;							// convert Q15 format//
	IpsfbQ15 = AmpFnd << 5;                         // convert the 10bit ADC valure to Q15 format
    BattIpsfbQ15 = BattAmpFnd << 5;                 // convert the 10bit ADC valure to Q15 format
	
	//======================Voltage Dropping Current=======================	
	IpsfbQ15_Flt = (__builtin_mulss(IpsfbQ15, Q15(0.2)) >> 15) + (__builtin_mulss(prevIpsfbQ15, Q15(0.8))>>15); //
    BattIpsfbQ15_Flt = (__builtin_mulss(BattIpsfbQ15, Q15(0.2)) >> 15) + (__builtin_mulss(prevBattIpsfbQ15, Q15(0.8))>>15); //

	//-----------------------------------------------
	prevIpsfbQ15 = IpsfbQ15_Flt;		//back up	
    prevBattIpsfbQ15 = BattIpsfbQ15_Flt;		//back up
	//-------------------------------------------------------------------
    
    Fault_Check();
    Control_Loop();

    _ADCP1IF = 0;						// Clear the ADC pair1 Interrupt Flag	
}

////////////////////////////////////////////////////////////////////////////////
//==================================== RS232 =================================//
////////////////////////////////////////////////////////////////////////////////
void __attribute__((__interrupt__, no_auto_psv)) _U2RXInterrupt()
{//RS-232
	unsigned char i = 0, RcvData = 0;
    unsigned int tmp = 0, Checksum = 0;
    
    //Clear Overflow Flag - KHJ
    if(U2STAbits.OERR == 1)     
    {
        U2STAbits.OERR = 0;
    }
    // Receive Data RX Buff 
    RcvData = U2RXREG;
    
    //Start of Transmission Packet - SOT => 0x5B '['
    if(RcvData == 0x5B && EOT_Flag == 0)             
	{
        f_SData = 1;                  //start serial data flag 
		Rindex = 0;
        EOT_Flag = 1;
	}
    
    //EOT of Transmission Packet - EOT => 0x5D ']'
	else if((f_SData == 1) && (RcvData == 0x5D) && (Rindex == 34))
	{
        for(i=0 ; i<32 ; i++)
        {
            Checksum ^= RxBuf[i];
        }   
        
        //Checksum Check
        tmp = ((RxBuf[32] - 0x30) << 4) & 0xF0;
        tmp = tmp + ((RxBuf[33] - 0x30) & 0x0F);        
        if(tmp == Checksum)             
        {
            f_EData = 1;
            EOT_Flag = 0;
            tmrtick3 = 0;
        }
        else
        {
            f_SData = 0;
            f_EData = 0;
            EOT_Flag = 0;
            tmrtick3 = 0;
        }	
	}		
	RxBuf[Rindex]= RcvData;
	Rindex++;
	
    //Occur overflow -> Reset buffer//
	if(Rindex>36)               
	{	
		Rindex=0;	
		f_SData=0;
		f_EData=0;
        EOT_Flag = 0;
        tmrtick3 = 0;
	}
	IFS1bits.U2RXIF=0;
}  

void __attribute__((__interrupt__, no_auto_psv)) _U2TXInterrupt()
{//RS-232
	IFS1bits.U2TXIF=0;
} 

////////////////////////////////////////////////////////////////////////////////
//==================================== RS485 =================================//
////////////////////////////////////////////////////////////////////////////////
void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt()
{//RS-485 - KHJ .2018-02-02 
	unsigned char i = 0, RcvData = 0;
    unsigned int tmp = 0, Checksum = 0;
   
	//Clear Overflow Flag - KHJ
    if(U1STAbits.OERR == 1)     
    {
        U1STAbits.OERR = 0;
    }
    RcvData = U1RXREG;   
    
   	//Start of Transmission Packet - SOT => 0x5B '['
	if(RcvData == 0x5B && EOT_Flag485 == 0)
    {
        f_SData485 = 1;         //start serial data flag 
        Rindex485 = 0;
        EOT_Flag485 = 1;
    }
    
    //EOT of Transmission Packet - EOT => 0x5D ']'
    else if((f_SData485 == 1) && (RcvData == 0x5D) && (Rindex485 == 9))
    {
        //Enable driver set
        for(i=0 ; i<7 ; i++)
        {
            Checksum ^= RxBuf485[i];
        }
        
        //Checksum Check
        tmp = ((RxBuf485[7] - 0x30) << 4) & 0xF0;
        tmp = tmp + ((RxBuf485[8] - 0x30) & 0x0F);
        if(tmp == Checksum)
        {
            f_EData485 = 1;
            EOT_Flag485 = 0;
            tmrtick2 = 0;
        }
        else
        {
            f_SData485 = 0;
            f_EData485 = 0;
            EOT_Flag485 = 0;
            tmrtick2 = 0;
        }	

    }		
    RxBuf485[Rindex485]=RcvData;
    Rindex485++;
    
    //Occur overflow -> Reset buffer//
    if(Rindex485>10)             
    {	
        Rindex485=0;	
        f_SData485=0;
        f_EData485=0;
        EOT_Flag485 = 0;
        tmrtick2 = 0;
    }
	IFS0bits.U1RXIF=0;
}  

void __attribute__((__interrupt__, no_auto_psv)) _U1TXInterrupt()
{//RS-485
    IFS0bits.U1TXIF=0;
}

