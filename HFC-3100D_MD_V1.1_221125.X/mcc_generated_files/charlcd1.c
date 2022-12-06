/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
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

#include <stdint.h>
#include <stdbool.h>

#include "mcc.h"
#include "charlcd1.h"
#include "pin_manager.h"

#define FCY (_XTAL_FREQ/2)

#include <libpic30.h>
#include "../var.h"
#include "../definition.h"
#include "../function.h"
#include "uart1.h"
#include "../ds1302.h"
#include "../i2c_master_eeprom_24fc256.h"

extern char lcdStr0[17];
extern char lcdStr1[17];
extern char lcdStr2[17];
extern char lcdStr3[17];
extern char lcdBuffer[17];

/* Private Definitions ***********************************************/
#define FAST_INSTRUCTION_TIME_US    50
#define SLOW_INSTRUCTION_TIME_US    1600
#define STARTUP_TIME_MS             30
#define SIGNAL_TIMING_US            1

#define MAX_COLUMN                  16

enum COMMAND
{
    COMMAND_CLEAR_SCREEN        = 0x01,
    COMMAND_RETURN_HOME         = 0x02,
    COMMAND_ENTER_DATA_MODE     = 0x06,
    COMMAND_CURSOR_OFF          = 0x0C,
    COMMAND_CURSOR_ON           = 0x0F,
    COMMAND_MOVE_CURSOR_LEFT    = 0x10,
    COMMAND_MOVE_CURSOR_RIGHT   = 0x14,
    COMMAND_SET_MODE_4_BIT      = 0x28,
    COMMAND_SET_MODE_8_BIT      = 0x38,
    COMMAND_ROW_0_HOME          = 0x80,
    COMMAND_ROW_1_HOME          = 0xC0,
    //    
    COMMAND_ROW_2_HOME          = 0x90,
    COMMAND_ROW_3_HOME          = 0xD0, 
    //
    COMMAND_START_UP_1          = 0x33,   
    COMMAND_START_UP_2          = 0x32 
};
   
/* Private Functions *************************************************/
static void CarriageReturn( void );
static void ShiftCursorLeft( void );
static void ShiftCursorRight( void );
static void ShiftCursorUp( void );
static void ShiftCursorDown( void );
static void SendData( char );
static void SendCommand( unsigned int );
static void DataNibbleWrite(uint8_t value);

/* Private variables ************************************************/
static uint8_t row;
static uint8_t column;
//

//Global Variable
uint8_t gTmp=0;

//Flag Variable
uint8_t fTimeSet=0;
uint8_t fAlarmStop=0;

uint16_t Temp_LCD_DATA;
uint8_t Temp_CMD;
uint8_t Str_Temp;
uint8_t iMenuIndex=0;
uint8_t iMenuCnt=0;

uint16_t Vvalue;
uint16_t Avalue;
uint8_t KeyValue;
//UINT8 uc_key=0;

int8_t inputCash=0;
uint8_t fPassSet=0;
uint8_t passWord[10]={'0','0','0','0','0','0','0','0','0','0'};
uint8_t SpassWord[10]={'1','1','1','1','1','1','1','1','1','1'};

//AlarmHistory
uint8_t gAlarmHistory[200][9];
uint8_t gAlarmHistoryCnt = 0;
uint8_t alarmList[38]={0,};
uint8_t gAlarmStatus[25] = {0,};
uint8_t gC_AlarmHistoryCnt=0;
uint8_t prev_alarm_name;
uint8_t gOldState = 0;
uint8_t dcGroundCnt = 0;
uint8_t ErrCode=0;
uint8_t ErrCnt;
uint8_t fCheckErr=0;
uint8_t fNfbCheckErr=0;
uint8_t gPacketsize = 0;
//RTC
uint8_t day,mth,year,dow,hour,min,sec,sec_old;

//LED STATUS
uint8_t StatusBuf[20] = {0,};

//Memory Set Data

//NFB STATUS
uint8_t fM1_NFB=0,fM2_NFB=0;

//Buzzer
uint8_t ferrBz[5]={0,};

//Battery Charge,Discharge Flag
uint8_t fCharge=' ';

uint8_t Module1_Sts[8]={0,};
uint8_t Module2_Sts[8]={0,};
uint8_t Battery_Sts[8]={0,};
uint8_t Warning_Sts[8]={0,};
uint8_t ModuleStartup_Sts[2]={0,};
uint8_t Module_CHG_Sts[8]={0,};


uint8_t prev_Module1_Sts[8]={0,};
uint8_t prev_Module2_Sts[8]={0,};
uint8_t prev_Battery_Sts[8]={0,};
uint8_t prev_Warning_Sts[8]={0,};
uint8_t prev_ModuleStartup_Sts[2]={0,};
uint8_t prev_Module_CHG_Sts[8]={0,};
uint8_t fNewErr=0;
uint8_t test[10]={0,};

void createChar(uint8_t location, uint8_t charmap[]) 
{
  location &= 0x7; // we only have 8 locations 0-7
  SendCommand(0x40 | (location << 3));
  for (int i=0; i<8; i++) {
    //write(charmap[i]);
      SendData(charmap[i]);
  }
}
/*********************************************************************
 * Function: bool CHARLCD1_Initialize(void);
 *
 * Overview: Initializes the LCD screen.  Can take several hundred
 *           milliseconds.
 *
 * PreCondition: none
 *
 * Input: None
 *
 * Output: true if initialized, false otherwise
 *
 ********************************************************************/
bool CHARLCD1_Initialize( void )
{          
    DataNibbleWrite(0);
    
    // RW pin is not selected in pin manager
    CharLCD1_RS_SetLow(); 
    CharLCD1_E_SetLow();  
    __delay_us(SIGNAL_TIMING_US);
    CharLCD1_E_SetHigh();
    //__delay_ms(STARTUP_TIME_MS);//30ms//
    __delay_ms(60);
    SendCommand( COMMAND_SET_MODE_4_BIT );
    SendCommand( COMMAND_CURSOR_OFF );
    SendCommand( COMMAND_ENTER_DATA_MODE );

    CHARLCD1_ClearScreen();

    return true;
}



/*********************************************************************
 * Function: void CHARLCD1_PutString(char* inputString, uint16_t length);
 *
 * Overview: Puts a string on the LCD screen.  Unsupported characters will be
 *           discarded.  May block or throw away characters is LCD is not ready
 *           or buffer space is not available.  Will terminate when either a
 *           null terminator character (0x00) is reached or the length number
 *           of characters is printed, which ever comes first.
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: char* - string to print
 *        uint16_t - length of string to print
 *
 * Output: None
 *
 ********************************************************************/
void CHARLCD1_PutString( char* inputString , uint16_t length )
{
    while(length--)
    {
        switch(*inputString)
        {
            case 0x00:
                return;

            default:
                CHARLCD1_PutChar( *inputString++ );
                break;
        }
    }
}
/*********************************************************************
 * Function: void CHARLCD1_PutChar(char);
 *
 * Overview: Puts a character on the LCD screen.  Unsupported characters will be
 *           discarded.  May block or throw away characters is LCD is not ready
 *           or buffer space is not available.
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: char - character to print
 *
 * Output: None
 *
 ********************************************************************/
void CHARLCD1_PutChar( char inputCharacter )
{
    static char lastCharacter = 0;
        
    switch(inputCharacter)
    {
        case '\r':
            if(lastCharacter != '\n')
            {
                CarriageReturn();
            }
            break;

        case '\n': 
            if(lastCharacter != '\r')
            {
                CarriageReturn();
            }
            
            if(row == 0)
            {
                ShiftCursorDown();
            }
            else
            {
                ShiftCursorUp();
            }
            break;

        case '\b':
            ShiftCursorLeft();
            CHARLCD1_PutChar( ' ' );
            ShiftCursorLeft();
            break;
            
        case '\f':
            CHARLCD1_ClearScreen();
            break;

        default:
            
//             if(column == MAX_COLUMN)
//            {
//                column = 0;
//                if(row == 0)
//                {
//                    SendCommand( COMMAND_ROW_1_HOME );
//                    row = 1;
//                }
//                else
//                {
//                    SendCommand( COMMAND_ROW_0_HOME );
//                    row = 0;
//                }
//            }
//            
//            SendData( inputCharacter );
//            column++;
//             
            if(column == MAX_COLUMN)
            {
                column = 0;
                //
                if(row == 0)
                {
                    SendCommand( COMMAND_ROW_1_HOME );
                    row = 1;
                }
                else if(row == 1)
                {
                    SendCommand( COMMAND_ROW_2_HOME );
                    row = 2;
                }
                else if(row == 2)
                {
                    SendCommand( COMMAND_ROW_3_HOME );
                    row = 3;
                }
                else if(row == 3)
                {
                    SendCommand( COMMAND_ROW_0_HOME );
                    row = 0;
                }
            }
            
            SendData( inputCharacter );
            column++;
            break;
    }
    
    lastCharacter = inputCharacter;
}
/*********************************************************************
 * Function: void CHARLCD1_ClearScreen(void);
 *
 * Overview: Clears the screen, if possible.
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
void CHARLCD1_ClearScreen( void )
{
    SendCommand( COMMAND_CLEAR_SCREEN );
    SendCommand( COMMAND_RETURN_HOME );

    row = 0;
    column = 0;
}

/*********************************************************************
 * Function: void CHARLCD1_CursorEnable(bool enable)
 *
 * Overview: Enables/disables the cursor
 *
 * PreCondition: None
 *
 * Input: bool - specifies if the cursor should be on or off
 *
 * Output: None
 *
 ********************************************************************/
void CHARLCD1_CursorEnable( bool enable )
{
    if(enable == true)
    {
        SendCommand( COMMAND_CURSOR_ON );
    }
    else
    {
        SendCommand( COMMAND_CURSOR_OFF );
    }
}

/*******************************************************************/
/*******************************************************************/
/* Private Functions ***********************************************/
/*******************************************************************/
/*******************************************************************/
/*********************************************************************
 * Function: static void CarriageReturn(void)
 *
 * Overview: Handles a carriage return
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void CarriageReturn( void )
{
    if(row == 0)
    {
        SendCommand( COMMAND_ROW_0_HOME );
    }
    else
    {
        SendCommand( COMMAND_ROW_1_HOME );
    }
    column = 0;
}
/*********************************************************************
 * Function: static void ShiftCursorLeft(void)
 *
 * Overview: Shifts cursor left one spot (wrapping if required)
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void ShiftCursorLeft( void )
{
    uint8_t i;

    if(column == 0)
    {
        if(row == 0)
        {
            SendCommand( COMMAND_ROW_1_HOME );
            row = 1;
        }
        else
        {
            SendCommand( COMMAND_ROW_0_HOME );
            row = 0;
        }

        //Now shift to the end of the row
        for(i = 0; i <( MAX_COLUMN - 1 ); i++)
        {
            ShiftCursorRight();
        }
    }
    else
    {
        column--;
        SendCommand( COMMAND_MOVE_CURSOR_LEFT );
    }
}
/*********************************************************************
 * Function: static void ShiftCursorRight(void)
 *
 * Overview: Shifts cursor right one spot (wrapping if required)
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void ShiftCursorRight( void )
{
    SendCommand( COMMAND_MOVE_CURSOR_RIGHT );
    column++;

    if(column == MAX_COLUMN)
    {
        column = 0;
        if(row == 0)
        {
            SendCommand( COMMAND_ROW_1_HOME );
            row = 1;
        }
        else
        {
            SendCommand( COMMAND_ROW_0_HOME );
            row = 0;
        }
    }
}
/*********************************************************************
 * Function: static void ShiftCursorUp(void)
 *
 * Overview: Shifts cursor up one spot (wrapping if required)
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void ShiftCursorUp( void )
{
    uint8_t i;

    for(i = 0; i < MAX_COLUMN; i++)
    {
        ShiftCursorLeft();
    }
}
/*********************************************************************
 * Function: static void ShiftCursorDown(void)
 *
 * Overview: Shifts cursor down one spot (wrapping if required)
 *
 * PreCondition: already initialized via CHARLCD1_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void ShiftCursorDown( void )
{
    uint8_t i;

    for(i = 0; i < MAX_COLUMN; i++)
    {
        ShiftCursorRight();
    }
}

/*********************************************************************
 * Function: static void SendData(char data)
 *
 * Overview: Sends data to LCD
 *
 * PreCondition: None
 *
 * Input: char - contains the data to be sent to the LCD
 *
 * Output: None
 *
 ********************************************************************/
static void SendData( char data )
{
  
    // RW pin is not selected in pin manager 
    CharLCD1_RS_SetHigh();
    
    DataNibbleWrite(data>>4);
    __delay_us(SIGNAL_TIMING_US);
    CharLCD1_E_SetHigh();
    __delay_us(SIGNAL_TIMING_US);
    
    CharLCD1_E_SetLow();
    
    DataNibbleWrite(data);
    __delay_us(SIGNAL_TIMING_US);
    CharLCD1_E_SetHigh();
    __delay_us(SIGNAL_TIMING_US);
    CharLCD1_E_SetLow();
    
    // RS pin is not selected in pin manager
    __delay_us(FAST_INSTRUCTION_TIME_US);
}

/*********************************************************************
 * Function: static void SendCommand(char data)
 *
 * Overview: Sends command to LCD
 *
 * PreCondition: None
 *
 * Input: char - contains the command to be sent to the LCD
 *        unsigned int - has the specific delay for the command
 *
 * Output: None
 *
 ********************************************************************/
static void SendCommand( enum COMMAND command )
{       
    // RW pin is not selected in pin manager 
    CharLCD1_RS_SetLow();
    
    DataNibbleWrite(command >> 4);
    CharLCD1_E_SetHigh();
    __delay_us(SIGNAL_TIMING_US);
    __delay_us(SIGNAL_TIMING_US);
    CharLCD1_E_SetLow();
    
    CharLCD1_RS_SetLow();
    DataNibbleWrite(command);
    CharLCD1_E_SetHigh();
    __delay_us(SIGNAL_TIMING_US);
    __delay_us(SIGNAL_TIMING_US);
    CharLCD1_E_SetLow();

    switch(command)
    {
        case COMMAND_MOVE_CURSOR_LEFT:
        case COMMAND_MOVE_CURSOR_RIGHT:
        case COMMAND_SET_MODE_8_BIT:
		case COMMAND_SET_MODE_4_BIT:
        case COMMAND_CURSOR_OFF:
            __delay_us(FAST_INSTRUCTION_TIME_US);
            break;
            
        case COMMAND_ENTER_DATA_MODE:
        case COMMAND_CLEAR_SCREEN:
        case COMMAND_RETURN_HOME:
        case COMMAND_CURSOR_ON:
        case COMMAND_ROW_0_HOME:
        case COMMAND_ROW_1_HOME:
            //
        case COMMAND_ROW_2_HOME:
        case COMMAND_ROW_3_HOME:
        default:
            __delay_us(SLOW_INSTRUCTION_TIME_US);
            break;
    }
    
}

static void DataNibbleWrite(uint8_t value)
{   
    if(value & 0x01)
    {
        CharLCD1_DB4_SetHigh();
    }
    else
    {
        CharLCD1_DB4_SetLow();
    }
    
    
    if(value & 0x02)
    {
        CharLCD1_DB5_SetHigh();
    }
    else
    {
        CharLCD1_DB5_SetLow();
    }
    
  
    if(value & 0x04)
    {
        CharLCD1_DB6_SetHigh();
    }
    else
    {   
        CharLCD1_DB6_SetLow();
    }
    

    if(value & 0x08)
    {
        CharLCD1_DB7_SetHigh();
    }
    else
    {
        CharLCD1_DB7_SetLow();
        
    }
}

