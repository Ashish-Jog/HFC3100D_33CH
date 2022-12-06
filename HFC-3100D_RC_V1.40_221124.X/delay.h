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
#ifndef __DELAY_H__
#define __DELAY_H__ 
/*
void Delay( unsigned int delay_count );
void Delay_Us( unsigned int delayUs_count );

#define Delay200uS_count  200
#define Delay_1mS_Cnt	  1
#define Delay_2mS_Cnt	  2
#define Delay_5mS_Cnt	  5
#define Delay_15mS_Cnt 	  15
#define Delay_1S_Cnt	  1000

#endif
*/
#define Fcy  16000000			//40000000

void Delay( unsigned int delay_count );
void Delay_Us( unsigned int delayUs_count );

#define Delay200uS_count  (Fcy * 0.0002) / 1080
#define Delay_1mS_Cnt	  (Fcy * 0.001) / 2950
#define Delay_2mS_Cnt	  (Fcy * 0.002) / 2950
#define Delay_5mS_Cnt	  (Fcy * 0.005) / 2950
#define Delay_15mS_Cnt 	  (Fcy * 0.015) / 2950
#define Delay_1S_Cnt	  (Fcy * 1) / 2950  		//5,423

#endif
