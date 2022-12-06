/*************************************************************************
 *  © 2019 Microchip Technology Inc.
 *
 *  Project Name:   dsPIC33C I2C Software Library
 *  FileName:       i2c_master_eeprom_24fc256.c
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 *  Description: This C source file contains functions implementations
 *               to access 24FC256 EEPROM.
 *
 *************************************************************************/
/**************************************************************************
 * MICROCHIP SOFTWARE NOTICE AND DISCLAIMER: You may use this software, and
 * any derivatives created by any person or entity by or on your behalf,
 * exclusively with Microchip's products in accordance with applicable
 * software license terms and conditions, a copy of which is provided for
 * your reference in accompanying documentation. Microchip and its licensors
 * retain all ownership and intellectual property rights in the
 * accompanying software and in all derivatives hereto.
 *
 * This software and any accompanying information is for suggestion only.
 * It does not modify Microchip's standard warranty for its products. You
 * agree that you are solely responsible for testing the software and
 * determining its suitability. Microchip has no obligation to modify,
 * test, certify, or support the software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE, ITS INTERACTION WITH
 * MICROCHIP'S PRODUCTS, COMBINATION WITH ANY OTHER PRODUCTS, OR USE IN ANY
 * APPLICATION.
 *
 * IN NO EVENT, WILL MICROCHIP BE LIABLE, WHETHER IN CONTRACT, WARRANTY,
 * TORT (INCLUDING NEGLIGENCE OR BREACH OF STATUTORY DUTY), STRICT
 * LIABILITY, INDEMNITY, CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE,
 * FOR COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE,
 * HOWSOEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY
 * OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWABLE BY LAW,
 * MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS
 * SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID
 * DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF
 * THESE TERMS.
 *************************************************************************
 *
 * written by Anton Alkhimenok 12/17/2019
 *
 *************************************************************************/

#include "i2c_master_eeprom_24fc256.h"

#define EEPROM_DEV_ADDRESS     0x50  // from 24FC256 datasheet

void EEPROMInit()
{
    I2CM_Init();
}

void EEPROMWrite(unsigned short address, unsigned char data)
{
    do{
        I2CM_Start();
    }while(I2CM_Write(EEPROM_DEV_ADDRESS<<1));    

    I2CM_Write(address>>8);

    I2CM_Write(address&0x00ff);

    I2CM_Write(data);
  
    I2CM_Stop();  
}

unsigned char EEPROMRead(unsigned short address)
{
unsigned char data;

    do{
        I2CM_Start();
    }while(I2CM_Write(EEPROM_DEV_ADDRESS<<1));

    I2CM_Write(address>>8);

    I2CM_Write(address&0x00ff);
    
    I2CM_Start();

    I2CM_Write((EEPROM_DEV_ADDRESS<<1)|1);

    data = I2CM_Read(1); // NOT ACKNOWLEDGE
    
    I2CM_Stop();

    return data;
}





