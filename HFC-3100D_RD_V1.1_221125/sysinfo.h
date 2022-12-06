//extern int read_EE1(int tbl_offset);
//extern void write_EE1(int tbl_offset, int data);		//4ms

//----------------------------------------------------------------------------
// User Variable
//------------------------------------------
#define ACV_HI_DFT		418		//LCD-418 ACV
#define ACV_HI_MAX		430
#define ACV_HI_MIN		380

#define ACV_LOW_DFT		342		//LCD-325 ACV
#define ACV_LOW_MAX		380
#define ACV_LOW_MIN		325

#define ACI_HI_DFT		15		//15A
#define ACI_HI_MAX		20
#define ACI_HI_MIN		10
			//
#define LDV_HI_DFT		550		//LCD-50.0 DCV
#define LDV_HI_MAX		600
#define LDV_HI_MIN		490

#define LDV_LOW_DFT		460		//LCD-46.0 DCV
#define LDV_LOW_MAX		490
#define LDV_LOW_MIN		430
			//
#define BTV_HI_DFT		600		//LCD-60.0 DCV
#define BTV_HI_MAX		600
#define BTV_HI_MIN		530

#define BTV_LOW_DFT		500
#define BTV_LOW_MAX		530
#define BTV_LOW_MIN		460

#define FLT_V_DFT		535
#define EQ_V_DFT		535

#ifdef MODULE50A
	#define MOD_CUR_DFT	550		//55A
	#define CUR_DFT		100		//100A//경보
	#define BAT_CUR_DFT	300		//30.0A 
#else
	#define MOD_CUR_DFT	600		//50.0A		//MainLcd확인.
	#define CUR_DFT		220		//100A//경보
	#define BAT_CUR_DFT	550		//55.0A 
#endif	

#define BUZZER_MODE_DFT 	'Y'		//buzzer operation 'Y' or 'N'
#define CHARGE_MODE_DFT		'F'		//CHARGE MODE 'F' FLOAT

#define	CHARGE_DURATION_DFT	1		//1Hr
#define	CHARGE_LAST_CUR_DFT	5		//5A

#define	ENERGY_RATE_DFT		40

//#define PASS_DFT		0		//초기비밀번호 0
//==============================================
// EEPROM ADDRESSS SETTING
//==============================================
#define	INIT_EE			1//0x3039,12345(Decimal):OK, or initialize	
//==========
//Parameter
//==========

//====================================================
#define	EQ_V_EE				21	//Equalizing voltage  
#define	FLT_V_EE			22	//Float charge voltage  
#define	BUZZER_MODE_EE	 		23	//'Y'/'N'  Y default
#define CHARGE_MODE_EE		24	//'F'/'E'  float default
#define	CHARGE_DURATION_EE		25	//1hr
#define	CHARGE_LAST_CUR_EE		26	//5A
//====================================================
