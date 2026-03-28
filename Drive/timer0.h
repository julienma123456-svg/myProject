#include "config.h"
#include "typedef.h"
#include "timer.h"
//定时器相关寄存器

#define US2RELOAD(us) (MAIN_Fosc / us)

sfr TMOD = 0x89;
sfr AUXR = 0x8E;
sfr INT_CLKO = 0x8F;

//sbit TR0  = 0x8C;
sbit ET0  = 0xA9;

void Timer0_Init(unsigned int reload,InVoid_OutVoid fCallBack);
