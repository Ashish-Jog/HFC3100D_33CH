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
#include "Variables_PSFB.h"
#include "Define_PSFB.h"
#include "dsp.h"
#include "delay.h"

void Dropping_Control(void);                // Voltage Dropping for Current Saturation
void LoadShareControl(void);                // Load Share 
void Voltage_Control(void);                 // Voltage Control for Reference Voltage Error is Zero
void DigitalCompensator(void);              // Module Control Sequence Main Function
void ModuleOnOff(void);
void Reset_Device(void);
void Fault_Check(void);
void Control_Loop(void);

int ReOCL_CNT = 0;

void Control_Loop(void)
{
    //========================== For Debug forced Clear//======================
//    UV_Fail=0;   //17.11.28 Autostop Disable
//    OV_Fail=0;  //17.11.28 KHJ Voltage limit
//    OT_Fail=0;
	//==========================================================================
    
    //======================//
	// Operation Logic      //
	//======================//	
    // none of the above 3 faults occured
    if(UV_Fail == 0 && OV_Fail == 0 && OT_Fail == 0)
	{
        FaultStatus = 0;                // No Fault
        //$$$$$$$$$$$$$$$$$$$		
        DigitalCompensator();			// If there is no fault execute the PI loop	
        //$$$$$$$$$$$$$$$$$$$
   	}
    // If fault exists, then...
 	else 								
	{
		FaultStatus = 1;				// Set the Fault Status Flag	
//		IOCON1bits.OVRENH = 1;			// Take Override Bits to control PWM Module
//		IOCON1bits.OVRENL = 1;			// Take Override Bits to control PWM Module	
//		IOCON2bits.OVRENH = 1;			// Take Override Bits to control PWM Module
//		IOCON2bits.OVRENL = 1;			// Take Override Bits to control PWM Module
//		IOCON1bits.OVRDAT = 0;			// Force the PWM Pin to Low State or Inactive State
//		IOCON2bits.OVRDAT = 0;			// Force the PWM Pin to Low State or Inactive State
	}
}



//=====================================================================
// 120khz-interval 16.8us (elapse time 6.8us) call from adc_interrupt//
// 15khz-interval 134us //
// 20Khz-interval 100us //
//=====================================================================
void DigitalCompensator(void) 
{	
	//=====================//
	// First Initializing  //
	//=====================//
	if(firstPass == 1)				// Check for First Iteration of the loop
    {
        VoReference = 0;			// Assign Ref. Volt. to actual measured DC Bus Volt.																	
        firstPass = 0;				// Clear the First Pass Flag(init value : firstPass = 1) //
        Relay = 1;                  // Relay ON
        Nfb_Trip = 1;
    }
    
    //================//
	// softstart Mode //
	//================//
    if(softstart == 1)				 		//Check for SoftStart Flag Status
	{
        // Until Reference Voltage is Less than ~125V
		if(VoReference < (Set_VRef_Q15 + (int)VRVoltRef))            
		{										
			VoReference = VoReference + 1; //17.11.22 +1 - KHJ //ORG. 5 // Softstart elapse time short	
		}        
        // Until Reference Voltage is More than 125V~
		else
		{
			VoReference = Set_VRef_Q15 +(int)VRVoltRef; // Else, FixTheReferenceVoltage to 125V (VoRefQ15 - VBus = 0)
			softstart = 0;                              // completed softstarting...//
            SystemSet = System_OFF;            // 18.06.18 Normal Operation // ORG : System Status for Turn ON Module : Standby
		}
	}    
    //================//
	// Normal Control //
	//================//
	else
	{
        // Voltage Reference Q15 Converting, Real Value Fitting, 0.124, 216.8, 3219.4 <- fitting value 
        Set_VRef_Q15 = -(0.124 * Set_VRef * Set_VRef) + (216.8 * Set_VRef) - 3219.4;      // y = 186.70x - 1510.26  => Voltage Reference -> Q15 Convert EQ
        // Voltage Reference Calculation (VBus Reference Added VR1)
		VoReference = Set_VRef_Q15 + (int)VRVoltRef;            //KHJ-Logic
		//max Reference Voltage Limitation//
        if(VoReference > Q15(0.99)) VoReference = Q15(0.99);	
		
        // Voltage Dropping for Current Saturation 
        Dropping_Control();
		VoReference = VoReference - (int)VoReferenceLimitOut - (int)VoReferenceLimitOut2; //Batt current & Load current
        
        //LoadShare Control
        if(OP_Type == 1)            // Load-Share Mode, Parallel type
        {
            SystemSet = System_ON;
            LoadShareControl();
            VoReference = VoReference + LoadShareControlOut; // Error current and load sharing control
        }
        else                        // Stand-alone Mode
        {
           //
        }
        if(VoReference > Q15(0.99))     VoReference = Q15(0.99);
        else if(VoReference < 0)        VoReference = 0;
	}//end else if(softstart==0)//	
    
    // Main Control Function
    Voltage_Control();   
}

