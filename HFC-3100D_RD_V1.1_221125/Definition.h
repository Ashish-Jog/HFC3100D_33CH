//====================================================================================//
//#define 	FCY  			7500000		//7.5 MIPs, Fosc=30MHz
//#define 	TMR1_PERIOD		46875		//FCY/(prescaler*20Hz),prescaler=8//   50ms
//#define 	TMR2_PERIOD		937			//FCY/(prescaler*1kHz),prescaler=8//  	1ms
#define		ADCNT			15				//moving average counter.
//#define     BAUDRATE		9600                      //9600bps
//#define     BRGVAL			(((FCY/BAUDRATE)/16)-1)        //KHJ 
#define     RS232_Period    200         //200ms

#define M0_LOGO					1
#define M0_RECTINFO				2
#define RDVERSION 110 //v1.10  
//////// LED define ////////////////////////
//active low
#define	SP1                 LATCbits.LATC12 //LATGbits.LATG15
#define	SP2                 LATCbits.LATC13	//LATCbits.LATC1
#define	Normal_LED          LATCbits.LATC1//		LATBbits.LATB9
#define	Fault_LED			LATCbits.LATC2//	LATBbits.LATB10
#define	Float_LED           LATCbits.LATC6//	LATBbits.LATB11

//#define	Manual_LED          LATBbits.LATB6	//LATBbits.LATB13
//#define	Equal_LED      LATBbits.LATB7    //LATBbits.LATB14

#define	Equal_LED          LATBbits.LATB6	//LATBbits.LATB13
#define	Manual_LED      LATBbits.LATB7    //LATBbits.LATB14

//////// LCD define ////////////////////////
#define	BL				LATCbits.LATC4//	LATDbits.LATD11

#define	DIR_BL			TRISCbits.TRISC4

//////// Buzzer define ////////////////////////
#define BZ					LATDbits.LATD15
#define BZ_ON()				LATDbits.LATD15=1
#define BZ_OFF()			LATDbits.LATD15=0
#define	DIR_BZ				TRISDbits.TRISD15

//////// RTC define ///////////////////////////
#define   RTC_RST     	LATDbits.LATD9//LATBbits.LATB15	//PORTBbits.RB15
#define   RTC_CLK     	LATDbits.LATD10//PORTGbits.RG2
#define   RTC_IO        LATDbits.LATD11//PORTGbits.RG3

#define   RTC_IO_PIN    _RD11

#define   RTC_RST_DIR  	TRISDbits.TRISD9//TRISBbits.TRISB15
#define   RTC_CLK_DIR  	TRISDbits.TRISD10
#define   RTC_IO_DIR 	TRISDbits.TRISD11

/////// TEST Point define //////////////////
#define TP1					LATBbits.LATB5
#define TP2					LATDbits.LATD14
//#define TP3					LATGbits.LATG7
#define TP4					LATCbits.LATC3
//#define TP5					LATBbits.LATB8
	

//#define		export(a,b)	(XBYTE[a]=b)	/* a 번�??�다 b??값을 보낸?? */
//#define		import(a)	(XBYTE[a])      /* i = import(a) : a 번�????�용??i registor�?. */

#define		LCD_ON		0x3f			/* LCD display on command code */
#define		LCD_OFF		0x3e			/* LCD display off command code */
#define		START		0xc0			/* Y-address start possition code(0-63)*/
#define		X_LINE		0x40			/* X-address set code(0-63) */
#define		Y_LINE		0xb8			/* Y-page address set code(0-7)*/
#define		ALM_ON		0x61			/* alarm status generation */
#define		ALM_OFF		0x60			/* alarm status un-generation */

// utillity pin define
//#define		WDT			LATGbits.LATG8//	P1_0			/* watch dog timer status active LO */
#define		CS1			LATBbits.LATB11			/* chip sellection 1 */
#define		CS2			LATBbits.LATB10			/* chip sellection 2 */
#define		D_I			LATDbits.LATD8			/* lcd command instruction */
#define		R_W			LATDbits.LATD7			/* lcd read & write signal */
#define		EN			LATDbits.LATD6          //|LATCbits.LATC2			/* lcd eneable signal */
#define		LCD_RST		LATDbits.LATD5			/* lcd reset port */

