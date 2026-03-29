#include "watchdog.h"

static unsigned char s_wdtRegVal = 0x00;

//看门狗初始化
void WatchDogInit(enumWDTResetTime resetTime)
{
	unsigned char regVal = 0x00;
	regVal |= SET_BIT5;		//使能
	regVal |= resetTime;
	WDT_CONTR = regVal;
}

//喂狗
void FeedWatchDog(void)
{
	if(s_wdtRegVal)
	{
		return;//重启设备
	}
	unsigned char regVal = 0x00;
	regVal |= SET_BIT4;		//清除计数
	WDT_CONTR = regVal;
}

void Trap(void)
{
	s_wdtRegVal = 1;
	while(1);
}