void Dropping_Control(void)
{
    //====================================================================//
    //                      Current Limit Voltage Dropping                //
    //====================================================================//

    // Current Limit Error Calculation for Current Limit Voltage Dropping 
    Io_CurrLimitErr = (long)IpsfbQ15_Flt - (long)AmpRef[Set_CL_ID];     //Logic                          
    Io_BattCurrLimitErr = (long)BattIpsfbQ15_Flt - (long)BattAmpRef[Set_BCL_ID];     //Logic		

    // Proportional Output Calculation for Current Limit Voltage Dropping
    Io_CurrLimitPropOutput = ((__builtin_mulss((int) KpCurrLimit,Io_CurrLimitErr)) >> 15);
    Io_BattCurrLimitPropOutput = ((__builtin_mulss((int) KpBattCurrLimit,Io_BattCurrLimitErr)) >> 15);
    
    // Integral Output Calculation for Current Limit Voltage Dropping
    Io_CurrLimitIntegralOutput = Io_CurrLimitIntegralOutput +  (__builtin_mulss((int) KiCurrLimit,Io_CurrLimitErr) >>15);
    Io_BattCurrLimitIntegralOutput = Io_BattCurrLimitIntegralOutput +  (__builtin_mulss((int) KiBattCurrLimit,Io_BattCurrLimitErr) >>15);
    
    // CurrLimitSaturation is 32767(Q15 trans. to DEC) for Current Limit Voltage Dropping
    if(Io_CurrLimitIntegralOutput > CurrLimitSaturation) 		
    {	 
        Io_CurrLimitIntegralOutput = CurrLimitSaturation;
    }	
    else if(Io_CurrLimitIntegralOutput < CurrLimitSaturationZero)
    {	
        Io_CurrLimitIntegralOutput = CurrLimitSaturationZero;		
    }
    
    if(Io_BattCurrLimitIntegralOutput > BattCurrLimitSaturation) 		
    {	 
        Io_BattCurrLimitIntegralOutput = BattCurrLimitSaturation;
    }	
    else if(Io_BattCurrLimitIntegralOutput < BattCurrLimitSaturationZero)
    {	
        Io_BattCurrLimitIntegralOutput = BattCurrLimitSaturationZero;		
    }
    
    // PI Control Calculation Result for Current Limit Voltage Dropping
    Vo_CurrLimitRef = (long)Io_CurrLimitPropOutput  + Io_CurrLimitIntegralOutput; 
    Vo_BattCurrLimitRef = (long)Io_BattCurrLimitPropOutput  + Io_BattCurrLimitIntegralOutput; 		

    // PI Control Output Limit for Current Limit Voltage Dropping        
    if(Vo_CurrLimitRef > CurrLimitSaturation)		//32767
    {	 
        Vo_CurrLimitRef = CurrLimitSaturation;
    }	
    else if(Vo_CurrLimitRef < CurrLimitSaturationZero)
    {	
        Vo_CurrLimitRef = CurrLimitSaturationZero;
    }	
    
    if(Vo_BattCurrLimitRef > BattCurrLimitSaturation)		//32767
    {	 
        Vo_BattCurrLimitRef = BattCurrLimitSaturation;
    }	
    else if(Vo_BattCurrLimitRef < BattCurrLimitSaturationZero)
    {	
        Vo_BattCurrLimitRef = BattCurrLimitSaturationZero;
    }	

    // PI Control Output Sclaed for Current Limit Voltage Dropping 
    Vo_CurrLimitRef = (Vo_CurrLimitRef << Io_CurrLimitRefShift);	//Io_CurrLimitRefShift is 2
    Vo_BattCurrLimitRef = (Vo_BattCurrLimitRef << Io_BattCurrLimitRefShift);	//Io_BattCurrLimitRefShift is 2
    
    // add limit check routine 
    if(Vo_CurrLimitRef > CurrLimitSaturation)		//32767
    {	 
        Vo_CurrLimitRef = CurrLimitSaturation;
    }	
    else if(Vo_CurrLimitRef < CurrLimitSaturationZero)
    {	
        Vo_CurrLimitRef = CurrLimitSaturationZero;
    }
    
    if(Vo_BattCurrLimitRef > BattCurrLimitSaturation)		//32767
    {	 
        Vo_BattCurrLimitRef = BattCurrLimitSaturation;
    }	
    else if(Vo_BattCurrLimitRef < BattCurrLimitSaturationZero)
    {	
        Vo_BattCurrLimitRef = BattCurrLimitSaturationZero; 
    }	

    VoReferenceLimitOut = __builtin_mulss((int)Vo_CurrLimitRef,VoRefQ15) >> 15;
    
    // Current Sensor Deadzone 
    if(BattAmpFnd < 1)       //Minimum : <1A is Full Charge Status
    {
        VoReferenceLimitOut2 = 0;
    }
    else
    {
        VoReferenceLimitOut2 = __builtin_mulss((int)Vo_BattCurrLimitRef,VoRefQ15) >> 15;
    }   
    
}

