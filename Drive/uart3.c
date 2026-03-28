#include "uart3.h"

static InVoid_OutVoid funSendOneDataOk;
static InU8_OutVoid funRecOneData;

//========================================================================
// 函数: void UART3_config(u8 brt)
// 描述: UART3初始化函数。
// 参数: baudRate: 波特率,使用Timer3做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART3_Config(unsigned long baudRate,InVoid_OutVoid fSend,InU8_OutVoid fRec)    
{
	unsigned long reload = 0;
	reload = 65536UL - (MAIN_Fosc / 4) / baudRate;

    S3CON = 0x50;       //8位数据, 使用Timer3做波特率发生器, 允许接收
    T3H = reload / 256;
    T3L = reload % 256;
    T4T3M |= 0x0a;

    IE2 |= 0x08;          //允许UART3中断

#ifdef SELECT_P0
	P0M1 &= 0xFC;   P0M0 &= 0xFC;   //设置为准双向口
    P_SW2 &= ~0x02; 	//UART3 switch bit1 to: 0: P0.0 P0.1
#else
    P_SW2 |= 0x02;      //UART3 switch bit1 to: 1: P5.0 P5.1
#endif

	funSendOneDataOk = fSend;
	funRecOneData = fRec;
}

//========================================================================
// 函数: void UART3_Int (void) interrupt UART3_VECTOR
// 描述: UART3中断函数。
// 参数: nine.
// 返回: none.
// 版本: VER1.0
// 日期: 2025-6-6
// 备注: 
//========================================================================
void UART3_Int (void) interrupt 17
{
	unsigned char recData = 0;

	EA = 0;     //关闭总中断
    if((S3CON & 0x01) != 0)
    {
        S3CON &= ~0x01;    //Clear Rx flag
        recData = S3BUF;
		funRecOneData(recData);
    }

    if((S3CON & 0x02) != 0)
    {
        S3CON &= ~0x02;    //Clear Tx flag
		funSendOneDataOk();
    }
	EA = 1;     //打开总中断
}

//发送一个字节
void UART3_SendOneData(unsigned char dataIn)
{
    S3BUF = dataIn;
}

