#include "measure.h"
#include <math.h>

static unsigned char OutPowerFreq = 90;
static unsigned int tickCount = 0;

static unsigned char GetInPowerAdjInx(void);
static float adc_to_temperature(int adc_value) ;
//ADC通道映射表
static unsigned char ADC_ChIndex[Analog_NUM] = 
{
ADC0,
ADC1,
ADC2,
//ADC3,
ADC4,
};

//转换倍数表
static float VolToAnalogCoeff[Analog_NUM] = 
{
VOL_AI_TEMP,
VOL_AI_INP,
VOL_AI_REP,
//VOL_AI_2A,
VOL_AI_17A,
};
static MeasureStructType MeasureStruct;

//结构体初始化
void MeasureInit(void)
{
	char i = 0;
	MeasureStruct.TempAlarmFlag = 0;
	//MeasureStruct.OutDAC_EnFlag = 0;

	for(i = 0;i < Analog_NUM;i++)
	{
		MeasureStruct.AnalogVal[i] = 0;		
	}
}

static float pa_calc_return_loss(float pf, float pr)
{
    // 防止非法输入
    if (pf <= 0.0f)
        return 0.0f;

    // 防止log(0)
    if (pr <= 1e-6f)
        return 50.0f;   // 认为匹配很好，给上限

    return 10.0f * log10f(pf / pr);
}

float getReturnLoss(void)
{
	MeasureStruct.lossVal;
}

/****************采集列表***************************/
//1.50V采集：	  ADC0采集结果
//2.入射功率采集：ADC1采集结果
//3.反射功率采集：ADC2采集结果
//4.2A电流采集：  ADC3采集结果
//5.17A电流采集： ADC4采集结果
//6.温度告警采集：TALARM IO口采集(低电平告警)
//7.DAC输出使能采集：PTT IO口采集(低电平有效)
//8.源电压异常告警: 50V源电压小于40V
//9.末级功放电流告警：17A电流小于300mA
//10.末前级功放电流告警：2A电流小于100mA
//11.驻波过大告警：入射功率 <= (反射功率 * 10)

//采集所有值
static void MeasureAllVal(void)
{
	char i = 0;
	char err = 0;
	char adcInx = 0;
	char adjInx = 0;
	unsigned int val = 0;
	float adcVol = 0.0f;
	float coeff = 0.0f;
	//DAC输出使能脚采集
//	if(GPIO_GetIn(enumPTT) == 0)
//		MeasureStruct.OutDAC_EnFlag = 1;
//	else
//		MeasureStruct.OutDAC_EnFlag = 0;
	
	//温度告警脚采集								    
	if(GPIO_GetIn(enumALARM_T) == 0)
		MeasureStruct.TempAlarmFlag = 1;
	else
		MeasureStruct.TempAlarmFlag = 0;
		
	for(i = 0;i < Analog_NUM;i++)
	{
		adcInx = ADC_ChIndex[i];
		coeff = VolToAnalogCoeff[i];
		val = ADC_GetResult(adcInx);
		//adcVol = (float)val / ADC_MAX_VAL * ADC_REF_VOL;
		adcVol = (float)val / ADC_MAX_VAL;
		MeasureStruct.AD_Val[i] = val; 

		
		if(i == Analog_TEMP)//温度要特殊处理
		{
			MeasureStruct.AnalogVal[i] = adc_to_temperature(val);
		}
		else if(i == Analog_InPower)//入射功率要校准
		{
			float temp = 0;
			adjInx = GetInPowerAdjInx();
			temp = GetAdjustResult(enumInputPower + adjInx,val,&err);
			if(!err)
			{
				MeasureStruct.AnalogVal[i] = temp / 100;
			}
			else
			{
				MeasureStruct.AnalogVal[i] = adcVol * coeff;
			}
		}
		else if(i == Analog_RefPower)//反射功率要校准
		{
			float temp = 0;
			temp = GetAdjustResult(enumRefPower,val,&err);
			if(!err)
			{
				MeasureStruct.AnalogVal[i] = temp / 100;
			}
			else
			{
				MeasureStruct.AnalogVal[i] = adcVol * coeff;
			}
		}
		else//其他模拟量直接转换
		{
			MeasureStruct.AnalogVal[i] = adcVol * coeff;
		}
	}
	
	MeasureStruct.lossVal = pa_calc_return_loss(MeasureStruct.AnalogVal[Analog_InPower],MeasureStruct.AnalogVal[Analog_RefPower]);

	//源电压异常告警
	if(MeasureStruct.AnalogVal[Analog_50V] < 40)
		MeasureStruct.VolAlarmFlag = 1;
	else
		MeasureStruct.VolAlarmFlag = 0;	
		
	//末级功放电流告警
	if(MeasureStruct.AnalogVal[Analog_17A] < 0.3f)
		MeasureStruct.Curr17AlarmFlag = 1;
	else
		MeasureStruct.Curr17AlarmFlag = 0;	
		
	//末前级功放电流告警
	if(MeasureStruct.AnalogVal[Analog_2A] < 0.1f)
		MeasureStruct.Curr2AlarmFlag = 1;
	else
		MeasureStruct.Curr2AlarmFlag = 0;
		
	//驻波过大告警
	if(MeasureStruct.AnalogVal[Analog_InPower] < 
						MeasureStruct.AnalogVal[Analog_RefPower] * 10)
		MeasureStruct.OverWaveAlarmFlag = 1;
	else
		MeasureStruct.OverWaveAlarmFlag = 0;							
}