void LoadShareControl(void)
{
		LoadShare_Err = canShareQ15-IpsfbQ15;				//Share Current - Current//
		LoadShare_Err =  ( (__builtin_mulss(Q15(0.3),LoadShare_Err) )>>15 ) + ( (__builtin_mulss(Q15(0.7),LoadShare_prevErr) )>>15 ); // 3:7 Load Error Compensation, 15bit
		LoadShare_prevErr  = LoadShare_Err;		//backup current error//		
        LoadShare_IntegralOutput = LoadShare_IntegralOutput + ((__builtin_mulss((int) KiLoadShare,LoadShare_Err))>>15);

        if(LoadShare_IntegralOutput > 2400)//LOADSHARE_SATURATION) //9600(300),12800(400),16000(500) 9600(300),6400(200) Experiance value			
        {	 
            LoadShare_IntegralOutput = 2400;//LOADSHARE_SATURATION;
        }	
        else if(LoadShare_IntegralOutput < -1600)//CurrLimitSaturationZero)	
        {	
            LoadShare_IntegralOutput = -1600;//CurrLimitSaturationZero;		
        }

        LoadShare_PropOutput = ((__builtin_mulss((int) KpLoadShare,LoadShare_Err)) >> 15);

        LoadShareControlOut = (long)LoadShare_PropOutput  + LoadShare_IntegralOutput ; 

        if(LoadShareControlOut > 9600)//LOADSHARE_SATURATION) 						//Saturation is 200//
        {	 
            LoadShareControlOut = 9600;//LOADSHARE_SATURATION;
        }
        else if(LoadShareControlOut < -1600)//CurrLimitSaturationZero)	
        {	
            LoadShareControlOut = -1600;//CurrLimitSaturationZero;		
        }

        LoadShareControlOut = LoadShareControlOut >> 5;		//17.11.23 MAX 2400 2^2=4 - KHJ  //divided by 2^5=32//		
}


void Voltage_Control(void)
{
    int temp = 0;
    // Diode Drop Compensation
	iVdcVbus = abs(VDcQ15-VBusQ15) >> 1;
    
    // Main Control PI Control Fuction
	Verror = (VoReference - VBusQ15 + iVdcVbus);
    temp = Verror;
    // Ratio Control Method Error Effect (Damping for Voltage Dynamics)
    Verror =  ((__builtin_mulss(Q15(0.1),Verror))>>15) + ((__builtin_mulss(Q15(0.9),prevVerror))>>15);
	
    //backup current Verror//
	prevVerror = Verror;

	//Integrator Control Outuput	
    if(SaturationFlag == 0 || Verror < 0)	
	{	
		VoltageIntegralOutput = VoltageIntegralOutput + ((__builtin_mulss((int) KiVoltage,Verror))>>15);
	}
	if(VoltageIntegralOutput > Saturation) 				//8190
	{	 
		VoltageIntegralOutput = Saturation;
	}	
	else if(VoltageIntegralOutput < -Saturation)	
	{	
		VoltageIntegralOutput = -Saturation;		
	}
    
    // Proportinal Control Output
    VoltagePropOutput = (long)((__builtin_mulss(KpVoltage,Verror)) >> 15);
    
    // PI Control Calculation Result
	CurrentRef = VoltagePropOutput + VoltageIntegralOutput; 
    
	// PI Control Result Saturation
	if(CurrentRef > Saturation)
	{	 
		CurrentRef = Saturation;
	}	
	else if(CurrentRef < -Saturation)
	{	
		CurrentRef = -Saturation;
	}	
	
    //Again making this back to Q15 format
	CurrentRef = (CurrentRef << Refshift);	
    
    VoltageDecoupleTerm = ((__builtin_mulss((int) Voltagedecouple,VBusQ15)) >> 15);
    
    // PI Control Result Limit
	if(CurrentRef < -32767)
		CurrentRef = -32767;	
	else if(CurrentRef > 32767)	
		CurrentRef = 32767;
	// Control Output Calculation
	PIOutput  =  (long)VoltageDecoupleTerm + (long)CurrentRef;     
    
    // PI Control Output Limit
	SaturationFlag = 0;
	if(PIOutput > 32767)		//Maximum Saturate				
	{
		PIOutput = 32767;
		SaturationFlag = 1;
	}
	else if(PIOutput < 0)
    //Minimum Output Zero, Not Negative.
	{	
        PIOutput = 0;			
	}
    
//     Jumping Volatage Protect - No Load  x > 3.0 %, Overshoot At Over 3% load to 0% load
    if(SystemState == System_ON && temp < 0 && softstart == 0 && Set_VRef < VBus_Val * 0.97)
    {
        PIOutput = 0;
    }
	//=======================================================================
	// The Calculated PhaseZVT is scaled down to maximum allowed Phase shift.
	//=======================================================================
    PhaseZVT = ((__builtin_mulss((int)PIOutput,MaxPhase)) >> 15);  //PHASE1
	duty =  PWMPeriod - PhaseZVT;
  	PHASE2 =  duty; 
}

