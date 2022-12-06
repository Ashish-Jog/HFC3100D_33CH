/**********************************************************************
* © 2005 Microchip Technology Inc.
*
* FileName:        i2cEmem.h
* Dependencies:    Other (.h) files if applicable, see below
* Processor:       dsPIC33Fxxxx//PIC24Hxxxx
* Compiler:        MPLAB® C30 v3.00 or higher
*
* SOFTWARE LICENSE AGREEMENT:
* Microchip Technology Incorporated ("Microchip") retains all ownership and 
* intellectual property rights in the code accompanying this message and in all 
* derivatives hereto.  You may use this code, and any derivatives created by 
* any person or entity by or on your behalf, exclusively with Microchip's
* proprietary products.  Your acceptance and/or use of this code constitutes 
* agreement to the terms and conditions of this notice.
*
* CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS".  NO 
* WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED 
* TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A 
* PARTICULAR PURPOSE APPLY TO THIS CODE, ITS INTERACTION WITH MICROCHIP'S 
* PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
*
* YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE LIABLE, WHETHER 
* IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), 
* STRICT LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, 
* PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR EXPENSE OF 
* ANY KIND WHATSOEVER RELATED TO THE CODE, HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN 
* ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT 
* ALLOWABLE BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO 
* THIS CODE, SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP SPECIFICALLY TO 
* HAVE THIS CODE DEVELOPED.
*
* You agree that you are solely responsible for testing the code and 
* determining its suitability.  Microchip has no obligation to modify, test, 
* certify, or support the code.
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author            Date      Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Settu D.			07/09/06  First release of source file
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
* ADDITIONAL NOTES:
*
**********************************************************************/
#ifndef __I2CEMEM_H__
#define __I2CEMEM_H__ 


#define MAX_RETRY	 1000
#define ONE_BYTE     1
#define TWO_BYTE     2

// EEPROM ADDRESS SIZE
#define ADDRWIDTH   TWO_BYTE     
  
// EEPROM DRIVER COMMAND DEFINITION
#define I2C_IDLE 		 0  
#define I2C_WRITE        1      
#define I2C_READ         2    
#define I2C_ERR        0xFFFF

// EEPROM DATA OBJECT
typedef struct { 
        unsigned int *buff;      
        unsigned int n;        
        unsigned int addr;       
        unsigned int csel;            
}I2CEMEM_DATA; 

// EEPROM DRIVER OBJECT
typedef struct { 
        unsigned int	cmd; 
		I2CEMEM_DATA	*oData;	        
        void (*init)(void *);                   
        void (*tick)(void *); 
        }I2CEMEM_DRV; 
    
#define I2CSEMEM_DRV_DEFAULTS { 0,\
        (I2CEMEM_DATA *)0,\
        (void (*)(void *))I2CEMEMinit,\
        (void (*)(void *))I2CEMEMdrv}  
        

void I2CEMEMinit(I2CEMEM_DRV *); 
void I2CEMEMdrv(I2CEMEM_DRV *);


#endif
        
