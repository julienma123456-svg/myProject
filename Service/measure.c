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

// log10(x) * 100, x = 1~255
// 精度约 ±0.2 dB（工程足够）
static const unsigned short log10_table[256] = {
    0,
    0,301,477,602,699,778,845,903,954,1000,1041,1078,1111,1141,1169,
    1195,1218,1240,1260,1278,1295,1311,1326,1340,1354,1366,1378,1389,1400,1410,1420,
    1429,1438,1447,1455,1463,1471,1478,1486,1493,1500,1506,1513,1519,1525,1531,1537,
    1543,1548,1554,1559,1564,1569,1574,1579,1584,1588,1593,1597,1602,1606,1610,1614,
    1618,1622,1626,1630,1633,1637,1640,1644,1647,1651,1654,1657,1660,1663,1666,1669,
    1672,1675,1678,1681,1683,1686,1689,1691,1694,1696,1699,1701,1704,1706,1708,1711,
    1713,1715,1717,1720,1722,1724,1726,1728,1730,1732,1734,1736,1738,1740,1742,1744,
    1746,1748,1750,1752,1753,1755,1757,1759,1760,1762,1764,1765,1767,1769,1770,1772,
    1773,1775,1776,1778,1779,1781,1782,1784,1785,1786,1788,1789,1790,1792,1793,1794,
    1796,1797,1798,1799,1801,1802,1803,1804,1805,1807,1808,1809,1810,1811,1812,1813,
    1814,1816,1817,1818,1819,1820,1821,1822,1823,1824,1825,1826,1827,1828,1829,1830,
    1831,1832,1833,1834,1835,1836,1837,1838,1839,1840,1841,1842,1843,1844,1845,1846,
    1847,1848,1849,1850,1851,1852,1853,1854,1855,1856,1857,1858,1859,1860,1861,1862,
    1863,1864,1865,1866,1867,1868,1869,1870,1871,1872,1873,1874,1875,1876,1877,1878,
    1879,1880,1881,1882,1883,1884,1885,1886,1887,1888,1889,1890,1891,1892,1893,1894,
    1895,1896,1897,1898,1899,1900,1901,1902,1903,1904,1905,1906,1907,1908,1909,1910
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

// ===== 主函数 =====
// 输入：pf/pr（单位：W，范围 0~400）
// 输出：RL（单位：0.1 dB）
float pa_calc_return_loss(float pf, float pr)
{
    unsigned short pf_i, pr_i;
    unsigned short idx_pf, idx_pr;
    unsigned short frac_pf, frac_pr;
    unsigned short log_pf, log_pr;
    short rl;

    // ===== 边界保护 =====
    if (pf <= 0.0f)
        return 0;

    if (pr <= 0.0001f)
        return 500;   // 50.0 dB

	if(pr >= pf)
		return 0;
    // ===== float → 定点（映射到 0~4095）=====
    // 400W → 4095
    pf_i = (unsigned short)(pf * 10.2375f);  // 4095/400 ≈ 10.2375
    pr_i = (unsigned short)(pr * 10.2375f);

    if (pf_i == 0) pf_i = 1;
    if (pr_i == 0) pr_i = 1;

    // ===== 8bit索引 + 插值 =====
    idx_pf = pf_i >> 4;
    frac_pf = pf_i & 0x0F;

    idx_pr = pr_i >> 4;
    frac_pr = pr_i & 0x0F;

    if (idx_pf == 0) idx_pf = 1;
    if (idx_pr == 0) idx_pr = 1;

    // ===== 插值 =====
    log_pf = log10_table[idx_pf] +
        ((log10_table[idx_pf + 1] - log10_table[idx_pf]) * frac_pf >> 4);

    log_pr = log10_table[idx_pr] +
        ((log10_table[idx_pr + 1] - log10_table[idx_pr]) * frac_pr >> 4);

    // ===== RL计算 =====
    rl = (log_pf - log_pr) / 10;   // 转成 0.1dB

    if (rl < 0)
        rl = 0;

    return (float)rl / 10.0f;
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
	
	MeasureStruct.lossVal = pa_calc_return_loss(MeasureStruct.AnalogVal[Analog_InPower],MeasureStruct.AnalogVal[Analog_RefPower]);
	//源电压异常告警
//	if(MeasureStruct.AnalogVal[Analog_50V] < 40)
//		MeasureStruct.VolAlarmFlag = 1;
//	else
//		MeasureStruct.VolAlarmFlag = 0;	
		
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
	// if(MeasureStruct.VolAlarmFlag)
	// 	alarm |= SET_BIT1;

	if(MeasureStruct.TempAlarmFlag)
		alarm |= SET_BIT3;

	// if(MeasureStruct.Curr17AlarmFlag)
	// 	alarm |= SET_BIT3;

	// if(MeasureStruct.Curr2AlarmFlag)
	// 	alarm |= SET_BIT4;

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





