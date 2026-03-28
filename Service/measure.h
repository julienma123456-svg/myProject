#ifndef MEASURE_H
#define MEASURE_H

#include "typedef.h"
#include "adc.h"
#include "debug.h"
#include "adjust.h"

#define		REFRESH_TIME_MS		100

//#define		ADC_REF_VOL		5
#define		ADC_MAX_VAL		4096

//采集信号转换倍数(电压转化为模拟量)
#define		VOL_AI_TEMP		NULL//需要单独处理
#define		VOL_AI_INP		400	//5V对应400W
#define		VOL_AI_REP		400	//5V对应400W
#define		VOL_AI_2A		(2 * 5 / 1)		//1V对应2A
#define		VOL_AI_17A		(17.0f * 5 / 3)	//3V对应17A

// 热敏电阻的常量（根据你的热敏电阻具体参数进行调整）
#define R4 10000            // 固定电阻值（单位：欧姆）
#define V_REF 5.0           // 参考电压（单位：伏特）
#define ADC_MAX 4095        // ADC最大值
#define B_FACTOR 3950              // 热敏电阻B系数（根据数据手册提供）

typedef enum
{
	Analog_TEMP,
	Analog_InPower,
	Analog_RefPower,
//	Analog_2A,
	Analog_17A,
	Analog_NUM,
}enumAnalogIndex;

typedef struct tagMeasure
{
	unsigned char VolAlarmFlag;
	unsigned char TempAlarmFlag;
	unsigned char Curr17AlarmFlag;
	unsigned char Curr2AlarmFlag;
	unsigned char OverWaveAlarmFlag;
//	unsigned char OutDAC_EnFlag;

	unsigned int  AD_Val[Analog_NUM];
	float AnalogVal[Analog_NUM];	//信号值
	float lossVal;	//回波损耗
}MeasureStructType;

void MeasureInit(void);
//void MeasureAllVal(void);
void MeasureTick(void);
void MeasureTask(void);
float getReturnLoss(void);
//unsigned char MeasureGetDacEn(void);
//unsigned char MeasureGetTempAlarm(void);
unsigned char MeasureGetAlarmFlag(void);
unsigned int MeasureGetAD_Val(enumAnalogIndex index);
float MeasureGetAnalog(enumAnalogIndex index);
void RefreshOutPowerFreq(unsigned char ctrInx);

#endif

