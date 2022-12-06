/*
******************************************************************************************
* PROJECT : HFC-3100D MAIN CONTROL
* FILE NAME: Operation.h
* DEVICE : dsPIC30F6012A
* COMPILER : MPLAB - C V3.12 
* REVISION HISTORY
* 2018.03.07 Draft Ver.
**************************************************************************************** */

extern void OP_Logic(void);


extern UINT8   RST_cnt;
extern UINT8   RST_Done;
extern UINT16   RST_Timer;          // Recovery Flag Timer During Recovery condition
extern UINT8   Recovery_Flag;      // Recovery Flag During Recovery condition
extern UINT8   Set_OK;             // OP_CMD => '1' : Module1 / '2' : Module2    
//extern UINT16  tmr2_2;
extern volatile UINT16 tmrOpLogic;


