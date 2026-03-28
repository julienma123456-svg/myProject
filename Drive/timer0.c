#include "timer0.h"

InVoid_OutVoid fIntCallBack;

//========================================================================
// 函数: void   Timer0_init(void)
// 描述: timer0初始化函数.
// 参数: reload
// 返回: none.
// 版本: V1.0, 2025-6-4
//========================================================================
void Timer0_Init(unsigned int reload,InVoid_OutVoid fCallBack)
{
	TR0 = 0;    //停止计数
	ET0 = 1;    //允许中断
	TMOD &= ~0x03; 	//工作模式bit1bit0, 0: 16位自动重装
	//TMOD |= 0; 
	TMOD &= ~0x04;  //bit2，0为定时器
	INT_CLKO &= ~0x01;  //不输出时钟(P3.5)
	
	AUXR |=  0x80;  //bit7为1，不分频
	TH0 = (unsigned char)((65536UL - reload) / 256);
	TL0 = (unsigned char)((65536UL - reload) % 256);
	
	TR0 = 1;    //开始运行

	fIntCallBack = fCallBack;
}

//定时器中断函数
void timer0_int (void) interrupt 1
{
	fIntCallBack();
}


