#include "uart1.h"

uart1_mng_struct gs_uart1Mng = {0, 0};

//========================================================================
// 函数: void UART1_config(u8 brt)
// 描述: UART1初始化函数。
// 参数: baudRate: 波特率,使用Timer1做波特率.
// 返回: none.
// 版本: VER1.0
// 日期: 2014-11-28
// 备注: 
//========================================================================
void UART1_Config(unsigned long baudRate)    
{
	unsigned long xdata reload = 0;
	reload = 65536UL - (MAIN_Fosc / 4) / baudRate;

	TR1 = 0;
    AUXR &= ~0x01;      //S1 BRT Use Timer1;
    AUXR |=  (1<<6);    //Timer1 set as 1T mode
    TMOD &= ~(1<<6);    //Timer1 set As Timer
    TMOD &= ~0x30;      //Timer1_16bitAutoReload;
    TH1 = reload / 256;
    TL1 = reload % 256;
    ET1 = 0;    //禁止中断
    INT_CLKO &= ~0x02;  //不输出时钟
    TR1  = 1;

	//UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 
	SCON = (SCON & 0x3f) | 0x40;    
//  PS  = 1;    //高优先级中断
    ES  = 1;    //允许中断
    REN = 1;    //允许接收
    P_SW1 &= 0x3f;
    P_SW1 |= 0x00;      //UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4
//  PCON2 |=  (1<<4);   //内部短路RXD与TXD, 做中继, ENABLE,DISABLE
}

void Uart1RegresiterCallback(InVoid_OutVoid fSend,InU8_OutVoid fRec)
{
    gs_uart1Mng.funSendOneDataOk = fSend;
    gs_uart1Mng.funRecOneData = fRec;
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
void UART1_Int (void) interrupt 4
{
	unsigned char recData = 0;

	EA = 0;     //关闭总中断
    if((SCON & 0x01) != 0)
    {
        SCON &= ~0x01;    //Clear Rx flag
        recData = S1BUF;
        if(gs_uart1Mng.funRecOneData != 0)
            gs_uart1Mng.funRecOneData(recData);
    }

    if((SCON & 0x02) != 0)
    {
        SCON &= ~0x02;    //Clear Tx flag
		if(gs_uart1Mng.funSendOneDataOk != 0)
            gs_uart1Mng.funSendOneDataOk();
    }
	EA = 1;     //打开总中断
}

//发送一个字节
void UART1_SendOneData(unsigned char dataIn)
{
    S1BUF = dataIn;
}