#define		DIR_CS1		TRISBbits.TRISB11	// Direction of control pins
#define		DIR_CS2		TRISBbits.TRISB10
#define		DIR_D_I		TRISDbits.TRISD8
#define		DIR_R_W		TRISDbits.TRISD7
#define		DIR_EN		TRISDbits.TRISD6
#define		DIR_LCD_RST		TRISDbits.TRISD5
//
#define     db7 LATDbits.LATD0
#define     db6 LATDbits.LATD1
#define     db5 LATDbits.LATD2
#define     db4 LATDbits.LATD3
#define     db3 LATDbits.LATD4
#define     db2 LATCbits.LATC11
#define     db1 LATCbits.LATC10
#define     db0 LATCbits.LATC5
//
#define		LCD_DATA	db7<<7|db6<<6|db5<<5|db4<<4|db3<<3|db2<<2|db1<<1|db0

//#define		MICOM		P1_3			/* microprocessor status */
#define		ESC			PORTBbits.RB12      //active low//
#define		ENT			PORTBbits.RB13
#define		DOWN		PORTBbits.RB14
#define		UP			PORTBbits.RB15

//#define		DIR_MICOM		P1_3			/* microprocessor status */
#define		DIR_ESC			TRISBbits.TRISB12
#define		DIR_ENT			TRISBbits.TRISB13		/* lcd reset port */
#define		DIR_DOWN		TRISBbits.TRISB14
#define		DIR_UP			TRISBbits.TRISB15

#define		HI              1				/* bit hgih(51-command:setb) */
#define		LO              0				/* bit low(51-command:clr) */
#define     ON      		(0)   //active low//
#define     OFF     		(!ON)
#define     INPUT			(1)
#define     OUTPUT			(0)

//  LCD Module commands --- These settings can be found in the LCD datasheet
#define DISP_2Line_8Bit		0x0038		// 2 lines & 8 bits setting
#define DISP_2Line_4Bit		0x0028		// 2 lines & 4 bits setting
#define DISP_ON				0x3F
#define DISP_ON_C			0x00E		// Display on, Cursor on
#define DISP_ON_B			0x00F		// Display on, Cursor on, Blink cursor
#define DISP_OFF			0x3E
#define CLR_DISP			0x001		// Clear the Display
#define ENTRY_INC			0x006		// Entry Increment and Cursor Move
#define ENTRY_INC_S			0x007		// Entry Increment and Display Shift
#define ENTRY_DEC			0x004		// Entry Decrement and Cursor Move
#define ENTRY_DEC_S			0x005		// Entry Decrement and Display Shift
#define DD_RAM_ADDR			0x080		// Least Significant 7-bit are for address
#define DD_RAM_UL			0x080		// Upper Left coner of the Display	

#define	REFRESH_LCD 	60				/* lcd reinitalizing time(1sec X REFRESH_LCD) */
#define _NOP()		__asm__ volatile ("NOP")  //about 260ns//
#define sdiv(a,b)			__builtin_divsd(a,b)
//typedef enum{false,true} 	BOOL;

/* Compiler-independent, fixed size types */
typedef signed char     	INT8 ;      /* Signed 8-bit integer    */
typedef unsigned char      	UINT8 ;     /* Unsigned 8-bit integer  */
typedef signed short int   	INT16 ;     /* Signed 16-bit integer   */
typedef unsigned short int 	UINT16 ;    /* Unsigned 16-bit integer */
typedef long int           	INT32 ;     /* Signed 32-bit integer   */
typedef unsigned long      	UINT32 ;
typedef float              	FLOAT32 ;   /* 32-bit IEEE single precisiON */
typedef double             	FLOAT64 ;   /* 64-bit IEEE double precisiON */

