#include "debug.h"


//打印一个数组
void PrintfArray(unsigned char *dataIn,unsigned char dataLen)
{
	Comm2SendData(dataIn,dataLen);	
}

//打印AD采样结果
void PrintfAD_Result(void)
{
//	char adcInx = 0;
//	unsigned int adc[ADC_NUM] = {0};
//	unsigned char ptADC[80] = {0};
//
//	for(adcInx = 0;adcInx < ADC_NUM;adcInx++)
//	{
//		adc[adcInx] = ADC_GetResult(adcInx);
//	}
//	sprintf(ptADC,"ADC[0]=%d,[1]=%d,[2]=%d,[3]=%d,[4]=%d\r\n",
//				adc[0],adc[1],adc[2],adc[3],adc[4]);
//
//	Comm2SendData(ptADC,sizeof(ptADC));
}

