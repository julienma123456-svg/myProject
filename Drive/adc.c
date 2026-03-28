#include "adc.h"

//ADC0功能：50V检测
//ADC1功能：入射功率检测
//ADC2功能：反射功率检测
//ADC4功能：17A电流检测

//采集通道P1.0 P1.1 P1.2 P1.4
const unsigned char xdata ADC_ChVal[ADC_NUM] = {0,1,2,4};

unsigned char ADC_ChIndex = 0; 
unsigned int ADC_Result[ADC_NUM] = {0};

//ADC初始化(采集一个数据需要32*16/375ms = 1.36ms)
void ADC_Init(void)
{
	//设置 P1.0 P1.1 P1.2 P1.4为 ADC 输入口
	P1M1 |= 0x17;   P1M0 &= 0x00;   //设置 ADC 输入口

	EnableXdata();
	ADCTIM = 0x3F;		//0 01 11111(采样时间为32个周期) 
	ADCEXCFG = 0x07;	//--00 -111(采样结果为16次平均值)	
	DisableXdata();	

	IE |= 0x20;          //允许ADC中断
	ADCCFG = 0x2F;		//右对齐，16分频(时钟频率约为12/32=0.375M)
	ADC_CONTR = 0xC0; 	//使能 ADC 模块，启动转换
}

//获取采集结果(刷新4个通道需要1.36*4ms = 5.44ms)
unsigned int ADC_GetResult(unsigned char adcCh)
{
	if(adcCh >= ADC_NUM)
		return 0xFFFF;

	return ADC_Result[adcCh];	
}

//ADC采集完成中断
void ADC_Int (void) interrupt 5
{
	ADC_CONTR &= ~0x20;		//bit5
	ADC_Result[ADC_ChIndex] = (ADC_RES << 8) | ADC_RESL;

	//准备采集下一个通道
	ADC_ChIndex++;
	ADC_ChIndex = ADC_ChIndex % ADC_NUM;
	ADC_CONTR = 0xC0 | ADC_ChVal[ADC_ChIndex];
}
