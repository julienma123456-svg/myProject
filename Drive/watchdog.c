#include "watchdog.h"
#include "delay.h"

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
	unsigned char regVal = 0x00;
	regVal |= SET_BIT4;		//清除计数
	WDT_CONTR = regVal;
}

void Trap(void)
{
	delay_ms(1000); // 确保串口数据发送完成
	// 直接进入死循环，等待看门狗复位
    WDT_CONTR = 0x3C;  
    // bit7: WDT enable
    // bit5: clear WDT
    // bit4: WDT idle run
    // 0x3C = 启动WDT + 清计数 + 设置溢出时间

    while(1);  // 等待WDT溢出触发复位
}