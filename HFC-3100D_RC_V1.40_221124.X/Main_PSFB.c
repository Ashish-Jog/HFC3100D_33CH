/****************************************************************************************
* Application : HFC-3100D														      	*
* Seoul Electric Power System co.,Ltd  	  2018/04/10  	First release of source file  	*
* Device : Rectifier(Microchip dspic33FJ64GS606)										*
* Target PCB No : 131227-V1.0                                                           *
* Compiler : mplab_C V3.31						
//=================================================					 					*
// under volt limit -15V for majang 5line subway (22.11.04)
// RS232.c if(tmpvalue>150)tmpvalue = tmpvalue - 150;//???????(min)?? -15.0V
****************************************************************************************/
#include "p33FJ64GS606.h"
#include "Define_PSFB.h"
#include "Variables_PSFB.h"
#include "dsp.h"
#include "delay.h"
#include "eCAN.h"
#include <stdlib.h>					//abs(int x), labs(long x)

//----------------------------------------------------------
_FOSCSEL(FNOSC_FRC)					// Internal FRC with non PLL
_FOSC(FCKSM_CSECMD & OSCIOFNC_ON) 	// clock Sw Enabled, Mon Disabled and OSCO pin has digital I/O function
_FWDT(FWDTEN_OFF)                   // & WDTPRE_PR32 & WDTPOST_PS128)	// Watchdog Timer disabled
//------------------------------------------------------------------------------
_FPOR(FPWRT_PWR128) 				// POR Timer Value 128mSec
_FICD(ICS_PGD2 & JTAGEN_OFF);   	// Use PGC2/EMUC2 and PGD2/EMUD2

//======================================================================

//----------------------------- PSFB Control ---------------------------------//
void init_Conf(void);                          // System Variant initialized
void init_PSFBDrive(void);                    // Full bridge gate drive is initialized
void init_ADC(void);                           // Analog ports are initialized
void init_Timer1_2(void);                     // Timer 1,2 initialized				
void LoadShareControl(void);                  // Loadshare Control
void DigitalCompensator(void);	 			// Voltage mode PI control
void GetDipSW(void);                           // Read SW1(Dip switch)-> Set ID

//--------------------------- LCD Comm(RS-232) -------------------------------//
void init_Uart2(void);                          // for RS-232
void Response_Serial(void);                    // Set Data Response 
void BittoByte(void);                           // Bit to Byte Conversion
void Continuous_FB(void);                      // Continuous Data 
void Response_FB(void);                        // Set Data TX
void Set_Value(void);                           // xxxx -> xxx.x & Save
void Serial_Process(void);                     // Serial Data Processing
void Input_Volt(void);                          // Serial Input Voltage Data
void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch);  // Send Form

//----------------------- Main Controller Comm(RS-485) -----------------------//
void init_Uart1(void);                          // for RS485
void BittoByte485(void);                        // Bit Conversion to Byte
void Continuous_FB485(void);                   // Continues Data-MainControl  
void Response_Serial_485(void);                // Set Data Response 
void Serial_Process_485(void);                 // Serial Data Processing
void Send_Serial_485(unsigned int ival,unsigned char pindex,unsigned char ch);  // Send Form

//------------------------ Loadshare eCAN ------------------------------------//
void init_CAN(void);
void CAN_Process(void);
void Conf_ECAN(void);

//============================= Module Control ===============================//
void Control_Loop(void);
void Reset_Device(void);                        //Reset
void ModuleOnOff(void);                         //ON/OFF
void Fault_Check(void);                         //Checking Fault

//-------------------------------- Data Conv ---------------------------------//
void ADC_Data_Conv(void);                       //Real Value Conversion
long mapping(long x, long inMin, long inMax, long outMin, long outMax);

