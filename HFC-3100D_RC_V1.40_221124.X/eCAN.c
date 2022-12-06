/****************************************************************************************
* Application : HFC-3100D														      	*
* Seoul Electric Power System co.,Ltd  	  2018/04/10  	First release of source file  	*
* Device : Rectifier(Microchip dspic33FJ64GS606)										*
* Target PCB No :                                                            *
* Compiler : mplab_C V3.31											 					*
****************************************************************************************/
#include "p33FJ64GS606.h"
#include "Define_PSFB.h"
#include "Variables_PSFB.h"
#include "eCAN.h"

/* function prototypes as defined in can.c */
void CAN_Process(void);
void Continuous_FB_CAN(void);
void Send_Serial_CAN(unsigned int ival,unsigned char pindex,unsigned char ch);
void sendECAN(mID *message);
void rxECAN(mID *message);
void clearRxFlags(unsigned char buffer_number);
void Conf_ECAN(void);

//========================= Define ECAN Message Buffers ======================*/
mID canTxMessage;			//CAN Transmit variable//
mID canRxMessage;			//CAN Recevie buffer //
ECAN1MSGBUF ecan1msgBuf __attribute__((space(dma),aligned(ECAN1_MSG_BUF_LENGTH*16)));




void CAN_Process(void)
{
    int tmpvalue = 0;
    unsigned char a = 0, b = 0, c = 0, d = 0;
    
    //Polling Can Recevied data//
    if(canRxMessage.buffer_status == CAN_BUF_FULL)	
    {
        rxECAN(&canRxMessage);

        // Received data from other Modules//
        a = canRxMessage.data[0];
        b = canRxMessage.data[1];
        c = canRxMessage.data[2];
        d = canRxMessage.data[3];
        if(a >= '0' && a <= '9')
        {
            if(b >= '0' && b <= '9')
            {
                if(c >= '0' && c <= '9')
                {
                    if(d >= '0' && d <= '9')
                    {
                        tmpvalue = (a-0x30)*1000 + (b-0x30)*100  + (c-0x30)*10 + (d-0x30)*1;		
                        canShare = tmpvalue * 0.1;  //  xxxx -> xxx.x
                        if(canShare >= 0.0 && canShare <= OCL)     // Min < canShare < Current Limit
                        {
                            canShare_B = canShare;
                        }               
                        else
                        {
                            canShare = canShare_B;
                        }
                    }
                }
            }		
        }      
        //					
        switch (canRxMessage.id)		//check rect ID (1~8)//
        {
            case 1:
                CanModAmp[0] = canShare;							
                break;

            case 2:
                CanModAmp[1] = canShare;
                break;

            case 3:
                CanModAmp[2] = canShare;
                break;

            case 4:
                CanModAmp[3] = canShare;
                break;	
        }	
    								
        canShareSum = canShareSum + canShare;// + AmpFnd_Avg;//CanModAmp[0]+CanModAmp[1]+CanModAmp[2]+CanModAmp[3];
        iCanShareCnt++;
        if(iCanShareCnt >= 20)//5ms * 10 = 50ms
        {							
            canShare = canShareSum/iCanShareCnt;
            canShareSum = 0;
            iCanShareCnt = 0;
        }                           	
    
        canMax = canShare; //if(canShare>canMax)??//
    //iCanBufA=(int)canShare;
        canShareQ15 = (canMax*8) <<5;		//canShare<<5;
    
        f_canReceive = 1;							
        cntNotCanData = 0;	
        canRxMessage.buffer_status = CAN_BUF_EMPTY;
    }
    
    else
    {	
        // CAN Message Tranmit Data
        if(tmrtick1 > 5)				//elapse time about 5ms
        {
            Continuous_FB_CAN();
            tmrtick1 = 0;                 //Tmr 1_1 Reset
        }
        
        // CAN Commumication Terminate
//        cntNotCanData++;
//        if(cntNotCanData>100)
//        {	
//            cntNotCanData = 101;
//            canShare = 0;
//            canMax = 0;				
//            canShareQ15 = 0;
//        } 
    }	
}