void setcurLCD(unsigned char CurX, unsigned char CurY)
{
	//WriteCmdLCD( 0x80 + CurY * 0x40 + CurX) ;
	
	switch(CurY)
	{
        
//        case 0 : WriteCmdLCD( 0x80 + CurX) ;break;
//		case 1 : WriteCmdLCD( 0xc0 + CurX) ;break;
//		case 2 : WriteCmdLCD( 0x90 + CurX) ;break;
//		case 3 : WriteCmdLCD( 0xd0 + CurX) ;break;
        
        case 0 : SendCommand( 0x80 + CurX) ;break;
		case 1 : SendCommand( 0xc0 + CurX) ;break;
		case 2 : SendCommand( 0x90 + CurX) ;break;
		case 3 : SendCommand( 0xd0 + CurX) ;break;		
	}
	LCD_S_Delay() ;
}

void LCD_S_Delay(void)
{
	int		S_Loop ;
	int		Null_Var ;

	for  	( S_Loop = 0 ; S_Loop < 200 ; S_Loop ++ )
				Null_Var += 1 ;
		
}
void customChar(unsigned char index)
{
    SendData(index);
}
//==============================================================================
//<LCD function>

void MainProcess()
{
		switch(iMenuIndex)
		{
			case M0_LOGO:
				DispLogo();
				break;
			case M0_OUTBATT:
				DispOutBatt();
				break;
			case M0_LOAD:
				DispLoad();
				break;
			case M0_SETUP:
				DispSetup();
				break;
			case M1_SETUPSET:
				DispSetupSet();
				break;
			case M1_BATTALARM:                
				DispBattAlarm();
				break;
			case M2_BATTALARMSET:
                DispBattAlarmSet();
				break;
			case M1_MODULERESTART:
				DispModuleRestart();
				break;
			case M2_MODULERESTARTSET:
				DispModuleRestartSet();
				break;
			case M1_SYSTEMINIT:
				DispSystemInit();
				break;
			case M2_SYSTEMINITSET:
				DispSystemInitSet();
				break;
			case M1_PASSWORD:
				DispPassword();
				break;
			case M2_PASSWORDSET:
				DispPasswordSet();
				break;
			case M1_ALARMSTOP:
				DispAlarmStop();
				break;
			case M2_ALARMSTOPSET:
				DispAlarmStopSet();
				break;
			case M0_HISTORY:
				DispHistory();
				break;
			case M0_TIME:
				DispTime();
				break;
			case M1_TIMESET:
				DispTimeSet();
				break;	
            case M3_System:
				SysSetup();
				break;
            case M4_SystemSet:
				SysSetupSet();
				break;
            case M3_BATTTYPE:
                DispBattType();
                break;
            case M4_BATTTYPESET:
                DispBattTypeSet();
                break;
            case M3_INPUTVOLT:
                DispInputVoltType();
                break;
            case M4_INPUTVOLTSET:
				DispInputVoltTypeSet();
				break;
            case M3_OPERATIONMODE:
                DispOperationMode();
                break;
            case M4_OPERATIONMODESET:
				DispOperationModeSet();
				break;
            case M3_SYSTYPE:
                DispSysType();
                break;
            case M4_SYSTYPESET:
                DispSysTypeSet();
                break;                
            //
			default: DispLogo();
				break;
		}	// END switch (iMenuIndex)
}
void DispLogo()
{                 //"0123456789abcdef" 
  //sprintf(lcdStr0,"HFC-3100D[12:59]");	
    sprintf(lcdStr0,"  [HFC-3100D]   ");	
	sprintf(lcdStr1,"Digital Charger ");	
	sprintf(lcdStr2,"Seoul Elec.Power");	
	sprintf(lcdStr3,"Tel)032-681-8411");
    //keyTest();
    switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M0_TIME;
			break;
		case keyKEY_DOWN:
			iMenuIndex = M0_OUTBATT;
			break;
		case keyKEY_LEFT:
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTERfast:
			break;
        case keyKEY_ENTER:
            break;            
		default:
			break;			
	}

}

void DispOutBatt()
{  
	UINT8 dg[10]={0,};
	dg[0]=((BattVoltage*10)/1000);
	dg[1]=(((BattVoltage*10)-(dg[0]*1000))/100);
	dg[2]=(((BattVoltage*10)-(dg[0]*1000))-(dg[1]*100))/10;
	dg[3]=((BattVoltage*10)-(dg[0]*1000))-(dg[1]*100)-(dg[2]*10);
	dg[4]=((BattCurrent*10)/1000);
	dg[5]=(((BattCurrent*10)-(dg[4]*1000))/100);
	dg[6]=(((BattCurrent*10)-(dg[4]*1000))-(dg[5]*100))/10;
	dg[7]=((BattCurrent*10)-(dg[4]*1000))-(dg[5]*100)-(dg[6]*10);
    //
                  //"0123456789abcdef"
    //sprintf(lcdStr0,"*Batt.Volt&Amp* ");    
			      //"0123456789abcdef"
    sprintf(lcdStr0,"1]Batt.Volt&Amp ");    
                  //"ReStart:001bcdef"  
    sprintf(lcdStr1,"                ");	
	              //"0123456789abcdef"  
	sprintf(lcdStr2,"Batt:   . V    A");	
	sprintf(lcdStr3,"                ");
    //sprintf(lcdStr3,"ReStart:%03d     ",RestartTimeValue);	
    
	lcdStr2[5]=dg[0]+'0';
	lcdStr2[6]=dg[1]+'0';
	lcdStr2[7]=dg[2]+'0';
	lcdStr2[9]=dg[3]+'0';
	lcdStr2[11]=fCharge;
	lcdStr2[12]=dg[4]+'0';
	lcdStr2[13]=dg[5]+'0';
	lcdStr2[14]=dg[6]+'0';
	//
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M0_LOGO;
			break;
		case keyKEY_DOWN:
			iMenuIndex = M0_LOAD;
			break;
		case keyKEY_LEFT:
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		default:
			break;
	}    
}

void DispLoad()
{
	UINT8 dg[10]={0,};
	dg[0]=((LoadVoltage*10)/1000);
	dg[1]=(((LoadVoltage*10)-(dg[0]*1000))/100);
	dg[2]=(((LoadVoltage*10)-(dg[0]*1000))-(dg[1]*100))/10;
	dg[3]=((LoadVoltage*10)-(dg[0]*1000))-(dg[1]*100)-(dg[2]*10);
	dg[4]=((LoadCurrent*10)/1000);
	dg[5]=(((LoadCurrent*10)-(dg[4]*1000))/100);
	dg[6]=(((LoadCurrent*10)-(dg[4]*1000))-(dg[5]*100))/10;
	dg[7]=((LoadCurrent*10)-(dg[4]*1000))-(dg[5]*100)-(dg[6]*10);
    //------------//"0123456789abcdef"
     setcurLCD(0, 0);
  //sprintf(lcdStr0,"*Load Volt&Amp* ");
    sprintf(lcdStr0,"2]Load Volt&Amp ");
	sprintf(lcdStr1,"                ");		
	sprintf(lcdStr2,"Load:   . V    A");	
	sprintf(lcdStr3,"                ");    
    /*
     setcurLCD(0, 0);
	sprintf(lcdStr1,"*Load Volt&Amp* ");
	//
	setcurLCD(0, 1);
	sprintf(lcdStr1,"                ");
	//
	setcurLCD(0, 2);
	sprintf(lcdStr1,"Load:   . V    A");
	setcurLCD(5, 2);
	CHARLCD1_PutChar(dg[0]+0x30);
	CHARLCD1_PutChar(dg[1]+0x30);
	CHARLCD1_PutChar(dg[2]+0x30);
	setcurLCD(9, 2);
	CHARLCD1_PutChar(dg[3]+0x30);
	setcurLCD(12, 2);
	CHARLCD1_PutChar(dg[4]+0x30);
	CHARLCD1_PutChar(dg[5]+0x30);
	CHARLCD1_PutChar(dg[6]+0x30);
	//
	setcurLCD(0, 3);
	sprintf(lcdStr1,"                ");
    */
    lcdStr2[5]=dg[0]+'0';
	lcdStr2[6]=dg[1]+'0';
	lcdStr2[7]=dg[2]+'0';
	//'.'
	lcdStr2[9]=dg[3]+'0';
	//' ' 
	lcdStr2[12]=dg[4]+'0';
	lcdStr2[13]=dg[5]+'0';
	lcdStr2[14]=dg[6]+'0';
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M0_OUTBATT;
			break;
		case keyKEY_DOWN:
			iMenuIndex = M0_SETUP;
			break;
		case keyKEY_LEFT:
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		default:
			break;
	}
}

void DispSetup()
{    
	fTimeSet = 0;
	//------------//"0123456789abcdef"
	sprintf(lcdStr0,"3]System Setup  ");
    sprintf(lcdStr1,"                ");	
	sprintf(lcdStr2,"  [*********]   ");	
	sprintf(lcdStr3,"                ");
    //
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M0_LOAD;
			break;
		case keyKEY_DOWN:
			iMenuIndex = M0_HISTORY;
			break;
		case keyKEY_LEFT:
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTER:
			iMenuIndex = M1_SETUPSET;
			iMenuCnt=0;
			inputCash='0';
			break;	
		default:
			break;
	}
}

void DispSetupSet()
{
	UINT8 i;
	//------------//"0123456789abcdef"    
	//sprintf(lcdStr0,"  ** Setup **   ");
    sprintf(lcdStr0,"3]System Setup  ");    
    sprintf(lcdStr1,"                ");	
	sprintf(lcdStr2,"  [*********]   ");	
	sprintf(lcdStr3,"                ");
    //lcdStr3[15] = iMenuCnt + '0';
    lcdStr2[iMenuCnt+3] = passWord[iMenuCnt];
	switch(hKey())
	{
		case keyKEY_UP:
			inputCash++;
			break;
		case keyKEY_UPfast:
			inputCash++;
			break;	
		case keyKEY_DOWN:
			inputCash--;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;	
		case keyKEY_ENTER:
			if(iMenuCnt == 8)
			{
				fPassSet=1;                
			}
			else if(iMenuCnt == 7)
			{
				inputCash=passWord[8];
				iMenuCnt = 8;
			}
			else if(iMenuCnt == 6)
			{
				inputCash=passWord[7];
				iMenuCnt = 7;
			}
			else if(iMenuCnt == 5)
			{
				inputCash=passWord[6];
				iMenuCnt = 6;
			}
			else if(iMenuCnt == 4)
			{
				inputCash=passWord[5];
				iMenuCnt = 5;
			}
			else if(iMenuCnt == 3)
			{
				inputCash=passWord[4];
				iMenuCnt = 4;
			}
			else if(iMenuCnt == 2)
			{
				inputCash=passWord[3];
				iMenuCnt = 3;
			}
			else if(iMenuCnt == 1)
			{
				inputCash=passWord[2];
				iMenuCnt = 2;
			}
			else if(iMenuCnt == 0)
			{
				inputCash=passWord[1];
				iMenuCnt = 1;
			}
            return;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
    //
	if(iMenuCnt == 0)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[0] = inputCash;
	}
	else if(iMenuCnt == 1)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[1] = inputCash;
	}
	else if(iMenuCnt == 2)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[2] = inputCash;
	}
	else if(iMenuCnt == 3)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[3] = inputCash;
	}
	else if(iMenuCnt == 4)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[4] = inputCash;
	}
	else if(iMenuCnt == 5)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[5] = inputCash;
	}
	else if(iMenuCnt == 6)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[6] = inputCash;
	}
	else if(iMenuCnt == 7)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[7] = inputCash;
	}
	else if(iMenuCnt == 8)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		passWord[8] = inputCash;
	}
    //---------------------------------------
	if(fPassSet == 1)
	{
		i = 0;
		for(gCnt=0;gCnt<9;gCnt++)
		{
			data_8 = gPass[gCnt];
			buff_8 = passWord[gCnt];
			i += (data_8 ^ buff_8);
		}
        //
		if(i == 0)
		{
			fTimeSet = 1;
			iMenuCnt=0;
			fPassSet=0;
			iMenuIndex = M1_BATTALARM;//DispBattAlarm()//
		}        
		else if(tmr1s>1)
		{
			tmr1s=0;
			for(gCnt=0;gCnt<9;gCnt++)
			{
				SpassWord[gCnt]='0';
			}
			tmr1s=0;
			fPassSet=0;
			iMenuIndex = M0_SETUP;			
			sprintf(lcdStr1,"                ");
			sprintf(lcdStr3,"                ");
		}        
		else
		{
			sprintf(lcdStr2,"  [*********]   ");		
			//sprintf(lcdStr2," PWD Not Match! ");		
			sprintf(lcdStr3," Not Match !!!  ");
		}
	//iMenuIndex[0] = M0_SETUP;
	}
	if(cursor1s>1)
	{
		cursor1s=0;
		//fPassSet=0;
	}
}//end DispSetupSet()

void SysSetup()
{                 //"0123456789abcdef"
    sprintf(lcdStr0,"S0]Debug        ");    	
	sprintf(lcdStr1,"                ");
	sprintf(lcdStr2,"  [*********]   ");
    sprintf(lcdStr3,"                ");
	//
	switch(hKey())
	{
		case keyKEY_UP:
            iMenuIndex = M1_ALARMSTOP;
			break;
		case keyKEY_DOWN:
            iMenuIndex = M1_BATTALARM;
			break;
		case keyKEY_LEFT:
            iMenuIndex = M0_SETUP;
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTER:
			iMenuIndex = M4_SystemSet;
			iMenuCnt=0;
			inputCash='0';
			break;	
		default:
			break;
	}
}

void SysSetupSet()
{
	UINT8 i;
	//sprintf(lcdStr0,"  ** Debug **   ");    	
    sprintf(lcdStr0,"S0]Debug        ");    	
	sprintf(lcdStr1,"                ");
	sprintf(lcdStr2,"  [*********]   ");
    sprintf(lcdStr3,"                ");
    lcdStr2[iMenuCnt+3] = SpassWord[iMenuCnt];    
	//
	switch(hKey())
	{
		case keyKEY_UP:
			inputCash++;
			break;
		case keyKEY_UPfast:
			inputCash++;
			break;	
		case keyKEY_DOWN:
			inputCash--;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;	
		case keyKEY_ENTER:
			if(iMenuCnt == 8)
			{
				fPassSet=1;
			}
			else if(iMenuCnt == 7)
			{
				inputCash=SpassWord[8];
				iMenuCnt = 8;
			}
			else if(iMenuCnt == 6)
			{
				inputCash=SpassWord[7];
				iMenuCnt = 7;
			}
			else if(iMenuCnt == 5)
			{
				inputCash=SpassWord[6];
				iMenuCnt = 6;
			}
			else if(iMenuCnt == 4)
			{
				inputCash=SpassWord[5];
				iMenuCnt = 5;
			}
			else if(iMenuCnt == 3)
			{
				inputCash=SpassWord[4];
				iMenuCnt = 4;
			}
			else if(iMenuCnt == 2)
			{
				inputCash=SpassWord[3];
				iMenuCnt = 3;
			}
			else if(iMenuCnt == 1)
			{
				inputCash=SpassWord[2];
				iMenuCnt = 2;
			}
			else if(iMenuCnt == 0)
			{
				inputCash=SpassWord[1];
				iMenuCnt = 1;
			}
			return;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
	if(iMenuCnt == 0)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[0] = inputCash;
	}
	else if(iMenuCnt == 1)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[1] = inputCash;
	}
	else if(iMenuCnt == 2)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[2] = inputCash;
	}
	else if(iMenuCnt == 3)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[3] = inputCash;
	}
	else if(iMenuCnt == 4)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[4] = inputCash;
	}
	else if(iMenuCnt == 5)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[5] = inputCash;
	}
	else if(iMenuCnt == 6)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[6] = inputCash;
	}
	else if(iMenuCnt == 7)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[7] = inputCash;
	}
	else if(iMenuCnt == 8)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		SpassWord[8] = inputCash;
	}
	if(fPassSet == 1)
	{
		//iMenuCnt=0;
		i = 0;
		for(gCnt=0;gCnt<9;gCnt++)
		{
			data_8 = SgPass[gCnt];
			buff_8 = SpassWord[gCnt];
			i += (data_8 ^ buff_8);
		}
		if(i == 0)
		{
			iMenuCnt=0;
			fPassSet=0;
			iMenuIndex = M3_BATTTYPE;
		}
		else if(tmr1s>1)
		{
			tmr1s=0;
			for(gCnt=0;gCnt<9;gCnt++)
			{
				SpassWord[gCnt]='0';
			}
			tmr1s=0;
			fPassSet=0;
			iMenuIndex = M3_System;			
			sprintf(lcdStr1,"                ");
			sprintf(lcdStr3,"                ");
		}
		else
		{
			sprintf(lcdStr1,"                ");			
			sprintf(lcdStr2,"  [*********]   ");						
			sprintf(lcdStr3," Not Match !!!  ");
		}
	//iMenuIndex[0] = M0_SETUP;
	}
	if(cursor1s>1)
	{
		cursor1s=0;
		//fPassSet=0;
	}
}