//-----------------------------------------------------------//
//                  System Mode Select                       //
//-----------------------------------------------------------//
void ModuleOnOff(void)
{
    //-----------------
    // Fault Condition
    //-----------------
	if(E_Stop == 0 && (FaultStatus == 1 || FLT_Flag == 1))     
	{
        IOCON1bits.OVRENH = 1;          // Take Override Bits to control PWM Module
        IOCON1bits.OVRENL = 1;          // Take Override Bits to control PWM Module	
        IOCON2bits.OVRENH = 1;          // Take Override Bits to control PWM Module
        IOCON2bits.OVRENL = 1;          // Take Override Bits to control PWM Module	
        IOCON1bits.OVRDAT = 0;          // Force the PWM Pin to Low State or Inactive State
		IOCON2bits.OVRDAT = 0;          // Force the PWM Pin to Low State or Inactive State
        //
        SystemState = System_OFF;       //System Status Change
        SystemSet = System_OFF;
        E_Stop = 1;                     //Emergency Stop    
        //sysLed = 0;    
	}    
    //-----------------
    //Standby Condition
    //-----------------
    else if(E_Stop == 0 && FaultStatus == 0 && 
        FLT_Flag == 0 && softstart == 0 && 
        SystemSet == System_OFF)
    { 
        if((CHG_Info & 0x10) == 0x10)//chg_info.bit4 battery type Li
        {
            Set_UV = Set_UV_B * 0.85;
            if((CHG_Info & 0x20) == 0x20) Set_VRef = STNDBYOutputVoltage_LI;//125v
            else if((CHG_Info & 0x20) == 0x00)//강제운전
            {
                Set_VRef = Set_UV_B * 0.9;
            }
        }
        else if((CHG_Info & 0x10) == 0x00)//battery type : PB
        {
            if((CHG_Info & 0x08) == 0x00)//125VDC chg_info.b3 DC voltage :125/110VDC
            {
                Set_UV = Set_UV_B * 0.85;
                if((CHG_Info & 0x20) == 0x20) Set_VRef = STNDBYOutputVoltage_PB;//125VDC
                else if((CHG_Info & 0x20) == 0x00) //강제운전
                {
                    Set_VRef = Set_UV_B * 0.9; 
                }
            }
            else if((CHG_Info & 0x08) == 0x08)//110VDC
            {
                Set_UV = Set_UV_B * 0.85;
                if((CHG_Info & 0x20) == 0x20) Set_VRef = STNDBYOutputVoltage2_PB;//110VDC
                else if((CHG_Info & 0x20) == 0x00)//강제운전
                {
                    Set_VRef = Set_UV_B * 0.9;
                }
            }      
        }
        SystemState = System_OFF;
    }
    //-------------------
    //System ON Condition
    //-------------------
    else if(E_Stop == 0 && FaultStatus == 0 && 
        FLT_Flag == 0 && softstart == 0 && 
        (SystemState == System_OFF && SystemSet == System_ON))
    {
        Set_UV = Set_UV_B;
        Set_VRef = Set_VRef_B;        
        OverVoltageStatus = 0;                      // Output voltage is in the specified Value
        UnderVoltageStatus = 0;                     //
        OverTemperatureStatus = 0;					// Temperature is in the specified Value     
        //
        SystemState = System_ON;
    }
    //-----------------
    //Reset Condition
    //-----------------
	else if(E_Stop == 1 && softstart == 0 && 
        SystemState == System_OFF && 
        SystemSet == System_ON)
	{
        SystemState = System_ON;
        E_Stop = 0;
        FaultStatus = 0;
        FLT_Flag = 0;
        TDI_count = 0;
        Reset_Device();
	}
}

