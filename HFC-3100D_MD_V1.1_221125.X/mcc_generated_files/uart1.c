/**
  UART1 Generated Driver File 

  @Company
    Microchip Technology Inc.

  @File Name
    uart1.c

  @Summary
    This is the generated source file for the UART1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for driver for UART1. 
    Generation Information : 
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device            :  dsPIC33CH128MP506
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB             :  MPLAB X v5.50
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "xc.h"
#include "uart1.h"
#include "pin_manager.h"
#include "../definition.h"
/**
  Section: Data Type Definitions
*/

/** UART Driver Queue Status

  @Summary
    Defines the object required for the status of the queue.
*/

static uint8_t * volatile rxTail;
static uint8_t *rxHead;
static uint8_t *txTail;
static uint8_t * volatile txHead;
static bool volatile rxOverflowed;

// Main Continous Data
// RS232 Variable
uint8_t       f_SData;
uint8_t       f_EData;
uint8_t       Rindex=0;
uint8_t       RxBuf[35];
//uint8_t       TxBuf[34];
char       TxBuf[35];
uint8_t       TxIndex=0;
//uint8_t       Set_OK = 0;
float		CSpare[10]={0,};
float		SSpare[10]={0,};
uint8_t		EOT_Flag=0;
// Main Continous Data
float     BattVoltage = 0;
float     BattCurrent = 0;
float     LoadVoltage = 0;
float     LoadCurrent = 0;

// Main Set Data
float	BattHiVoltage=0;
float	BattLowVoltage=0;
uint16_t	RestartTimeValue=0;

// Module Receive Command Feedback

float	BattHiVoltage_FB=0;
float	BattLowVoltage_FB=0;
uint16_t	RestartTimeValue_FB=0;

uint8_t gOperationMode;
uint8_t	gAutoManual_FB;
uint8_t	gSelectModule_FB;
uint8_t	gOperationMode_FB;
uint8_t   gSysTypeMode_FB;

//UINT8	gRestartTime;
uint8_t	gAutoManual=0;
uint8_t	gSelectModule=0;
uint8_t	gOperationMode=0;
//uint8_t	gBattTypeMode=0;
uint8_t	gSysTypeMode=0;
//uint8_t   gInVoltMode=0;
uint8_t	LedState[10]={0x00,};
uint8_t	ModuleState[10]={0x00,};
uint8_t	BatteryState[10]={0x00,};
uint8_t	WarningState[10]={0x00,};
uint8_t   ChargeState[10]={0x00,};

// RS-232 Count
uint8_t	CheckDataCnt=0;

/** UART Driver Queue Length

  @Summary
    Defines the length of the Transmit and Receive Buffers

*/

/* We add one extra byte than requested so that we don't have to have a separate
 * bit to determine the difference between buffer full and buffer empty, but
 * still be able to hold the amount of data requested by the user.  Empty is
 * when head == tail.  So full will result in head/tail being off by one due to
 * the extra byte.
 */
#define UART1_CONFIG_TX_BYTEQ_LENGTH (8+1)
#define UART1_CONFIG_RX_BYTEQ_LENGTH (8+1)

/** UART Driver Queue

  @Summary
    Defines the Transmit and Receive Buffers

*/

static uint8_t txQueue[UART1_CONFIG_TX_BYTEQ_LENGTH];
static uint8_t rxQueue[UART1_CONFIG_RX_BYTEQ_LENGTH];

void (*UART1_TxDefaultInterruptHandler)(void);
void (*UART1_RxDefaultInterruptHandler)(void);

/**
  Section: Driver Interface
*/