void DispBattAlarm()
{
    //UINT8 tmp;
	//tmp=0;
    sprintf(lcdStr0,"S1]Batt.Alarm   ");        
    sprintf(lcdStr1,"                ");        
    sprintf(lcdStr2,"Max :      V    ");
    sprintf(lcdStr3,"Min :      V    ");
    //
    if(EEPROMRead(BATT_TYPE) == 0)    
    {        
        //sprintf(lcdStr0,"** Batt.Alarm **");        
        sprintf(lcdStr0,"S1]Batt.Alarm0  ");
        lcdStr2[8] = gBattVoltSet[0];
        lcdStr2[9] = gBattVoltSet[1];
        lcdStr2[10] = gBattVoltSet[2];
        //
        lcdStr3[8] = gBattVoltSet[4];
        lcdStr3[9] = gBattVoltSet[5];
        lcdStr3[10] = gBattVoltSet[6];        
    }
    else if(EEPROMRead(BATT_TYPE) == 1)    
    {
        sprintf(lcdStr0,"S1]Batt.Alarm1  ");
		lcdStr2[8] = gBattVoltSet[0];
        lcdStr2[9] = gBattVoltSet[1];
        lcdStr2[10] = gBattVoltSet[2];
        //
        lcdStr3[8] = gBattVoltSet[4];
        lcdStr3[9] = gBattVoltSet[5];
        lcdStr3[10] = gBattVoltSet[6];        
    }
    else if(EEPROMRead(BATT_TYPE) == 2)
    {        
        sprintf(lcdStr0,"S1]Batt.Alarm2  ");        
        sprintf(lcdStr1,"                ");        
        sprintf(lcdStr2,"Max :      .  V ");
        sprintf(lcdStr3,"Min :      .  V ");
        
		lcdStr2[8] = gBattVoltSet[0];
        lcdStr2[9] = gBattVoltSet[1];
        lcdStr2[10] = gBattVoltSet[2];
		//'
		lcdStr2[12] = gBattVoltSet[3];
        //--------------------------------
        lcdStr3[8] = gBattVoltSet[4];
        lcdStr3[9] = gBattVoltSet[5];
        lcdStr3[10] = gBattVoltSet[6];        
		//
		lcdStr3[12] = gBattVoltSet[7];       
    }
	//
	switch(hKey())
	{
		case keyKEY_UP:
            iMenuIndex = M3_System;
			break;
		case keyKEY_DOWN:
			iMenuIndex = M1_MODULERESTART;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M0_SETUP;
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTER:
			iMenuIndex = M2_BATTALARMSET;
			iMenuCnt=0;
			inputCash=gBattVoltSet[0];
			break;	
		default:
			break;
	}
}

void DispBattAlarmSet()
{
	UINT8 i;
   	int tmpValue=0;
	//int tmpvalue = 0;
    if(EEPROMRead(BATT_TYPE) == 0)      //pb125v  
    {
        //sprintf(lcdStr0,"** Batt.Alarm **");        
        sprintf(lcdStr0,"S1]Batt.Alarm0  ");        
        sprintf(lcdStr1,"                ");        
        //
        sprintf(lcdStr2,"Max :   ___V    ");
        lcdStr2[8]=gBattVoltSet[0];
        lcdStr2[9]=gBattVoltSet[1];
        lcdStr2[10]=gBattVoltSet[2];
        //
        sprintf(lcdStr3,"Min :   ___V    ");
        
        lcdStr3[8]=gBattVoltSet[4];
        lcdStr3[9]=gBattVoltSet[5];
        lcdStr3[10]=gBattVoltSet[6];
     
        lcdBlink++;                        
        //--------------------
        switch(iMenuCnt)
        {
            case 0 :
                {
                    if(lcdBlink & 0x0002)
                    {
                        lcdStr2[8] = gBattVoltSet[0];// + '0';
                    }
                    else// if(cursor100ms<6)
                    {
                        //setcurLCD(8, 2);
                        //CHARLCD1_PutChar('_');
                        lcdStr2[8]='_';
                    }
                }
                break;
            case 1 :
                {
                    if(lcdBlink & 0x0002)
                    {
                        lcdStr2[9]=gBattVoltSet[1];
                    }
                    else// if(cursor100ms)
                    {                     
                        lcdStr2[9]='_';
                    }
                }
                break;
            case 2 :
                {
                    if(lcdBlink & 0x0002)
                    {
                        
                        lcdStr2[10]=gBattVoltSet[2];
                    }
                    else// if(cursor100ms<6)
                    {
                        lcdStr2[10]='_';
                    }
                }
                break;
            case 3 :
                {
                    if(lcdBlink & 0x0002)
                    {
                        lcdStr3[8]=gBattVoltSet[4];
                    }
                    else
                    {
                        lcdStr3[8]='_';
                    }
                }
                break;
            case 4 :
                {
                    if(lcdBlink & 0x0002)
                    {
                        lcdStr3[9]=gBattVoltSet[5];
                    }
                    else// if(cursor100ms<6)
                    {
                        lcdStr3[9]='_';
                    }
                }
                break;
            case 5 :
                {
                    if(lcdBlink & 0x0002)
                    {
                        lcdStr3[10]=gBattVoltSet[6];
                    }
                    else// if(cursor100ms<6)
                    {
                        lcdStr3[10]='_';
                    }
                }
                break;
            default :
                break;
        }
        switch(hKey())
        {
            case keyKEY_UP:
                inputCash++;
                break;
            case keyKEY_UPfast:
                inputCash++;
                break;	
            case keyKEY_DOWN:
                inputCash--;
                break;
            case keyKEY_DOWNfast:
                inputCash--;
                break;
            case keyKEY_LEFT:
                iMenuIndex = M1_BATTALARM;
                break;
            case keyKEY_ENTER:
                if(iMenuCnt == 5)
                {
                    for(i = 0; i < 8; i++)
                    {
                        EEPROMWrite(BATTVOLT_0+i, gBattVoltSet[0+i]);
                    }
                    iMenuIndex = M1_BATTALARM;
                    //
                    tmpValue = (gBattVoltSet[0]-0x30)*1000 + (gBattVoltSet[1]-0x30)*100  + (gBattVoltSet[2]-0x30)*10 + (gBattVoltSet[3]-0x30);
                    BattHiVoltage = tmpValue * 0.1;  //  xxxx -> xxx.x
                    tmpValue = (gBattVoltSet[4]-0x30)*1000 + (gBattVoltSet[5]-0x30)*100  + (gBattVoltSet[6]-0x30)*10 + (gBattVoltSet[7]-0x30);
                    BattLowVoltage = tmpValue * 0.1;  //  xxxx -> xxx.x
                    Set_OK=1;
                }
                else if(iMenuCnt == 4)
                {
                    inputCash = gBattVoltSet[6];
                    iMenuCnt = 5;
                }
                else if(iMenuCnt == 3)
                {
                    inputCash=gBattVoltSet[5];
                    iMenuCnt = 4;
                }
                else if(iMenuCnt == 2)
                {
                    inputCash=gBattVoltSet[4];
                    iMenuCnt = 3;
                }
                else if(iMenuCnt == 1)
                {
                    inputCash=gBattVoltSet[2];
                    iMenuCnt = 2;
                }
                else if(iMenuCnt == 0)
                {
                    inputCash=gBattVoltSet[1];
                    iMenuCnt = 1;
                }
                return;
                break;
            case keyKEY_BZ_OFF:
                break;
            case keyKEY_BZ_OFFfast:
                break;
        }
        if(iMenuCnt == 0)//max Nxx
        {
            inputCash = '1';
            gBattVoltSet[0] = inputCash;
        }

        else if(iMenuCnt == 1)//max xNx 127~150
        {
            if(inputCash > '5') inputCash = '2';
            if(inputCash < '2') inputCash = '5';
            gBattVoltSet[1] = inputCash;
        }
        else if(iMenuCnt == 2) //max xxN 
        {
            if(gBattVoltSet[1] <= '2')
            {
                if(inputCash > '9') inputCash = '7';
                if(inputCash < '7') inputCash = '9';
            }
            else if(gBattVoltSet[1] >= '5')
            {
                inputCash = '0';
            }            
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[2] = inputCash;
        }//------------------------------------------------------------
        else if(iMenuCnt == 3)//min Nxx 95~120
        {
            if(inputCash > '1') inputCash = '0';
            if(inputCash < '0') inputCash = '1';
            gBattVoltSet[4] = inputCash;
        }
        else if(iMenuCnt == 4)//min xNx
        {
            if(gBattVoltSet[4] <= '0')
            {
                inputCash = '9';                 
            }
            else if(gBattVoltSet[4] >= '1')
            {
                if(inputCash > '2') inputCash = '0';
                if(inputCash < '0') inputCash = '2';                 
            }
            gBattVoltSet[5] = inputCash;                
        }
        else if(iMenuCnt == 5)
        {
            if(gBattVoltSet[4] >= '1' && gBattVoltSet[5] >= '2')
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            else if(gBattVoltSet[4] <= '0' && gBattVoltSet[5] <= '9')
            {
                if(inputCash > '9') inputCash = '5';
                if(inputCash < '5') inputCash = '9';
            }
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[6] = inputCash;
        }      
    }
    else if(EEPROMRead(BATT_TYPE) == 1)//pb110V    
    {
        //sprintf(lcdStr0,"** Batt.Alarm **");        
        sprintf(lcdStr0,"S1]Batt.Alarm1  ");
        sprintf(lcdStr1,"                ");        
                       //"0123456789abcdef"
        sprintf(lcdStr2,"Max :   ___V    ");
        lcdStr2[8] = gBattVoltSet[0];
        lcdStr2[9] = gBattVoltSet[1];
        lcdStr2[10] = gBattVoltSet[2];
                      //"0123456789abcdef"  
        sprintf(lcdStr3,"Min :   ___V    ");
        
        lcdStr3[8] = gBattVoltSet[4];
        lcdStr3[9] = gBattVoltSet[5];
        lcdStr3[10] = gBattVoltSet[6];
		lcdBlink++;
        switch(iMenuCnt)
        {
            case 0 :
                {
                   	if(lcdBlink & 0x0002)
                    {
						lcdStr2[8] = gBattVoltSet[0];
                    }
                    else//if(cursor100ms>=4)
                    {
						lcdStr2[8] = '_';
                    }
                }
                break;
            case 1 :
                {
                    if(lcdBlink & 0x0002)//
                    {
						lcdStr2[9] = gBattVoltSet[1];
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr2[9] = '_';
                    }
                }
                break;
            case 2 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr2[10] = gBattVoltSet[2];
                    }
                    else
                    {
						lcdStr2[10] = '_';
                    }
                }
                break;
            case 3 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr3[8] = gBattVoltSet[4];
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr3[8] = '_';
                    }
                }
                break;
            case 4 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr3[9] = gBattVoltSet[5];
                    }
                    else //if(cursor100ms>=2)
                    {
						lcdStr3[9] = '_';
                    }
                }
                break;
            case 5 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr3[10] = gBattVoltSet[6];
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr3[10] = '_';
                    }
                }
                break;
            default :
                break;
        }
        switch(hKey())
        {
            case keyKEY_UP:
                inputCash++;
                break;
            case keyKEY_UPfast:
                inputCash++;
                break;	
            case keyKEY_DOWN:
                inputCash--;
                break;
            case keyKEY_DOWNfast:
                inputCash--;
                break;
            case keyKEY_LEFT:
                iMenuIndex = M1_BATTALARM;
                break;
            case keyKEY_ENTER:
                if(iMenuCnt == 5)
                {
                    for(i = 0; i < 8; i++)
                    {
                        EEPROMWrite(BATTVOLT_0+i, gBattVoltSet[0+i]);
                    }
                    iMenuIndex = M1_BATTALARM;

                    tmpValue = (gBattVoltSet[0]-0x30)*1000 + (gBattVoltSet[1]-0x30)*100  + (gBattVoltSet[2]-0x30)*10 + (gBattVoltSet[3]-0x30)*1;
                    BattHiVoltage = tmpValue * 0.1;  //  xxxx -> xxx.x
                    tmpValue = (gBattVoltSet[4]-0x30)*1000 + (gBattVoltSet[5]-0x30)*100  + (gBattVoltSet[6]-0x30)*10 + (gBattVoltSet[7]-0x30)*1;
                    BattLowVoltage = tmpValue * 0.1;  //  xxxx -> xxx.x
                    Set_OK=1;
                }
                else if(iMenuCnt == 4)
                {
                    inputCash=gBattVoltSet[6];
                    iMenuCnt = 5;
                }
                else if(iMenuCnt == 3)
                {
                    inputCash=gBattVoltSet[5];
                    iMenuCnt = 4;
                }
                else if(iMenuCnt == 2)
                {
                    inputCash=gBattVoltSet[4];
                    iMenuCnt = 3;
                }
                else if(iMenuCnt == 1)
                {
                    inputCash=gBattVoltSet[2];
                    iMenuCnt = 2;
                }
                else if(iMenuCnt == 0)
                {
                    inputCash=gBattVoltSet[1];
                    iMenuCnt = 1;
                }
                return;
                break;
            case keyKEY_BZ_OFF:
                break;
            case keyKEY_BZ_OFFfast:
                break;
        }
        if(iMenuCnt == 0)//PB110V max 120~150V
        {
            inputCash = '1';
            gBattVoltSet[0] = inputCash;
        }

        else if(iMenuCnt == 1)
        {
            if(inputCash > '5') inputCash = '2';
            if(inputCash < '2') inputCash = '5';
            gBattVoltSet[1] = inputCash;
        }
        else if(iMenuCnt == 2)
        {
            if(gBattVoltSet[1] <= '2')
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            else if(gBattVoltSet[1] >= '5')
            {
                inputCash = '0';
            }            
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[2] = inputCash;
        }
        else if(iMenuCnt == 3) //pb110v min 95~120//
        {
            if(inputCash > '1') inputCash = '0';
            if(inputCash < '0') inputCash = '1';
            gBattVoltSet[4] = inputCash;
        }
        else if(iMenuCnt == 4)//pb110v min xNx
        {
            if(gBattVoltSet[4] <= '0')inputCash = '9';
            if(gBattVoltSet[4] >= '1')
            {
                if(inputCash > '2') inputCash = '0';
                if(inputCash < '0') inputCash = '2';                 
            }
            gBattVoltSet[5] = inputCash;                
        }
        else if(iMenuCnt == 5)//pb110 min xxN
        {
            if(gBattVoltSet[4] >= '1' && gBattVoltSet[5] >= '2')
            {
                inputCash = '0';
            }
//            else if(gBattVoltSet[4] <= '1' && gBattVoltSet[5] <= '0')
//            {
//                if(inputCash > '9') inputCash = '5';
//                if(inputCash < '5') inputCash = '5';
//            }                
//            else
//            {
//                if(inputCash > '9') inputCash = '0';
//                if(inputCash < '0') inputCash = '9';
//            }
            else if(gBattVoltSet[4] == '0')
            {
                if(inputCash > '9') inputCash = '5';
                if(inputCash < '5') inputCash = '9';
            }
            else
            {    
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[6] = inputCash;
        }  
    }
    else if(EEPROMRead(BATT_TYPE) == 2) //LI ion    
    {
        //sprintf(lcdStr0,"** Batt.Alarm **");        
        sprintf(lcdStr0,"S1]Batt.Alarm2  ");        
        sprintf(lcdStr1,"                ");   
                      //"0123456789abcdef
        sprintf(lcdStr2,"Max :      .  V ");
        lcdStr2[8] = gBattVoltSet[0];
        lcdStr2[9] = gBattVoltSet[1];
        lcdStr2[10] = gBattVoltSet[2];
        lcdStr2[12] = gBattVoltSet[3];
                      //"0123456789abcdef
        sprintf(lcdStr3,"Min :      .  V ");
        lcdStr3[8] = gBattVoltSet[4];
        lcdStr3[9] = gBattVoltSet[5];
        lcdStr3[10] = gBattVoltSet[6];
        lcdStr3[12] = gBattVoltSet[7];
        
	    lcdBlink++;
        switch(iMenuCnt)
        {
            case 0 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr2[8]= gBattVoltSet[0];
                    }
                    else //if(cursor100ms>=2)
                    {
						lcdStr2[8] = '_';
                    }
                }
                break;
            case 1 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr2[9] = gBattVoltSet[1];
                    }
                    else //if(cursor100ms>=2)
                    {
						lcdStr2[9] = '_';
                    }
                }
                break;
            case 2 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr2[10] = gBattVoltSet[2];
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr2[10] = '_';
                    }
                }
                break;
            case 3 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr2[12] = gBattVoltSet[3];
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr2[12] = '_';
                    }
                }
                break;   
            case 4 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr3[8] = gBattVoltSet[4];	
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr3[8] = '_';
                    }
                }
                break;
            case 5 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr3[9] = gBattVoltSet[5];
                    }
                    else	//if(cursor100ms>=2)
                    {
						lcdStr3[9] = '_';
                    }
                }
                break;
            case 6 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr3[10] = gBattVoltSet[6];
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr3[10] = '_';
                    }
                }
                break;
            case 7 :
                {
                    if(lcdBlink & 0x0002)
                    {
						lcdStr3[12] = gBattVoltSet[7];
                    }
                    else//if(cursor100ms>=2)
                    {
						lcdStr3[12] = '_';
                    }
                }
                break;
            default :
                break;
        }
        switch(hKey())
        {
            case keyKEY_UP:
                inputCash++;
                break;
            case keyKEY_UPfast:
                inputCash++;
                break;	
            case keyKEY_DOWN:
                inputCash--;
                break;
            case keyKEY_DOWNfast:
                inputCash--;
                break;
            case keyKEY_LEFT:
                iMenuIndex = M1_BATTALARM;
                break;
            case keyKEY_ENTER:
                if(iMenuCnt == 7)
                {
                    for(i = 0; i < 8; i++)
                    {
                        EEPROMWrite(BATTVOLT_0+i, gBattVoltSet[0+i]);
                    }
                    iMenuIndex = M1_BATTALARM;

                   tmpValue = (gBattVoltSet[0]-0x30)*1000 + (gBattVoltSet[1]-0x30)*100  + (gBattVoltSet[2]-0x30)*10 + (gBattVoltSet[3]-0x30)*1;
                   BattHiVoltage = tmpValue * 0.1;  //  xxxx -> xxx.x
                   tmpValue = (gBattVoltSet[4]-0x30)*1000 + (gBattVoltSet[5]-0x30)*100  + (gBattVoltSet[6]-0x30)*10 + (gBattVoltSet[7]-0x30)*1;
                   BattLowVoltage = tmpValue * 0.1;  //  xxxx -> xxx.x
                   Set_OK=1;
                }
                else if(iMenuCnt == 6)
                {
                    inputCash=gBattVoltSet[7];
                    iMenuCnt = 7;
                }
                else if(iMenuCnt == 5)
                {
                    inputCash=gBattVoltSet[6];
                    iMenuCnt = 6;
                }
                else if(iMenuCnt == 4)
                {
                    inputCash=gBattVoltSet[5];
                    iMenuCnt = 5;
                }
                else if(iMenuCnt == 3)
                {
                    inputCash=gBattVoltSet[4];
                    iMenuCnt = 4;
                }
                else if(iMenuCnt == 2)
                {
                    inputCash=gBattVoltSet[3];
                    iMenuCnt = 3;
                }
                else if(iMenuCnt == 1)
                {
                    inputCash=gBattVoltSet[2];
                    iMenuCnt = 2;
                }
                else if(iMenuCnt == 0)
                {
                    inputCash=gBattVoltSet[1];
                    iMenuCnt = 1;
                }
                return;
                break;
            case keyKEY_BZ_OFF:
                break;
            case keyKEY_BZ_OFFfast:
                break;
        }
        if(iMenuCnt == 0) //liIon max 118.4 ~135.0
        {
            inputCash = '1';
            gBattVoltSet[0] = inputCash;
        }

        else if(iMenuCnt == 1)//1Nx.x (11x.x ~13x.x)
        {
            if(inputCash > '3') inputCash = '1';
            if(inputCash < '1') inputCash = '3';
            gBattVoltSet[1] = inputCash;
        }
        else if(iMenuCnt == 2)//max 1xN.x
        {
            if(gBattVoltSet[1] <= '1') //11N.x 
            {
                if(inputCash > '9') inputCash = '8';
                if(inputCash < '8') inputCash = '9';
            }
            else if(gBattVoltSet[1] >= '3')//13N.x
            {
                if(inputCash > '5') inputCash = '0';
                if(inputCash < '0') inputCash = '5';
            }
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[2] = inputCash;
        }
        else if(iMenuCnt == 3) //max xxx.n
        {
            if(gBattVoltSet[1] <= '1' && gBattVoltSet[2] <= '8')
            {
                if(inputCash > '9') inputCash = '4';
                if(inputCash < '4') inputCash = '9';
            }
            else if(gBattVoltSet[1] >= '3' && gBattVoltSet[2] >= '5')
            {
                inputCash = '0';
            }
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[3] = inputCash;
        }
        else if(iMenuCnt == 4)
        {
            if(inputCash > '1') inputCash = '0';
            if(inputCash < '0') inputCash = '1';
            gBattVoltSet[4] = inputCash;
        }
        else if(iMenuCnt == 5)
        {
            if(gBattVoltSet[4] == '1')
            {
                if(inputCash > '1') inputCash = '0';
                if(inputCash < '0') inputCash = '1';
            }
            else if(gBattVoltSet[4] == '0')
            {
                inputCash = '9';
            }
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[5] = inputCash;
        }
        else if(iMenuCnt == 6)
        {
            if(gBattVoltSet[4] >= '1' && gBattVoltSet[5] >= '1')
            {
                if(inputCash > '8') inputCash = '0';
                if(inputCash < '0') inputCash = '8';
            }
            else if(gBattVoltSet[4] <= '0' && gBattVoltSet[5] <= '9')
            {
                if(inputCash > '9') inputCash = '6';
                if(inputCash < '6') inputCash = '9';
            }
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[6] = inputCash;
        }
        else if(iMenuCnt == 7)
        {
            if(gBattVoltSet[5] >= '1' && gBattVoltSet[6] >= '8')
            {
                if(inputCash > '4') inputCash = '0';
                if(inputCash < '0') inputCash = '4';
            }
            else
            {
                if(inputCash > '9') inputCash = '0';
                if(inputCash < '0') inputCash = '9';
            }
            gBattVoltSet[7] = inputCash;
        }   
    }
}

