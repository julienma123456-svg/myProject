#include "adjust.h"

//校准相关数据结构体
AdjustManageType AdjustManage;

AdjustDataType	 AdjustData[enumAdjustNum];

//获取是否完成校准
unsigned char AdjustIsFinish(enumAdjustIndex index)
{
	if(index >= enumAdjustNum)
	{
		return 0;
	}

	return AdjustManage.IsAdjust[index];
}

//获取校准日期
unsigned char AdjustGetDate(enumAdjustIndex inx,DateStructType *date)
{
	if(inx >= enumAdjustNum)
	{
		return 0;
	}

	//年月日
	date->Year = AdjustData[inx].AdjustDate.Year;
	date->Month = AdjustData[inx].AdjustDate.Month;
	date->Day = AdjustData[inx].AdjustDate.Day;
	//时分秒
	date->Hour = AdjustData[inx].AdjustDate.Hour;
	date->Min = AdjustData[inx].AdjustDate.Min;
	date->Sec = AdjustData[inx].AdjustDate.Sec;
	
	return 1;
}

//开始校准
void AdjustStart(enumAdjustIndex inx)
{
	char i = 0;
	int  arrayLen = 0;

	if(inx >= enumAdjustNum)
	{
		return ;
	}

	AdjustManage.WriteInx[inx] = 0;

	arrayLen = sizeof(AdjustData[0].Point) / sizeof(AdjustData[0].Point[0]);
	for(i = 0;i < arrayLen;i++)
	{
		AdjustData[inx].Point[i].ValY = 0xFFFF;
		AdjustData[inx].Point[i].ValX = 0xFFFF;
	}
}

//开始读取校准数据
void AdjustStartRead(enumAdjustIndex inx)
{
	if(inx >= enumAdjustNum)
	{
		return ;
	}

	AdjustManage.ReadInx[inx] = 0;
}

//校准数据时，添加一个点的数据
char AdjustAddPoint(enumAdjustIndex inx,AdjustPointType point)
{
	char ip = 0;
	int  arrayLen = 0;

	if(inx >= enumAdjustNum)
	{
		return 0;
	}

	ip = AdjustManage.WriteInx[inx];
	arrayLen = sizeof(AdjustData[0].Point) / sizeof(AdjustData[0].Point[0]);
	if(ip >= arrayLen)
		return 0;

	AdjustData[inx].Point[ip].ValY = point.ValY;
	AdjustData[inx].Point[ip].ValX = point.ValX;
	AdjustManage.WriteInx[inx]++;
	return 1; 
}

//读取一个校准数据点
char AdjustReadPoint(enumAdjustIndex inx,AdjustPointType *point)
{
	char ip = 0;
	int  arrayLen = 0;

	if(inx >= enumAdjustNum)
	{
		return 0;
	}

	ip = AdjustManage.ReadInx[inx];
	arrayLen = sizeof(AdjustData[0].Point) / sizeof(AdjustData[0].Point[0]);

	if(ip < arrayLen)
	{
		point->ValY = AdjustData[inx].Point[ip].ValY; 
		point->ValX = AdjustData[inx].Point[ip].ValX;
		AdjustManage.ReadInx[inx]++;
		return 1;
	}
	else
	{
		point->ValY = 0xFFFF;
		point->ValX   = 0xFFFF;
		return 2;
	}
}

//保存校准数据到EEPROM
void SaveAdjustData(enumAdjustIndex inx,DateStructType date)
{
	unsigned int eeAdd = 0;

	if(inx >= enumAdjustNum)
	{
		return;
	}

	eeAdd = EE_ADD_InPower + inx * 0x100;

	AdjustData[inx].Flag = 0x11 * (inx + 1);
	AdjustData[inx].AdjustDate.Year  = date.Year;
	AdjustData[inx].AdjustDate.Month = date.Month;
	AdjustData[inx].AdjustDate.Day   = date.Day;
	AdjustData[inx].AdjustDate.Hour  = date.Hour;
	AdjustData[inx].AdjustDate.Min   = date.Min;
	AdjustData[inx].AdjustDate.Sec   = date.Sec;
	ArrayAddCheck((u8 *)&AdjustData[inx],EE_SIZE_ADJ);

	if(inx % 2 == 0)	//偶数，写入当前数据和下一个校准数据
	{
		EEPROM_SectorErase(eeAdd);
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx],EE_SIZE_ADJ);  //写入第一份数据
		eeAdd = eeAdd + EE_SIZE_ADJ;
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx],EE_SIZE_ADJ);  //写入第二份数据

		eeAdd = eeAdd + EE_SIZE_ADJ;
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx + 1],EE_SIZE_ADJ);  //写入第一份数据
		eeAdd = eeAdd + EE_SIZE_ADJ;
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx + 1],EE_SIZE_ADJ);  //写入第二份数据
	}
	else		   //偶数，写入上一个校准数据和当前数据
	{
		eeAdd = eeAdd - 0x100;
		EEPROM_SectorErase(eeAdd);
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx - 1],EE_SIZE_ADJ);  //写入第一份数据
		eeAdd = eeAdd + EE_SIZE_ADJ;
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx - 1],EE_SIZE_ADJ);  //写入第二份数据

		eeAdd = eeAdd + EE_SIZE_ADJ;
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx],EE_SIZE_ADJ);  //写入第一份数据
		eeAdd = eeAdd + EE_SIZE_ADJ;
		EEPROM_WriteBytes(eeAdd,(u8 *)&AdjustData[inx],EE_SIZE_ADJ);  //写入第二份数据
	}
}

