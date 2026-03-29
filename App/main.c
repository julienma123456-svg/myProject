#include "drive.h"
#include "delay.h"
#include "comm1.h"
#include "comm2.h"
#include "stdio.h"
#include "modbusCRC.h"
#include "measure.h"
#ifdef __USEMODBUS_SERVICE__
#include "modbus.h"
#endif
#ifdef __USEMASTERCOMM_SERVICE__
#include "mastercomm.h"
#endif
#include "dac7311.h"
#include "adjust.h"
#include "led.h"

char Timer0Flag = 0;
void Timer0_IntService(void);
void Timer0_Service(void);

unsigned char recLen = 0;
unsigned char xdata recArray[100] = {0};

void main(void)
{
	char i = 0;
	char adcInx = 0;
    DriveInit();
	EnableInt();
	//配置定时器0，每1ms中断一次
	Timer0_Init(US2RELOAD(1000),Timer0_IntService);
	//串口3初始化，用于实现Comm1(485通信)
	UART3_Config(9600,Comm1SendOneDataOK,Comm1RecOneData);
	//串口1初始化，用于实现Comm2(打印调试口)
	UART1_Config(115200,Comm2SendOneDataOK,Comm2RecOneData);

	Comm1Init();   //Comm1初始化
	Comm2Init();   //Comm2初始化
	ADC_Init();	   //ADC初始化，采集5个通道
	MeasureInit(); //模拟量采集初始化
	PowerOnFlash();
	WatchDogInit(enumReset1050ms);
	LoadAdjustData();

	while(1)
	{
		MeasureTask();		 //模拟量采集的任务函数
		#ifdef __USEMODBUS_SERVICE__
		ModBusService();	 //485通信服务函数
		#endif
		#ifdef __USEMASTERCOMM_SERVICE__
		MasterCommService(); //主控通信服务函数
		#endif
		LED_Task();			 //LED点灯任务
//	    if(Comm2GetRecData(recArray,&recLen))
//		{
//			Comm2SendData(recArray,recLen);
//		}
		if(Timer0Flag)
		{
			Timer0Flag = 0;
			Timer0_Service();
			FeedWatchDog();		 //喂狗
		}
	}
}

//========================================================================
// 函数: void Timer0_IntService (void)
// 描述: timer0中断回调函数(用于添加应用代码)
// 参数: none.
// 返回: none.
// 版本: V1.0, 2025-6-14
//========================================================================
void Timer0_IntService(void)
{
   Timer0Flag = 1;
}

int count;
void Timer0_Service (void)
{
	count++;
	Comm1Tick();
	Comm2Tick();
	MeasureTick();
	DAC7311Tick();
	if(count == 500)
	{
		GPIO_OutHigh(enumLED);
	}
	else if(count >= 900)
	{
		GPIO_OutLow(enumLED);
		count = 0;
	}
}