void UART1_Initialize(void)
{
    IEC0bits.U1TXIE = 0;
    IEC0bits.U1RXIE = 0;

    // URXEN disabled; RXBIMD RXBKIF flag when Break makes low-to-high transition after being low for at least 23/11 bit periods; UARTEN enabled; MOD Asynchronous 8-bit UART; UTXBRK disabled; BRKOVR TX line driven by shifter; UTXEN disabled; USIDL disabled; WAKE disabled; ABAUD disabled; BRGH enabled; 
    // Data Bits = 8; Parity = None; Stop Bits = 1 Stop bit sent, 1 checked at RX;
    U1MODE = (0x8080 & ~(1<<15));  // disabling UART ON bit
    // STSEL 1 Stop bit sent, 1 checked at RX; BCLKMOD disabled; SLPEN disabled; FLO Off; BCLKSEL FOSC/2; C0EN disabled; RUNOVF disabled; UTXINV disabled; URXINV disabled; HALFDPLX disabled; 
    U1MODEH = 0x00;
    // OERIE disabled; RXBKIF disabled; RXBKIE disabled; ABDOVF disabled; OERR disabled; TXCIE disabled; TXCIF disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; CERIF disabled; PERIE disabled; 
    U1STA = 0x00;
    // URXISEL RX_ONE_WORD; UTXBE enabled; UTXISEL TX_BUF_EMPTY; URXBE enabled; STPMD disabled; TXWRE disabled; 
    U1STAH = 0x22;
    // BaudRate = 9600; Frequency = 25000000 Hz; BRG 650; 
    U1BRG = 0x28A;
    // BRG 0; 
    U1BRGH = 0x00;
    // P1 0; 
    U1P1 = 0x00;
    // P2 0; 
    U1P2 = 0x00;
    // P3 0; 
    U1P3 = 0x00;
    // P3H 0; 
    U1P3H = 0x00;
    // TXCHK 0; 
    U1TXCHK = 0x00;
    // RXCHK 0; 
    U1RXCHK = 0x00;
    // T0PD 1 ETU; PTRCL disabled; TXRPT Retransmit the error byte once; CONV Direct logic; 
    U1SCCON = 0x00;
    // TXRPTIF disabled; TXRPTIE disabled; WTCIF disabled; WTCIE disabled; BTCIE disabled; BTCIF disabled; GTCIF disabled; GTCIE disabled; RXRPTIE disabled; RXRPTIF disabled; 
    U1SCINT = 0x00;
    // ABDIF disabled; WUIF disabled; ABDIE disabled; 
    U1INT = 0x00;
    
    txHead = txQueue;
    txTail = txQueue;
    rxHead = rxQueue;
    rxTail = rxQueue;
   
    rxOverflowed = false;

    UART1_SetTxInterruptHandler(&UART1_Transmit_CallBack);

    UART1_SetRxInterruptHandler(&UART1_Receive_CallBack);

    IEC0bits.U1RXIE = 1;
    
    //Make sure to set LAT bit corresponding to TxPin as high before UART initialization
    U1MODEbits.UARTEN = 1;   // enabling UART ON bit
    U1MODEbits.UTXEN = 1;
    U1MODEbits.URXEN = 1;
}

/**
    Maintains the driver's transmitter state machine and implements its ISR
*/

void UART1_SetTxInterruptHandler(void (* interruptHandler)(void))
{
    if(interruptHandler == NULL)
    {
        UART1_TxDefaultInterruptHandler = &UART1_Transmit_CallBack;
    }
    else
    {
        UART1_TxDefaultInterruptHandler = interruptHandler;
    }
} 


void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1TXInterrupt ( void )
{
    if(UART1_TxDefaultInterruptHandler)
    {
        UART1_TxDefaultInterruptHandler();
    }
    
    if(txHead == txTail)
    {
        IEC0bits.U1TXIE = 0;
    }
    else
    {
        IFS0bits.U1TXIF = 0;

        while(!(U1STAHbits.UTXBF == 1))
        {
            U1TXREG = *txHead++;

            if(txHead == (txQueue + UART1_CONFIG_TX_BYTEQ_LENGTH))
            {
                txHead = txQueue;
            }

            // Are we empty?
            if(txHead == txTail)
            {
                break;
            }
        }
    }
}

void __attribute__ ((weak)) UART1_Transmit_CallBack ( void )
{ 
    //return;
}

