/**
  ADC1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    adc1.c

  @Summary
    This is the generated driver implementation file for the ADC1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for ADC1.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device            :  dsPIC33CH128MP506      
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB 	          :  MPLAB X v5.50
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/

#include "adc1.h"
#include "../typeDef.h"
#include "../funcData.h"
#include "../main.h"
#define LoadV_Gain  253
#define LoadA_Gain  23
#define BattV_Gain  253
#define BattA_Gain  24
#define BattT_Gain  86

UINT32 	Ad_Sum_Buffer[11]={0,};             //ADC sum buffer
UINT8	ad_cnt;
UINT16 	_avgLdV,_avgOutV,_avgLdI,_avgBatI,_avgBatTmp;
UINT16 	avgLdV,avgLdI,avgOutV,avgBatI,avgBatTmp;
UINT8	adcerr1=0,adcerr2=0,adcerr3=0,adcerr4=0,adcerr5=0,adcerr6=0,adcerr7=0,adcerr8=0,adcerr9=0,adcerr10=0;
UINT16 	alm_ldv_hi, alm_ldv_low;
UINT16 	alm_btv_hi, alm_btv_low ;
UINT16	Cur_Lmt, Mod_Cur_Lmt, Bat_Cur_Lmt,Aci_Lmt;
UINT8	adtimer;

/**
 Section: File specific functions
*/

static void (*ADC1_CommonDefaultInterruptHandler)(void);
static void (*ADC1_ACI_SDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_ACI_RDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_ACV_TDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_ACV_SDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_ACV_RDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_BATT_TEMPDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_BATT_IDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_ACI_TDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_LOAD_VDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_OUT_VDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_LOAD_IDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN16DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN17DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN18DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN19DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_channel_AN20DefaultInterruptHandler)(uint16_t adcVal);

/**
  Section: Driver Interface
*/