//==========//
// Main Loop//
//==========//
int main (void)
{
    //===============================================================
	// Device Module Initialize Routine//
	//---------------------------------------------------------------	
    init_Conf();                                //System Configuration

    init_ADC();                                 //ADC Initializing
    init_Timer1_2();                            //Timer1,2 setting//
    init_Uart1();                               //Uart 1 for RS485 - Main controller//	
    init_Uart2();                               //Uart 2 for RS232 - LCD Display//		
    GetDipSW();                                 //Read Dip SW 1~4, value 1, 2, 4, 8 -> Device ID//
    init_PSFBDrive();                           //Phase Shift PWM
    init_CAN();
    Conf_ECAN();
    //===============================================================
	// Device Module Boot Sequence                                       //
	//---------------------------------------------------------------
    INTCON1bits.NSTDIS = 0; 					//disable Int. nesting//
    INTCON2bits.INT1EP = 1;						//external int1 negative edge//
    IFS1bits.INT1IF = 0;	 					// Clear interrupt flag
    IEC1bits.INT1IE = 1;						// Enable interrupt
    PTCONbits.PTEN = 1;							// Enable the PWM
    //--------------------//
    // Wait system stable - Important for Timing// Boot fail : MT Kim - 20210105 
    //--------------------//
    Delay(Delay_1S_Cnt);
    Delay(Delay_1S_Cnt);
    Delay(Delay_1S_Cnt);
    
    // A/D converter module is operating
    ADCONbits.ADON = 1;                         
        
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//
    while(1)                                    // Main Infinite Loop //		
    {
        ////////////////////////////////////////////////////////////////////
        //     RS-232 & RS-485 serial data Transmit Timer Set-KHJ         //
        ////////////////////////////////////////////////////////////////////
        if(firstPass != 1)			//firstpass init value is '1'
        {
            if(tmrtick4 >= 1)//10us ??
            {
                ADC_Data_Conv(); 
                tmrtick4 = 0;
            }
            Serial_Process_485();	//between MainController//
            Serial_Process(); 		//between Rectifier Display//
            ModuleOnOff();
            CAN_Process();			//between other Rectifier Module//
        }
		//-----------------------------------------------------
//        if(softstart == 0)         // After Softstart
        {  
            // Check the RS-485 : Turn Off the Rectifier to Communication Timeout for Maincontrol
            if(tmrtick6 >= 15000)  //1ms * 15000 = 15000ms(15sec))    //timeout for RS485
            {
                tmrtick6 = 15000;
                SystemSet = System_OFF;
                sysLed ^= 1; 
            }
        }
	}
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//



//------------------------------------------------------------//
//                        Mapping Data                        //
//------------------------------------------------------------//
long mapping(long x, long inMin, long inMax, long outMin, long outMax)
{
	long tmp1 = 0, tmp2 = 0;
	//return ( ( (x-inMin) * (outMax - outMin) ) /(inMax-inMin) ) + outMin;
	tmp1 = __builtin_muluu((x-inMin),(outMax-outMin));
	tmp2 = __builtin_divud(tmp1,(inMax-inMin));
	return (tmp2 + outMin);
}

//-----------------------------------------------------------//
//             ADC Data Converting & Calibration             //
//-----------------------------------------------------------//
void ADC_Data_Conv()
{
    unsigned int    tmpvalue = 0;
    float    tmpvalue2 = 0;

    //---------------
    // Output Voltage //1.6Vdc/1ADC 
    //---------------
    tmpvalue = VoltFnd;                             // 10Bit ADC Value
    tmpvalue2 = (float)tmpvalue * (Volt_Gain + (float)(ADJ_VGain * 0.0001));        // Scale Factor 
    Vout_Val = tmpvalue2;                           // 125Vdc 0.167 - KHJ

    //---------------
    // VBus Voltage
    //---------------
    tmpvalue = VBusFnd;                             // 10Bit ADC Value
    tmpvalue2 = (float)tmpvalue * (Volt_Gain + (float)(ADJ_VGain * 0.0001));        //18.04.10 125Vdc 0.167 - KHJ
    VBus_Val = tmpvalue2;                           // 125Vdc 0.167 - KHJ
    
    //------------------ For Display - Average Filter --------------------//
    avgCnt2++;                                      // Display Tx Speed Change for LCD                     
    SumVolt += VBus_Val;                           // Summation
    if(avgCnt2>1000)
    {	
        VoltFnd_Avg = SumVolt/(float)avgCnt2;       // Filtered Data N = 200 Average
        avgCnt2 = 0;                                // 
        SumVolt = 0;                                // 
    }
    //--------------------------------------------------------------------//

    //---------------
    // Current
    //--------------- 
    // Damping for Current Dynamics 
    tmpvalue =  ( (__builtin_mulss(Q15(0.5),AmpFnd) )>>15 ) + ( (__builtin_mulss(Q15(0.5),prev_AmpFnd) )>>15 );
    prev_AmpFnd  = tmpvalue;
    tmpvalue2 = (float)tmpvalue * (Current_Gain + (float)(ADJ_CGain * 0.0001));            //+125Vdc ///250Vdc 0.135 for 100A CT - KHJ
    Amp_Val = tmpvalue2;
    
    
    
    //------------------ For Display - Average Filter --------------------//
    avgCnt++;
    SumCurr += tmpvalue2;
    //
    if(avgCnt>200)                          // Filtered Data N = 200 Average for LCD
    {	
        AmpFnd_Avg=SumCurr/(float)avgCnt;    
        avgCnt=0;                            
        SumCurr=0;                           
    }

    //---------------
    // Temperature - 3 Order Linear EQ - CRT Thermistor(3-Order)
    //---------------
    tmpvalue = TempFnd;
    tmpvalue2 = ((float)tmpvalue * (float)tmpvalue * (float)tmpvalue * Temp_Gain_TO) + ((float)tmpvalue * (float)tmpvalue * Temp_Gain_SO) + ((float)tmpvalue * Temp_Gain_FO) + Temp_Offset;        //18.04.10 3-order EQ
    Temp_Val = tmpvalue2;  
    
    //---------------
    // Battery Current
    //--------------- 
    // Damping for Current Dynamics 
    tmpvalue =  ( (__builtin_mulss(Q15(0.5),BattAmpFnd) )>>15 ) + ( (__builtin_mulss(Q15(0.5),prev_BattAmpFnd) )>>15 );
    prev_BattAmpFnd  = tmpvalue;
    tmpvalue2 = (float)tmpvalue * (Current_Gain + (float)(ADJ_CGain * 0.0001));            //+125Vdc ///250Vdc 0.135 for 100A CT - KHJ
    BattAmp_Val = tmpvalue2;
    //------------------ For Display - Average Filter --------------------//
    avgCnt3++;
    SumBattCurr += tmpvalue2;
    //
    if(avgCnt3>200)                          // Filtered Data N = 200 Average
    {	
        BattAmpFnd_Avg=SumBattCurr/(float)avgCnt3;    
        avgCnt3=0;                            
        SumBattCurr=0;                           
    }
}

//void LPF(float *Input, float *Output, float Samp_Freq, float Cutoff_Freq, float *PastInput, float *PastOutput)
//{
//    float a1, b0, b1, w0;
//    w0 = 2 * 3.14 * Cutoff_Freq;
//    a1 = (w0 - 2 * Samp_Freq) / (2 * Samp_Freq + w0);
//    b0 = w0 / (2 * Samp_Freq + w0);
//    b1 = b0;
//    *Output = b0 * (*Input) + b1 * (*PastInput) - a1*(*PastOutput);
//    *PastOutput = *Output;
//    *PastInput = *Input;
//}