void Continuous_FB_CAN(void)
{
    unsigned char i = 0;
    int tmpvalue = 0;
    
    ///////////////////////////////////////////
    //          Metering data(8byte)         //
    ///////////////////////////////////////////   
    tmpvalue = Amp_Val*10;              //Current_out             //Vout_Val*10;
    Send_Serial_CAN(tmpvalue,1,'C');
    tmpvalue = Verror;//IpsfbQ15_Flt;      //Bus Voltage out
    Send_Serial_CAN(tmpvalue,2,'C');

    //------------------------
    //Transciver Buffer Setup 
    //------------------------
    canTxMessage.message_type = CAN_MSG_DATA;
    canTxMessage.frame_type = CAN_FRAME_STD;
    canTxMessage.buffer = 0;
    canTxMessage.id = ID485;		//
    //-----------------------------------			//
    for(i=0; i<8; i++)                                 //Data Packet
    {
        canTxMessage.data[i] = TxBuf_CAN[i];			//current High byte
    }
    canTxMessage.data_length = 8;    	
    //Send Current Data 
    if(softstart == 0)
    {
        if(C1TR01CONbits.TXREQ0==0)
        {					
            sendECAN(&canTxMessage);							
        }				
        f_canReceive=0;					  	//clear buffer//        				
    }	
}


void Send_Serial_CAN(unsigned int ival,unsigned char pindex,unsigned char ch)
{
    unsigned int tmpval;
	unsigned char a,b,c,d,e;
	//==================================================================// 
	//TxBuf[0~23] ===> 'C' Amp_Val[0~3] / VBus_Val[4~7]                                //
	//==================================================================//
	tmpval = ival;
	//
	a=__builtin_divud((long)tmpval,10000);
	b=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000)) ),1000);
	c=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000)) ),100);
	d=__builtin_divud((long)(tmpval - (__builtin_muluu(a,10000) + __builtin_muluu(b,1000) + __builtin_muluu(c,100)) ),10);
	e= tmpval%10;
    
    switch(ch)		//KHJ - RS-232 & RS-485
    {
        case 'C':	//Tx Type -Continuous
            switch(pindex)
            {
                case 1:
                    TxBuf_CAN[0] = b + 0x30;                 //Currentt_Out/1000 => 1000
                    TxBuf_CAN[1] = c + 0x30;                 //Currentt_Out/100 => 100
                    TxBuf_CAN[2] = d + 0x30;                 //Currentt_Out/10  => 10
                    TxBuf_CAN[3] = e + 0x30;                 //Currentt_Out%1  => 1
                break;
                        //
                case 2:
                    TxBuf_CAN[4] = b + 0x30;                 //Volt_Out_BUS/1000 => 1000
                    TxBuf_CAN[5] = c + 0x30;                 //Volt_Out_BUS/100 => 100
                    TxBuf_CAN[6] = d + 0x30;                //Volt_Out_BUS/10  => 10
                    TxBuf_CAN[7] = e + 0x30;                //Volt_Out_BUS%1  => 1
                break;
                
                default:
                break;
            }
        break; 
        
        default:
        break;          
    }
		
}

void Conf_ECAN(void)
{
    canTxMessage.message_type = CAN_MSG_DATA;		/* configure and send a message */
    canTxMessage.frame_type = CAN_FRAME_STD;		//Standard ID - 11bit//
    canTxMessage.buffer = 0;
    canTxMessage.id = ID485;
    canTxMessage.data[0] = 0x00;					//
    canTxMessage.data[1] = 0x00;
    canTxMessage.data[2] = 0x03;
    canTxMessage.data[3] = 0x51;
    canTxMessage.data[4] = 0x00;
    canTxMessage.data[5] = 0x00;
    canTxMessage.data[6] = 0x00;
    canTxMessage.data[7] = 0x00;
    canTxMessage.data_length = 8;					//data lenght 8 bytes//
    //-----------------------------------------------------------------
    //sendECAN(&canTxMessage);					//Transmit Can Data  //
    //-----------------------------------------------------------------
    
    //###################################################################
}