void UART1_SetRxInterruptHandler(void (* interruptHandler)(void))
{
    if(interruptHandler == NULL)
    {
        UART1_RxDefaultInterruptHandler = &UART1_Receive_CallBack;
    }
    else
    {
        UART1_RxDefaultInterruptHandler = interruptHandler;
    }
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1RXInterrupt( void )
{
       /*
        * move to below this routine 
        */ 
//    if(UART1_RxDefaultInterruptHandler) 
//    {
//        UART1_RxDefaultInterruptHandler();
//    }
    
    IFS0bits.U1RXIF = 0;
    
    while(!(U1STAHbits.URXBE == 1))
    {
        *rxTail = U1RXREG;

        // Will the increment not result in a wrap and not result in a pure collision?
        // This is most often condition so check first
        if ( ( rxTail    != (rxQueue + UART1_CONFIG_RX_BYTEQ_LENGTH-1)) &&
             ((rxTail+1) != rxHead) )
        {
            rxTail++;
        } 
        else if ( (rxTail == (rxQueue + UART1_CONFIG_RX_BYTEQ_LENGTH-1)) &&
                  (rxHead !=  rxQueue) )
        {
            // Pure wrap no collision
            rxTail = rxQueue;
        } 
        else // must be collision
        {
            rxOverflowed = true;
        }
    }
    //NORMAL_LED_Toggle();
    if(UART1_RxDefaultInterruptHandler)
    {
        UART1_RxDefaultInterruptHandler();
    }
}

void __attribute__ ((weak)) UART1_Receive_CallBack(void)
{
//    uint8_t rxData;
//    rxData = UART1_Read();
//    NORMAL_LED_Toggle();
//    if(UART1_Read() == 'a')
//    {
//        NORMAL_LED_Toggle();
//    }
    UINT8 RcvData;
	UINT8 tmp=0,checksum=0,i;
    // if(U1STAbits.OERR == 1)     //Clear Overflow Flag - KHJ
    // {
    //     U1STAbits.OERR = 0;
    // }
    //RcvData = U1RXREG;              //RXbuff
    RcvData = UART1_Read();
	
    if(RcvData == '<' && EOT_Flag == 0)				//Start of Transmission Packet - SOT => 0x3C '<'
	{
        f_SData=1;                  //start serial data flag 
		Rindex=0;
		EOT_Flag=1;
	}    
	else if((f_SData == 1) && (RcvData == 0x3E) && (Rindex == 34))//EOT of Transmission Packet - EOT => 0x3E '>'
	{
		for(i=0;i<32;i++)
		{
			checksum^=RxBuf[i];
		}
		tmp=((RxBuf[32]-0x30)<<4)&0xF0;
		tmp=(tmp+((RxBuf[33]-0x30)&0x0F));
        //
		if(tmp == checksum)
		{
			f_EData = 1;
			EOT_Flag = 0;
		}	
		else
		{
            #if(serialDebug==1)
              //  printf("check sum error!");
            #endif                
			f_SData = 0;
            f_EData=0;
            EOT_Flag = 0;
		}
	}
	RxBuf[Rindex]=RcvData;
	Rindex++;
	//
	if(Rindex>36)	//Occur overflow -> Reset buffer//
	{	
		Rindex=0;	
		f_SData=0;
		f_EData=0;
		EOT_Flag = 0;
	}    
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1EInterrupt ( void )
{
    if ((U1STAbits.OERR == 1))
    {
        U1STAbits.OERR = 0;
    }
    
    IFS3bits.U1EIF = 0;
}

/* ISR for UART Event Interrupt */

void __attribute__ ( ( interrupt, no_auto_psv ) ) _U1EVTInterrupt ( void )
{
    /* Add handling for UART events here */

    IFS11bits.U1EVTIF = false;
}

/**
  Section: UART Driver Client Routines
*/

uint8_t UART1_Read( void)
{
    uint8_t data = 0;

    while (rxHead == rxTail )
    {
    }
    
    data = *rxHead;

    rxHead++;

    if (rxHead == (rxQueue + UART1_CONFIG_RX_BYTEQ_LENGTH))
    {
        rxHead = rxQueue;
    }
    return data;
}

void UART1_Write( uint8_t byte)
{
    while(UART1_IsTxReady() == 0)
    {
    }

    *txTail = byte;

    txTail++;
    
    if (txTail == (txQueue + UART1_CONFIG_TX_BYTEQ_LENGTH))
    {
        txTail = txQueue;
    }

    IEC0bits.U1TXIE = 1;
}

bool UART1_IsRxReady(void)
{    
    return !(rxHead == rxTail);
}

bool UART1_IsTxReady(void)
{
    uint16_t size;
    uint8_t *snapshot_txHead = (uint8_t*)txHead;
    
    if (txTail < snapshot_txHead)
    {
        size = (snapshot_txHead - txTail - 1);
    }
    else
    {
        size = ( UART1_CONFIG_TX_BYTEQ_LENGTH - (txTail - snapshot_txHead) - 1 );
    }
    
    return (size != 0);
}

bool UART1_IsTxDone(void)
{
    if(txTail == txHead)
    {
        return (bool)U1STAbits.TRMT;
    }
    
    return false;
}

int __attribute__((__section__(".libc.write"))) write(int handle, void *buffer, unsigned int len) 
{
    unsigned int i;
    uint8_t *data = buffer;

    for(i=0; i<len; i++)
    {
        while(UART1_IsTxReady() == false)
        {
        }

        UART1_Write(*data++);
    }
  
    return(len);
}

/*******************************************************************************

  !!! Deprecated API !!!
  !!! These functions will not be supported in future releases !!!

*******************************************************************************/

static uint8_t UART1_RxDataAvailable(void)
{
    uint16_t size;
    uint8_t *snapshot_rxTail = (uint8_t*)rxTail;
    
    if (snapshot_rxTail < rxHead) 
    {
        size = ( UART1_CONFIG_RX_BYTEQ_LENGTH - (rxHead-snapshot_rxTail));
    }
    else
    {
        size = ( (snapshot_rxTail - rxHead));
    }
    
    if(size > 0xFF)
    {
        return 0xFF;
    }
    
    return size;
}

static uint8_t UART1_TxDataAvailable(void)
{
    uint16_t size;
    uint8_t *snapshot_txHead = (uint8_t*)txHead;
    
    if (txTail < snapshot_txHead)
    {
        size = (snapshot_txHead - txTail - 1);
    }
    else
    {
        size = ( UART1_CONFIG_TX_BYTEQ_LENGTH - (txTail - snapshot_txHead) - 1 );
    }
    
    if(size > 0xFF)
    {
        return 0xFF;
    }
    
    return size;
}

unsigned int __attribute__((deprecated)) UART1_ReadBuffer( uint8_t *buffer ,  unsigned int numbytes)
{
    unsigned int rx_count = UART1_RxDataAvailable();
    unsigned int i;
    
    if(numbytes < rx_count)
    {
        rx_count = numbytes;
    }
    
    for(i=0; i<rx_count; i++)
    {
        *buffer++ = UART1_Read();
    }
    
    return rx_count;    
}

unsigned int __attribute__((deprecated)) UART1_WriteBuffer( uint8_t *buffer , unsigned int numbytes )
{
    unsigned int tx_count = UART1_TxDataAvailable();
    unsigned int i;
    
    if(numbytes < tx_count)
    {
        tx_count = numbytes;
    }
    
    for(i=0; i<tx_count; i++)
    {
        UART1_Write(*buffer++);
    }
    
    return tx_count;  
}

UART1_TRANSFER_STATUS __attribute__((deprecated)) UART1_TransferStatusGet (void )
{
    UART1_TRANSFER_STATUS status = 0;
    uint8_t rx_count = UART1_RxDataAvailable();
    uint8_t tx_count = UART1_TxDataAvailable();
    
    switch(rx_count)
    {
        case 0:
            status |= UART1_TRANSFER_STATUS_RX_EMPTY;
            break;
        case UART1_CONFIG_RX_BYTEQ_LENGTH:
            status |= UART1_TRANSFER_STATUS_RX_FULL;
            break;
        default:
            status |= UART1_TRANSFER_STATUS_RX_DATA_PRESENT;
            break;
    }
    
    switch(tx_count)
    {
        case 0:
            status |= UART1_TRANSFER_STATUS_TX_FULL;
            break;
        case UART1_CONFIG_RX_BYTEQ_LENGTH:
            status |= UART1_TRANSFER_STATUS_TX_EMPTY;
            break;
        default:
            break;
    }

    return status;    
}

uint8_t __attribute__((deprecated)) UART1_Peek(uint16_t offset)
{
    uint8_t *peek = rxHead + offset;
    
    while(peek > (rxQueue + UART1_CONFIG_RX_BYTEQ_LENGTH))
    {
        peek -= UART1_CONFIG_RX_BYTEQ_LENGTH;
    }
    
    return *peek;
}

bool __attribute__((deprecated)) UART1_ReceiveBufferIsEmpty (void)
{
    return (UART1_RxDataAvailable() == 0);
}

bool __attribute__((deprecated)) UART1_TransmitBufferIsFull(void)
{
    return (UART1_TxDataAvailable() == 0);
}

uint32_t __attribute__((deprecated)) UART1_StatusGet (void)
{
    uint32_t statusReg = U1STAH;
    return ((statusReg << 16 ) | U1STA);
}

unsigned int __attribute__((deprecated)) UART1_TransmitBufferSizeGet(void)
{
    if(UART1_TxDataAvailable() != 0)
    { 
        if(txHead > txTail)
        {
            return((txHead - txTail) - 1);
        }
        else
        {
            return((UART1_CONFIG_TX_BYTEQ_LENGTH - (txTail - txHead)) - 1);
        }
    }
    return 0;
}

unsigned int __attribute__((deprecated)) UART1_ReceiveBufferSizeGet(void)
{
    if(UART1_RxDataAvailable() != 0)
    {
        if(rxHead > rxTail)
        {
            return((rxHead - rxTail) - 1);
        }
        else
        {
            return((UART1_CONFIG_RX_BYTEQ_LENGTH - (rxTail - rxHead)) - 1);
        } 
    }
    return 0;
}

void __attribute__((deprecated)) UART1_Enable(void)
{
    U1MODEbits.UARTEN = 1;
    U1MODEbits.UTXEN = 1; 
    U1MODEbits.URXEN = 1;
}

void __attribute__((deprecated)) UART1_Disable(void)
{
    U1MODEbits.UARTEN = 0;
    U1MODEbits.UTXEN = 0; 
    U1MODEbits.URXEN = 0;
}
//==============================================================================
// '<' + 'U'+ '0' + 'S' + TxBuf[4~27] + 
void Response_Serial(void)
{
    
	switch(RxBuf[1])                            //Unit Select
	{
        case 'U':
            switch(RxBuf[2])                    //Module ID Select          
            {                
				case '0':                       //Module 1
					switch(RxBuf[3])            //Ctrl Code Select
					{
						case 'R':               //Setting Data Set Start
                            //------------------------
                            //from mc[ Response_FB() ]
                            //------------------------
						    Response_FB();          //Module Response Data Recieve
						    Check_Data();                        
						break;                  //Setting Data Set Start END
						
						case 'C':
                            //--------------------------
                            //from mc[ Continuous_FB() ]
                            //--------------------------                            
						    Receive_Data();           //LCD Continues Data Recieve
						break;
					  
						default:
						break;
					}                           //Ctrl Code Select
				break;                          //Module 1
                               
				default:
				break;
			}
		break;
        
		default:
		break;
	}
	f_SData = 0;
	f_EData = 0;
    EOT_Flag = 0;
}

void Check_Data(void)
{
	if(CheckDataCnt < 3)
	{
		CheckDataCnt++;
		if(BattHiVoltage != BattHiVoltage_FB || 
         BattLowVoltage != BattLowVoltage_FB || 
         RestartTimeValue != RestartTimeValue_FB)
		{
			Set_OK = 1;
		}
	}
	else	CheckDataCnt=0;
}

void Response_FB(void)
{
    int tmpvalue = 0;
    unsigned char a,b,c,d;
    // Battery Hi Voltage Setting Feedback
    a = RxBuf[4];
    b = RxBuf[5];
    c = RxBuf[6];
    d = RxBuf[7];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    BattHiVoltage_FB = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                          
                               
    // Battery Low Voltage Setting Feedback
    a = RxBuf[8];
    b = RxBuf[9];
    c = RxBuf[10];
    d = RxBuf[11];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    BattLowVoltage_FB = tmpvalue * 0.1;    //  xxxx -> xxx.x
                }
            }
        }
    }

    // Restart Time Setting Feedback
    a = RxBuf[12];
    b = RxBuf[13];
    c = RxBuf[14];
    d = RxBuf[15];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    RestartTimeValue_FB = tmpvalue/10;      //  xxxx -> xxx.x
                }
            }
        }		
    }
    // Spare1 Feedback
    a = RxBuf[16];
    b = RxBuf[17];
    c = RxBuf[18];
    d = RxBuf[19];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    //Spare1 = tmpvalue * 0.1;      //xxxx -> xxx.x
                }
            }
        }		
    }
    // Spare2 Feedback
    a = RxBuf[20];
    b = RxBuf[21];
    c = RxBuf[22];
    d = RxBuf[23];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;	
                    //Spare2 = tmpvalue * 0.1;        //xxxx -> xxx.x
                }
            }
        }		
    }
    // Spare3 Feedback
    a = RxBuf[24];
    b = RxBuf[25];
    c = RxBuf[26];
    d = RxBuf[27];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;	
                    //Spare3 = tmpvalue * 0.1;        //xxxx -> xxx.x
                }
            }
        }
    }
    
    // Mode, Module Select Feedback
    a = RxBuf[28];
    b = RxBuf[29];
    c = RxBuf[30];
    d = RxBuf[31];
	gAutoManual_FB = a-0x30;
	gSelectModule_FB = b-0x30;
    gOperationMode_FB = c-0x30;
    gSysTypeMode_FB = d-0x30;
}