//采集功能tick(1ms调用一次)
void MeasureTick(void)
{
	tickCount++;
}

//main函数调用的服务函数
void MeasureTask(void)
{
	if(tickCount % REFRESH_TIME_MS == 0)
	{
		MeasureAllVal();
	}

	if(tickCount % 1000 == 0)
	{
		PrintfAD_Result();
	}
}


// 函数：将ADC值转换为温度
static float adc_to_temperature(unsigned int adc_value) 
{
    // 第一步：将ADC值转换为电压
    float V_NTC = (float)adc_value / ADC_MAX * V_REF;

    // 第二步：计算热敏电阻的电阻值
    float R_NTC = (V_NTC * R4) / (V_REF - V_NTC);

    // 第三步：使用斯坦哈特方程或者B系数近似计算温度
    // 使用B系数的简单近似公式
    float T_kelvin = 1 / (1 / 298.15 + (1 / B_FACTOR) * log(R_NTC / 10000));  // B是热敏电阻的B系数
    float T_celsius = T_kelvin - 273.15;  // 转换为摄氏度

    return T_celsius;
}

//获取温度告警
//unsigned char MeasureGetTempAlarm(void)
//{
//	return MeasureStruct.TempAlarmFlag;
//}

//获取输出DAC使能引脚
//unsigned char MeasureGetDacEn(void)
//{
//	return MeasureStruct.OutDAC_EnFlag;
//}

//获取采集AD量
unsigned int MeasureGetAD_Val(enumAnalogIndex index)
{
	if(index >= Analog_NUM)
		return 0;

	return MeasureStruct.AD_Val[index];	
}

//获取采集模拟量
float MeasureGetAnalog(enumAnalogIndex index)
{
	if(index >= Analog_NUM)
		return 0;

	if(index == Analog_InPower)	  //入射清零值5W
	{
		if(MeasureStruct.AnalogVal[index] < 5)
			return 0;
	}
	else if(index == Analog_RefPower)	  //反射清零值3W
	{
		if(MeasureStruct.AnalogVal[index] < 3)
			return 0;
	}

	return MeasureStruct.AnalogVal[index];	
}

//获取告警值
unsigned char MeasureGetAlarmFlag(void)
{
	unsigned char alarm = 0x00;
	if(MeasureStruct.VolAlarmFlag)
		alarm |= SET_BIT1;

	if(MeasureStruct.TempAlarmFlag)
		alarm |= SET_BIT2;

	if(MeasureStruct.Curr17AlarmFlag)
		alarm |= SET_BIT3;

	if(MeasureStruct.Curr2AlarmFlag)
		alarm |= SET_BIT4;

	if(MeasureStruct.OverWaveAlarmFlag)
		alarm |= SET_BIT5;

	return alarm;
}

//记录输出功率的频率(75~125MHz)
void RefreshOutPowerFreq(unsigned char ctrInx)
{
	 //ctrInx对应没5MHz的段序号
	 OutPowerFreq = 75 + ctrInx * 5;
}

//获取输入功率校准序号
//[75,90)对应0
//[90,110)对应0
//[110,125]对应0
static unsigned char GetInPowerAdjInx(void)
{
	if(OutPowerFreq < 90)
		return 0;
	else if(OutPowerFreq < 110)
		return 1;
	else
		return 2;
}





