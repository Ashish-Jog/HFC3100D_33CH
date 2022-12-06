#include "Definition.h"
#include "Alram_Sts.h"

//====================================================================================//
//                          Variables Declaration                                     //
//====================================================================================//
//unsigned int tmr1Count1,tmr1Count3,
volatile unsigned int tmr1_60sec=0;
//unsigned int tmr2Count0,tmr2Count1,tmr2Count2,tmr2Count3;
//volatile unsigned int tmr200ms=0,tmr500ms=0,tmr1Min=0, tmr1hour=0;
volatile unsigned int tmr1Min=0;
volatile unsigned int errBz100ms;
//unsigned int soft_1sec[5];
unsigned int iMenuIndex=0;//[5] = {0,0,0,0,0};
unsigned int fManVolt = 0;//manual voltage setting menu //
unsigned int fEepromRead = 0;//equal time set lcd flickery element
//unsigned char lampTestFlag;

//unsigned char Cnt=0,Cnt1=0,initCnt=0;
//unsigned char fTestPage=0,fTest=0;
unsigned char fKey=0,setKey=0;
unsigned char fAlarmStop=0;
unsigned char fbattVoltSet=0,fOutVoltSet=0,fGain=0;
unsigned char fEqFl=0, Auto_Flag = 0;
unsigned char fFaultState=0,fAlarmState=0;

unsigned char day,mth,year,dow,hour,min,sec,sec_old;
unsigned char day_buf,mth_buf,year_buf,hour_buf,min_buf,sec_buf;
//unsigned char CURTIME[12]={0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
//unsigned char debug_buffer[3];
unsigned char temp_buffer[15];//={0,};				// 15byte
unsigned char gain_buffer[6];

unsigned char gPin[10]={0,};
unsigned char gPin2[10]={0,};
//unsigned char gSerial[8]={0,};
//unsigned char kVaSet[8]={0,};
//unsigned char kVaSetOld[8]={0,};
unsigned char battSet[3]={0,};
unsigned char battVoltSet[10]={0,};
//unsigned char battVoltSetOld[10]={0,};
unsigned char battAmpSet[10]={0,};
unsigned char LoadAmpSet[10]={0,};
unsigned char goutAalarm[10]={0,};

unsigned char goutValarm[10]={0,};
unsigned char ginValarm[10]={0,};
unsigned char timeBuf[12]={0,};
unsigned char Set_time[13]={0,};
unsigned char Set_COND[4]={0,};
unsigned char outSetNew[10]={0,};
unsigned char shutDownOutV[10]={0,};
unsigned char shutDownOutA[10]={0,};
unsigned char vGain = 0;
unsigned char cGain = 0;
unsigned char CHG_Cut_TIME = 0;
unsigned char CHG_Cut_CURR = 0;
volatile unsigned int Cut_Timer = 0;
unsigned char Set_cnt=0;


unsigned int cs_bit;							/* chip sellection flag */
unsigned char modi, modi_debug;							/* modify motion flag */
unsigned char up,down,esc,enter;
unsigned char up_old,down_old,esc_old,enter_old;
unsigned char poss_x,poss_y;
unsigned char cal_error,modi_end;
//unsigned int		*XBYTE;
unsigned char Key_Cnt = 0;
unsigned char err_menu;
unsigned char bar_poss;
unsigned char buff_8;
unsigned int hex1_16;
unsigned char hex1_8;
unsigned char thnd,hund,ten,one;
unsigned char gCnt=0;
float Set_Input_UV = 0;
float Set_Input_OV = 0;
unsigned char Output_OV=0,Output_UV=0;
float Set_Output_OV=0;
float Set_Output_UV=0;

//unsigned char dg10 =0, dg1=0;