void Set_Value()
{
    UINT8 i = 0,checksum=0;
    INT16 tmpvalue = 0;
    tmpvalue = BattHiVoltage * 10;
    Send_Serial(tmpvalue,1,'S');
    tmpvalue = BattLowVoltage * 10;
    Send_Serial(tmpvalue,2,'S');
    tmpvalue = RestartTimeValue * 10;
    Send_Serial(tmpvalue,3,'S');
    tmpvalue = SSpare[0] * 10;
    Send_Serial(tmpvalue,4,'S');
    tmpvalue = SSpare[1] * 10;
    Send_Serial(tmpvalue,5,'S');
    tmpvalue = SSpare[2] * 10;
    Send_Serial(tmpvalue,6,'S');
    //tmpvalue = SSpare[4] * 10;
    //Send_Serial(tmpvalue,7,'S');

    TxBuf[0] = '[';                      //Start of Transmission Packet - SOT => 0x5B '['
    TxBuf[1] = 'D';                      //Display Unit      => 0x44 'D'
    TxBuf[2] = '0';                      //Module ID data   => 0
    TxBuf[3] = 'S';                      //Data Type => Response => 0x53 'S'
	
	TxBuf[28] = gAutoManual+0x30;
	TxBuf[29] = gSelectModule+0x30;
	TxBuf[30] = gOperationMode+0x30;
	TxBuf[31] = gSysTypeMode+0x30;      
	
	for(i=0;i<32;i++)
	{
		checksum^=TxBuf[i];
	}
	TxBuf[32] = (checksum>>4)+0x30;
	TxBuf[33] = (checksum&0x0F)+0x30;
	TxBuf[34] = ']';					//End of Transmission Packet - EOT => 0x5D ']'
    //
    //printf(TxBuf);
    // for(i=0; i<35; i++)                 //Data Packet 28Byte
    // {
    //     U1TXREG = TxBuf[i];
    //     while(!U1STAbits.TRMT);
    // }
    for(i=0 ; i<35 ; i++)
    {       
        UART1_Write(TxBuf[i]);
        while(!U1STAbits.TRMT); 
    }
}