void DispModuleRestart()
{
    unsigned char a,b;
  //sprintf(lcdStr0,"Restart Time Set");
    sprintf(lcdStr0,"S2]Restart Time ");        
	sprintf(lcdStr1,"                ");	
                  //"0123456789abcdef"
	sprintf(lcdStr2,"        Min     ");	
//	put_Num_LCD(gRestartTime[0]);
    
    if(gRestartTime[0]>9)
    {
        if(gRestartTime[0]>60)gRestartTime[0]=60;
        a=gRestartTime[0]/10;
        b=gRestartTime[0]%10;
        lcdStr2[4]=a + '0';
        lcdStr2[5]=b + '0';
    }
    else
    {
        lcdStr2[5]=gRestartTime[0] + '0';
    } 
    
	sprintf(lcdStr3,"                ");
	//
	switch(hKey())
	{
		case keyKEY_UP:			
			iMenuIndex = M1_BATTALARM;
			break;
        case keyKEY_DOWN:
			iMenuIndex = M0_TIME;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M0_SETUP;
			break;
		case keyKEY_ENTER:
			iMenuIndex = M2_MODULERESTARTSET;
			inputCash=gRestartTime[0];
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispModuleRestartSet()
{
	int tmpvalue = 0;	
    unsigned char a, b;
	//sprintf(lcdStr0,"Restart Time Set");	
    sprintf(lcdStr0,"S2]Restart Time ");
	sprintf(lcdStr1,"                ");	
				  //"0123456789abcdef"
	sprintf(lcdStr2,"        Min     ");
	//
	lcdBlink++;
	if(lcdBlink & 0x0002)//year
	{
		//setcurLCD(5, 2);
//		put_Num_LCD(gRestartTime[0]);
        if(gRestartTime[0]>9)
        {
            if(gRestartTime[0]>60)gRestartTime[0]=60;
            a=gRestartTime[0]/10;
            b=gRestartTime[0]%10;
            lcdStr2[4]=a +'0';
            lcdStr2[5]=b +'0';
        }
        else
        {
            lcdStr2[5]=gRestartTime[0]+'0';
        }        
	}
	else
	{
		
		//setcurLCD(5, 2);
		//sprintf(lcdStr1,"  ");
        lcdStr2[4]=' ';
        lcdStr2[5]=' ';
	}	
	sprintf(lcdStr3,"                ");
	
	switch(hKey())
	{
		case keyKEY_UP:
			inputCash++;
			break;
		case keyKEY_UPfast:
			inputCash++;
			break;	
		case keyKEY_DOWN:
			inputCash--;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M1_MODULERESTART;
			break;
		case keyKEY_ENTER:
            gRestartTime[0] = inputCash;
			EEPROMWrite(RESTARTTIME_0, gRestartTime[0]);
			//
			Set_OK=1;
			iMenuIndex = M1_MODULERESTART;
			tmpvalue = (gRestartTime[0])*10;// + (gBattVoltSet[1]-0x30)*100  + (gBattVoltSet[2]-0x30)*10 + (gBattVoltSet[3]-0x30)*1;
            
            //RestartTimeValue = tmpvalue * 0.1;  //  xxxx -> xxx.x
            RestartTimeValue = tmpvalue/10;  //  xxxx -> xxx.x
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
	//
	if(inputCash > 60) inputCash = 60;
	if(inputCash < 0) inputCash = 0;
	gRestartTime[0] = inputCash;
}

void DispSystemInit()
{	
//	sprintf(lcdStr0," **Initialize** ");	
    sprintf(lcdStr0,"S4]Init.System  ");
	sprintf(lcdStr1,"                ");	
	              //"0123456789abcdef"
	sprintf(lcdStr2,"      NO        ");	
	sprintf(lcdStr3,"                ");	
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M0_TIME;
			break;
        case keyKEY_DOWN:
			iMenuIndex = M1_PASSWORD;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M0_SETUP;
			break;			
		case keyKEY_ENTER:
			iMenuIndex = M2_SYSTEMINITSET;
			iMenuCnt = 0;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispSystemInitSet()
{
	//sprintf(lcdStr0," **Initialize** ");	
	              //"0123456789abcdef"
    sprintf(lcdStr0,"S4]Init.System  ");
	sprintf(lcdStr1,"                ");	
	sprintf(lcdStr2,"                ");	
	sprintf(lcdStr3,"                ");		
	lcdBlink++;
	
	switch(iMenuCnt)
	{
		case 0 :
			{
				if(lcdBlink & 0x0002)//year
				{
                    lcdStr2[6]='N';
                    lcdStr2[7]='O';
				}
				else
				{
                    lcdStr2[6]='_';
                    lcdStr2[7]='_';
				}
			}
			break;
		case 1 :
			{
				if(lcdBlink & 0x0002)
				{
                    lcdStr2[6]='Y';
                    lcdStr2[7]='E';
                    lcdStr2[8]='S';
				}
				else
				{
                    lcdStr2[6]='_';
                    lcdStr2[7]='_';
                    lcdStr2[8]='_';
				}
			}
			break;
		default :
				break;
	}
	
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuCnt ^= 1;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuCnt ^= 1;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;	
		case keyKEY_ENTER:
			if(iMenuCnt == 0)
			{
				iMenuIndex = M1_SYSTEMINIT;
			}
			else if(iMenuCnt == 1)
			{
				iMenuCnt=0;
				SystemInitialize();
				Set_OK=1;
				iMenuIndex = M1_SYSTEMINIT;
			}
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispPassword()
{
	//sprintf(lcdStr0," ** Password ** ");	
    sprintf(lcdStr0,"S5]Password Set ");	
	sprintf(lcdStr1,"                ");	
	sprintf(lcdStr2,"OLD :           ");
    
    lcdStr2[6] = gPass[0];
    lcdStr2[7] = gPass[1];
    lcdStr2[8] = gPass[2];
    lcdStr2[9] = gPass[3];
    lcdStr2[10] = gPass[4];
    lcdStr2[11] = gPass[5];
    lcdStr2[12] = gPass[6];
    lcdStr2[13] = gPass[7];
    lcdStr2[14] = gPass[8];
    
    /*
     setcurLCD(6, 2);
	CHARLCD1_PutChar(gPass[0]);
	CHARLCD1_PutChar(gPass[1]);
	CHARLCD1_PutChar(gPass[2]);
	CHARLCD1_PutChar(gPass[3]);
	CHARLCD1_PutChar(gPass[4]);
	CHARLCD1_PutChar(gPass[5]);
	CHARLCD1_PutChar(gPass[6]);
	CHARLCD1_PutChar(gPass[7]);
	CHARLCD1_PutChar(gPass[8]);
    */
	sprintf(lcdStr3,"NEW : ********* ");
	//
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M1_SYSTEMINIT;
			break;
		case keyKEY_DOWN:
			iMenuIndex = M1_ALARMSTOP;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M0_SETUP;
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_ENTER:
			iMenuIndex = M2_PASSWORDSET;
			iMenuCnt=0;
			inputCash=gPass[0];			
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		default:
			break;			
	}
}

void DispPasswordSet()
{
	unsigned char i;	
	sprintf(lcdStr0,"S5]Password Set ");	
	sprintf(lcdStr1,"                ");	
	sprintf(lcdStr3,"NEW : ********* ");
	//
    
    lcdStr3[6 + iMenuCnt]= gPass[iMenuCnt];
    /*
     switch(iMenuCnt)
	{
		case 0 :setcurLCD(6, 3);
				CHARLCD1_PutChar(gPass[0]);
				break;
		case 1 :setcurLCD(7, 3);
				CHARLCD1_PutChar(gPass[1]);
				break;
		case 2 :setcurLCD(8, 3);
				CHARLCD1_PutChar(gPass[2]);
				break;
		case 3 :setcurLCD(9, 3);
				CHARLCD1_PutChar(gPass[3]);
				break;
		case 4 :setcurLCD(10, 3);
				CHARLCD1_PutChar(gPass[4]);
				break;
		case 5 :setcurLCD(11, 3);
				CHARLCD1_PutChar(gPass[5]);
				break;
		case 6 :setcurLCD(12, 3);
				CHARLCD1_PutChar(gPass[6]);
				break;
		case 7 :setcurLCD(13, 3);
				CHARLCD1_PutChar(gPass[7]);
				break;
		case 8 :setcurLCD(14, 3);
				CHARLCD1_PutChar(gPass[8]);
				break;
	}
     */
	
	switch(hKey())
	{
		case keyKEY_UP:
			inputCash++;
			break;
		case keyKEY_UPfast:
			inputCash++;
			break;	
		case keyKEY_DOWN:
			inputCash--;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;	
		case keyKEY_ENTER:
			if(iMenuCnt == 8)
			{
				for(i = 0; i < 9; i++)
				{
					EEPROMWrite(PASSWD_0+i, gPass[0+i]);
				}
				iMenuIndex = M1_PASSWORD;
			}
			else if(iMenuCnt == 7)
			{
				inputCash=gPass[8];
				iMenuCnt = 8;
			}
			else if(iMenuCnt == 6)
			{
				inputCash=gPass[7];
				iMenuCnt = 7;
			}
			else if(iMenuCnt == 5)
			{
				inputCash=gPass[6];
				iMenuCnt = 6;
			}
			else if(iMenuCnt == 4)
			{
				inputCash=gPass[5];
				iMenuCnt = 5;
			}
			else if(iMenuCnt == 3)
			{
				inputCash=gPass[4];
				iMenuCnt = 4;
			}
			else if(iMenuCnt == 2)
			{
				inputCash=gPass[3];
				iMenuCnt = 3;
			}
			else if(iMenuCnt == 1)
			{
				inputCash=gPass[2];
				iMenuCnt = 2;
			}
			else if(iMenuCnt == 0)
			{
				inputCash=gPass[1];
				iMenuCnt = 1;
			}
			return;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
	if(iMenuCnt == 0)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[0] = inputCash;
	}
	else if(iMenuCnt == 1)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[1] = inputCash;
	}
	else if(iMenuCnt == 2)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[2] = inputCash;
	}
	else if(iMenuCnt == 3)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[3] = inputCash;
	}
	else if(iMenuCnt == 4)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[4] = inputCash;
	}
	else if(iMenuCnt == 5)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[5] = inputCash;
	}
	else if(iMenuCnt == 6)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[6] = inputCash;
	}
	else if(iMenuCnt == 7)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[7] = inputCash;
	}
	else if(iMenuCnt == 8)
	{
		if(inputCash > '9') inputCash = '0';
		if(inputCash < '0') inputCash = '9';
		gPass[8] = inputCash;
	}
}

