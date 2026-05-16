#include "measure.h"
#include "delay.h"
static unsigned char OutPowerFreq = 90;
static unsigned int tickCount = 0;

static unsigned char GetInPowerAdjInx(void);
//ADC通道映射表
static unsigned char ADC_ChIndex[Analog_NUM] = 
{
ADC0,
ADC1,
ADC2,
ADC3,
ADC4,
};

//转换倍数表
static float VolToAnalogCoeff[Analog_NUM] = 
{
VOL_AI_TEMP,
VOL_AI_INP,
VOL_AI_REP,
VOL_AI_50V,
VOL_AI_17A,
};

// Table: 1000 * log10(x), x = 1 to 256
static const unsigned short log10_table[256] = {
    0, 301, 477, 602, 699, 778, 845, 903, 954, 1000, 1041, 1079, 1114, 1146, 1176, 1204,
    1230, 1255, 1279, 1301, 1322, 1342, 1362, 1380, 1398, 1415, 1431, 1447, 1462, 1477, 1491, 1505,
    1519, 1531, 1544, 1556, 1568, 1580, 1591, 1602, 1613, 1623, 1633, 1643, 1653, 1663, 1672, 1681,
    1690, 1699, 1708, 1716, 1724, 1732, 1740, 1748, 1756, 1763, 1771, 1778, 1785, 1792, 1799, 1806,
    1813, 1820, 1826, 1833, 1839, 1845, 1851, 1857, 1863, 1869, 1875, 1881, 1886, 1892, 1898, 1903,
    1908, 1914, 1919, 1924, 1929, 1934, 1940, 1944, 1949, 1954, 1959, 1964, 1968, 1973, 1978, 1982,
    1987, 1991, 1996, 2000, 2004, 2009, 2013, 2017, 2021, 2025, 2029, 2033, 2037, 2041, 2045, 2049,
    2053, 2057, 2061, 2064, 2068, 2072, 2076, 2079, 2083, 2086, 2090, 2093, 2097, 2100, 2104, 2107,
    2111, 2114, 2117, 2121, 2124, 2127, 2130, 2134, 2137, 2140, 2143, 2146, 2149, 2152, 2155, 2158,
    2161, 2164, 2167, 2170, 2173, 2176, 2179, 2182, 2185, 2188, 2190, 2193, 2196, 2199, 2201, 2204,
    2207, 2210, 2212, 2215, 2217, 2220, 2223, 2225, 2228, 2230, 2233, 2236, 2238, 2241, 2243, 2246,
    2248, 2250, 2253, 2255, 2258, 2260, 2262, 2265, 2267, 2270, 2272, 2274, 2277, 2279, 2281, 2283,
    2286, 2288, 2290, 2292, 2294, 2297, 2299, 2301, 2303, 2305, 2307, 2310, 2312, 2314, 2316, 2318,
    2320, 2322, 2324, 2326, 2328, 2330, 2332, 2334, 2336, 2338, 2340, 2342, 2344, 2346, 2348, 2350,
    2352, 2354, 2356, 2358, 2360, 2362, 2364, 2366, 2368, 2369, 2371, 2373, 2375, 2377, 2378, 2380,
    2382, 2384, 2386, 2387, 2389, 2391, 2393, 2394, 2396, 2398, 2400, 2401, 2403, 2405, 2407, 2408
};