void Receive_Data(void)
{
    int tmpvalue = 0;
    unsigned char a,b,c,d;
    // Voltage Output
    a = RxBuf[4];
    b = RxBuf[5];
    c = RxBuf[6];
    d = RxBuf[7];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    BattVoltage = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }                      
                               
    // Current Output
    a = RxBuf[8];
    b = RxBuf[9];
    c = RxBuf[10];
    d = RxBuf[11];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    BattCurrent = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }
    }

    // Temperature
    a = RxBuf[12];
    b = RxBuf[13];
    c = RxBuf[14];
    d = RxBuf[15];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;
                    LoadVoltage = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }
	
    a = RxBuf[16];
    b = RxBuf[17];
    c = RxBuf[18];
    d = RxBuf[19];
    if(a >= '0' && a <= '9')
    {
        if(b >= '0' && b <= '9')
        {
            if(c >= '0' && c <= '9')
            {
                if(d >= '0' && d <= '9')
                {
                    tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                    LoadCurrent = tmpvalue * 0.1;  //  xxxx -> xxx.x
                }
            }
        }		
    }
	//Module NFB LED Status
    LedState[0] = RxBuf[20];
    LedState[1] = RxBuf[21];//
    LedState[2] = RxBuf[22];
    LedState[3] = RxBuf[23];
    ModuleState[0] = RxBuf[24];		//#1 Module Status
	ModuleState[1] = RxBuf[25];		//#2 Module Status
	ModuleState[2] = RxBuf[26];		//#3 Module Status
	ModuleState[3] = RxBuf[27];		//#4 Module Status
	BatteryState[0] = RxBuf[28];	//Battery Status bit3 => 0:Charge 1:Discharge
	WarningState[0] = RxBuf[29];	//Warning stats
    ChargeState[0] = RxBuf[30];     //Charge stats
	//ModuleState[7] = RxBuf[31];	//Spare3

}


