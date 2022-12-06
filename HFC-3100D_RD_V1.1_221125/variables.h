//file name variables.h//
#define CheckBit(x,y) 	((x)>>(y)) & (0x1)
#define SetBit(x,y) 	(x)|=((0x1)<<(y))
#define ClrBit(x,y) 	(x)&=~((0x1)<<(y))

extern volatile unsigned int tmrCnt[10];
extern volatile unsigned int tmr232;
extern volatile unsigned int tmr500ms;
// extern unsigned char menu_gain[1024];
// extern unsigned char menu_run[1024];
// extern unsigned char menu_debug[1024];

//extern unsigned char menu2[][1024];