void DispAlarmStop()
{
	//sprintf(lcdStr0," **Alarm Stop** ");	
    sprintf(lcdStr0,"S6]Alarm BZ Set ");	
	sprintf(lcdStr1,"                ");	
	if(fAlarmStop == 1)
					 //	"0123456789abcdef"
        sprintf(lcdStr2,"  Buzzer : ON   ");
	else
        sprintf(lcdStr2,"  Buzzer : OFF  ");	
				//	"0123456789abcdef"	
	sprintf(lcdStr3,"                ");
	//
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M1_PASSWORD;
			break;
		case keyKEY_DOWN:
            iMenuIndex = M3_System;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M0_SETUP;
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTER:
			iMenuCnt=fAlarmStop;
			iMenuIndex = M2_ALARMSTOPSET;
			break;
		default:
			break;			
	}
}

void DispAlarmStopSet()
{
	//unsigned char fAlarmStop;	
	//sprintf(lcdStr0," **Alarm Stop** ");	
	              //"0123456789abcdef"
    sprintf(lcdStr0,"S6]Alarm BZ Set ");	
	sprintf(lcdStr1,"                ");	
	sprintf(lcdStr2,"  Buzzer :      ");
	sprintf(lcdStr3,"                ");
	//
	lcdBlink++;	
	switch(iMenuCnt)
	{
		case 0 :
			{
				if(lcdBlink & 0x0002)
				{                 //"0123456789abcdef"
					sprintf(lcdStr2,"  Buzzer : OFF  ");
				}
				else//if(cursor100ms>=2)
				{
					              //"0123456789abcdef"
					sprintf(lcdStr2,"  Buzzer :      ");
				}
			}
			break;
		case 1 :
			{
				if(lcdBlink & 0x0002)
				{    			  //"0123456789abcdef"	
					sprintf(lcdStr2,"  Buzzer : ON   ");
				}
				else//if(cursor100ms>=2)
				{
					              //"0123456789abcdef"
					sprintf(lcdStr2,"  Buzzer :      ");
				}
			}
			break;
		default :
				break;
	}
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuCnt^=1;
			break;
		case keyKEY_DOWN:
			iMenuCnt^=1;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M1_ALARMSTOP;
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTER:
			fAlarmStop=iMenuCnt;
			EEPROMWrite(ALARM_STOP, fAlarmStop);
			iMenuIndex = M1_ALARMSTOP;
			break;
		default:
			break;			
	}
}

void DispHistory()
{

    unsigned char tmpCnt=0,i;	//,alarmNum=0;

    sprintf(lcdStr0,"4]Alarm History ");
	//
	if(gAlarmHistoryCnt>0)	tmpCnt=gAlarmHistoryCnt;
	else tmpCnt=0;
	//    
	if( gAlarmHistory[tmpCnt - gC_AlarmHistoryCnt][0] == 1)
	{    //index is [0] ~ [149]
        if( (tmpCnt - gC_AlarmHistoryCnt) >= 0 && (tmpCnt - gC_AlarmHistoryCnt) < 150)
            AlarmHistoryDisplay(tmpCnt - gC_AlarmHistoryCnt);
	}
	else
	{                 //"0123456789abcdef"
		sprintf(lcdStr1,"                ");	
		sprintf(lcdStr2,"    Nothing     ");		
		sprintf(lcdStr3,"                ");		
	}
	//
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M0_SETUP;
			gC_AlarmHistoryCnt=0;
			break;
		case keyKEY_DOWN:
			iMenuIndex = M0_TIME;
			gC_AlarmHistoryCnt=0;
			break;
		case keyKEY_LEFT:
			if((gC_AlarmHistoryCnt+1) < gAlarmHistoryCnt)
			{	
				gC_AlarmHistoryCnt++;
				if(gAlarmHistory[gC_AlarmHistoryCnt][0] == 0) gC_AlarmHistoryCnt--;
			}
			break;
		case keyKEY_RIGHT:
			if(gC_AlarmHistoryCnt>0) gC_AlarmHistoryCnt--;
			break;		
		case keyKEY_LEFTfast:		
			if((gC_AlarmHistoryCnt+1) < gAlarmHistoryCnt)
			{	
				gC_AlarmHistoryCnt++;		
				if(gAlarmHistory[gC_AlarmHistoryCnt][0] == 0) gC_AlarmHistoryCnt--;
			}
			break;
		case keyKEY_RIGHTfast:
			if(gC_AlarmHistoryCnt>0) gC_AlarmHistoryCnt--;
			break;	
		case keyKEY_ENTERfast:
			//for(i = 0; i < 200; i++)
            if(gAlarmHistoryCnt>0)
            {
                for(i = 0; i < 150; i++)
                {
                    gAlarmHistory[i][0] = 0;
                    gAlarmHistoryCnt = 0;
                    EEPROMWrite(ALARM_STORAGE + (i*10), 0);
                    EEPROMWrite(ALARM_HISTORY_CNT, gAlarmHistoryCnt);
                }
                for(i = 1; i < 8; i++)
                {
                    Module1_Sts[i] = 0;
                    prev_Module1_Sts[i] = 0;
                    Module2_Sts[i] = 0;
                    prev_Module2_Sts[i] = 0;
                }
                for(i = 0; i < 2; i++)
                {
                    Battery_Sts[i] = 0;
                    prev_Battery_Sts[i] = 0;
                }
                inputCash = 0;
                fBzOn=1;
                tmrBz = 40;
            }    
            
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTER:
			break;	
		default:
			break;
	}
}


