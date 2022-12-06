/*************************************************************************
 *  © 2019 Microchip Technology Inc.
 *
 *  Project Name:   dsPIC33C I2C Software Library
 *  FileName:       i2c_master.c
 * 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 *  Description: This assembler source file contains MASTER I2C functions implementations.
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

#include "i2c_master.h"

void I2CM_Init(){    
    SCL_ODC = 1;
    SDA_ODC = 1;
    
    SCL_LAT = 1;
    SDA_LAT = 1;
    
    SCL_TRIS = 0;
    SDA_TRIS = 0;
}    


short I2CM_Start(){
    SDA_LAT = 1; // SDA to high
    I2CM_Delay();
    I2CM_Delay();

    SCL_LAT = 1; // SCL to high
    I2CM_Delay();
    I2CM_Delay();
    
    if(SCL_PORT == 0){
        return 1; // bus collision
    }

    if(SDA_PORT == 0){
        return 1; // bus collision
    }

    SDA_LAT = 0; // SDA to low
    I2CM_Delay();
    I2CM_Delay();

    SCL_LAT = 0; // SCL to low
    I2CM_Delay();
    I2CM_Delay();
    
    return 0;
}

short I2CM_Stop(){
    SCL_LAT = 0; // SCL to low
    I2CM_Delay();
    I2CM_Delay();
    
    SDA_LAT = 0; // SDA to low
    I2CM_Delay();
    I2CM_Delay();

    SCL_LAT = 1; // SCL to high
    I2CM_Delay();
    I2CM_Delay();
    
    SDA_LAT = 1; // SDA to high
    I2CM_Delay();
    I2CM_Delay();

    if(SCL_PORT == 0){
        return 1; // bus collision
    }

    if(SDA_PORT == 0){
        return 1; // bus collision
    }
    
    I2CM_Delay();
    I2CM_Delay();
    
    return 0;
}

short I2CM_Write(unsigned char data){
short counter;
short ack;

    SCL_LAT = 0; // SCL to low

    counter = 8;    
    while(counter--){ 
        if(data&0x80){
            SDA_LAT = 1;
        }else{
            SDA_LAT = 0;            
        }
        data <<= 1;        
        I2CM_Delay();
        SCL_LAT = 1;
        while(SCL_PORT == 0); // clock stretching
        I2CM_Delay();
        I2CM_Delay();    
        SCL_LAT = 0;
        I2CM_Delay();
    }
        
    SDA_TRIS = 1; // SDA to input
    I2CM_Delay();
    SCL_LAT = 1;    
    while(SCL_PORT == 0); // clock stretching
    I2CM_Delay();
    I2CM_Delay();    
    
    if(SDA_PORT){
        ack = 1;
    }else{
        ack = 0;        
    }
    SDA_TRIS = 0; // SDA to output
    SCL_LAT = 0;
    I2CM_Delay();

    return ack;
}

unsigned char I2CM_Read(short ack){
short counter;
unsigned char data;

    SCL_LAT = 0;    
    SDA_TRIS = 1; // SDA to input
    
    counter = 8;
    data = 0;    
    while(counter--){    
        I2CM_Delay();
        SCL_LAT = 1;
        while(SCL_PORT == 0); // clock stretching
        I2CM_Delay();
        I2CM_Delay();
        data <<= 1;
        if(SDA_PORT == 1){
            data |= 1;
        }        
        SCL_LAT = 0;
        I2CM_Delay();
    }
    
    SDA_TRIS = 0; //SDA to output
    
    if(ack){       
        SDA_LAT = 1;
    }else{
        SDA_LAT = 0;
    }
    
    I2CM_Delay();
    SCL_LAT = 1;
    I2CM_Delay();
    I2CM_Delay();        
    SCL_LAT = 0;
    I2CM_Delay();
    
    return data;  
}