/***************************************************************************
 *  specific definitiONs
 ***************************************************************************/
typedef struct 
{
	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;	
} bool8;

typedef struct 
{
	unsigned b0 :1,b1 :1,b2 :1,b3 :1,b4 :1,b5 :1,b6 :1,b7 :1;
	unsigned b8 :1,b9 :1,b10:1,b11:1,b12:1,b13:1,b14:1,b15:1;
} bool16;

typedef struct  Tds1302_Time
{      
 		UINT8 Second;
 		UINT8 Minute;
 		UINT8 Hour;
 		UINT8 Date;
 		UINT8 Month;
 		UINT8 Year;
 		UINT8 Am;
 		UINT8 Md;
} ds1302_Time;
//struct Tds1302_Time  Ds1302_Time;

//////// EEPROM Address /////////
#define INIT_STATE				1
#define OP_MODE                 6
#define INPUT_VOLT              7
#define BATT_TYPE               8
#define ALARM_STOP				9
//TEMP
#define ALARM_TEMP_0			10
#define ALARM_TEMP_1			11
#define ALARM_TEMP_2			12
#define ALARM_TEMP_3			13
//Password
#define PASSWD_0				20
#define PASSWD_1				21
#define PASSWD_2			    22
#define PASSWD_3			    23
#define PASSWD_4			    24
#define PASSWD_5			    25
#define PASSWD_6			    26
#define PASSWD_7			    27
#define PASSWD_8			    28
#define PASSWD_9			    29
//Batt
#define OUTVALARM_0				30
#define OUTVALARM_1				31
#define OUTVALARM_2				32
#define OUTVALARM_3				33
#define OUTVALARM_4				34
#define OUTVALARM_5				35
#define OUTVALARM_6				36
#define OUTVALARM_7				37
#define OUTVALARM_8				38
#define OUTVALARM_9				39
//OutVoltSet	//OutVoleSet add JSS 2018.01.08
#define OUTAALARM_0				40
#define OUTAALARM_1				41
#define OUTAALARM_2				42
#define OUTAALARM_3				43
#define OUTAALARM_4				44
#define OUTAALARM_5				45
#define OUTAALARM_6				46
#define OUTAALARM_7				47
#define OUTAALARM_8				48
#define OUTAALARM_9				49
//BattVoltSet	//BattVoltSet add JSS 2018.01.08
#define BATTVOLT_0				50
#define BATTVOLT_1				51
#define BATTVOLT_2				52
#define BATTVOLT_3				53
#define BATTVOLT_4				54
#define BATTVOLT_5				55
#define BATTVOLT_6				56
#define BATTVOLT_7				57
//Current Limit
#define LOADAMP_0               58
#define LOADAMP_1               59
#define LOADAMP_2               60
#define LOADAMP_3               61
//BattAmpSet add JSS 2018.06.11
#define BATTAMP_0				62
#define BATTAMP_1				63
#define BATTAMP_2				64
#define BATTAMP_3				65
//involt Gain set add JSS 2018.01.18
#define INVALARM_0				70
#define INVALARM_1				71
#define INVALARM_2				72
#define INVALARM_3				73
#define INVALARM_4				74
#define INVALARM_5				75
#define INVALARM_6				76
#define INVALARM_7				77
#define INVALARM_8				78
#define INVALARM_9				79
//OUTV_ShutDown
#define SHUTDOWNOUTV_0			80
#define SHUTDOWNOUTV_1			81
#define SHUTDOWNOUTV_2			82
#define SHUTDOWNOUTV_3			83
#define SHUTDOWNOUTV_4			84
#define SHUTDOWNOUTV_5			85
#define SHUTDOWNOUTV_6			86
#define SHUTDOWNOUTV_7			87
#define SHUTDOWNOUTV_8			88
#define SHUTDOWNOUTV_9			89
//OUTA_ShutDown
#define SHUTDOWNOUTA_0			90
#define SHUTDOWNOUTA_1			91
#define SHUTDOWNOUTA_2			92
#define SHUTDOWNOUTA_3			93
#define SHUTDOWNOUTA_4			94
#define SHUTDOWNOUTA_5			95
#define SHUTDOWNOUTA_6			96
#define SHUTDOWNOUTA_7			97
#define SHUTDOWNOUTA_8			98
#define SHUTDOWNOUTA_9			99
//ADC Gain
#define ADJVGain_0				100
#define ADJCGain_0				101
//CHG 
#define CHG_STS_0               102
#define CHG_STS_1               103
//CHG Set No.
#define CHG_SET_NO              105
//CHG Set time
#define CHG_SET_TIME            106
//CHG Set Current
#define CHG_SET_CURR            107