static const ntc_table_t ntc_table[34] = {
    {3977, -400}, {3937, -350}, {3886, -300}, {3822, -250},
    {3744, -200}, {3649, -150}, {3535, -100}, {3405, -50},
    {3249,    0}, {3077,   50}, {2891,  100}, {2688,  150},
    {2478,  200}, {2048,  250}, {1823,  300}, {1612,  350},
    {1420,  400}, {1240,  450}, {1083,  500}, {945,   550},
    {819,   600}, {712,   650}, {625,   700}, {534,   750},
    {470,   800}, {405,   850}, {355,   900}, {310,   950},
    {268,  1000}, {231,  1050}, {202,  1100}, {176,  1150},
    {154,  1200}, {135,  1250}
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

/**
 * @brief 纯定点计算 RL (0.1dB 分辨率)
 * @param pf 入射功率 (W)
 * @param pr 反射功率 (W)
 * @return float 返回值 (单位: dB)
 */
// float pa_calc_return_loss(float pf, float pr)
// {
//     unsigned int ratio_fixed; // 放大 16 倍的定点比值
//     unsigned short idx;       // 查表索引
//     unsigned short frac;      // 插值余数
//     unsigned int log_val;     // 插值后的结果 (1000倍)
//     int rl_01db;              // 最终结果 (0.1dB单位)
// 	unsigned int base_ratio;

//     // 1. 边界保护
//     if (pf <= 0.0f || pr >= pf) return 0.0f;
//     if (pr <= 0.0001f) return 50.0f;

//     // 2. 计算比值并转为定点数 (放大16倍，为了匹配你原来的插值逻辑)
//     // ratio = (pf / pr) * 16
//     ratio_fixed = (unsigned int)((pf / pr) * 16.0f);

//     // 3. 拆分索引和余数
//     // 索引偏移：因为表里索引0代表比值1，所以要减去16(即比值1.0)
//     if (ratio_fixed < 16) return 0.0f; 
    
//     base_ratio = ratio_fixed / 16; // 整数比值
//     idx = base_ratio - 1;                       // 查表索引 (比值1对应idx 0)
//     frac = ratio_fixed & 0x0F;                  // 16进制下的余数

//     // 4. 防止索引越界 (你的表长256)
//     if (idx >= 254) return 30.0f; // 比值超过255倍，RL通常认为很大了

//     // 5. 线性插值
//     // log10_table[idx] 是 1000 * log10(x)
//     log_val = log10_table[idx] + 
//               ((log10_table[idx + 1] - log10_table[idx]) * frac >> 4);

//     // 6. 转换单位
//     // 公式: RL = 10 * log10(ratio)
//     // 此时 log_val 是 1000 * log10(ratio)
//     // 我们要 0.1dB 单位，即 10 * 10 * log10(ratio) = 100 * log10(ratio)
//     // 所以 rl_01db = log_val / 10
//     rl_01db = log_val / 10;

//     return (float)rl_01db / 10.0f;
// }



/**
 * @brief 32位整数开方（牛顿法，快速且精确）
 * @param x 被开方数 (0 ~ 0xFFFFFFFF)
 * @return floor(sqrt(x))
 */
static unsigned long isqrt32(unsigned long x) {
    unsigned long result = 0;
    unsigned long bittmp = 1UL << 30;          // 从次高位开始
    while (bittmp > x) bittmp >>= 2;
    while (bittmp != 0) {
        unsigned long sum = result + bittmp;
        if (x >= sum) {
            x -= sum;
            result = (result >> 1) + bittmp;
        } else {
            result >>= 1;
        }
        bittmp >>= 2;
    }
    return result;
}

/**
 * @brief 定点高精度计算 VSWR（无浮点，适合 C51 等）
 * @param pf 入射功率 (×1, 实际单位)
 * @param pr 反射功率 (×1)
 * @return 驻波比 × 100（即保留两位小数，例如 571 代表 5.71）
 */
static unsigned int pa_calc_vswr_fixed_int(float pf, float pr) {
    unsigned long ratio_q16;       // (pf/pr) 放大 65536 倍
    unsigned long sqrt_ratio;      // sqrt(ratio_q16)
    unsigned long numerator, denominator;

    // 边界保护
    if (pf <= 0.0f) return 0;
    if (pr <= 0.0f) return 100;      // 1.00
    if (pr >= pf)  return 65535;     // 表示无穷大（VSWR > 655.35）

    // 1. 计算 pf/pr 并转为 Q16.16 定点，四舍五入
    ratio_q16 = (unsigned long)((pf / pr) * 65536.0f + 0.5f);
    if (ratio_q16 <= 65536) return 100;   // 实际不会发生

    // 2. 开方得到 |Γ| × 65536? 注意：ratio_q16 = R × 65536，其中 R = pf/pr
    //    我们需要 gamma = sqrt(pr/pf) = 1 / sqrt(R) = 1 / sqrt(ratio_q16/65536)
    //                 = sqrt(65536) / sqrt(ratio_q16) = 256 / sqrt(ratio_q16)
    // 因此 gamma_q16 = (256 << 16) / sqrt(ratio_q16)   (Q16)
    // 简化：先计算 sqrt(ratio_q16)
    sqrt_ratio = isqrt32(ratio_q16);   // sqrt(R × 65536) = sqrt(R) × 256

    // 防止分母为零（sqrt_ratio 最小为 256）
    if (sqrt_ratio <= 256) return 65535;
{
    // gamma_q16 = (256 * 65536) / sqrt_ratio
    unsigned long gamma_q16 = (256UL * 65536UL) / sqrt_ratio;   // Q16.16

    // 3. VSWR = (1 + gamma) / (1 - gamma)
    //    用 Q16.16 计算：1 对应 65536
    numerator   = 65536UL + gamma_q16;
    denominator = 65536UL - gamma_q16;
    if (denominator == 0) return 65535;

    // 结果放大 100 倍输出
	{
    unsigned long vswr_x100 = (numerator * 100) / denominator;
    return (unsigned int)(vswr_x100 > 65535 ? 65535 : vswr_x100);
	}
}
}

/**
 * @brief 调用接口（与原函数签名兼容，返回 float）
 */
static float pa_calc_vswr_fixed(float pf, float pr) {
    unsigned int result_x100 = pa_calc_vswr_fixed_int(pf, pr);
    return result_x100 / 100.0f;
}

static float adc_to_temperature(unsigned short adc)
{
    unsigned char i;

    // 高温（ADC很小）
    if (adc <= ntc_table[sizeof(ntc_table)/sizeof(ntc_table[0]) - 1].adc)
        return (float)(ntc_table[sizeof(ntc_table)/sizeof(ntc_table[0]) - 1].temp)/ 10.0;

    // 低温（ADC很大）
    if (adc >= ntc_table[0].adc)
        return (float)(ntc_table[0].temp) / 10.0;

    // 查表 + 线性插值
    for (i = 0; i < sizeof(ntc_table)/sizeof(ntc_table[0]) - 1; i++)
    {
        if (adc <= ntc_table[i].adc && adc >= ntc_table[i+1].adc)
        {
            float adc1 = ntc_table[i].adc;
            float adc2 = ntc_table[i+1].adc;
            float  t1   = ntc_table[i].temp;
            float  t2   = ntc_table[i+1].temp;

            return (t1 + (adc - adc1) * (t2 - t1) / (adc2 - adc1))/10.0;
        }
    }

    return 250.0; // fallback：25.0°C
}

float getReturnLoss(void)
{
	#ifdef MOCK_DATA_FOR_TEST
	// 模拟数据，测试用
	MeasureStruct.AnalogVal[Analog_17A] = 12.35;
	MeasureStruct.AnalogVal[Analog_InPower] = 102.5;
	MeasureStruct.AnalogVal[Analog_RefPower] = 20.5;
	MeasureStruct.lossVal = pa_calc_vswr_fixed(MeasureStruct.AnalogVal[Analog_InPower],MeasureStruct.AnalogVal[Analog_RefPower]);
	#endif
	return MeasureStruct.lossVal;
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
	if(GPIO_GetIn(enumPTT) == 0)
		MeasureStruct.OutDAC_EnFlag = 1;
	else
		MeasureStruct.OutDAC_EnFlag = 0;
	
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
		// adcVol = (float)val / ADC_MAX_VAL * ADC_REF_VOL;
		adcVol = (float)val / ADC_MAX_VAL;
		MeasureStruct.AD_Val[i] = val; 
		
		if(i == Analog_TEMP)//温度要特殊处理
		{
			MeasureStruct.AnalogVal[i] = adc_to_temperature(val);
			
			// sprintf(pstring,"Temp ADC=%d\t Temp = %.1f C\r\n",val,MeasureStruct.AnalogVal[i]);
			// PrintfArray(pstring,strlen(pstring));
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
	MeasureStruct.lossVal = pa_calc_vswr_fixed(MeasureStruct.AnalogVal[Analog_InPower],MeasureStruct.AnalogVal[Analog_RefPower]);
	// sprintf(pstring,"Analog_InPower = %.1f\t Analog_RefPower = %.1f\t lossVal = %.1f\t\r\n",MeasureStruct.AnalogVal[Analog_InPower],MeasureStruct.AnalogVal[Analog_RefPower],MeasureStruct.lossVal);
	// PrintfArray(pstring,strlen(pstring));
	
	#ifdef MOCK_DATA_FOR_TEST
    MeasureStruct.AnalogVal[Analog_InPower] = 102.5;
	MeasureStruct.AnalogVal[Analog_RefPower] = 0.5;
    #endif
	// 源电压异常告警
	if(MeasureStruct.AnalogVal[Analog_50V] < 25)
		MeasureStruct.VolAlarmFlag = VOLT_LOW_ALM;
	else if(MeasureStruct.AnalogVal[Analog_50V] > 31)
		MeasureStruct.VolAlarmFlag = VOLT_HIGH_ALM;
	else
		MeasureStruct.VolAlarmFlag = 0;	
		
	//末级功放电流告警
	if(MeasureStruct.AnalogVal[Analog_17A] < 0.3f)
		MeasureStruct.Curr17AlarmFlag = 1;
	else
		MeasureStruct.Curr17AlarmFlag = 0;	
		
	//末前级功放电流告警
//	if(MeasureStruct.AnalogVal[Analog_2A] < 0.1f)
//		MeasureStruct.Curr2AlarmFlag = 1;
//	else
//		MeasureStruct.Curr2AlarmFlag = 0;
		
	//驻波过大告警
	if(MeasureStruct.AnalogVal[Analog_InPower] < 
						MeasureStruct.AnalogVal[Analog_RefPower] * 5)
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
	if(MeasureStruct.VolAlarmFlag == VOLT_HIGH_ALM)
		alarm = 1;
	else if(MeasureStruct.VolAlarmFlag == VOLT_LOW_ALM)
	{
		alarm = 2;
	}
	else if(MeasureStruct.TempAlarmFlag)
		alarm = 3;
	// if(MeasureStruct.Curr17AlarmFlag)
	// 	alarm |= SET_BIT3;

	// if(MeasureStruct.Curr2AlarmFlag)
	// 	alarm |= SET_BIT4;

	else if(MeasureStruct.OverWaveAlarmFlag)
		alarm = 5;

	if(alarm == 0x00)
	{
		alarm = 0xFF; //无告警时返回0xFF，方便上位机区分
	}
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
	//if(OutPowerFreq < 90)
		return 0;
	// else if(OutPowerFreq < 110)
	// 	return 1;
	// else
	// 	return 2;
}





