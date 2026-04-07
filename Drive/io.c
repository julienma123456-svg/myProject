#include "io.h"

//M1M0配置：00 		 01 	  10 	   11
//模式：    准双向口 推挽输出 高阻输入 开漏输出	

/*************输出引脚******************************/

/*************输入引脚******************************/
//IO口初始化
void GPIO_Init(void)
{
	//准双向口:00
	P2M1 &= CLEAR_BIT7;   P2M0 &= CLEAR_BIT7;   //P2.7 uart cs
	
	P2M1 &= CLEAR_BIT4;   P2M0 &= CLEAR_BIT4;   //P2.4 spi sclk
	P2M1 &= CLEAR_BIT5;   P2M0 &= CLEAR_BIT5;   //P2.5 spi cs 
	P2M1 &= CLEAR_BIT3;   P2M0 &= CLEAR_BIT3;   //P2.3 spi data
	
	P4M1 &= CLEAR_BIT3;   P4M0 &= CLEAR_BIT3;   //P4.3 PTT

	P2M1 &= CLEAR_BIT6;   P2M0 &= CLEAR_BIT6;   //P2.6 射频源控制ONoff
	P4M1 &= CLEAR_BIT4;   P4M0 &= CLEAR_BIT4;   //P4.4 led
	//高阻输入:10
	P0M1 |= SET_BIT2;   P0M0 &= CLEAR_BIT2;   //P0.2 alarm

	GPIO_OutHigh(enumFREGSWONFF);//默认射频源关闭
	GPIO_OutHigh(enum485CTRL);//高电平光耦不导通 接受模式
}

//输出高电平
void GPIO_OutHigh(enumGPIOName gpioName)
{
	if(gpioName == enum485CTRL)
		P27 = 1;
	else if(gpioName == enumSCLK)
		P24 = 1;
	else if(gpioName == enumSYNC)
		P25 = 1;
	else if(gpioName == enumDIN)\
		P23 = 1;
	else if(gpioName == enumLED)
		P44 = 1;
//	else if(gpioName == enumLED1)
//		P42 = 1;
//	else if(gpioName == enumLED2)
//		P20 = 1;
//	else if(gpioName == enumLED3)
//		P41 = 1;
//	else if(gpioName == enumLED4)
//		P37 = 1;
	else if(gpioName == enumPTT)
		P07 = 1;				
	else if(gpioName == enumFREGSWONFF)
		P26 = 1;
}

//输出低电平
void GPIO_OutLow(enumGPIOName gpioName)
{
	if(gpioName == enum485CTRL)
		P27 = 0;
	else if(gpioName == enumSCLK)
		P24 = 0;
	else if(gpioName == enumSYNC)
		P23 = 0;
	else if(gpioName == enumDIN)
		P25 = 0;
	else if(gpioName == enumLED)
		P44 = 0;
//	else if(gpioName == enumLED1)
//		P42 = 0;
//	else if(gpioName == enumLED2)
//		P20 = 0;
//	else if(gpioName == enumLED3)
//		P41 = 0;
//	else if(gpioName == enumLED4)
//		P37 = 0;
	else if(gpioName == enumPTT)
		P07 = 0;				
	else if(gpioName == enumFREGSWONFF)
		P26 = 0;					
}

//获取电平状态
unsigned char GPIO_GetIn(enumGPIOName gpioName)
{
	if(gpioName == enumDIN)
		return P23;
	else if(gpioName == enumALARM_T)
		return P02;
	else if(gpioName == enumPTT)
		return P07;
	else if(gpioName == enumFREGSWONFF)
		return P26;
	return 0xFF;
}