//Set time History
#define SET_YEAR_0              110
#define SET_MONTH_0             111
#define SET_DAY_0               112
#define SET_HOUR_0              113
#define SET_MIN_0               114
#define SET_SEC_0               115
#define SET_YEAR_1              116
#define SET_MONTH_1             117
#define SET_DAY_1               118
#define SET_HOUR_1              119
#define SET_MIN_1               120
#define SET_SEC_1               121
#define SET_YEAR_2              122
#define SET_MONTH_2             123
#define SET_DAY_2               124
#define SET_HOUR_2              125
#define SET_MIN_2               126
#define SET_SEC_2               127
#define SET_YEAR_3              128
#define SET_MONTH_3             129
#define SET_DAY_3               130
#define SET_HOUR_3              131
#define SET_MIN_3               132
#define SET_SEC_3               133
#define SET_YEAR_4              134
#define SET_MONTH_4             135
#define SET_DAY_4               136
#define SET_HOUR_4              137
#define SET_MIN_4               138
#define SET_SEC_4               139
#define SET_YEAR_5              140
#define SET_MONTH_5             141
#define SET_DAY_5               142
#define SET_HOUR_5              143
#define SET_MIN_5               144
#define SET_SEC_5               145
#define SET_YEAR_6              146
#define SET_MONTH_6             147
#define SET_DAY_6               148
#define SET_HOUR_6              149
#define SET_MIN_6               150
#define SET_SEC_6               151
#define SET_YEAR_7              152
#define SET_MONTH_7             153
#define SET_DAY_7               154
#define SET_HOUR_7              155
#define SET_MIN_7               156
#define SET_SEC_7               157
#define SET_YEAR_8              158
#define SET_MONTH_8             159
#define SET_DAY_8               160
#define SET_HOUR_8              161
#define SET_MIN_8               162
#define SET_SEC_8               163
#define SET_YEAR_9              164
#define SET_MONTH_9             165
#define SET_DAY_9               166
#define SET_HOUR_9              167
#define SET_MIN_9               168
#define SET_SEC_9               169
//-------------------------------------------------

//#define MENU_LOGO                   0
#define MENU_CHARGER_INFO           0
#define MENU_AC_INFO                1
#define MENU_TIME_DISP              2
#define MENU_TEMPERATURE            3
#define MENU_LOGIN                  4
#define MENU_CHARGE_VOLT_SET_LI     5//9-3
#define MENU_CHARGE_VOLT_SET_PB     6//10-3
#define MENU_MANUAL_SET             7//11-3
#define MENU_CURRENT_LIMIT          8//12-3
#define MENU_INPUT_ALARM_SET        9//13-3
#define MENU_OUTPUT_ALARM_SET       10//14-3
#define MENU_OUT_CUR_ALARM_SET      11//15-3
#define MENU_CUTOFF_VOLT_SET        12//16-3
#define MENU_BUZZER_SET             13//17-3
#define MENU_TIME_SET               14//18-3
#define MENU_PASS_SET               15//19-3
#define MENU_SYS_INIT               16//20-3
#define MENU_ADCGAIN            17  	//21-3
#define MENU_CHG_MODE_SET       18//    18//22-3
#define MENU_CHG_TIME_SET       19//    19//23-3
#define MENU_CHG_CONDITION_SET      20//24-3
#define MENU_DEBUG              21    //25-3
#define MENU_DEVICE_RUNNING    22//    26-3
#define MENU_BATTYPE_ACVOLT    23// 27-3


 