void sendECAN(mID *message)
{
	tstport4 = 1;
	unsigned long word0=0;
	unsigned long word1=0;
	unsigned long word2=0;
	
	/*
	Message Format: 
	Word0 : 0bUUUx xxxx xxxx xxxx
			     |____________|||
 					SID10:0   SRR IDE(bit 0)     
	Word1 : 0bUUUU xxxx xxxx xxxx
			   	   |____________|
						EID17:6
	Word2 : 0bxxxx xxx0 UUU0 xxxx
			  |_____||	     |__|
			  EID5:0 RTR   	  DLC
	
	Remote Transmission Request Bit for standard frames 
	SRR->	"0"	 Normal Message 
			"1"  Message will request remote transmission
	Substitute Remote Request Bit for extended frames 
	SRR->	should always be set to "1" as per CAN specification
	
	Extended  Identifier Bit			
	IDE-> 	"0"  Message will transmit standard identifier
	   		"1"  Message will transmit extended identifier
	
	Remote Transmission Request Bit for extended frames 
	RTR-> 	"0"  Message transmitted is a normal message
			"1"  Message transmitted is a remote message
	Don't care for standard frames 
	*/
		
	/* check to see if the message has an extended ID */
	if(message->frame_type == CAN_FRAME_EXT)
	{
		/* get the extended message id EID28..18*/		
		word0=(message->id & 0x1FFC0000) >> 16;			
		/* set the SRR and IDE bit */
		word0=word0+0x0003;
		/* the the value of EID17..6 */
		word1=(message->id & 0x0003FFC0) >> 6;
		/* get the value of EID5..0 for word 2 */
		word2=(message->id & 0x0000003F) << 10;			
	}	
	else
	{
		/* get the SID */
		word0=((message->id & 0x000007FF) << 2);	
	}
	/* check to see if the message is an RTR message */
	if(message->message_type==CAN_MSG_RTR)
	{		
		if(message->frame_type==CAN_FRAME_EXT)
			word2=word2 | 0x0200;
		else
			word0=word0 | 0x0002;	
								
		ecan1msgBuf[message->buffer][0]=word0;
		ecan1msgBuf[message->buffer][1]=word1;
		ecan1msgBuf[message->buffer][2]=word2;
	}
	else
	{
		word2=word2+(message->data_length & 0x0F);
		ecan1msgBuf[message->buffer][0]=word0;
		ecan1msgBuf[message->buffer][1]=word1;
		ecan1msgBuf[message->buffer][2]=word2;
		/* fill the data */
		ecan1msgBuf[message->buffer][3]=((message->data[1] << 8) + message->data[0]);
		ecan1msgBuf[message->buffer][4]=((message->data[3] << 8) + message->data[2]);
		ecan1msgBuf[message->buffer][5]=((message->data[5] << 8) + message->data[4]);
		ecan1msgBuf[message->buffer][6]=((message->data[7] << 8) + message->data[6]);
	}
	/* set the message for transmission */
	C1TR01CONbits.TXREQ0=1;
	tstport4 = 0;
}

/******************************************************************************
*                                                                             
*    Function:			rxECAN
*    Description:       moves the message from the DMA memory to RAM
*                                                                             
*    Arguments:			*message: a pointer to the message structure in RAM 
*						that will store the message. 
*	 Author:            Jatinder Gharoo                                                      
*	                                                                 
*                                                                              
******************************************************************************/
void rxECAN(mID *message)
{
	unsigned int ide=0;
	unsigned int rtr=0;
	unsigned long id=0;
			
	/*
	Standard Message Format: 
	Word0 : 0bUUUx xxxx xxxx xxxx
			     |____________|||
 					SID10:0   SRR IDE(bit 0)     
	Word1 : 0bUUUU xxxx xxxx xxxx
			   	   |____________|
						EID17:6
	Word2 : 0bxxxx xxx0 UUU0 xxxx
			  |_____||	     |__|
			  EID5:0 RTR   	  DLC
	word3-word6: data bytes
	word7: filter hit code bits
	
	Remote Transmission Request Bit for standard frames 
	SRR->	"0"	 Normal Message 
			"1"  Message will request remote transmission
	Substitute Remote Request Bit for extended frames 
	SRR->	should always be set to "1" as per CAN specification
	
	Extended  Identifier Bit			
	IDE-> 	"0"  Message will transmit standard identifier
	   		"1"  Message will transmit extended identifier
	
	Remote Transmission Request Bit for extended frames 
	RTR-> 	"0"  Message transmitted is a normal message
			"1"  Message transmitted is a remote message
	Don't care for standard frames 
	*/
		
	/* read word 0 to see the message type */
	ide=ecan1msgBuf[message->buffer][0] & 0x0001;			
	
	/* check to see what type of message it is */
	/* message is standard identifier */
	if(ide==0)
	{
		message->id=(ecan1msgBuf[message->buffer][0] & 0x1FFC) >> 2;		
		message->frame_type=CAN_FRAME_STD;
		rtr=ecan1msgBuf[message->buffer][0] & 0x0002;
	}
	/* mesage is extended identifier */
	else
	{
		id=ecan1msgBuf[message->buffer][0] & 0x1FFC;		
		message->id=id << 16;
		id=ecan1msgBuf[message->buffer][1] & 0x0FFF;
		message->id=message->id+(id << 6);
		id=(ecan1msgBuf[message->buffer][2] & 0xFC00) >> 10;
		message->id=message->id+id;		
		message->frame_type=CAN_FRAME_EXT;
		rtr=ecan1msgBuf[message->buffer][2] & 0x0200;
	}
	/* check to see what type of message it is */
	/* RTR message */
	if(rtr==1)
	{
		message->message_type=CAN_MSG_RTR;	
	}
	/* normal message */
	else
	{
		message->message_type=CAN_MSG_DATA;
		message->data[0]=(unsigned char)ecan1msgBuf[message->buffer][3];
		message->data[1]=(unsigned char)((ecan1msgBuf[message->buffer][3] & 0xFF00) >> 8);
		message->data[2]=(unsigned char)ecan1msgBuf[message->buffer][4];
		message->data[3]=(unsigned char)((ecan1msgBuf[message->buffer][4] & 0xFF00) >> 8);
		message->data[4]=(unsigned char)ecan1msgBuf[message->buffer][5];
		message->data[5]=(unsigned char)((ecan1msgBuf[message->buffer][5] & 0xFF00) >> 8);
		message->data[6]=(unsigned char)ecan1msgBuf[message->buffer][6];
		message->data[7]=(unsigned char)((ecan1msgBuf[message->buffer][6] & 0xFF00) >> 8);
		message->data_length=(unsigned char)(ecan1msgBuf[message->buffer][2] & 0x000F);
	}
	clearRxFlags(message->buffer);	
}

