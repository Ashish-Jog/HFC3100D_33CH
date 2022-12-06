

#include 	<xc.h>
#include    "mcc_generated_files/pin_manager.h"
#include 	"Definition.h"
#include	"Function.h"

#include "var.h"

//====================================================================================//
//                          Function Declaration                                      //
//====================================================================================//
extern void DelayNop(UINT16 N);


//====================================================================================//
//                          Variable Declaration                                      //
//====================================================================================//
//static UINT16	key_cnt[9] = {0,0,0,0,0,0,0,0,0};
//static UINT8	KeyBuffer;
//static UINT8 	KeyDataTemp;
//static UINT16	KeyOnfTime; 
//static UINT16 	keySequence;

UINT16	key_cnt[9] = {0,0,0,0,0,0,0,0,0};
UINT8	KeyBuffer;
UINT8 	KeyDataTemp;
UINT16	KeyOnfTime; 
UINT16 	keySequence;

void KeyDetecting(void)//interval 5ms//
{
	UINT8 i;
	unsigned char KeyData;
	if (KeyOnfTime > 0) KeyOnfTime--;
    //
    switch(getKeyData())
	{
		case KEY_ENTER:			KeyData = keyKEY_ENTER;			
                                key_cnt[F_KEY_ENTER]++;			
                                break;
		case KEY_LEFT:	        KeyData = keyKEY_LEFT;	 		
                                key_cnt[F_KEY_LEFT]++;   		
                                break;
		case KEY_UP:			KeyData = keyKEY_UP;			
                                key_cnt[F_KEY_UP]++;     		                                
                                break;
		case KEY_RIGHT:         KeyData = keyKEY_RIGHT;			
                                key_cnt[F_KEY_RIGHT]++;			
                                break;
		case KEY_DOWN:			KeyData = keyKEY_DOWN;			
                                key_cnt[F_KEY_DOWN]++;			
                                break;
		case KEY_AUTOMANUAL:  	KeyData = keyKEY_AUTOMANUAL;	
                                key_cnt[F_KEY_AUTOMANUAL]++;  	
                                break;
		case KEY_NO_SELECT:  	KeyData = keyKEY_NO_SELECT;		
                                key_cnt[F_KEY_NO_SELECT]++;  	
                                break;
		case KEY_LAMP:			KeyData = keyKEY_LAMP;			
                                key_cnt[F_KEY_LAMP]++;			
                                break;
		case KEY_BZ_OFF:		KeyData = keyKEY_BZ_OFF;		
                                key_cnt[F_KEY_BZ_OFF]++;		
                                break;
        //
        default:                for (i=0; i<9; i++)
                                {
                                    key_cnt[i] = 0;   
                                }
                                //KeyData = keyNULL;//goooo
                                break;
	}//End switch(getKeyData())
    switch (keySequence)
	{
        case SEQ_NOP:
            if (KeyData != keyNULL)
            {
                KeyDataTemp = KeyData;
                keySequence = SEQ_SOME;                
                
            }
            break;

        case SEQ_SOME:
            if (KeyData == keyNULL)
			{
                keySequence = SEQ_NOP;                
                break;
            }
            //-------------------------escape switch
            if (KeyDataTemp == KeyData)
			{
                KeyOnfTime  = KEY_ON_TIME;//3
                keySequence = SEQ_ON_TIME;//2
            }
            else
			{
                keySequence = SEQ_NOP;
            }
            break;

        case SEQ_ON_TIME:
            if (KeyDataTemp != KeyData){
                keySequence = SEQ_NOP;
            }
            else if (KeyOnfTime==0) //for debounce//
            {
                keySequence = SEQ_CATCH;
                //
            }
            break;

        case SEQ_CATCH: // 3
            KeyBuffer = KeyDataTemp;		
			//
            KeyOnfTime  = KEY_REJECT_TIME;//36
            keySequence = SEQ_REJECT;                        
            break;

        case SEQ_REJECT:	// 4
            if (KeyOnfTime == 0)
            {
                keySequence = SEQ_OFF;
            }
            break;

        case SEQ_OFF:	// 5
            if (KeyData == keyNULL)
            {
                KeyOnfTime  = KEY_OFF_TIME;//5
                keySequence = SEQ_OFF_TIME;
            }
            break;

        case SEQ_OFF_TIME:	// 6
            if (KeyData != keyNULL)
            {
                keySequence = SEQ_OFF;
            }
            else if (KeyOnfTime == 0)
            {
                keySequence = SEQ_END;
            }
            break;

        case SEQ_END:	// 7
            keySequence = SEQ_NOP;
            break;
    }
}