void initLCD(void);
void lcd_refresh();											// lcd reinitailizing(timer) function
void key_oper();
void key_clr();
void delay(unsigned int cnt);										// polling delay timer

//====================================================================================//
//                          Variables Declaration                                     //
//====================================================================================//
//extern unsigned int tmr1Count1,tmr1Count3,tmr1Count4;
extern volatile unsigned int tmr1_60sec;
//extern unsigned int tmr2Count0,tmr2Count1,tmr2Count2,tmr2Count3;
//extern volatile unsigned int tmr200ms,tmr500ms,tmr1Min,tmr1hour;
extern volatile unsigned int tmr1Min;
extern volatile unsigned int errBz100ms;
//extern unsigned int soft_1sec[5];
extern unsigned int iMenuIndex;//[5] = {0,0,0,0,0};
extern unsigned int fManVolt;
extern unsigned int fEepromRead;
//extern unsigned char lampTestFlag;

//extern unsigned char Cnt,Cnt1,initCnt;
//extern unsigned char fTestPage,fTest;
extern unsigned char fKey,setKey;
extern unsigned char fAlarmStop;
extern unsigned char fbattVoltSet,fOutVoltSet,fGain;
extern unsigned char fEqFl, Auto_Flag;
extern unsigned char fFaultState,fAlarmState;

extern unsigned char day,mth,year,dow,hour,min,sec,sec_old;
extern unsigned char day_buf,mth_buf,year_buf,hour_buf,min_buf,sec_buf;
//extern unsigned char CURTIME[12];
//extern unsigned char debug_buffer[3];
extern unsigned char temp_buffer[15];//={0,};				// 15byte
extern unsigned char gain_buffer[6];

extern unsigned char gPin[10];
extern unsigned char gPin2[10];
//extern unsigned char gSerial[8];
//extern unsigned char kVaSet[8];
//extern unsigned char kVaSetOld[8];
extern unsigned char battSet[3];
extern unsigned char battVoltSet[10];
//extern unsigned char battVoltSetOld[10];
extern unsigned char battAmpSet[10];
extern unsigned char LoadAmpSet[10];
extern unsigned char goutAalarm[10];

//extern unsigned char outSetOld[10];

extern unsigned char goutValarm[10];
extern unsigned char ginValarm[10];
extern unsigned char timeBuf[12];
extern unsigned char Set_time[13];
extern unsigned char Set_COND[4];
extern unsigned char outSetNew[10];
extern unsigned char shutDownOutV[10];
extern unsigned char shutDownOutA[10];
extern unsigned char vGain;
extern unsigned char cGain;
extern unsigned char CHG_Cut_TIME;
extern unsigned char CHG_Cut_CURR;
extern volatile unsigned int Cut_Timer;
extern unsigned char Set_cnt;

extern unsigned int cs_bit;							/* chip sellection flag */
extern unsigned char modi, modi_debug;							/* modify motion flag */
extern unsigned char up,down,esc,enter;
extern unsigned char up_old,down_old,esc_old,enter_old;
extern unsigned char poss_x,poss_y;
extern unsigned char cal_error,modi_end;
//extern unsigned int		*XBYTE;
extern unsigned char Key_Cnt;
extern unsigned char err_menu;
extern unsigned char bar_poss;
extern unsigned char buff_8;
extern unsigned int hex1_16;
extern unsigned char hex1_8;
extern unsigned char thnd,hund,ten,one;
extern unsigned char gCnt;
extern float Set_Input_UV;
extern float Set_Input_OV;
extern unsigned char Output_OV,Output_UV;
extern float Set_Output_OV;
extern float Set_Output_UV;

//extern unsigned char dg10, dg1;