void ADC1_Initialize (void)
{
    // ADSIDL disabled; CVDEN disabled; ADON enabled; 
    ADCON1L = (0x8000 & 0x7FFF); //Disabling ADON bit
    // FORM Integer; SHRRES 12-bit resolution; 
    ADCON1H = 0x60;
    // SHRADCS 2; REFCIE disabled; SHREISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; REFERCIE disabled; EIEN disabled; 
    ADCON2L = 0x00;
    // CVDCAP disabled; SHRSAMC 0; 
    ADCON2H = 0x00;
    // SWCTRG disabled; SHRSAMP disabled; SUSPEND disabled; SWLCTRG disabled; SUSPCIE disabled; CNVCHSEL AN0; REFSEL disabled; 
    ADCON3L = 0x00;
    // SHREN enabled; CLKDIV 1; CLKSEL FOSC/2; 
    ADCON3H = (0x80 & 0xFF00); //Disabling C0EN, C1EN, C2EN, C3EN and SHREN bits
    // SIGN0 disabled; SIGN4 disabled; SIGN3 disabled; SIGN2 disabled; SIGN1 disabled; SIGN7 disabled; SIGN6 disabled; SIGN5 disabled; 
    ADMOD0L = 0x00;
    // SIGN10 disabled; SIGN11 disabled; SIGN12 disabled; SIGN13 disabled; SIGN8 disabled; SIGN14 disabled; SIGN15 disabled; SIGN9 disabled; 
    ADMOD0H = 0x00;
    // SIGN20 disabled; SIGN16 disabled; SIGN17 disabled; SIGN18 disabled; SIGN19 disabled; 
    ADMOD1L = 0x00;
    // IE15 disabled; IE1 disabled; IE0 disabled; IE3 disabled; IE2 disabled; IE5 disabled; IE4 disabled; IE10 disabled; IE7 disabled; IE6 disabled; IE9 disabled; IE13 disabled; IE8 disabled; IE14 disabled; IE11 disabled; IE12 disabled; 
    ADIEL = 0x00;
    // IE17 enabled; IE18 enabled; IE16 enabled; IE19 enabled; IE20 enabled; 
    ADIEH = 0x1F;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP0ENL = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP1ENL = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP2ENL = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP3ENL = 0x00;
    // CMPEN20 disabled; CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP0ENH = 0x00;
    // CMPEN20 disabled; CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP1ENH = 0x00;
    // CMPEN20 disabled; CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP2ENH = 0x00;
    // CMPEN20 disabled; CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; 
    ADCMP3ENH = 0x00;
    // CMPLO 0; 
    ADCMP0LO = 0x00;
    // CMPLO 0; 
    ADCMP1LO = 0x00;
    // CMPLO 0; 
    ADCMP2LO = 0x00;
    // CMPLO 0; 
    ADCMP3LO = 0x00;
    // CMPHI 0; 
    ADCMP0HI = 0x00;
    // CMPHI 0; 
    ADCMP1HI = 0x00;
    // CMPHI 0; 
    ADCMP2HI = 0x00;
    // CMPHI 0; 
    ADCMP3HI = 0x00;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL0CON = 0x400;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL1CON = 0x400;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL2CON = 0x400;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL3CON = 0x400;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP0CON = 0x00;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP1CON = 0x00;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP2CON = 0x00;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP3CON = 0x00;
    // LVLEN9 disabled; LVLEN8 disabled; LVLEN11 disabled; LVLEN7 disabled; LVLEN10 disabled; LVLEN6 disabled; LVLEN13 disabled; LVLEN5 disabled; LVLEN12 disabled; LVLEN4 disabled; LVLEN15 disabled; LVLEN3 disabled; LVLEN14 disabled; LVLEN2 disabled; LVLEN1 disabled; LVLEN0 disabled; 
    ADLVLTRGL = 0x00;
    // LVLEN20 disabled; LVLEN17 disabled; LVLEN16 disabled; LVLEN19 disabled; LVLEN18 disabled; 
    ADLVLTRGH = 0x00;
    // EIEN9 disabled; EIEN7 disabled; EIEN8 disabled; EIEN5 disabled; EIEN6 disabled; EIEN3 disabled; EIEN4 disabled; EIEN1 disabled; EIEN2 disabled; EIEN13 disabled; EIEN0 disabled; EIEN12 disabled; EIEN11 disabled; EIEN10 disabled; EIEN15 disabled; EIEN14 disabled; 
    ADEIEL = 0x00;
    // EIEN17 disabled; EIEN16 disabled; EIEN19 disabled; EIEN18 disabled; EIEN20 disabled; 
    ADEIEH = 0x00;
    // SHRCIE disabled; WARMTIME 32768 Source Clock Periods; 
    ADCON5H = (0xF00 & 0xF0FF); //Disabling WARMTIME bit
	
    //Assign Default Callbacks
    ADC1_SetCommonInterruptHandler(&ADC1_CallBack);
    ADC1_SetACI_SInterruptHandler(&ADC1_ACI_S_CallBack);
    ADC1_SetACI_RInterruptHandler(&ADC1_ACI_R_CallBack);
    ADC1_SetACV_TInterruptHandler(&ADC1_ACV_T_CallBack);
    ADC1_SetACV_SInterruptHandler(&ADC1_ACV_S_CallBack);
    ADC1_SetACV_RInterruptHandler(&ADC1_ACV_R_CallBack);
    ADC1_SetBATT_TEMPInterruptHandler(&ADC1_BATT_TEMP_CallBack);
    ADC1_SetBATT_IInterruptHandler(&ADC1_BATT_I_CallBack);
    ADC1_SetACI_TInterruptHandler(&ADC1_ACI_T_CallBack);
    ADC1_SetLOAD_VInterruptHandler(&ADC1_LOAD_V_CallBack);
    ADC1_SetOUT_VInterruptHandler(&ADC1_OUT_V_CallBack);
    ADC1_SetLOAD_IInterruptHandler(&ADC1_LOAD_I_CallBack);
    ADC1_Setchannel_AN16InterruptHandler(&ADC1_channel_AN16_CallBack);
    ADC1_Setchannel_AN17InterruptHandler(&ADC1_channel_AN17_CallBack);
    ADC1_Setchannel_AN18InterruptHandler(&ADC1_channel_AN18_CallBack);
    ADC1_Setchannel_AN19InterruptHandler(&ADC1_channel_AN19_CallBack);
    ADC1_Setchannel_AN20InterruptHandler(&ADC1_channel_AN20_CallBack);
    
    // Clearing channel_AN16 interrupt flag.
    IFS6bits.ADCAN16IF = 0;
    // Enabling channel_AN16 interrupt.
    IEC6bits.ADCAN16IE = 1;
    // Clearing channel_AN17 interrupt flag.
    IFS6bits.ADCAN17IF = 0;
    // Enabling channel_AN17 interrupt.
    IEC6bits.ADCAN17IE = 1;
    // Clearing channel_AN18 interrupt flag.
    IFS6bits.ADCAN18IF = 0;
    // Enabling channel_AN18 interrupt.
    IEC6bits.ADCAN18IE = 1;
    // Clearing channel_AN19 interrupt flag.
    IFS6bits.ADCAN19IF = 0;
    // Enabling channel_AN19 interrupt.
    IEC6bits.ADCAN19IE = 1;
    // Clearing channel_AN20 interrupt flag.
    IFS6bits.ADCAN20IF = 0;
    // Enabling channel_AN20 interrupt.
    IEC6bits.ADCAN20IE = 1;

    // Setting WARMTIME bit
    ADCON5Hbits.WARMTIME = 0xF;
    // Enabling ADC Module
    ADCON1Lbits.ADON = 0x1;
    // Enabling Power for the Shared Core
    ADC1_SharedCorePowerEnable();

    //TRGSRC0 Common Software Trigger; TRGSRC1 Common Software Trigger; 
    ADTRIG0L = 0x101;
    //TRGSRC3 Common Software Trigger; TRGSRC2 Common Software Trigger; 
    ADTRIG0H = 0x101;
    //TRGSRC4 Common Software Trigger; TRGSRC5 None; 
    ADTRIG1L = 0x01;
    //TRGSRC6 None; TRGSRC7 Common Software Trigger; 
    ADTRIG1H = 0x100;
    //TRGSRC8 None; TRGSRC9 Common Software Trigger; 
    ADTRIG2L = 0x100;
    //TRGSRC11 None; TRGSRC10 None; 
    ADTRIG2H = 0x00;
    //TRGSRC13 Common Software Trigger; TRGSRC12 Common Software Trigger; 
    ADTRIG3L = 0x101;
    //TRGSRC15 Common Software Trigger; TRGSRC14 Common Software Trigger; 
    ADTRIG3H = 0x101;
    //TRGSRC17 Common Software Trigger; TRGSRC16 Common Software Trigger; 
    ADTRIG4L = 0x101;
    //TRGSRC19 Common Software Trigger; TRGSRC18 Common Software Trigger; 
    ADTRIG4H = 0x101;
    //TRGSRC20 Common Software Trigger; 
    ADTRIG5L = 0x01;
}