UINT8 getKeyData(void)
{
	UINT8 uc_key = 0;	
    //
    SCAN1_SetHigh();
    SCAN2_SetLow();        
    SCAN3_SetLow();
	DelayNop(20); //10-6.8us
    if      ( KEY_NUM1_GetValue() == 1 )uc_key = KEY_ENTER; //1
	else if ( KEY_NUM2_GetValue() == 1 )uc_key = KEY_BZ_OFF;//9
	else if ( KEY_NUM3_GetValue() == 1 )uc_key = KEY_LAMP;  //8
    //        
	SCAN1_SetLow();
    SCAN2_SetHigh();    
    SCAN3_SetLow();
	DelayNop(20);
	if      ( KEY_NUM1_GetValue() == 1 )uc_key = KEY_DOWN;      //5
	else if ( KEY_NUM2_GetValue() == 1 )uc_key = KEY_LEFT;      //2		
	else if ( KEY_NUM3_GetValue() == 1 )uc_key = KEY_NO_SELECT; //7   
    //
	SCAN1_SetLow();
    SCAN2_SetLow();    
    SCAN3_SetHigh();
	DelayNop(20);
	if      ( KEY_NUM1_GetValue() == 1 )uc_key = KEY_UP;        //3
	else if ( KEY_NUM2_GetValue() == 1 )uc_key = KEY_RIGHT;     //4
	else if ( KEY_NUM3_GetValue() == 1 )uc_key = KEY_AUTOMANUAL;//6
    //
	return uc_key;
}


UINT16 hKey(void)
{
	UINT16 KeyValue=0;

	if (key_cnt[F_KEY_ENTER] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_ENTER] = DELAY_LONG_PRESSED;
		KeyValue             = keyKEY_ENTERfast;//10     
	}
	else if (key_cnt[F_KEY_LEFT] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_LEFT] = DELAY_LONG_PRESSED;
		KeyValue            = keyKEY_LEFTfast;//11        
	}
	else if (key_cnt[F_KEY_UP] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_UP] = DELAY_LONG_PRESSED;
		KeyValue          = keyKEY_UPfast;//12        
	}
	else if (key_cnt[F_KEY_RIGHT] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_RIGHT] = DELAY_LONG_PRESSED;
		KeyValue             = keyKEY_RIGHTfast;//13        
	}
	else if (key_cnt[F_KEY_DOWN] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_DOWN] = DELAY_LONG_PRESSED;
		KeyValue            = keyKEY_DOWNfast;//14        
	}
	else if (key_cnt[F_KEY_AUTOMANUAL] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_AUTOMANUAL] = DELAY_LONG_PRESSED;
		KeyValue                  = keyKEY_AUTOMANUALfast;//15        
	}
	else if (key_cnt[F_KEY_NO_SELECT] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_NO_SELECT] = DELAY_LONG_PRESSED;
		KeyValue                 = keyKEY_NO_SELECTfast;//16        
	}
	else if (key_cnt[F_KEY_LAMP] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_LAMP] = DELAY_LONG_PRESSED;
		KeyValue            = keyKEY_LAMPfast;
	}
	else if (key_cnt[F_KEY_BZ_OFF] >= DELAY_LONG_PRESSED)
	{
		key_cnt[F_KEY_BZ_OFF] = DELAY_LONG_PRESSED;
		KeyValue              = keyKEY_BZ_OFFfast;
	}		
	else
	{
		if (KeyBuffer != keyNULL  )
		{            
			KeyValue = KeyBuffer;           
		}
		else
		{
			KeyValue = keyNULL;            
		}
	}
	KeyBuffer = keyNULL;
    //    
	return KeyValue;    
}

//

