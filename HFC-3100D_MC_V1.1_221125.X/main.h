//main.h//

extern unsigned int mainCnt;
extern unsigned int mainToggle;
extern unsigned char mainByte;
extern unsigned int mainValue[10];

extern unsigned int sendCmd;//OP_CMD;
extern unsigned int sendTimer;
extern unsigned int fSendTwice;
extern unsigned int normalCnt;


extern void Request_Send(void);
extern void Set_Send(void);
extern void Serial_Process(void);
extern void Serial_Process_485(void);
//extern void Send_Data_485(UINT8 data);
extern void Send_Data_485(char *data);
extern void Set485_Rx_Mode(void);
extern void Response_Serial(void);
extern void Response_Serial_485(void);
extern void Continuous_Send(void);
extern void Receive_Data_485(UINT8 ID);
extern void Response_Data(void);
extern void Response_FB(void);					// Set Data Feedback RX
extern void Response_FB_Debug(void);			// Set Data Feedback RX
extern void Continuous_FB(void);
extern void BittoByte(void);
extern void Send_Serial(unsigned int ival,unsigned char pindex,unsigned char ch);

extern void BittoByte(void);

//
extern void GetNfbState1(void);
extern void GetNfbState2(void);
extern void GetContact(void);

extern void EEPROMSet(void);