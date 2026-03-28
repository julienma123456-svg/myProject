#include "drive.h"

//初始化
void DriveInit(void)
{
	GPIO_Init();
}

//打开中断
void EnableInt(void)
{
   EA = 1;     //打开总中断
}


//关闭中断
//void DisableInt(void)
//{
//   EA = 0;     //关闭总中断
//}

//使能访问外部数据
void EnableXdata(void)
{
	P_SW2 |= 0x80;
}

//关闭访问外部数据
void DisableXdata(void)
{
	P_SW2 &= 0x7f;
}