//                                              Module -> Module LCD Transmit-KHJ                                    //
//===================================================================================================================//
//                                       Continuous Data Protocol Packet                                             //
//  '<' + 'U'+ '0' + 'C' + TxBuf[0~7](Vlotage[0~3] / Ampere[4~7]) / Temperature[8~11]                  //   
//  Input_Volt_R[12~15] / Input_Volt_S[16~19] / Input_Volt_S[20~23]) / STS[24] / SP1[25] / SP2[26] / SP2[27] + '>'   //
//--------------------------------------------------------------------------------------------------------------------------------------------//
//                                        Response Data Protocol Packet                                                                       //
// '<' + 'U'+ '0' + 'R' + TxBuf[0~19](C_Ref[0~3] / OC_Limt[4~7] / V_Ref[8~11] / UV_Limit[12~15] / OV_Limit[16~19]) + '>'      //
//============================================================================================================================================//
void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch)
{
	unsigned int tmpval;
	unsigned char a,b,c,d,e;
	//==================================================================// 
	//TxBuf[0~7] ===> 'C' [0~3] / [4~7]									//
	//TxBuf[0~19] ===> 'R' [1~3] / [4~7] / [8~11] / [12~15] / [16~19]	//
	//==================================================================//
	tmpval = ival;
	//
	a=__builtin_divud((long)tmpval,10000);
	b=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000)) ),1000);
	c=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000)) ),100);
	d=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000) + __builtin_muluu(c,100)) ),10);
	e= tmpval%10;
	//
    switch(ch)		//KHJ - RS-232 & RS-485
    {
        case 'C':	//Tx Type -Continuous
            switch(pindex)
            {
                case 1:
                    TxBuf[4]= b + 0x30;                 //Spare1
                    TxBuf[5]= c + 0x30;
                    TxBuf[6]= d + 0x30;
                    TxBuf[7]= e + 0x30;
                break;
                        //
                case 2:
                    TxBuf[8]= b + 0x30;                 //Spare2
                    TxBuf[9]= c + 0x30;
                    TxBuf[10]= d + 0x30;
                    TxBuf[11]= e + 0x30;
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                 //Spare3
                    TxBuf[13]= c + 0x30;
                    TxBuf[14]= d + 0x30;
                    TxBuf[15]= e + 0x30;
                break;
                
                case 4:
                    TxBuf[16]= b + 0x30;                 //Spare4
                    TxBuf[17]= c + 0x30;
                    TxBuf[18]= d + 0x30;
                    TxBuf[19]= e + 0x30;
                break;
                
                case 5:
                    TxBuf[20]= b + 0x30;                 //Spare5
                    TxBuf[21]= c + 0x30;
                    TxBuf[22]= d + 0x30;
                    TxBuf[23]= e + 0x30;
                break;
                
                case 6:
                    TxBuf[24]= b + 0x30;                 //Spare6
                    TxBuf[25]= c + 0x30;
                    TxBuf[26]= d + 0x30;
                    TxBuf[27]= e + 0x30;
                break;
				
				case 7:
                    TxBuf[28]= b + 0x30;                 //Spare7
                    TxBuf[29]= c + 0x30;
                    TxBuf[30]= d + 0x30;
                    TxBuf[31]= e + 0x30;
                break;

                default:
                break;
            }
        break; 
        
        case 'S':		//Tx Type - Request
            switch(pindex)
            {
                case 1:
                    TxBuf[4]= b + 0x30;                 //Set_ARef/1000 => 1000
                    TxBuf[5]= c + 0x30;                 //Set_ARef/100 => 100
                    TxBuf[6]= d + 0x30;                 //Set_ARef/10  => 10
                    TxBuf[7]= e + 0x30;                 //Set_ARef%1  => 1
                break;
                //
                case 2:
                    TxBuf[8]= b + 0x30;                 //Set_CL/1000 => 1000
                    TxBuf[9]= c + 0x30;                 //Set_CL/100 => 100
                    TxBuf[10]= d + 0x30;                 //Set_CL/10  => 10
                    TxBuf[11]= e + 0x30;                 //Set_CL%1  => 1
                break;
                
                case 3:
                    TxBuf[12]= b + 0x30;                 //Set_VRef/1000 => 1000
                    TxBuf[13]= c + 0x30;                 //Set_VRef/100 => 100
                    TxBuf[14]= d + 0x30;                //Set_VRef/10  => 10
                    TxBuf[15]= e + 0x30;                //Set_VRef%1  => 1
                break;
                //
                case 4:
                    TxBuf[16]= b + 0x30;                 //Set_UV/1000 => 1000
                    TxBuf[17]= c + 0x30;                 //Set_UV/100 => 100
                    TxBuf[18]= d + 0x30;                 //Set_UV/10  => 10
                    TxBuf[19]= e + 0x30;                 //Set_UV%1  => 1
                break;
                
                case 5:
                    TxBuf[20]= b + 0x30;                 //Set_OV/1000 => 1000
                    TxBuf[21]= c + 0x30;                 //Set_OV/100 => 100
                    TxBuf[22]= d + 0x30;                 //Set_OV/10  => 10
                    TxBuf[23]= e + 0x30;                 //Set_OV%1  => 1
                break;
                
                case 6:
                    TxBuf[24]= b + 0x30;                 //Set_OT/1000 => 1000
                    TxBuf[25]= c + 0x30;                 //Set_OT/100 => 100
                    TxBuf[26]= d + 0x30;                 //Set_OT/10  => 10
                    TxBuf[27]= e + 0x30;                 //Set_OT%1  => 1
                break;
                
                case 7:
                    TxBuf[28]= b + 0x30;                 //Spare
                    TxBuf[29]= c + 0x30;                 //Spare
                    TxBuf[30]= d + 0x30;                 //Spare
                    TxBuf[31]= e + 0x30;                 //Spare
                break;
                   
                default:
                break;
                
            }
        break; 
     
        default:
        break;
    }

}
//------------------------------------------------------------------------------
void Serial_Process(void)
{
    uint8_t i=0,checksum=0;
    int tmpvalue=0;
    
    if(f_SData && f_EData)
	{
		Response_Serial();
	}
    else
    {
        //-------------------------------------------------------------//
        //                  Control Request Data Send                  //
        //-------------------------------------------------------------//        
        if(TM8 >= RS232_Period)//interval 300ms
        {
            TM8=0;//clear timer//            
            //=========================================================//
            //    Continuous Data Protocol Packet                      //
            //    '[' + 'U'+ '0' + 'C' + TxBuf[0~24]+ SP4 + ']'        // 
            //=========================================================//            
            if(Set_OK == 1)     //'S'command send flag 
            {
                if(Key_Cnt == 0)
                {
                    Key_Cnt = 1;
                    Set_Value();//'S'command transmit to MainControl//
                    Set_OK = 0; 
                }
                else{Set_OK = 0;}
            }
            //----------------------------------------------------------//
            //     Metering & Alarm Data Transmit-KHJ                   //
            //----------------------------------------------------------//  
            else
            {   
                Key_Cnt = 0;
                ///////////////////////////////////////////
                //          Metering data(24byte)        //
                ///////////////////////////////////////////                   
                tmpvalue = CSpare[0]*10;             //Spare1
                Send_Serial(tmpvalue,1,'C');
                tmpvalue = CSpare[1]*10;             //Spare2
                Send_Serial(tmpvalue,2,'C');
                tmpvalue = CSpare[2]*10;             //Spare3
                Send_Serial(tmpvalue,3,'C');
                tmpvalue = CSpare[3]*10;             //Spare4
                Send_Serial(tmpvalue,4,'C');
                tmpvalue = CSpare[4]*10;             //Spare5
                Send_Serial(tmpvalue,5,'C');
                tmpvalue = CSpare[5]*10;             //Spare6
                Send_Serial(tmpvalue,6,'C');
                tmpvalue = CSpare[6]*10;             //Spare7
                Send_Serial(tmpvalue,7,'C');
                //RS-232 Module
                TxBuf[0] = '[';      //Start of Transmission Packet - SOT => 0x5B '[' 
                TxBuf[1] = 'D';      //Chrger Unit      => 0x44 'D'
                TxBuf[2] = '0';      //Module ID data   => 0x30 '0'
                TxBuf[3] = 'C';      //Data Type => Continues 'C'
                //        
                for(i=0;i<32;i++)
                {
                    checksum ^= TxBuf[i];
                }
                TxBuf[32]=(checksum>>4)+0x30;
                TxBuf[33]=(checksum&0x0F)+0x30;
                TxBuf[34]=']';						//End of Transmission Packet - EOT                                
                //
                for(i=0 ; i<35 ; i++)
                {       
                    UART1_Write(TxBuf[i]);
                    while(!U1STAbits.TRMT); 
                }    
            }
        }
    }         
}
