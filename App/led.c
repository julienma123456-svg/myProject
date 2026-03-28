#include "led.h"

/*
LED正极接单片机IO口，负极接地
点亮LED时，IO口输出高电平
DAC输出时，点亮LED1
电源告警时，点亮LED2
温度告警时，点亮LED3
其它告警，点亮LED4
*/

//LED点灯任务
void LED_Task(void)
{
	unsigned char alarm = 0;
	alarm = MeasureGetAlarmFlag();

	//LED1
	if(GetOutputDAC() > 0)
	{
		GPIO_OutHigh(enumLED1);
	}
	else
	{
		GPIO_OutLow(enumLED1);
	}

	//LED2
	if(alarm & SET_BIT1)
	{
		GPIO_OutHigh(enumLED2);
	}
	else
	{
		GPIO_OutLow(enumLED2);
	}
	alarm = alarm & CLEAR_BIT1;

	//LED3
	if(alarm & SET_BIT2)
	{
		GPIO_OutHigh(enumLED3);
	}
	else
	{
		GPIO_OutLow(enumLED3);
	}
	alarm = alarm & CLEAR_BIT2;

	//LED4
	if(alarm)	 //如果还有告警
	{
		GPIO_OutHigh(enumLED4);
	}
	else
	{
		GPIO_OutLow(enumLED4);
	}
}