void ADC1_SharedCorePowerEnable ( ) 
{
    ADCON5Lbits.SHRPWR = 1;   
    while(ADCON5Lbits.SHRRDY == 0);
    ADCON3Hbits.SHREN = 1;   
}


void __attribute__ ((weak)) ADC1_CallBack ( void )
{ 

}

void ADC1_SetCommonInterruptHandler(void* handler)
{
    ADC1_CommonDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_Tasks ( void )
{
    if(IFS5bits.ADCIF)
    {
        if(ADC1_CommonDefaultInterruptHandler) 
        { 
            ADC1_CommonDefaultInterruptHandler(); 
        }

        // clear the ADC1 interrupt flag
        IFS5bits.ADCIF = 0;
    }
}

void __attribute__ ((weak)) ADC1_ACI_S_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetACI_SInterruptHandler(void* handler)
{
    ADC1_ACI_SDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_ACI_S_Tasks ( void )
{
    uint16_t valACI_S;

    if(ADSTATLbits.AN0RDY)
    {
        //Read the ADC value from the ADCBUF
        valACI_S = ADCBUF0;

        if(ADC1_ACI_SDefaultInterruptHandler) 
        { 
            ADC1_ACI_SDefaultInterruptHandler(valACI_S); 
        }
    }
}

void __attribute__ ((weak)) ADC1_ACI_R_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetACI_RInterruptHandler(void* handler)
{
    ADC1_ACI_RDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_ACI_R_Tasks ( void )
{
    uint16_t valACI_R;

    if(ADSTATLbits.AN1RDY)
    {
        //Read the ADC value from the ADCBUF
        valACI_R = ADCBUF1;

        if(ADC1_ACI_RDefaultInterruptHandler) 
        { 
            ADC1_ACI_RDefaultInterruptHandler(valACI_R); 
        }
    }
}

void __attribute__ ((weak)) ADC1_ACV_T_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetACV_TInterruptHandler(void* handler)
{
    ADC1_ACV_TDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_ACV_T_Tasks ( void )
{
    uint16_t valACV_T;

    if(ADSTATLbits.AN2RDY)
    {
        //Read the ADC value from the ADCBUF
        valACV_T = ADCBUF2;

        if(ADC1_ACV_TDefaultInterruptHandler) 
        { 
            ADC1_ACV_TDefaultInterruptHandler(valACV_T); 
        }
    }
}

void __attribute__ ((weak)) ADC1_ACV_S_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetACV_SInterruptHandler(void* handler)
{
    ADC1_ACV_SDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_ACV_S_Tasks ( void )
{
    uint16_t valACV_S;

    if(ADSTATLbits.AN3RDY)
    {
        //Read the ADC value from the ADCBUF
        valACV_S = ADCBUF3;

        if(ADC1_ACV_SDefaultInterruptHandler) 
        { 
            ADC1_ACV_SDefaultInterruptHandler(valACV_S); 
        }
    }
}

void __attribute__ ((weak)) ADC1_ACV_R_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetACV_RInterruptHandler(void* handler)
{
    ADC1_ACV_RDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_ACV_R_Tasks ( void )
{
    uint16_t valACV_R;

    if(ADSTATLbits.AN4RDY)
    {
        //Read the ADC value from the ADCBUF
        valACV_R = ADCBUF4;

        if(ADC1_ACV_RDefaultInterruptHandler) 
        { 
            ADC1_ACV_RDefaultInterruptHandler(valACV_R); 
        }
    }
}

void __attribute__ ((weak)) ADC1_BATT_TEMP_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetBATT_TEMPInterruptHandler(void* handler)
{
    ADC1_BATT_TEMPDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_BATT_TEMP_Tasks ( void )
{
    uint16_t valBATT_TEMP;

    if(ADSTATLbits.AN7RDY)
    {
        //Read the ADC value from the ADCBUF
        valBATT_TEMP = ADCBUF7;

        if(ADC1_BATT_TEMPDefaultInterruptHandler) 
        { 
            ADC1_BATT_TEMPDefaultInterruptHandler(valBATT_TEMP); 
        }
    }
}

void __attribute__ ((weak)) ADC1_BATT_I_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetBATT_IInterruptHandler(void* handler)
{
    ADC1_BATT_IDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_BATT_I_Tasks ( void )
{
    uint16_t valBATT_I;

    if(ADSTATLbits.AN9RDY)
    {
        //Read the ADC value from the ADCBUF
        valBATT_I = ADCBUF9;

        if(ADC1_BATT_IDefaultInterruptHandler) 
        { 
            ADC1_BATT_IDefaultInterruptHandler(valBATT_I); 
        }
    }
}

void __attribute__ ((weak)) ADC1_ACI_T_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetACI_TInterruptHandler(void* handler)
{
    ADC1_ACI_TDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_ACI_T_Tasks ( void )
{
    uint16_t valACI_T;

    if(ADSTATLbits.AN12RDY)
    {
        //Read the ADC value from the ADCBUF
        valACI_T = ADCBUF12;

        if(ADC1_ACI_TDefaultInterruptHandler) 
        { 
            ADC1_ACI_TDefaultInterruptHandler(valACI_T); 
        }
    }
}

void __attribute__ ((weak)) ADC1_LOAD_V_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetLOAD_VInterruptHandler(void* handler)
{
    ADC1_LOAD_VDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_LOAD_V_Tasks ( void )
{
    uint16_t valLOAD_V;

    if(ADSTATLbits.AN13RDY)
    {
        //Read the ADC value from the ADCBUF
        valLOAD_V = ADCBUF13;

        if(ADC1_LOAD_VDefaultInterruptHandler) 
        { 
            ADC1_LOAD_VDefaultInterruptHandler(valLOAD_V); 
        }
    }
}

void __attribute__ ((weak)) ADC1_OUT_V_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetOUT_VInterruptHandler(void* handler)
{
    ADC1_OUT_VDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_OUT_V_Tasks ( void )
{
    uint16_t valOUT_V;

    if(ADSTATLbits.AN14RDY)
    {
        //Read the ADC value from the ADCBUF
        valOUT_V = ADCBUF14;

        if(ADC1_OUT_VDefaultInterruptHandler) 
        { 
            ADC1_OUT_VDefaultInterruptHandler(valOUT_V); 
        }
    }
}

void __attribute__ ((weak)) ADC1_LOAD_I_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetLOAD_IInterruptHandler(void* handler)
{
    ADC1_LOAD_IDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_LOAD_I_Tasks ( void )
{
    uint16_t valLOAD_I;

    if(ADSTATLbits.AN15RDY)
    {
        //Read the ADC value from the ADCBUF
        valLOAD_I = ADCBUF15;

        if(ADC1_LOAD_IDefaultInterruptHandler) 
        { 
            ADC1_LOAD_IDefaultInterruptHandler(valLOAD_I); 
        }
    }
}

void __attribute__ ((weak)) ADC1_channel_AN16_CallBack( uint16_t adcVal )
{ 

}

void ADC1_Setchannel_AN16InterruptHandler(void* handler)
{
    ADC1_channel_AN16DefaultInterruptHandler = handler;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN16Interrupt ( void )
{
    uint16_t valchannel_AN16;
    //Read the ADC value from the ADCBUF
    valchannel_AN16 = ADCBUF16;

    if(ADC1_channel_AN16DefaultInterruptHandler) 
    { 
        ADC1_channel_AN16DefaultInterruptHandler(valchannel_AN16); 
    }

    //clear the channel_AN16 interrupt flag
    IFS6bits.ADCAN16IF = 0;
}

void __attribute__ ((weak)) ADC1_channel_AN17_CallBack( uint16_t adcVal )
{ 

}

void ADC1_Setchannel_AN17InterruptHandler(void* handler)
{
    ADC1_channel_AN17DefaultInterruptHandler = handler;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN17Interrupt ( void )
{
    uint16_t valchannel_AN17;
    //Read the ADC value from the ADCBUF
    valchannel_AN17 = ADCBUF17;

    if(ADC1_channel_AN17DefaultInterruptHandler) 
    { 
        ADC1_channel_AN17DefaultInterruptHandler(valchannel_AN17); 
    }

    //clear the channel_AN17 interrupt flag
    IFS6bits.ADCAN17IF = 0;
}

void __attribute__ ((weak)) ADC1_channel_AN18_CallBack( uint16_t adcVal )
{ 

}

void ADC1_Setchannel_AN18InterruptHandler(void* handler)
{
    ADC1_channel_AN18DefaultInterruptHandler = handler;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN18Interrupt ( void )
{
    uint16_t valchannel_AN18;
    //Read the ADC value from the ADCBUF
    valchannel_AN18 = ADCBUF18;

    if(ADC1_channel_AN18DefaultInterruptHandler) 
    { 
        ADC1_channel_AN18DefaultInterruptHandler(valchannel_AN18); 
    }

    //clear the channel_AN18 interrupt flag
    IFS6bits.ADCAN18IF = 0;
}

void __attribute__ ((weak)) ADC1_channel_AN19_CallBack( uint16_t adcVal )
{ 

}

void ADC1_Setchannel_AN19InterruptHandler(void* handler)
{
    ADC1_channel_AN19DefaultInterruptHandler = handler;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN19Interrupt ( void )
{
    uint16_t valchannel_AN19;
    //Read the ADC value from the ADCBUF
    valchannel_AN19 = ADCBUF19;

    if(ADC1_channel_AN19DefaultInterruptHandler) 
    { 
        ADC1_channel_AN19DefaultInterruptHandler(valchannel_AN19); 
    }

    //clear the channel_AN19 interrupt flag
    IFS6bits.ADCAN19IF = 0;
}

void __attribute__ ((weak)) ADC1_channel_AN20_CallBack( uint16_t adcVal )
{ 

}

void ADC1_Setchannel_AN20InterruptHandler(void* handler)
{
    ADC1_channel_AN20DefaultInterruptHandler = handler;
}

void __attribute__ ( ( __interrupt__ , auto_psv, weak ) ) _ADCAN20Interrupt ( void )
{
    uint16_t valchannel_AN20;
    //Read the ADC value from the ADCBUF
    valchannel_AN20 = ADCBUF20;

    if(ADC1_channel_AN20DefaultInterruptHandler) 
    { 
        ADC1_channel_AN20DefaultInterruptHandler(valchannel_AN20); 
    }

    //clear the channel_AN20 interrupt flag
    IFS6bits.ADCAN20IF = 0;
}

unsigned int adcGetValue(ADC1_CHANNEL channel)
{
    unsigned int i;    
    unsigned int retVal;
    ADC1_ChannelSelect(channel);
    ADC1_SoftwareTriggerEnable();    
    for(i=0;i <1000;i++)
    {
    }
    ADC1_SoftwareTriggerDisable();    
    while(!ADC1_IsConversionComplete(channel));
    retVal = ADC1_ConversionResultGet(channel);      
    //
    switch(channel)
    {
        case BATT_TEMP:
            mainValue[0] = retVal;
            break;
        case BATT_I:
            mainValue[1] = retVal;;
            break;
        case LOAD_V:
            mainValue[2] = retVal;;
            break;
        case OUT_V:
            mainValue[3] = retVal;;
            break;
        case LOAD_I:
            mainValue[4] = retVal;;
            break;
        default:
            break;
    }      
    return retVal;
}

void Adc_Sensing()
{
	UINT8 		a,b,c,d;
	UINT16 		AdBuf;
	//------------------------------------------------------
	UINT16 		Loadv_s,Outp_s,Loadi_s,Bati_s,Tmp_s;
	UINT16		pre_Data,new_Data,filter_Data;
	//------------------------------------------------------			        
	Loadv_s		= adcGetValue(LOAD_V);// ADC_LOAD;             //Load Voltage ADC CH
	Outp_s		= adcGetValue(OUT_V);//ADC_OUTP;             //Output Voltage ADC CH
	Loadi_s		= adcGetValue(LOAD_I);//ADC_LOADI;            //Load Current ADC CH
	Bati_s		= adcGetValue(BATT_I);//ADC_BATTI;            //Battary Current ADC CH
	Tmp_s		= adcGetValue(BATT_TEMP);//ADC_TMP;              //Temperature ADC CH
	//------------------------------------------------------
	if(Loadi_s>=10)Loadi_s=Loadi_s - LDI_OFFSET;        // ADC Reference Offset Voltage
    
	// Average Filtering (Normal Average)
	Ad_Sum_Buffer[0]+=(Loadv_s);        // Integrate Load Voltage for Avarage Filter 
	Ad_Sum_Buffer[1]+=(Outp_s);         // Integrate Output Voltage for Avarage Filter 
	ad_cnt++;                           // AD Count Increase
	if(ad_cnt>=32)                      // Filtering No.
	{
        //Average Calculation 
		_avgLdV = (UINT16)(Ad_Sum_Buffer[0]>>5);        // Avg_Load Voltage = Voltage / 2^5(32) 
		_avgOutV = (UINT16)(Ad_Sum_Buffer[1]>>5);       // Avg_Output Voltage = Voltage / 2^5(32)
		ad_cnt=0;                                       // AD count Reset
		Ad_Sum_Buffer[0]=0;                             // AVG Buffer Reset
		Ad_Sum_Buffer[1]=0;                             
	}		

    // Load Current MAF(Moving Average Filter)
	pre_Data = sdiv((long)((ADCNT * _avgLdI)-_avgLdI),ADCNT);		
	new_Data = sdiv((long)(Loadi_s),ADCNT);
	filter_Data = pre_Data + new_Data;
	_avgLdI = filter_Data;				//final value		
	// Battary Current MAF(Moving Average Filter)
	pre_Data = sdiv((long)((ADCNT * _avgBatI)-_avgBatI),ADCNT);		
	new_Data = sdiv((long)(Bati_s),ADCNT);
	filter_Data = pre_Data + new_Data;
	_avgBatI = filter_Data;				//final value
	//Battery Temperature MAF(Moving Average Filter)	
	pre_Data = sdiv((long)((ADCNT * _avgBatTmp)-_avgBatTmp),ADCNT);		
	new_Data = sdiv((long)(Tmp_s),ADCNT);
	filter_Data = pre_Data + new_Data;
	_avgBatTmp = filter_Data;				//final value		
	//=================================================	
     //ADC Converting Real Value - Load Voltage 
	//AdBuf	= 	sdiv((long)_avgLdV*100,167);	
    AdBuf	= 	sdiv((long)_avgLdV*100,LoadV_Gain);	
	avgLdV=AdBuf;
	// Check
	if(adcerr7>CHKERRCNT)//CHKERRCNT is 50
	{
		adcerr7=CHKERRCNT+1;
		Load_Volt=avgLdV*0.1;
	}
	else if(avgLdV>=alm_ldv_hi || avgLdV<=alm_ldv_low)
	{
		++adcerr7;
	}
	else	
	{
		adcerr7=0;
	}
	Load_Volt=avgLdV*0.1;
#if(adcTest == 1)
    //Load_Volt=125.1;
#endif
    
    //ADC Converting Real Value - Output Voltage		
	//AdBuf = sdiv((long)_avgOutV*100,167);	
    AdBuf = sdiv((long)_avgOutV*100,BattV_Gain);	
	avgOutV = AdBuf;
	if(adcerr8>CHKERRCNT)
	{
		adcerr8=CHKERRCNT+1;
		Batt_Volt=avgOutV*0.1;
	}
	else if(avgOutV>=alm_btv_hi || avgOutV<=alm_btv_low)
	{
		++adcerr8;
	}
	else
	{
		adcerr8=0;
		Batt_Volt=avgOutV*0.1;
	}
	Batt_Volt=avgOutV*0.1;
#if(adcTest == 1)
    //Batt_Volt=132.1;
#endif
//-------------------
    //Check Battery Over Voltage
    if(Batt_Volt >= Set_Batt_OV)
    {
        Batt_OV_STS = 1;
    }
    //Check Battery Under Voltage
    else if(Batt_Volt <= Set_Batt_UV)
    {
        Batt_UV_STS = 1;
    }
    else
    {
        Batt_OV_STS = 0;
        Batt_UV_STS = 0;
    }
    
	//ADC Converting Real Value - Load Current
	//AdBuf	= 	sdiv((long)_avgLdI * 10,15);	//100A 
    AdBuf	= 	sdiv((long)_avgLdI * 10,LoadA_Gain);	//100A 
	avgLdI = AdBuf;

	if(adcerr9>CHKERRCNT)
	{
		adcerr9=CHKERRCNT+1;
		a = (avgLdI/1000);b = (avgLdI % 1000)/100;	c = (avgLdI % 100)/10;  d = (avgLdI % 10);
		Load_Amp = avgLdI*0.1;
	}
	else if(avgLdI >=Cur_Lmt)
	{
		++adcerr9;
	}
	else
	{
		adcerr9=0;
		a = (avgLdI/1000);b = (avgLdI % 1000)/100;	c = (avgLdI % 100)/10;  d = (avgLdI % 10);
		Load_Amp = avgLdI*0.1;
	}
    
#if(adcTest == 1)
    //Load_Amp = 56.1;
#endif
    //ADC Converting Real Value - Battery Temp
	//AdBuf	= 	sdiv((long)_avgBatTmp*10,57);	
    AdBuf	= 	sdiv((long)_avgBatTmp*10,BattT_Gain);	
	avgBatTmp = AdBuf;	    
#if(adcTest == 1)
    //avgBatTmp = 55;	    
#endif    
	a = (avgBatTmp/1000);b = (avgBatTmp % 1000)/100;c = (avgBatTmp % 100)/10;d = (avgBatTmp % 10);		
    //
	AdBuf=_avgBatI;
    //Charge state Check - Initial value 2020 
	if(AdBuf >= BATI_OFFSET)		
	{	
		Batt_Mode_STS = 0;			//0 is Charge 
		AdBuf=AdBuf-BATI_OFFSET;
		if(AdBuf<10)
		{	
			AdBuf=0;
		}	
	}	
    //Discharge state
	else    
	{	
		Batt_Mode_STS = 1;			//1 is Dis-Charge
		AdBuf=BATI_OFFSET-AdBuf;
		if(AdBuf<10)
		{	
			AdBuf=0;		
			Batt_Mode_STS=0;
		}	
	}
				
	//ADC Converting Real Value - Battery Current
	//avgBatI = sdiv((long)(AdBuf),16);			//CT 100A		
    avgBatI = sdiv((long)(AdBuf),BattA_Gain);			//CT 100A		

    //Forcing Change Battery Status 
	if(avgBatI <= 1) avgBatI = 0;                   // Forcing Change Zero Current for Minimun Current
	if(avgBatI == 0) Batt_Mode_STS = 0;         	// Battery Status 3-state Define?//
	
	if(adcerr10>CHKERRCNT)
	{
		adcerr10=CHKERRCNT+1;
		a = (avgBatI/1000);
		b = (avgBatI % 1000)/100;	c = (avgBatI % 100)/10;  d = (avgBatI % 10);
		Batt_Amp = avgBatI;
	}
	else if((10*avgBatI) >= Bat_Cur_Lmt)
	{
		++adcerr10;
	}	
	else
	{
		adcerr10=0;
		a = (avgBatI/1000);
		b = (avgBatI % 1000)/100;	c = (avgBatI % 100)/10;  d = (avgBatI % 10);
		Batt_Amp = avgBatI;
	}
	Batt_Amp = avgBatI;
#if(adcTest == 1)
    //Batt_Amp = 23.1;
#endif
	adtimer++;                  //not effect
	if(adtimer>200)adtimer=200; //not effect
}//End Adc_Sensing()//

/**
  End of File
*/
