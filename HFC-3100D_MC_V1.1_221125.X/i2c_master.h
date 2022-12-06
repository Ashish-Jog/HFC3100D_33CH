/*************************************************************************
 *  © 2019 Microchip Technology Inc.
 *
 *  Project Name:   dsPIC33C I2C Software Library
 *  FileName:       i2c_master.h
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 *  Description: This header contains I/O definitions/selection and
 *  timing/clock speed settings used for the MASTER I2C interface.
 *  Also this file includes prototypes of MASTER I2C functions.
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

#ifndef _I2C_MASTER_
#define _I2C_MASTER_

#include <xc.h>

////////////////////////////////////////////////////////////////////////////////            
// This parameter is a quarter of I2C clock period in instruction cycles.
// It defines timing for I2C interface. I2C MAY NOT WORK IF THIS PARAMETER IS
// WRONG (CLOCK IS TOO FAST).
#define I2C_CLOCK_DELAY  200    

////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the TRIS register of I/O used for SCL signal.
// VERIFY THAT IN YOUR APPLICATION THE SCL PIN IS CONFIGURED AS
// A DIGITAL INPUT IN ANSEL REGISTER.///
#define SCL_TRIS  _TRISB8
////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the ODC register of I/O used for SCL signal.///
#define SCL_ODC   _ODCB8
////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the LAT register of I/O used for SCL signal.///            
#define SCL_LAT   _LATB8    
////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the PORT register of I/O used for SCL signal.///                        
#define SCL_PORT  _RB8            

////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the TRIS register of I/O used for SDA signal.
// VERIFY THAT IN YOUR APPLICATION THE SCL PIN IS CONFIGURED AS
// A DIGITAL INPUT IN ANSEL REGISTER.///            
#define SDA_TRIS  _TRISB9
////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the ODC register of I/O used for SDA signal.///                       
#define SDA_ODC   _ODCB9
////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the LAT register of I/O used for SDA signal.///                                               
#define SDA_LAT   _LATB9
////////////////////////////////////////////////////////////////////////////////            
// This parameter sets the PORT register of I/O used for SDA signal.///                                               
#define SDA_PORT  _RB9

////////////////////////////////////////////////////////////////////////////////            
// I2CM_Delay()
// Description: This macro is used to generate SCL clock (period is equal to two these delays).
#define I2CM_Delay()  asm volatile ("repeat #%0 \n nop" : : "i"(I2C_CLOCK_DELAY));

////////////////////////////////////////////////////////////////////////////////            
// void I2CM_Init()
// Description: This function initializes SDA and SCL I/Os.
// Parameters: None.
// Returned data: None.            
void I2CM_Init();
    
////////////////////////////////////////////////////////////////////////////////
// short I2CM_Start()
// Description: This function generates an I2C start signal.
// Parameters: None.
// Returned data: The function returns non-zero value if the bus collision is detected.    
short I2CM_Start();

////////////////////////////////////////////////////////////////////////////////
// short I2CM_Stop()
// Description: This function generates I2C stop signal.
// Parameters: None.
// Returned data:
// The function returns non-zero value if the bus collision is detected.
short I2CM_Stop();

////////////////////////////////////////////////////////////////////////////////
// short I2CM_Write(unsigned char data)
// Description: This function transmits 8-bit data to slave.
// Parameters:
// unsigned char data - data to be transmitted
// Returned data:
// This function returns acknowledgment from slave (0 means ACK and 1 means NACK).
short I2CM_Write(unsigned char data);

////////////////////////////////////////////////////////////////////////////////
// unsigned char I2CM_Read(short ack)
// Description: This function reads 8-bit data from slave.
// Parameters:
// short ack - acknowledgment to be sent to slave
// Returned data:
// This function returns 8-bit data read.
unsigned char I2CM_Read(short ack);

#endif