void DispBattType()
{
  //sprintf(lcdStr0," **Batt  Type** ");
    sprintf(lcdStr0,"D1]Battery Type ");                     
	//setcurLCD(0, 1);
    sprintf(lcdStr1,"1.PB-125        ");
    sprintf(lcdStr2,"2.PB-110        ");	
    sprintf(lcdStr3,"3.Li-Ion        ");
//    gBattTypeMode = read_EE(BATT_TYPE);
	if(gBattTypeMode == 0)	sprintf(lcdStr1,"1.PB-125     *  ");
	else	sprintf(lcdStr1,"1.PB-125        ");
//	setcurLCD(0, 2);
    if(gBattTypeMode == 1)	sprintf(lcdStr2,"2.PB-110     *  ");
	else	sprintf(lcdStr2,"2.PB-110        ");	
//	setcurLCD(0, 3);
	if(gBattTypeMode == 2)	sprintf(lcdStr3,"3.Li-Ion     *  ");
	else	sprintf(lcdStr3,"3.Li-Ion        ");
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M3_SYSTYPE;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuIndex = M3_OPERATIONMODE;
			break;
		case keyKEY_DOWNfast:
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_System;
			break;
		case keyKEY_ENTER:
			iMenuIndex = M4_BATTTYPESET;
            iMenuCnt = gBattTypeMode;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispBattTypeSet()
{
	UINT8 i;
    int tmpvalue;
	sprintf(lcdStr0,"D1]Battery Type ");	
	sprintf(lcdStr1,"1.PB-125        ");	
	sprintf(lcdStr2,"2.PB-110        ");	
	sprintf(lcdStr3,"3.Li-Ion        ");	
	switch(iMenuCnt)
	{
		case 0 ://setcurLCD(13, 1);
                lcdStr1[13] ='*';
				lcdStr1[14] ='<';   //,"*<-");
                lcdStr1[15] ='-';
				break;
        case 1 ://setcurLCD(13, 2);
                lcdStr2[13] ='*';
				lcdStr2[14] ='<';   //,"*<-");
                lcdStr2[15] ='-';
                break;
		case 2 ://setcurLCD(13, 3);
				lcdStr3[13] ='*';
				lcdStr3[14] ='<';   //,"*<-");
                lcdStr3[15] ='-';
				break;
		default :
				break;
	}
	switch(hKey())
	{
		case keyKEY_UP:			
			if(iMenuCnt <= 0) iMenuCnt = 2;
            else iMenuCnt--;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			if(iMenuCnt >= 2) iMenuCnt = 0;
            else iMenuCnt++;
			break;
		case keyKEY_DOWNfast:
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_BATTTYPE;
			break;
		case keyKEY_ENTER:
			gBattTypeMode=iMenuCnt;
            EEPROMWrite(BATT_TYPE, gBattTypeMode);
            if(gBattTypeMode == 0)//PB125B
            {
                EEPROMWrite(BATTVOLT_0,			'1');
                EEPROMWrite(BATTVOLT_1,			'5');
                EEPROMWrite(BATTVOLT_2,			'0');
                EEPROMWrite(BATTVOLT_3,			'0');
                EEPROMWrite(BATTVOLT_4,			'1');
                EEPROMWrite(BATTVOLT_5,			'0');
                EEPROMWrite(BATTVOLT_6,			'5');
                EEPROMWrite(BATTVOLT_7,			'0');
            }
            else if(gBattTypeMode == 1)//pb 110V goooo//
            {
                EEPROMWrite(BATTVOLT_0,			'1');
                EEPROMWrite(BATTVOLT_1,			'5');
                EEPROMWrite(BATTVOLT_2,			'0');
                EEPROMWrite(BATTVOLT_3,			'0');
                EEPROMWrite(BATTVOLT_4,			'1');
                EEPROMWrite(BATTVOLT_5,			'0');
                EEPROMWrite(BATTVOLT_6,			'0');
                EEPROMWrite(BATTVOLT_7,			'0');
            }
            else if(gBattTypeMode == 2)
            {
                EEPROMWrite(BATTVOLT_0,			'1');
                EEPROMWrite(BATTVOLT_1,			'3');
                EEPROMWrite(BATTVOLT_2,			'5');
                EEPROMWrite(BATTVOLT_3,			'0');
                EEPROMWrite(BATTVOLT_4,			'0');
                EEPROMWrite(BATTVOLT_5,			'9');
                EEPROMWrite(BATTVOLT_6,			'6');
                EEPROMWrite(BATTVOLT_7,			'0');
            }
            
            iMenuIndex = M3_BATTTYPE;
            
            for(i = 0; i < 8; i++)
            {
                gBattVoltSet[0+i] = EEPROMRead(BATTVOLT_0+i);
            }

            tmpvalue = (gBattVoltSet[0]-0x30)*1000 + (gBattVoltSet[1]-0x30)*100  + (gBattVoltSet[2]-0x30)*10 + (gBattVoltSet[3]-0x30)*1;
            BattHiVoltage = tmpvalue * 0.1;  //  xxxx -> xxx.x
            tmpvalue = (gBattVoltSet[4]-0x30)*1000 + (gBattVoltSet[5]-0x30)*100  + (gBattVoltSet[6]-0x30)*10 + (gBattVoltSet[7]-0x30)*1;
            BattLowVoltage = tmpvalue * 0.1;  //  xxxx -> xxx.x
                    
			Set_OK = 1;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispInputVoltType()
{
	sprintf(lcdStr0," **Input Volt** ");
	sprintf(lcdStr1,"380VAC       *  ");
    sprintf(lcdStr2,"                ");
    sprintf(lcdStr3,"220VAC          ");
    //
	if(gInVoltMode == 0)	sprintf(lcdStr1,"380VAC       *  ");
	else	sprintf(lcdStr1,"380VAC          ");
    //
	sprintf(lcdStr2,"                ");
    //
	if(gInVoltMode == 0)	sprintf(lcdStr3,"220VAC          ");
	else	sprintf(lcdStr3,"220VAC       *  ");
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M3_BATTTYPE;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuIndex = M3_OPERATIONMODE;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_System;
			break;
		case keyKEY_ENTER:
			iMenuIndex = M4_INPUTVOLTSET;
			iMenuCnt = gInVoltMode;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispInputVoltTypeSet()
{
	//UINT8 i;
	
//	setcurLCD(0, 0);
//	sprintf(lcdStr1," **Input Volt** ");
//	setcurLCD(0, 1);
//	sprintf(lcdStr1,"380VAC          ");
//	setcurLCD(0, 2);	
//	sprintf(lcdStr1,"                ");
//	setcurLCD(0, 3);
//	sprintf(lcdStr1,"220VAC          ");	
	sprintf(lcdStr0," **Input Volt** ");	
	sprintf(lcdStr1,"380VAC          ");	
	sprintf(lcdStr2,"                ");	
	sprintf(lcdStr3,"220VAC          ");
	switch(iMenuCnt)
	{
		case 0 ://setcurLCD(13, 1);
				//sprintf(lcdStr1,"*<-");
                lcdStr1[13] ='*';
				lcdStr1[14] ='<';
                lcdStr1[15] ='-';
				break;
		case 1 ://setcurLCD(13, 3);
				//sprintf(lcdStr1,"*<-");
                lcdStr3[13] ='*';
				lcdStr3[14] ='<';
                lcdStr3[15] ='-';
				break;
		default :
				break;
	}
	switch(hKey())
	{
		case keyKEY_UP:			
			iMenuCnt=0;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuCnt=1;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_INPUTVOLT;
			break;
		case keyKEY_ENTER:
			gInVoltMode=iMenuCnt;
			Set_OK=1;
			iMenuIndex = M3_INPUTVOLT;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispOperationMode()
{
    gOperationMode = EEPROMRead(OP_MODE);
                  //"0123456789abcdef"
	sprintf(lcdStr0,"D2]Operate Type ");  //"0123456789abcdef"
	if(gOperationMode == 0)	sprintf(lcdStr1,"1.Stand-By   *  ");
	else                    sprintf(lcdStr1,"1.Stand-By      ");	
				  //"0123456789abcdef"	
	sprintf(lcdStr2,"                ");  //"0123456789abcdef"	
	if(gOperationMode == 0)	sprintf(lcdStr3,"2.Parallel      ");
	else                    sprintf(lcdStr3,"2.Parallel   *  ");
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M3_BATTTYPE;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuIndex = M3_SYSTYPE;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_System;
			break;
		case keyKEY_ENTER:
			iMenuIndex = M4_OPERATIONMODESET;
			iMenuCnt = gOperationMode;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispOperationModeSet()
{
	//UINT8 i;
	
	//setcurLCD(0, 0);
//sprintf(lcdStr0,"*Operation Type*");	
                  //"0123456789abcdef" 
    sprintf(lcdStr0,"D2]Operate Type ");	
	sprintf(lcdStr1,"1.Stand-By      ");	
	sprintf(lcdStr2,"                ");	
	sprintf(lcdStr3,"2.Parallel      ");	
	switch(iMenuCnt)
	{
		case 0 :lcdStr1[13] ='*';
				lcdStr1[14] ='<';
                lcdStr1[15] ='-';
				break;
		case 1 :lcdStr3[13] ='*';
				lcdStr3[14] ='<';
                lcdStr3[15] ='-';
				break;
		default :
				break;
	}
	switch(hKey())
	{
		case keyKEY_UP:			
			iMenuCnt=0;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuCnt=1;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_OPERATIONMODE;
			break;
		case keyKEY_ENTER:
			gOperationMode=iMenuCnt;
//            write_EE(OP_MODE, gOperationMode);
            EEPROMWrite(OP_MODE,gOperationMode);
			Set_OK=1;
			iMenuIndex = M3_OPERATIONMODE;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispSysType()
{
    gSysTypeMode = EEPROMRead(SYS_TYPE);
//	setcurLCD(0, 0);
  //sprintf(lcdStr0,"  **SYS Type**  "); 
                  //"0123456789abcdef"
    sprintf(lcdStr0,"D3]Module Type  "); 
//	setcurLCD(0, 1);                      //"0123456789abcdef"
	if(gSysTypeMode == 0)	sprintf(lcdStr1,"1.Dual Type  *  ");
	else                    sprintf(lcdStr1,"1.Dual Type     ");
//	setcurLCD(0, 2);	
	sprintf(lcdStr2,"                ");
//	setcurLCD(0, 3);                      //"0123456789abcdef"
	if(gSysTypeMode == 0)	sprintf(lcdStr3,"2.Single Type   ");
	else                    sprintf(lcdStr3,"2.Single Type*  ");
	switch(hKey())
	{
		case keyKEY_UP:
			iMenuIndex = M3_OPERATIONMODE;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuIndex = M3_BATTTYPE;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_System;
			break;
		case keyKEY_ENTER:
			iMenuIndex = M4_SYSTYPESET;
			iMenuCnt = gSysTypeMode;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void DispSysTypeSet()
{
	//UINT8 i;
	
	//sprintf(lcdStr0,"  **SYS Type**  ");	
	              //"0123456789abcdef"
    sprintf(lcdStr0,"D3]Module Type  "); 
	sprintf(lcdStr1,"1.Dual Type     ");	
	sprintf(lcdStr2,"                ");	
	sprintf(lcdStr3,"2.Single Type   ");
	//
	switch(iMenuCnt)
	{
		case 0 :lcdStr1[13] ='*';
				lcdStr1[14] ='<';
                lcdStr1[15] ='-';
				break;
		case 1 :lcdStr3[13] ='*';
				lcdStr3[14] ='<';
                lcdStr3[15] ='-';
				break;
		default :
				break;
	}
	switch(hKey())
	{
		case keyKEY_UP:			
			iMenuCnt=0;
			break;
		case keyKEY_UPfast:
			break;	
		case keyKEY_DOWN:
			iMenuCnt=1;
			break;
		case keyKEY_DOWNfast:
			break;
		case keyKEY_LEFT:
			iMenuIndex = M3_SYSTYPE;
			break;
		case keyKEY_ENTER:
			gSysTypeMode=iMenuCnt;
            EEPROMWrite(SYS_TYPE, gSysTypeMode);
			Set_OK=1;
			iMenuIndex = M3_SYSTYPE;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
}

void AlarmHistoryDisplay(UINT8 index)
{
	UINT8 a, b ,c,tmp;
	//	
  //sprintf(lcdStr0,"**AlarmHistory**");	
				  //"0123456789abcdef"
    sprintf(lcdStr0,"4]Alarm History ");	
	sprintf(lcdStr1,"   <   /   >    ");
	sprintf(lcdStr3,"   /  /     :   ");
	switch(gAlarmHistory[index][7])
	{
		case 11:          //"0123456789abcdef"
			sprintf(lcdStr2,"#1 In. High Volt");
			break;
		case 12:
			sprintf(lcdStr2,"#1 In. Low  Volt");
			break;
		case 13:
            sprintf(lcdStr2,"   #1 AC Fail   ");
            //sprintf(lcdStr2,"   #1 NFB OFF   ");
			break;
		case 14:
			sprintf(lcdStr2,"#1 Out.High Volt");
			break;
		case 15:
			sprintf(lcdStr2,"#1 Out.Low  Volt");
			break;
		case 16:
			sprintf(lcdStr2,"#1 Out.Over Curr");
			break;
		case 17:
			sprintf(lcdStr2,"  #1 Over Temp  ");
			break;
		case 21:
			sprintf(lcdStr2,"#2 In. High Volt");
			break;
		case 22:
			sprintf(lcdStr2,"#2 In. Low  Volt");
			break;
		case 23:
            sprintf(lcdStr2,"   #2 AC Fail   ");
            //sprintf(lcdStr2,"   #2 NFB OFF   ");            
			break;
		case 24:
			sprintf(lcdStr2,"#2 Out.High Volt");
			break;
		case 25:
			sprintf(lcdStr2,"#2 Out.Low  Volt");
			break;                       
		case 26:
			sprintf(lcdStr2,"#2 Out.Over Load");
			break;                       
		case 27:
			sprintf(lcdStr2,"  #2 Over Temp  ");
			break;                       
		case 51:
			sprintf(lcdStr2,"Battery HighVolt");
			break;
		case 52:
			sprintf(lcdStr2,"Battery Low Volt");
			break;
		case 61:
			sprintf(lcdStr2,"#1 Out.High Alm ");
			break;
		case 62:
			sprintf(lcdStr2,"#1 Out.Low Alm  ");
			break;
		case 63:
			sprintf(lcdStr2,"#2 Out.High Alm ");
			break;
		case 64:
			sprintf(lcdStr2,"#2 Out.Low Alm  ");
			break;
		case 71:
			sprintf(lcdStr2,"   #1 Startup   ");
			break;
		case 72:
			sprintf(lcdStr2,"   #2 Startup   ");
			break;
        case 81:
			sprintf(lcdStr2,"#1 FLOAT Charge ");
			break;
		case 82:
			sprintf(lcdStr2,"#1 EQUAL Charge ");
			break;
        case 83:
			sprintf(lcdStr2,"#2 FLOAT Charge ");
			break;
		case 84:
			sprintf(lcdStr2,"#2 EQUAL Charge ");
			break;
        case 99:
            sprintf(lcdStr2,"   DC Ground    ");
            break;
   		default :
            sprintf(lcdStr2,"#Error          ");
			break;
	}	
	//
    
    //Recovery timer 
    //0110-0000;
    //ModuleState[2] = 0x60;
//    c = ModuleState[2] & 0xc0;//1100-0000
//    c = c>>6;   
//    lcdStr1[0]= c + '0';//recovery count//
//    //
//    lcdStr1[1] = '>';
//            //
//    a = (ModuleState[2] & 0x3f)/10;//min10
//    b = (ModuleState[2] & 0x3f)%10;//min1
//    lcdStr1[2] = a + '0';
//    lcdStr1[3] = b + '0';
//    //
//    lcdStr1[4] = ':';
//    //ModuleState[3] = 59;
//    a = ModuleState[3]/10;//sec10
//    b = ModuleState[3]%10;//sec1
//    lcdStr1[5] = a + '0';
//    lcdStr1[6] = b + '0'; 
//    
	tmp=index;	//gAlarmHistoryCnt;
	a = tmp / 100;
	b = (tmp - a*100)/10;
	c = (tmp - a*100)-(b*10);
//	setcurLCD(4, 1);
//	//put_Num3_LCD(index);
//	CHARLCD1_PutChar(a + 0x30);	//4
//	CHARLCD1_PutChar(b + 0x30);	//5
//	CHARLCD1_PutChar(c + 0x30);	//6
//	CHARLCD1_PutChar('/');		//7
	lcdStr1[4]=a + '0';
	lcdStr1[5]=b + '0';
	lcdStr1[6]=c + '0';
	lcdStr1[7]='/';
           
    //-------------------------        
//    lcdStr1[8]=a + '0';
//	lcdStr1[9]=b + '0';
//	lcdStr1[10]=c + '0';
//	lcdStr1[11]='/';
	a=(gAlarmHistoryCnt%1000)/100;
	b=(gAlarmHistoryCnt%100)/10;
	c=gAlarmHistoryCnt%10;
//	setcurLCD(8, 1);
//	CHARLCD1_PutChar(a + 0x30);	//8
//	CHARLCD1_PutChar(b + 0x30);	//9
//	CHARLCD1_PutChar(c + 0x30);	//10
	lcdStr1[8]=a + '0';
	lcdStr1[9]=b + '0';
	lcdStr1[10]=c + '0';
//    lcdStr1[12]=a + '0';
//	lcdStr1[13]=b + '0';
//	lcdStr1[14]=c + '0';
//	//------------------------------
	a = gAlarmHistory[index][1] / 10;
	b = gAlarmHistory[index][1] - a*10;
//	setcurLCD(1, 3);
//	CHARLCD1_PutChar(a + 0x30);
//	setcurLCD(2, 3);
//	CHARLCD1_PutChar(b + 0x30);
	lcdStr3[1] = a + '0';
	lcdStr3[2] = b + '0'; 
	//
	a = gAlarmHistory[index][2] / 10;
	b = gAlarmHistory[index][2] - a*10;
//	setcurLCD(4, 3);
//	CHARLCD1_PutChar(a + 0x30);
//	setcurLCD(5, 3);
//	CHARLCD1_PutChar(b + 0x30);
	lcdStr3[4] = a + '0';
	lcdStr3[5] = b + '0'; 
	//
	a = gAlarmHistory[index][3] / 10;
	b = gAlarmHistory[index][3] - a*10;
//	setcurLCD(7, 3);
//	CHARLCD1_PutChar(a + 0x30);
//	setcurLCD(8, 3);
//	CHARLCD1_PutChar(b + 0x30);
	lcdStr3[7] = a + '0';
	lcdStr3[8] = b + '0'; 
	//
	a = gAlarmHistory[index][4] / 10;
	b = gAlarmHistory[index][4] - a*10;	
//	setcurLCD(10, 3);
//	CHARLCD1_PutChar(a + 0x30);
//	setcurLCD(11, 3);
//	CHARLCD1_PutChar(b + 0x30);
	lcdStr3[10] = a + '0';
	lcdStr3[11] = b + '0'; 
	//
	a = gAlarmHistory[index][5] / 10;
	b = gAlarmHistory[index][5] - a*10;
//	setcurLCD(13, 3);
//	CHARLCD1_PutChar(a + 0x30);
//	setcurLCD(14, 3);
//	CHARLCD1_PutChar(b + 0x30);
	lcdStr3[13] = a + '0';
	lcdStr3[14] = b + '0'; 
}

void AlarmOccurrenceApply(unsigned int packetsize)
{
	//UINT8 i;
	//if((prev_alarm_name != ErrCode)	&& (gAlarmHistory[gAlarmHistoryCnt-1][7] != ErrCode))// && (gAlarmHistory[gAlarmHistoryCnt-2][7] != errCnt))		//(prev_alarm_status !=	RxBuf[41]))
	if(ErrCode != 0 && prev_alarm_name != ErrCode)
	{
	//----------------------------------------------------------
        //iMenuIndex = M0_HISTORY;
        iLcdBackLightTm = 4;
        //
		rtc_get_time();
		rtc_get_date();
		//
		ErrCnt=ErrCode;
		if(gAlarmHistoryCnt == 150)	gAlarmHistoryCnt = 0;
		else	gAlarmHistoryCnt++;
		
		gAlarmHistory[gAlarmHistoryCnt][0] = 1;
		gAlarmHistory[gAlarmHistoryCnt][1] = year;
		gAlarmHistory[gAlarmHistoryCnt][2] = mth;
		gAlarmHistory[gAlarmHistoryCnt][3] = day;
		gAlarmHistory[gAlarmHistoryCnt][4] = hour;
		gAlarmHistory[gAlarmHistoryCnt][5] = min;
		gAlarmHistory[gAlarmHistoryCnt][6] = sec;
		gAlarmHistory[gAlarmHistoryCnt][7] = ErrCnt;
		gAlarmHistory[gAlarmHistoryCnt][8] = gAlarmHistoryCnt;
		gAlarmHistory[gAlarmHistoryCnt][9] = gC_AlarmHistoryCnt;
		//
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10),     gAlarmHistory[gAlarmHistoryCnt][0]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 1, gAlarmHistory[gAlarmHistoryCnt][1]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 2, gAlarmHistory[gAlarmHistoryCnt][2]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 3, gAlarmHistory[gAlarmHistoryCnt][3]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 4, gAlarmHistory[gAlarmHistoryCnt][4]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 5, gAlarmHistory[gAlarmHistoryCnt][5]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 6, gAlarmHistory[gAlarmHistoryCnt][6]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 7, gAlarmHistory[gAlarmHistoryCnt][7]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 8, gAlarmHistory[gAlarmHistoryCnt][8]);
		EEPROMWrite(ALARM_STORAGE + (gAlarmHistoryCnt*10) + 9, gAlarmHistory[gAlarmHistoryCnt][9]);
		EEPROMWrite(ALARM_HISTORY_CNT, gAlarmHistoryCnt);
		//
		prev_alarm_name	= ErrCode;
		
		switch(ErrCode)
		{
			case 11: prev_Module1_Sts[1] = Module1_Sts[1];break;
			case 12: prev_Module1_Sts[2] = Module1_Sts[2];break;
			case 13: prev_Module1_Sts[3] = Module1_Sts[3];break;
			case 14: prev_Module1_Sts[4] = Module1_Sts[4];break;
			case 15: prev_Module1_Sts[5] = Module1_Sts[5];break;
			case 16: prev_Module1_Sts[6] = Module1_Sts[6];break;
			case 17: prev_Module1_Sts[7] = Module1_Sts[7];break;
            //
			case 21: prev_Module2_Sts[1] = Module2_Sts[1];break;
			case 22: prev_Module2_Sts[2] = Module2_Sts[2];break;
			case 23: prev_Module2_Sts[3] = Module2_Sts[3];break;
			case 24: prev_Module2_Sts[4] = Module2_Sts[4];break;
			case 25: prev_Module2_Sts[5] = Module2_Sts[5];break;
			case 26: prev_Module2_Sts[6] = Module2_Sts[6];break;
			case 27: prev_Module2_Sts[7] = Module2_Sts[7];break;
			//
			case 51: prev_Battery_Sts[0] = Battery_Sts[0];break;
			case 52: prev_Battery_Sts[1] = Battery_Sts[1];break;
			//
			case 61: prev_Warning_Sts[0] = Warning_Sts[0];break;
			case 62: prev_Warning_Sts[1] = Warning_Sts[1];break;
			case 63: prev_Warning_Sts[2] = Warning_Sts[2];break;
			case 64: prev_Warning_Sts[3] = Warning_Sts[3];break;
			//
			case 71: prev_ModuleStartup_Sts[0] = ModuleStartup_Sts[0];break;
			case 72: prev_ModuleStartup_Sts[1] = ModuleStartup_Sts[1];break;
            //
			case 81: prev_Module_CHG_Sts[1] = Module_CHG_Sts[1];break;
			case 82: prev_Module_CHG_Sts[1] = Module_CHG_Sts[1];break;
			case 83: prev_Module_CHG_Sts[3] = Module_CHG_Sts[3];break;
			case 84: prev_Module_CHG_Sts[3] = Module_CHG_Sts[3];break;  
//          case 85: prev_Module_CHG_Sts[5] = Module_CHG_Sts[5];break;
//			case 86: prev_Module_CHG_Sts[5] = Module_CHG_Sts[5];break;
//			case 87: prev_Module_CHG_Sts[7] = Module_CHG_Sts[7];break;
//			case 88: prev_Module_CHG_Sts[7] = Module_CHG_Sts[7];break;
		}
		ErrCode = 0;
	}
}

void DispTime()
{
	//UINT8 a, b, c;
    
	if(fTimeSet == 0)
	{
		//setcurLCD(0, 0);
	  //sprintf(lcdStr0,"**Time & Date** ");
        //sprintf(lcdStr0,"5]Time & Date   ");
                      //"0123456789abcdef"
        sprintf(lcdStr0,"5]Time&Date     ");
	}
	else if(fTimeSet == 1)
	{
		//setcurLCD(0, 0);
		//sprintf(lcdStr0,"Time & Date Set ");
      //sprintf(lcdStr0,"5]Time & Date   ");
                      //"0123456789abcdef"
        sprintf(lcdStr0,"5]Time&Date Set ");
	}
	//setcurLCD(0, 1);
                  //"0123456789abcdef"  
	sprintf(lcdStr1,"                ");
	//
	rtc_get_date();
	rtc_get_time();
	//
    if(RtcDateTime_IsValid())
    {
        sprintf(lcdStr2,"Date[20%02d/%02d/%02d]",year,mth,day);    
                        //"0123456789abcdef"
                        //"Date[2022/06/07]"                   
        sprintf(lcdStr3,"Time[%02d:%02d:%02d]  ",hour,min,sec);
                        //"0123456789abcdef"   
                        //"Time[16:21:59]  " 
    }
	else
    {                 //"0123456789abcdef"
        sprintf(lcdStr2,"Error RTC data! ");    
        sprintf(lcdStr3,"                ");
    }
	//
	
	//
    
	switch(hKey())
	{
		case keyKEY_UP:
			if(fTimeSet == 0)	iMenuIndex = M0_HISTORY;
			else if(fTimeSet == 1)	iMenuIndex = M1_MODULERESTART;
			break;
		case keyKEY_DOWN:
			if(fTimeSet == 0)	iMenuIndex = M0_LOGO;
			else if(fTimeSet == 1)	iMenuIndex = M1_SYSTEMINIT;
			break;
		case keyKEY_LEFT:
			if(fTimeSet == 0)	break;
			else if(fTimeSet == 1)	iMenuIndex = M0_SETUP;
			break;
		case keyKEY_RIGHT:
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
		case keyKEY_ENTER:
			if(fTimeSet == 0)	break;
			else if(fTimeSet == 1)
			{
				gTmp=1;
				rtc_get_date();
				rtc_get_time();
				inputCash = year;
				iMenuIndex = M1_TIMESET;
			}
			break;
		default:
			break;
	}
}

void DispTimeSet(void)
{    
//	char a, b;
	//
	//setcurLCD(0, 0);
                    
  //sprintf(lcdStr0,"Time & Date Set ");	
                  //"0123456789abcdef"  
    sprintf(lcdStr0,"5]Time&Date Set ");
	//setcurLCD(0, 1);
	sprintf(lcdStr1,"                ");
	//setcurLCD(0, 2);
	//sprintf(lcdStr1,"Date :20  :  :  ");
    if(RtcDateTime_IsValid())
    {
        sprintf(lcdStr2,"Date[20%02d/%02d/%02d]",year,mth,day);    
                        //"0123456789abcdef"
                        //"Date[2022/06/07]"                   
        sprintf(lcdStr3,"Time[%02d:%02d:%02d]  ",hour,min,sec);
                        //"0123456789abcdef"   
                        //"Time[16:21:59]  " 
    }
	else
    {                 //"0123456789abcdef"
        sprintf(lcdStr2,"Error RTC data! ");    
        sprintf(lcdStr3,"                ");
    }
	//
    //
    lcdBlink++;// ^= 1;
    //0000 - 0001 - 0010 - 0011 //
    //0100 - 0101 - 0110 - 0111 //
    //1000 - 1001 - 1010 - 1011 //
    //1100    
	switch(gTmp)
	{
		case 1: 
            if(lcdBlink & 0x0002)//year
            {
                lcdStr2[7]= '_';
                lcdStr2[8]= '_';
            }
				break;
		case 2: 
			if(lcdBlink & 0x0002)//mth
            {
                lcdStr2[10]= '_';
                lcdStr2[11]= '_';
            }
				break;
		case 3: 
            if(lcdBlink & 0x0002)//day
            {
                lcdStr2[13]= '_';
                lcdStr2[14]= '_';
            }
				break;
		case 4: 
            if(lcdBlink & 0x0002)//hour
            {
                lcdStr3[5]= '_';
                lcdStr3[6]= '_';
            }
				break;
		case 5: 
            if(lcdBlink & 0x0002)//min
            {
                lcdStr3[8]= '_';
                lcdStr3[9]= '_';
            }
				break;
		case 6: 
            if(lcdBlink & 0x0002)//day
            {
                lcdStr3[11]= '_';
                lcdStr3[12]= '_';
            }
				break;
		//default : break;
	}
	switch(hKey())
	{
		case keyKEY_UP:
			inputCash++;
			break;
		case keyKEY_UPfast:
			inputCash++;
			break;	
		case keyKEY_DOWN:
			inputCash--;
			break;
		case keyKEY_DOWNfast:
			inputCash--;
			break;	
		case keyKEY_ENTER:
			if(gTmp == 6)
			{
				inputCash=0;
				rtc_set_datetime(day, mth, year, 0, hour, min);
				iMenuIndex = M0_TIME;
			}
			else if(gTmp == 5)
			{
				inputCash=sec;
				gTmp=6;
			}
			else if(gTmp == 4)
			{
				inputCash=min;
				gTmp=5;
			}
			else if(gTmp == 3)
			{
				inputCash=hour;
				gTmp=4;
			}
			else if(gTmp == 2)
			{
				inputCash=day;
				gTmp=3;
			}
			else if(gTmp == 1)
			{
				inputCash=mth;
				gTmp=2;
			}
			//return;
			break;
		case keyKEY_BZ_OFF:
			break;
		case keyKEY_BZ_OFFfast:
			break;
	}
	//
	if(gTmp == 1)
	{
		if(inputCash > 99) inputCash = 0;
		if(inputCash < 0) inputCash = 0;
		year = inputCash;
	}
	else if(gTmp == 2)
	{
		if(inputCash > 12) inputCash = 1;
		if(inputCash < 1) inputCash = 1;
		mth = inputCash;
	}
	else if(gTmp == 3)
	{
		if(inputCash > 31) inputCash = 1;
		if(inputCash < 1) inputCash = 1;
		day = inputCash;
	}
	else if(gTmp == 4)
	{
		if(inputCash > 23) inputCash = 0;
		if(inputCash < 0) inputCash = 0;
		hour = inputCash;
	}
	else if(gTmp == 5)
	{
		if(inputCash > 59) inputCash = 0;
		if(inputCash < 0) inputCash = 0;
		min = inputCash;
	}
	else if(gTmp == 6)
	{
		if(inputCash > 59) inputCash = 0;
		if(inputCash < 0) inputCash = 0;
		sec = inputCash;
	}

}
//
void InitSetValue()
{
	int tmpvalue=0;
		tmpvalue = (gBattVoltSet[0]-0x30)*1000 + 
					(gBattVoltSet[1]-0x30)*100  + 
					(gBattVoltSet[2]-0x30)*10 + 
					(gBattVoltSet[3]-0x30)*1;
    BattHiVoltage = tmpvalue * 0.1;  //  xxxx -> xxx.x
		tmpvalue = (gBattVoltSet[4]-0x30)*1000 + 
					(gBattVoltSet[5]-0x30)*100  + 
					(gBattVoltSet[6]-0x30)*10 + 
					(gBattVoltSet[7]-0x30)*1;
	BattLowVoltage = tmpvalue * 0.1;  //  xxxx -> xxx.x
		tmpvalue = (gRestartTime[0])*10;// + (gBattVoltSet[1]-0x30)*100  + (gBattVoltSet[2]-0x30)*10 + (gBattVoltSet[3]-0x30)*1;
    //RestartTimeValue = tmpvalue * 0.1;  //  xxxx -> xxx.x	
        RestartTimeValue = tmpvalue/10;  //  xxxx -> xxx.x	
}

void AutoManual()
{
	if(uc_key == KEY_AUTOMANUAL)
	{
		key1s = 0;
		uc_key = 0;
		if(fKeyAutoManual == 0)
		{
			fKeyAutoManual = 1;
			gAutoManual ^= 1;
            //Set_OK=1;//add goooo
		}
	}
	else
	{
		fKeyAutoManual=0;
	}
    //--------------------------------
	if(gAutoManual == 0)
	{
		AUTO_LED_SetLow();//active low
		MANUAL_LED_SetHigh();//active low
	}
	else
	{
		AUTO_LED_SetHigh();
		MANUAL_LED_SetLow();//active low
	}
}

void SelectModule()
{
	if(gAutoManual == 1)//manual mode//
	{
		if(uc_key == KEY_NO_SELECT)	// || (uc_key == KEY_NO_2))
		{
			key1s=0;
			uc_key = 0;
			if(fKeyNoSelect == 0)
			{
				fKeyNoSelect = 1;
				if(gSelectModule == 1)
				{
					gSelectModule = 2;
				}
				else
				{
					gSelectModule = 1;
				}
				Set_OK=1;
			}
		}
		else
		{
			fKeyNoSelect = 0;
		}
	}
    //
	if(key1s >= 5)
	{
		key1s=0;
		if(Module1_Sts[0] == 1)         gSelectModule = 1;
		else if(Module2_Sts[0] == 1)	gSelectModule = 2;
	}
}

void LedStatus()
{
	//UINT8 i=0;
	UINT8 NfbCheckErr=0;
	StatusBuf[0] = (LedState[0]&0x80)>>7;
    //StatusBuf[0] = 1;
	StatusBuf[1] = (LedState[0]&0x40)>>6;
	StatusBuf[2] = (LedState[0]&0x20)>>5;
	StatusBuf[3] = (LedState[0]&0x10)>>4;
	StatusBuf[4] = (LedState[0]&0x08)>>3;
	StatusBuf[5] = (LedState[0]&0x04)>>2;
	StatusBuf[6] = (LedState[0]&0x02)>>1;
	StatusBuf[7] = (LedState[0]&0x01);
	if(StatusBuf[0]==1)	M1_NFB_LED_SetLow();    // M1_NFB=ON;
	else M1_NFB_LED_SetHigh();                  // M1_NFB=OFF;
	//
    if(StatusBuf[4]==1)	DC1_NFB_LED_SetLow();   // DC1_NFB=ON;
	else DC1_NFB_LED_SetHigh();                 // DC1_NFB=OFF;
    //
	if(StatusBuf[1]==1)	M2_NFB_LED_SetLow();    // M2_NFB=ON;
	else M2_NFB_LED_SetHigh();                  // M2_NFB=OFF;
	//
	if(StatusBuf[5]==1) DC2_NFB_LED_SetLow();   // DC2_NFB=ON;
	else DC2_NFB_LED_SetHigh();                 // DC2_NFB=OFF;
	//
	StatusBuf[8]  = (LedState[1]&0x80)>>7;
	StatusBuf[9]  = (LedState[1]&0x40)>>6;
	StatusBuf[10] = (LedState[1]&0x20)>>5;
	StatusBuf[11] = (LedState[1]&0x10)>>4;
	StatusBuf[12] = (LedState[1]&0x08)>>3;
	StatusBuf[13] = (LedState[1]&0x04)>>2;
	StatusBuf[14] = (LedState[1]&0x02)>>1;
	StatusBuf[15] = (LedState[1]&0x01);
    //
    
        
	//
	if(StatusBuf[8]==1) BATT_NFB_LED_SetLow();  // BATT_NFB=ON;
	else BATT_NFB_LED_SetHigh();                // BATT_NFB=OFF;
	//
	if(StatusBuf[11]==1) LOAD_NFB_LED_SetLow(); // LOAD_NFB=ON;
	else LOAD_NFB_LED_SetHigh();                // LOAD_NFB=OFF;
	//
    if(gSysTypeMode == 0)
    {
       //NfbCheckErr = (StatusBuf[0] &= StatusBuf[1] &= StatusBuf[4] &= StatusBuf[5] &= StatusBuf[8] &= StatusBuf[11]);
        NfbCheckErr = (StatusBuf[0] && StatusBuf[1] && StatusBuf[4] && 
                StatusBuf[5] && StatusBuf[8] && StatusBuf[11] && 
                !StatusBuf[6]);//StatusBuf[6] is spare1(DC Ground)
    }
    else
    {
       //NfbCheckErr = (StatusBuf[0] &= StatusBuf[4] &= StatusBuf[8] &= StatusBuf[11]);
       NfbCheckErr = (StatusBuf[0] && StatusBuf[4] && StatusBuf[8] && StatusBuf[11] &&
                !StatusBuf[6]);//StatusBuf[6] is spare1(DC Ground)
    }
    //
	if(NfbCheckErr != 1)
	{
		NfbCheckErr=1;
		fCheckErr=1;	
		FAIL_LED_SetLow();      // FAIL=ON;
		NORMAL_LED_SetHigh();   // NORMAL=OFF;
	}
	else 
	{
		if(fCheckErr == 1)
		{
			FAIL_LED_SetLow();      // FAIL=ON;
			NORMAL_LED_SetHigh();   // NORMAL=OFF;		
		}
		else
		{
			NfbCheckErr = 1;
			FAIL_LED_SetHigh();     // FAIL=OFF;
			NORMAL_LED_SetLow();    // NORMAL=ON;
		}
	}	
	//NFB Status Setting Data transmit
}

void ModuleStatus()
{
	UINT8 i=0;
	UINT8 ModuleCheckErr=0;
	//#1 Module Status
	Module1_Sts[0] = (ModuleState[0]&0x80)>>7;
	Module1_Sts[1] = (ModuleState[0]&0x40)>>6;
	Module1_Sts[2] = (ModuleState[0]&0x20)>>5;//#1 in. low Volt//
	Module1_Sts[3] = (ModuleState[0]&0x10)>>4;//???ac fail
	Module1_Sts[4] = (ModuleState[0]&0x08)>>3;
	Module1_Sts[5] = (ModuleState[0]&0x04)>>2;//#1 ac fail//
	Module1_Sts[6] = (ModuleState[0]&0x02)>>1;
	Module1_Sts[7] = (ModuleState[0]&0x01);
	//#2 Module Status
	Module2_Sts[0] = (ModuleState[1]&0x80)>>7;
	Module2_Sts[1] = (ModuleState[1]&0x40)>>6;
	Module2_Sts[2] = (ModuleState[1]&0x20)>>5;
	Module2_Sts[3] = (ModuleState[1]&0x10)>>4;
	Module2_Sts[4] = (ModuleState[1]&0x08)>>3;
	Module2_Sts[5] = (ModuleState[1]&0x04)>>2;
	Module2_Sts[6] = (ModuleState[1]&0x02)>>1;
	Module2_Sts[7] = (ModuleState[1]&0x01);
    //
    if(gSysTypeMode == 0)
    {
        if(Module1_Sts[0]==1)
        {
			//NO_1=ON,M1_LINE=ON,OUT_LINE=ON,
			NO1_LED_SetLow();
			M1_LINE_LED_SetLow();
			OUT_LINE_LED_SetLow();
			//NO_2=OFF,M2_LINE=OFF;	// 01 : #1 Module Run
			NO2_LED_SetHigh();
			M2_LINE_LED_SetHigh();
	   }	
	   //       
       if(Module2_Sts[0]==1)
	   {
		   	//NO_2=ON,M2_LINE=ON,OUT_LINE=ON,
			NO2_LED_SetLow();
			M2_LINE_LED_SetLow();
			OUT_LINE_LED_SetLow();   
			//NO_1=OFF,M1_LINE=OFF;	// 03 : #2 Module Run	
			NO1_LED_SetHigh();
			M1_LINE_LED_SetHigh();
	   }	
       
       for(i = 1; i < 8; i++)
       {
           ModuleCheckErr |= Module1_Sts[i];
           ModuleCheckErr |= Module2_Sts[i];
       }
   }
   else
   {
       if(Module1_Sts[0]==1)	
	   {
		   //NO_1=ON,M1_LINE=ON,OUT_LINE=ON,
		   NO1_LED_SetLow();
		   M1_LINE_LED_SetLow();
		   OUT_LINE_LED_SetLow();
		   //NO_2=OFF,M2_LINE=OFF;	// 01 : #1 Module Run
		   NO2_LED_SetHigh();
		   M2_LINE_LED_SetHigh();
	   }	   
       //
       for(i = 1; i < 8; i++)
       {
           ModuleCheckErr |= Module1_Sts[i];
       }
   }
       
	if(ModuleCheckErr != 0)
	{
		ModuleCheckErr=0;
		fCheckErr=1;		
		FAIL_LED_SetLow();      //on		
		NORMAL_LED_SetHigh();   //off
	}
	else 
	{
		if(fCheckErr == 1 || fNfbCheckErr == 1)
		{			
			FAIL_LED_SetLow();			
			NORMAL_LED_SetHigh();	
		}
		else
		{
			ModuleCheckErr=0;			
			FAIL_LED_SetHigh();			
			NORMAL_LED_SetLow();
		}
	}
    //--------------------------------------------------------------------------
	if(Module1_Sts[1] != prev_Module1_Sts[1])
	{
		if(Module1_Sts[1] == 1)
		{
			ErrCode = 11;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Module1_Sts[1] == 1)
		{
			prev_Module1_Sts[1]=0;
		}
	}
	else if(Module1_Sts[2] != prev_Module1_Sts[2])
	{
		if(Module1_Sts[2] == 1)
		{
			ErrCode = 12;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Module1_Sts[2] == 1)
		{
			prev_Module1_Sts[2]=0;
		}
	}
	else if(Module1_Sts[3] != prev_Module1_Sts[3])
	{
		if(Module1_Sts[3] == 1)
		{
			ErrCode=13;//#1 ac fail?//
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Module1_Sts[3] == 1)
		{
			prev_Module1_Sts[3]=0;
		}
	}
	else if(Module1_Sts[4] != prev_Module1_Sts[4])
	{
		if(Module1_Sts[4] == 1)
		{
			ErrCode=14;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Module1_Sts[4] == 1)
		{
			prev_Module1_Sts[4]=0;
		}
	}
	else if(Module1_Sts[5] != prev_Module1_Sts[5])
	{
		if(Module1_Sts[5] == 1)
		{
			ErrCode=15;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Module1_Sts[5] == 1)
		{
			prev_Module1_Sts[5]=0;
		}
	}
	else if(Module1_Sts[6] != prev_Module1_Sts[6])
	{
		if(Module1_Sts[6] == 1)
		{
			ErrCode=16;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Module1_Sts[6] == 1)
		{
			prev_Module1_Sts[6]=0;
		}
	}
	else if(Module1_Sts[7] != prev_Module1_Sts[7])
	{
		if(Module1_Sts[7] == 1)
		{
			ErrCode=17;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Module1_Sts[7] == 1)
		{
			prev_Module1_Sts[7]=0;
		}
	}
   //---------------------------------------------------------------------------
   if(gSysTypeMode == 0)
   {
       if(Module2_Sts[1] != prev_Module2_Sts[1])
       {
           if(Module2_Sts[1] == 1)
           {
               ErrCode=21;
               AlarmOccurrenceApply(gPacketsize);
           }
           else if(prev_Module2_Sts[1] == 1)
           {
               prev_Module2_Sts[1]=0;
           }
       }
       else if(Module2_Sts[2] != prev_Module2_Sts[2])
       {
           if(Module2_Sts[2] == 1)
           {
               ErrCode=22;
               AlarmOccurrenceApply(gPacketsize);
           }
           else if(prev_Module2_Sts[2] == 1)
           {
               prev_Module2_Sts[2]=0;
           }
       }
       else if(Module2_Sts[3] != prev_Module2_Sts[3])
       {
           if(Module2_Sts[3] == 1)
           {
               ErrCode=23;
               AlarmOccurrenceApply(gPacketsize);
               
           }
           else if(prev_Module2_Sts[3] == 1)
           {
               prev_Module2_Sts[3]=0;
               
           }
       }
       else if(Module2_Sts[4] != prev_Module2_Sts[4])
       {
           if(Module2_Sts[4] == 1)
           {
               ErrCode=24;
               AlarmOccurrenceApply(gPacketsize);
           }
           else if(prev_Module2_Sts[4] == 1)
           {
               prev_Module2_Sts[4]=0;
           }
       }
       else if(Module2_Sts[5] != prev_Module2_Sts[5])
       {
           if(Module2_Sts[5] == 1)
           {
               ErrCode=25;
               AlarmOccurrenceApply(gPacketsize);
           }
           else if(prev_Module2_Sts[5] == 1)
           {
               prev_Module2_Sts[5]=0;
           }
       }
       else if(Module2_Sts[6] != prev_Module2_Sts[6])
       {
           if(Module2_Sts[6] == 1)
           {
               ErrCode=26;
               AlarmOccurrenceApply(gPacketsize);
           }
           else if(prev_Module2_Sts[6] == 1)
           {
               prev_Module2_Sts[6]=0;
           }
       }
       else if(Module2_Sts[7] != prev_Module2_Sts[7])
       {
           if(Module2_Sts[7] == 1)
           {
               ErrCode=27;
               AlarmOccurrenceApply(gPacketsize);
           }
           else if(prev_Module2_Sts[7] == 1)
           {
               prev_Module2_Sts[7]=0;
           }
       }
   }
}

void BatteryStatus()
{
	UINT8 i=0;
	UINT8 BattCheckErr=0;
	Battery_Sts[0] = (BatteryState[0]&0x80)>>7;
	Battery_Sts[1] = (BatteryState[0]&0x40)>>6;
	Battery_Sts[2] = (BatteryState[0]&0x20)>>5;
	//Battery_Sts[3] = (BatteryState[0]&0x10)>>4;
	for(i = 0; i < 2; i++)
	{
		BattCheckErr |= Battery_Sts[i];
	}
	if(BattCheckErr != 0)
	{
		BattCheckErr=0;
		fCheckErr=1;
		FAIL_LED_SetLow();
		NORMAL_LED_SetHigh();
	}
	else 
	{
		if(fCheckErr == 1)
		{
			FAIL_LED_SetLow();
			NORMAL_LED_SetHigh();
		}
		else
		{
			BattCheckErr=0;
			FAIL_LED_SetHigh();
			NORMAL_LED_SetLow();
		}
	}
	if(Battery_Sts[2]==0)       fCharge=' ';	//Charge
	else if(Battery_Sts[2]==1)	fCharge='-';	//Discharge
	//
	if(Battery_Sts[0] != prev_Battery_Sts[0])
	{
		if(Battery_Sts[0] == 1)
		{
			ErrCode=51;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Battery_Sts[0] == 1)
		{
			prev_Battery_Sts[0]=0;
		}
	}
	else if(Battery_Sts[1] != prev_Battery_Sts[1])
	{
		if(Battery_Sts[1] == 1)
		{
			ErrCode=52;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Battery_Sts[1] == 1)
		{
			prev_Battery_Sts[1]=0;
		}
	}
}

void WarningStatus()
{
	UINT8 i=0;
	UINT8 WarningCheckErr=0;
	Warning_Sts[0] = (WarningState[0]&0x80)>>7;
	Warning_Sts[1] = (WarningState[0]&0x40)>>6;
	Warning_Sts[2] = (WarningState[0]&0x20)>>5;
	Warning_Sts[3] = (WarningState[0]&0x10)>>4;
	Warning_Sts[4] = (WarningState[0]&0x08)>>3;
	Warning_Sts[5] = (WarningState[0]&0x04)>>2;
	Warning_Sts[6] = (WarningState[0]&0x02)>>1;
	Warning_Sts[7] = (WarningState[0]);
	for(i = 0; i < 4; i++)
	{
		WarningCheckErr |= Warning_Sts[i];
	}
	if(WarningCheckErr != 0)
	{
		WarningCheckErr=0;
		fCheckErr=1;
		FAIL_LED_SetLow();      //active low
		NORMAL_LED_SetHigh();   //active low
	}
	else 
	{
		if(fCheckErr == 1)
		{
			FAIL_LED_SetLow();
			NORMAL_LED_SetHigh();
		}
		else
		{
			WarningCheckErr=0;
			FAIL_LED_SetHigh();
			NORMAL_LED_SetLow();
		}
	}
	if(Warning_Sts[0] != prev_Warning_Sts[0])
	{
		if(Warning_Sts[0] == 1)
		{
			ErrCode=61;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Warning_Sts[0] == 1)
		{
			prev_Warning_Sts[0]=0;
		}
	}
	else if(Warning_Sts[1] != prev_Warning_Sts[1])
	{
		if(Warning_Sts[1] == 1)
		{
			ErrCode=62;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Warning_Sts[1] == 1)
		{
			prev_Warning_Sts[1]=0;
		}
	}
	else if(Warning_Sts[2] != prev_Warning_Sts[2])
	{
		if(Warning_Sts[2] == 1)
		{
			ErrCode=63;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Warning_Sts[2] == 1)
		{
			prev_Warning_Sts[2]=0;
		}
	}
	else if(Warning_Sts[3] != prev_Warning_Sts[3])
	{
		if(Warning_Sts[3] == 1)
		{
			ErrCode=64;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_Warning_Sts[3] == 1)
		{
			prev_Warning_Sts[3]=0;
		}
	}
}

void ModuleStartupStatus()
{
	ModuleStartup_Sts[0] = Module1_Sts[0];
	ModuleStartup_Sts[1] = Module2_Sts[0];
	if(ModuleStartup_Sts[0] != prev_ModuleStartup_Sts[0])
	{
		if(Module1_Sts[0] == 1)
		{
			ErrCode=71;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_ModuleStartup_Sts[0] == 1)
		{
			prev_ModuleStartup_Sts[0]=0;
		}
	}
	else if(ModuleStartup_Sts[1] != prev_ModuleStartup_Sts[1])
	{
		if(Module2_Sts[0] == 1)
		{
			ErrCode=72;
			AlarmOccurrenceApply(gPacketsize);
		}
		else if(prev_ModuleStartup_Sts[1] == 1)
		{
			prev_ModuleStartup_Sts[1]=0;
		}
	}
}

void ChargeStatus(void)
{
	Module_CHG_Sts[0] = (ChargeState[0]&0x80)>>7;
	Module_CHG_Sts[1] = (ChargeState[0]&0x40)>>6;
	Module_CHG_Sts[2] = (ChargeState[0]&0x20)>>5;
	Module_CHG_Sts[3] = (ChargeState[0]&0x10)>>4;
	Module_CHG_Sts[4] = (ChargeState[0]&0x08)>>3;
	Module_CHG_Sts[5] = (ChargeState[0]&0x04)>>2;
	Module_CHG_Sts[6] = (ChargeState[0]&0x02)>>1;
	Module_CHG_Sts[7] = (ChargeState[0]);

	if(Module_CHG_Sts[1] != prev_Module_CHG_Sts[1])
	{
		if(Module_CHG_Sts[1] == 1)
		{
			ErrCode = 81; // float
			AlarmOccurrenceApply(gPacketsize);
		}
       else if(Module_CHG_Sts[1] == 0)
		{
			ErrCode = 82; // Equal
			AlarmOccurrenceApply(gPacketsize);
		}
	}
	if(Module_CHG_Sts[3] != prev_Module_CHG_Sts[3])
	{
		if(Module_CHG_Sts[3] == 1)
		{
			ErrCode = 83; // float
			AlarmOccurrenceApply(gPacketsize);
		}
       else if(Module_CHG_Sts[3] == 0)
		{
			ErrCode = 84; // Equal
			AlarmOccurrenceApply(gPacketsize);
		}
	}
}

void dcGroundCheck(void)
{
    if(StatusBuf[6] != gOldState)
    {
        if(StatusBuf[6] == 1)
        {
            if(++dcGroundCnt>10)
            {
                ErrCode = 99;
                AlarmOccurrenceApply(gPacketsize);
                gOldState = StatusBuf[6];
            }    
        }
        else
        {
            if(dcGroundCnt>0)dcGroundCnt--;
            else  gOldState = StatusBuf[6];
        }
            
    }else dcGroundCnt = 0;
    
}

void bzOperation(void)
{
    if(fAlarmStop == 1 && fCheckErr == 1)	// || fNfbCheckErr == 1))
	{
		if(errBz100ms>20)
		{
			BZ_SetLow();   
		}
		if(errBz100ms>40)
		{
			errBz100ms=0;
			BZ_SetHigh();//active high
            
		}
		fCheckErr=0;
		//fNfbCheckErr=0;
	}
	else if(fAlarmStop == 0 && fCheckErr == 1)	// || fNfbCheckErr == 1))
	{
		fCheckErr=0;
		//fNfbCheckErr=0;
		BZ_SetLow();
	}
	else BZ_SetLow(); 
    //-------------------
//    if(fBzOn==1)
//    {
//        BZ_SetHigh();   
//        if(tmrBz <= 1)
//		{
//			BZ_SetLow();
//            fBzOn =0;
//		}		
//    }
    
}