/******************************************************************************
*                                                                             
*    Function:			clearRxFlags
*    Description:       clears the rxfull flag after the message is read
*                                                                             
*    Arguments:			buffer number to clear 
*	 Author:            Jatinder Gharoo                                                      
*	                                                                 
*                                                                              
******************************************************************************/
void clearRxFlags(unsigned char buffer_number)
{
	if((C1RXFUL1bits.RXFUL1) && (buffer_number==1))
		/* clear flag */
		C1RXFUL1bits.RXFUL1=0;		
	/* check to see if buffer 2 is full */
	else if((C1RXFUL1bits.RXFUL2) && (buffer_number==2))
		/* clear flag */
		C1RXFUL1bits.RXFUL2=0;				
	/* check to see if buffer 3 is full */
	else if((C1RXFUL1bits.RXFUL3) && (buffer_number==3))
		/* clear flag */
		C1RXFUL1bits.RXFUL3=0;				
	else;

}

/* CAN Communication Interrupt Service Routine 1    */
/* No fast context save, and no variables stacked   */
void __attribute__((interrupt,no_auto_psv))_C1Interrupt(void)  
{
	/* clear interrupt flag */
	IFS2bits.C1IF=0;
	/* check to see if the interrupt is caused by receive */     	 
    if(C1INTFbits.RBIF)
    {
		/* clear flag */
		C1INTFbits.RBIF = 0;
	    /* check to see if buffer 1 is full */
	    if(C1RXFUL1bits.RXFUL1)
	    {			
			/* set the buffer full flag and the buffer received flag */
			canRxMessage.buffer_status=CAN_BUF_FULL;
			canRxMessage.buffer=1;	
		}		
		/* check to see if buffer 2 is full */
		else if(C1RXFUL1bits.RXFUL2)
		{
			/* set the buffer full flag and the buffer received flag */
			canRxMessage.buffer_status=CAN_BUF_FULL;
			canRxMessage.buffer=2;					
		}
		/* check to see if buffer 3 is full */
		else if(C1RXFUL1bits.RXFUL3)
		{
			/* set the buffer full flag and the buffer received flag */
			canRxMessage.buffer_status=CAN_BUF_FULL;
			canRxMessage.buffer=3;					
		}
		C1RXFUL1=C1RXFUL2=0x0000;
	}//end ifC1INTFbits.RBIF)
	else if(C1INTFbits.TBIF)
    {
	     /* clear flag */		
		C1INTFbits.TBIF = 0;	    
	}//end if(C1INTFbits.TBIF)
	
	if(C1INTFbits.ERRIF == 1)
	{	
		C1INTFbits.ERRIF = 0;		
	}//end if(C1INTFbits.ERRIF == 1)
}//end _C1Interrupt(void)  

