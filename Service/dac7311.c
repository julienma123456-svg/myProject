#include "dac7311.h"

static unsigned int OutputDAC = 0x00;

//向DAC7311写数据
void  WriteDAC(unsigned int dacData)
{
	unsigned char i;
	unsigned int temp;

	dacData = dacData & 0x0FFF;
	OutputDAC = dacData;
	temp = dacData << 2;
	GPIO_OutHigh(enumSYNC);
	;;;
	;;;
	;;;
	GPIO_OutLow(enumSYNC);
	for(i = 0;i < 16;i++)
	{
		GPIO_OutHigh(enumSCLK);
		if(temp & 0x8000)
			GPIO_OutHigh(enumDIN);
		else
		{
			GPIO_OutLow(enumDIN);
		}
		;;;
		GPIO_OutLow(enumSCLK);
		;;;
		temp = temp << 1;		
	}
	GPIO_OutHigh(enumSYNC);	
}

//输出功率
void OutputPower(float power)
{
	unsigned int dac = 0x00;
	dac = power / MAX_VAL_POWER * MAX_VAL_DAC;
	WriteDAC(dac);
}

//获取正在输出的DAC值
unsigned int GetOutputDAC(void)
{
	return OutputDAC;
}

//1ms调用一次
int DacTick = 0;
void DAC7311Tick(void)
{
	DacTick++;
//	if(!MeasureGetDacEn())
//	{
//		if(OutputDAC != 0)
//		{
//			WriteDAC(0x00);
//		}
//		else if(DacTick >= 5000)
//		{
//			DacTick = 0;
//			WriteDAC(0x00);
//		}
//	}

	if(DacTick >= 10000)
	{
		DacTick = 0;
	}
}

/*
//向DAC7311写数据
void  WriteDAC(uint dacdata1)
{
	uchar i;
	uint temp;
	temp=dacdata1<<2;
	SBIT_SYNC=1;
	Delaynop(1);
	SBIT_SYNC=0;
	for(i=0;i<16;i++)
	{
		SBIT_SCLK=1;
		if(temp&0x8000)
			SBIT_DIN=1;
		else
			SBIT_DIN=0;
		//Delaynop(1);
		SBIT_SCLK=0;
		//Delaynop(1);
		temp=temp<<1;		
	}
	SBIT_SYNC=1;	
}

*/