//从EEPROM中加载校准数据
void LoadAdjustData(void)
{
#ifdef	DEBUG_ADJUST
	char i = 0;
	for(i = 0;i < 20;i++)
	{
	 	AdjustData[0].Point[i].RealVal = 0x01 + i;
		AdjustData[0].Point[i].ADVal = 0x11 + i;
		AdjustData[1].Point[i].RealVal = 0x02 + i;
		AdjustData[1].Point[i].ADVal = 0x22 + i;
		AdjustData[2].Point[i].RealVal = 0x03 + i;
		AdjustData[2].Point[i].ADVal = 0x33 + i;
	}
#else
	char i = 0;
	unsigned int eeAdd = 0;
	for(i = 0;i < enumAdjustNum;i++)
	{
		eeAdd = EE_ADD_InPower + i * 0x100;
		EEPROM_ReadBytes(eeAdd,  (u8 *)&AdjustData[i],EE_SIZE_ADJ);
		if(CheckArray((u8 *)&AdjustData[i],EE_SIZE_ADJ))
		{
			AdjustManage.IsAdjust[i] = 1;
		}
		else  //校验不过则读取备份数据
		{
		 	EEPROM_ReadBytes(eeAdd + EE_SIZE_ADJ,  (u8 *)&AdjustData[i],EE_SIZE_ADJ);
			if(CheckArray((u8 *)&AdjustData[i],EE_SIZE_ADJ))
			{
				AdjustManage.IsAdjust[i] = 2;
			}
			else
			{
				AdjustManage.IsAdjust[i] = 0;
			}
		}
	}
#endif

}
//0~200分配到enum_seg0_20_POWER~enum_seg180_200_POWER
unsigned char GetOutputPowerIndex(float freq)
{
	unsigned char index = 0;
    if (freq < 0.0f) {
        return enum_seg0_20_POWER;
    }
    if (freq >= 200.0f) {
        return enum_seg180_200_POWER;
    }
    // 每20MHz一个段
    index = (unsigned char)(freq / 20.0f);
    return index;
}

//计算校准结果
float GetAdjustResult(enumAdjustIndex inx,unsigned int input,char *err)
{
	char i = 0;
	char findInx = 0;
	int  arrayLen = 0;
	int  pointNum = 0;
	float k = 0,b = 0;
	if(inx >= enumInputPower && inx <= enumInputPower3)
	{
		inx = enumInputPower;//固定只校准一个频段
	}
	else if(inx >= enum75MHzPower && inx <= enum120MHzPower)
	{
		inx = enum75MHzPower;//固定只校准一个频段
	}
	if(inx >= enumAdjustNum)
	{
		*err = 1;
	 	return 0;
	}

	//没有校准记录
	if(!AdjustManage.IsAdjust[inx])
	{
		*err = 2;
	 	return 0;
	}

	arrayLen = sizeof(AdjustData[0].Point) / sizeof(AdjustData[0].Point[0]);
	pointNum = arrayLen;
	//统计有多少个校准点(0xFFFF表示没有校准)
	for(i = 0;i < arrayLen;i++)
	{
		if(AdjustData[inx].Point[i].ValX == 0xFFFF)
		{
			pointNum = i;
			break;
		}
	}
	
	//校准点小于2，计算不了
	if(pointNum < 2)
	{
		*err = 3;
	 	return 0;
	}
	
	//如果小于最小的一个校准点，按正比处理
	if(input < AdjustData[inx].Point[0].ValX)
	{
		float k = (float)AdjustData[inx].Point[0].ValY / AdjustData[inx].Point[0].ValX;
		*err = 0;
		return (k * input);
	}	

	//找到区间
	for(i = 0;i < pointNum;i++)
	{
	 	if(AdjustData[inx].Point[i].ValX > input)
		{
			findInx = i;
			break;
		}
	}

	//如果大于最大一个校准点，按最后两个校准点直线校准
	if(findInx == 0)
	{
		findInx = pointNum - 1;
	}

	//区间为(findInx - 1)和(findInx)之间
	{
		//y1 = k * x1 + b;
		//y2 = k * x2 + b;
	 	unsigned int x1 = AdjustData[inx].Point[findInx - 1].ValX;
		unsigned int y1 = AdjustData[inx].Point[findInx - 1].ValY; 
		unsigned int x2 = AdjustData[inx].Point[findInx].ValX;
		unsigned int y2 = AdjustData[inx].Point[findInx].ValY;
		k = (float)(y2 - y1)/(x2 - x1);
		b = y1 - k * x1;
	}

	*err = 0;
	return (k * input + b);
}