//-----------------------------------------------------------//
//                    Module Reset Function                  //
//-----------------------------------------------------------//
void Reset_Device(void)
{
    _ADCP1IF = 0;
    _ADCP1IE = 0;                           //ADC Interupt Enable
    PTCONbits.PTEN = 0;                     // Turn OFF the PWM module
    IOCON1bits.OVRENH = 0;                  //Take Override Bits to control PWM Module
    IOCON1bits.OVRENL = 0;
    IOCON2bits.OVRENH = 0;
    IOCON2bits.OVRENL = 0;
    
    // Forcing the system to enter into the Softstart mode				
    firstPass = 1;                           // firstpass Flag Reset		
    softstart = 1;                           // softstart Flag Reset
    INTCON2bits.INT1EP = 0;                 // External Interrupt 1 Edge Detect Polarity Select bit; positive edge
    VoltageIntegralOutput = 0;				// Initializing PI variables
    prevVerror = 0;                         // Initializing Error
    CurrentIntegralOutput = 0;               // Initializing Integrator
    PIOutput = 0;                          // Initializing PI Output
    PhaseZVT  = 0;                          // Initializing PhaseZVD
    SaturationFlag = 0;                     // Initializing Saturation Flag
    PHASE2 = PWMPeriod;	      				// During startup there is no overlap between diagonal switches, so there is no output voltage during start up.
    _ADCP1IE = 1;                           // Enable Adc interrupt  				
    PTCONbits.PTEN = 1;						// Enable the PWM
}	

void Fault_Check(void)
{
    //==================//
	// Check OverCurrent//
	//==================//	
    if(Amp_Val >= Set_CL * 1.25)            // Checking for the Overcurrent fault
	{	
        ReOCL_CNT++; // 110Vdc 50A, 2021.11.16 PARK
        if(ReOCL_CNT >= Delay_1S_Cnt)		// Modification that fault delay 1Sec. (5423 count)
        {									//    because the booting did not end, but module start
           FLT_Flag = 1;                    // Set the Fault Status Flag   
           ReOCL_CNT = Delay_1S_Cnt;
        }
 	}	
    else if(Set_CL * 1.25 > Amp_Val && Amp_Val >= Set_ARef)
    {
        OverCurrentStatus = 1;              // set the flag if output Current Warning exceeds
        ReOCL_CNT = 0; // 110Vdc 50A, 2021.11.16 PARK
    }
    else
	{	
        OverCurrentStatus = 0;               // Output Current is in the specified Value
        ReOCL_CNT = 0; // 110Vdc 50A, 2021.11.16 PARK
    }
    
	//==================//
	// Check Voltage//
	//==================//	
	if(VoltFnd_Avg > Set_OV)            // Checking for the Overvoltage fault
	{	
        OverVoltageStatus = 1;          // set the flag if output volatge limit exceeds
        OV_Fail = 1;                    // Set the Fault Status Flag	
	}	
    else if((softstart == 0) && (tmrtick5 >= 6000) && (VoltFnd_Avg < Set_UV)) 	// Checking for the Undervoltage fault //boot time ~=4sec
	{	
        UnderVoltageStatus = 1;         // set the flag if output volatge limit exceeds
        UV_Fail = 1;                    // Set the Fault Status Flag		
	}    
	else
	{	
        OV_Fail = 0;                    // Reset the Fault Status Flag	
        UV_Fail = 0;                    // Reset the Fault Status Flag	
	}
    
    //======================//
	// Check OverTemperature//
	//======================//	
    if(Temp_Val > Set_OT)                               // Checking for the OverTemp fault
	{	
        OverTemperatureStatus = 1;					// set the flag if output OverTemp Warning exceeds
        OT_Fail = 1;                              // Set the Fault Status Flag		
	}	
    else
	{	
        OT_Fail = 0;                              // Reset the Fault Status Flag	
	}
}


