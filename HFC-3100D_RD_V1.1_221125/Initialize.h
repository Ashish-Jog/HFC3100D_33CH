/* 
 * File:   Initialize.h
 * Author: Martin. Kim
 *
 * Created on 2019? 1? 29? (?), ?? 2:52
 */

#ifndef INITIALIZE_H
#define	INITIALIZE_H

//extern int _EEDATA(2) EE_mem[2048];
extern void Set_EEPROM();
extern void sysInit();
extern void EEPROM_Initialize(void);
extern void ParameterInitialize(void);
extern void Init_Port();
extern void Init_Timer1();
extern void Init_Timer2();
extern void rtc_set_datetime(UINT8 day, UINT8 mth, UINT8 year, UINT8 dow, UINT8 hour, UINT8 min);

#endif	/* INITIALIZE_H */

