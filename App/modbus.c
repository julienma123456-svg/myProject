#include "modbus.h"

/*	0x03功能码示例--------------------------------------------------------------
[设备地址][03][起始地址Hi][起始地址Lo][寄存器数量Hi][寄存器数量Lo][CRC]
01 03 00 00 00 02 C4 0B

[设备地址][功能码][字节计数][数据1Hi][数据1Lo]...[数据NHi][数据NLo][CRC]
01 03 04 00 0A 00 64 45 87
*/

/* 0x10功能码示例------------------------------------------------------------
[设备地址][10][起始地址Hi][起始地址Lo][寄存器数量Hi][寄存器数量Lo][字节计数][数据1Hi][数据1Lo]...[CRC]
01 10 00 00 00 02 04 00 0A 00 64 8B 16

[设备地址][功能码][起始地址Hi][起始地址Lo][寄存器数量Hi][寄存器数量Lo][CRC]
01 10 00 00 00 02 01 09
*/

/*********************本功能0x03命令实例*****************************************/
//Tx：01 03 00 64 00 00 04 15
//Rx: 01 03 0C 00 01 06 CA 00 56 00 4C 00 4D 00 3E 6F 3A  //运行信息
//Tx: 01 03 00 C8 00 00 C4 34
//Rx: 01 03 02 01 00 B9 D4								  //软件版本

/*********************本功能0x10命令实例*****************************************/
//Tx: 01 10 01 2C 00 01 02 FF FF B0 8C

static unsigned char recArray[256] = {0};
static unsigned char sendArray[256] = {0};

static void ModBusDecode(unsigned char *mbdata,unsigned char dataLen);
//读取命令
static void PushSwAdjustInfo(unsigned char *outData,unsigned char *dataLen);
static void PushRunInfo(unsigned char *outData,unsigned char *dataLen);
static void PushADValInfo(unsigned char *outData,unsigned char *dataLen);
static void ModBusReadAdjustPoint(u8 *outData,u8 *dataLen,u8 adjInx);
//设置和控制命令
static void CtrlOutputPower(u8 *inData,u8 *outData,unsigned char *dataLen);
static void CtrlOutputPowerDAC(u8 *inData,u8 *outData,unsigned char *dataLen);
static void StartAdjust(u8 *outData,unsigned char *dataLen,unsigned int regAdd);
static void AddAdjustPointInput(u8 *inData,u8 *outData,u8 *dataLen,u8 adjInx);
static void AddAdjustPointOutput(u8 *inData,u8 *outData,u8 *dataLen,u8 adjInx);
static void StartReadAdjust(u8 *outData,unsigned char *dataLen,unsigned int regAdd);
static void ModBusSaveAdjustData(u8 *inData,u8 *outData,u8 *dataLen,u8 AdjInx);
static void ModBusCtrlOutPower(u8 *inData,u8 *outData,u8 *dataLen,u8 ctrInx);
static void ReadEEPROMData(u8 *inData,u8 *outData,unsigned char *dataLen);
static void OutputPowerEn(u8 *inData,u8 *outData,unsigned char *dataLen);


/****************modbus信号列表***************************/
//温度保护状态(bool)
//50V电压值(u16 * 100)
//入射功率(u16 * 100)
//反射功率(u16 * 100)
//2A电流(u16 * 1000)
//17A电流(u16 * 1000)
//软件版本(byte.byte)
//DAC输出控制(u16 * 100)

#ifdef DEBUG_COM1
void ModBusService(unsigned char* pdta, unsigned char dataLen)
{
	if(pdta == 0 || dataLen == 0)
		retutn;
	Comm1SendData(pdta,dataLen);
}
#else
//main函数调用的服务函数
void ModBusService(unsigned char* pdta, unsigned char dataLen)
{
	if(pdta == 0 || dataLen == 0)
		retutn;
	PrintfArray(pdta,dataLen);
	if(CheckArray(pdta,dataLen))			//CRC校验通过
	{
		//地址符合，长度大于等于6
		if((pdta[0] == SELF_ADDRESS) && (dataLen >= 6))		
		{
			ModBusDecode(pdta,dataLen);	//数据解析加应答
		}		 	
	}
}
#endif

//协议解析(简易ModBus协议：对于读数据只看寄存器地址不看数量，对于写数据只能写一个寄存器)
void ModBusDecode(unsigned char *mbdata,unsigned char dataLen)
{
	unsigned char cmd = 0;
	unsigned char sendLen = 0;
	unsigned int regAdd = 0x00;

	cmd = mbdata[1];
	dataLen = dataLen;
	regAdd = mbdata[2] << 8 | mbdata[3];

	if(cmd == CMD_READ_VAL)	   //读取数据命令
	{
		if(regAdd == REG_ReadInfo_SwAdjust)
		{
			PushSwAdjustInfo(sendArray,&sendLen);
		}
		else if(regAdd == REG_ReadInfo_Run)
		{
			PushRunInfo(sendArray,&sendLen);
		}
		else if(regAdd == REG_ReadInfo_AD)
		{
			PushADValInfo(sendArray,&sendLen);		
		}
		//读取一个校准数据点
		else if((regAdd >= REG_GetOneAdj_Min) && (regAdd <= REG_GetOneAdj_Max))
		{
			char adjInx = regAdd - REG_GetOneAdj_Min;	
			ModBusReadAdjustPoint(sendArray,&sendLen,adjInx);
		}
		else if(regAdd == REG_ReadEEData)
		{
			ReadEEPROMData(mbdata,sendArray,&sendLen);
		}	
	}
	else if(cmd == CMD_WRITE_VAL)	  //写数据命令
	{
		if(regAdd == REG_Ctrl_OutPower)  		//控制命令寄存器
		{
			if(dataLen == 11)
			{
				CtrlOutputPower(mbdata,sendArray,&sendLen);
			}
		}
		//开始校准
		else if((regAdd >= REG_StartAdj_Min) && (regAdd <= REG_StartAdj_Max))
		{
			char adjInx = 0;
			adjInx = regAdd - REG_StartAdj_Min;
		 	StartAdjust(sendArray,&sendLen,regAdd);
			AdjustStart(enumInputPower + adjInx);
		}
		//开始读取校准数据
		else if((regAdd >= REG_ReadAdj_Min) && (regAdd <= REG_ReadAdj_Max))
		{
			char adjInx = regAdd - REG_ReadAdj_Min;
		 	StartReadAdjust(sendArray,&sendLen,regAdd);
			AdjustStartRead(enumInputPower + adjInx);
		}
		//添加校准数据
		else if((regAdd >= REG_AddAdj_Min) && (regAdd <= REG_AddAdj_RefPower))
		{
			char adjInx = 0;
			if(dataLen == 11)
			{
				adjInx = regAdd - REG_AddAdj_Min;
				AddAdjustPointInput(mbdata,sendArray,&sendLen,adjInx);
			}
		}
		else if((regAdd >= REG_AddAdj_75MHzPower) && (regAdd <= REG_AddAdj_Max))
		{
			char adjInx = 0;
			if(dataLen == 11)
			{
				adjInx = regAdd - REG_AddAdj_75MHzPower;
				AddAdjustPointOutput(mbdata,sendArray,&sendLen,adjInx);
			}
		}
		//保存校准数据
		else if((regAdd >= REG_SaveAdj_Min) && (regAdd <= REG_SaveAdj_Max))
		{
			char adjInx = 0;
			if(dataLen == 17)
			{
				adjInx = regAdd - REG_SaveAdj_Min;
				ModBusSaveAdjustData(mbdata,sendArray,&sendLen,adjInx);
			}
		}
		//功率控制电压DAC输出
		else if(regAdd == REG_Ctrl_OutPowerDAC)
		{
			if(dataLen == 11)
			{
			 	CtrlOutputPowerDAC(mbdata,sendArray,&sendLen);
			}
		}
		//功率校准后输出
		else if((regAdd >= REG_Ctrl_OutPower_Min) && (regAdd <= REG_Ctrl_OutPower_Max))
		{
			char ctrInx = 0;
			if(dataLen == 11)
			{
			 	ctrInx = regAdd - REG_Ctrl_OutPower_Min;
				ModBusCtrlOutPower(mbdata,sendArray,&sendLen,ctrInx);
			}
		}
		//固态源使能控制
		else if(regAdd == REG_OutputPowerEn)
		{
		 	if(dataLen == 11)
			{
			 	OutputPowerEn(mbdata,sendArray,&sendLen);
			}
		}
	}

	//需要应答数据
	if(sendLen > 0)
	{
		sendLen = sendLen + 2;
		ArrayAddCheck(sendArray,sendLen);  //添加CRC校验值
		Comm1SendData(sendArray,sendLen);	//应答数据
	}
}

/*************************03读取数据*************************************************/
//获取软件版本和校准信息
void PushSwAdjustInfo(unsigned char *outData,unsigned char *dataLen)
{
	char i = 0;
	unsigned char inx = 0;
	DateStructType adjDate;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_READ_VAL;
	inx++;	//长度先不放

	//软件版本
	outData[inx++] = SW_VER_HIGH;
	outData[inx++] = SW_VER_LOW;

	for(i = 0;i < enumAdjustNum;i++)
	{
		AdjustGetDate((enumAdjustIndex)i,&adjDate);
		outData[inx++] = 0x00;
		outData[inx++] = AdjustIsFinish((enumAdjustIndex)i);
		outData[inx++] = adjDate.Year - 2000;
		outData[inx++] = adjDate.Month;
		outData[inx++] = adjDate.Day;
		outData[inx++] = adjDate.Hour;
		outData[inx++] = adjDate.Min;
		outData[inx++] = adjDate.Sec;
	}

	outData[2] = inx - 3;
	*dataLen = inx;
}

//运行信息
void PushRunInfo(unsigned char *outData,unsigned char *dataLen)
{
	unsigned char inx = 0;
	float analog = 0.0f;
	unsigned int av = 0;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_READ_VAL;
	inx++;	//长度先不放
	//温度告警
	outData[inx++] = 0;
	outData[inx++] = MeasureGetAlarmFlag();	
	//50V
//	analog = MeasureGetAnalog(Analog_50V);
	av = analog * 1000;
	outData[inx++] = (av >> 8);
	outData[inx++] = (av & 0xFF);
	//17A电流
	analog = MeasureGetAnalog(Analog_17A);
	av = analog * 1000;
	outData[inx++] = (av >> 8);
	outData[inx++] = (av & 0xFF);
	//2A电流
//	analog = MeasureGetAnalog(Analog_2A);
	av = analog * 1000;
	outData[inx++] = (av >> 8);
	outData[inx++] = (av & 0xFF);
	//入射功率
	analog = MeasureGetAnalog(Analog_InPower);
	av = analog * 100;
	outData[inx++] = (av >> 8);
	outData[inx++] = (av & 0xFF);
	//反射功率
	analog = MeasureGetAnalog(Analog_RefPower);
	av = analog * 100;
	outData[inx++] = (av >> 8);
	outData[inx++] = (av & 0xFF);

	outData[2] = inx - 3;
	*dataLen = inx;
}

//获取AD值
void PushADValInfo(unsigned char *outData,unsigned char *dataLen)
{
	unsigned char inx = 0;
	unsigned int adVal = 0;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_READ_VAL;
	inx++;	//长度先不放

	//源电压AD值
//	adVal = MeasureGetAD_Val(Analog_50V);
	outData[inx++] = (adVal >> 8);
	outData[inx++] = (adVal & 0xFF);
	//末级功放电流AD值
	adVal = MeasureGetAD_Val(Analog_17A);
	outData[inx++] = (adVal >> 8);
	outData[inx++] = (adVal & 0xFF);
	//末前级功放电流AD值
//	adVal = MeasureGetAD_Val(Analog_2A);
	outData[inx++] = (adVal >> 8);
	outData[inx++] = (adVal & 0xFF);
	//入射功率AD值
	adVal = MeasureGetAD_Val(Analog_InPower);
	outData[inx++] = (adVal >> 8);
	outData[inx++] = (adVal & 0xFF);
	//反射功率AD值
	adVal = MeasureGetAD_Val(Analog_RefPower);
	outData[inx++] = (adVal >> 8);
	outData[inx++] = (adVal & 0xFF);
	//功率电压输出DAC值
	adVal = GetOutputDAC();
	outData[inx++] = (adVal >> 8);
	outData[inx++] = (adVal & 0xFF);

	outData[2] = inx - 3;
	*dataLen = inx;
}

//读取一个校准点
void ModBusReadAdjustPoint(u8 *outData,u8 *dataLen,u8 adjInx)
{
	unsigned char inx = 0;
	AdjustPointType point;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_READ_VAL;
	inx++;	//长度先不放

	AdjustReadPoint(enumInputPower + adjInx,&point);
	//真实值
	outData[inx++] = (point.ValY >> 8);
	outData[inx++] = (point.ValY & 0xFF);
	//采集值
	outData[inx++] = (point.ValX >> 8);
	outData[inx++] = (point.ValX & 0xFF);

	outData[2] = inx - 3;
	*dataLen = inx;
}

//读取EEPROM数据
void ReadEEPROMData(u8 *inData,u8 *outData,unsigned char *dataLen)
{
	char err = 0;
	unsigned char *pRead = 0;
	unsigned char inx = 0;
	unsigned char readInx = 0;
	unsigned int readAdd = 0;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_READ_VAL;
	inx++;	//长度先不放

	readInx = inData[5];		//读取EEPROM第几个128数据
	if(readInx < 40)  	//最多支持读取5K数据
	{
		readAdd = 128 * readInx;	//计算读取地址
		pRead = outData + 3;
		EEPROM_ReadBytes(readAdd,pRead,128);
		outData[2] = 128;	//返回数据长度为0
		inx += 128;
	}
	else
	{
		outData[2] = 0x00;	//返回数据长度为0
	}

	*dataLen = inx;
}
/*************************03读取数据*************************************************/

/*************************10设置和控制***********************************************/
//输出功率控制命令
void CtrlOutputPower(u8 *inData,u8 *outData,unsigned char *dataLen)
{
	unsigned char inx = 0;
	unsigned int outVol = 0x00;
	float power = 0.0f;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址REG_Ctrl_OutPower
	outData[inx++] = REG_Ctrl_OutPower >> 8;
	outData[inx++] = REG_Ctrl_OutPower & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	outVol = inData[7] << 8 | inData[8];//单位10mW
	power = (float)outVol / 100;
	OutputPower(power);

	*dataLen = inx;
}

//输出功率控制命令
void CtrlOutputPowerDAC(u8 *inData,u8 *outData,unsigned char *dataLen)
{
	unsigned char inx = 0;
	unsigned int outDAC = 0x00;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址REG_Ctrl_OutPowerDAC
	outData[inx++] = REG_Ctrl_OutPowerDAC >> 8;
	outData[inx++] = REG_Ctrl_OutPowerDAC & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	outDAC = inData[7] << 8 | inData[8];	//DAC输出值
	WriteDAC(outDAC);	//不用判断MeasureGetDacEn使能信号

	*dataLen = inx;
}

//开始校准
void StartAdjust(u8 *outData,unsigned char *dataLen,unsigned int regAdd)
{
	unsigned char inx = 0;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址
	outData[inx++] = regAdd >> 8;
	outData[inx++] = regAdd & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	*dataLen = inx;
}

//添加输入采集一个校准点
void AddAdjustPointInput(u8 *inData,u8 *outData,u8 *dataLen,u8 adjInx)
{
	unsigned char inx = 0;
	unsigned int regAdd = 0;
	AdjustPointType point;
	if(adjInx >= 4)
	{
		return;
	}

	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址
	regAdd = REG_AddAdj_InPower1 + adjInx;
	outData[inx++] = regAdd >> 8;
	outData[inx++] = regAdd & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	//ADC计算功率
	point.ValY = inData[7] << 8 | inData[8];//给定值
	if(adjInx < 3) //0 1 2为输入功率
		point.ValX = MeasureGetAD_Val(Analog_InPower);//采集值
	else   //3为反射功率
	{
		point.ValX = MeasureGetAD_Val(Analog_RefPower);	
	}				
	AdjustAddPoint(enumInputPower + adjInx,point);

	*dataLen = inx;
}

//添加功率输出控制一个校准点
void AddAdjustPointOutput(u8 *inData,u8 *outData,u8 *dataLen,u8 adjInx)
{
	unsigned char inx = 0;
	unsigned int regAdd = 0;
	AdjustPointType point;
	if(adjInx >= 10)
	{
		return;
	}
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址
	regAdd = REG_AddAdj_75MHzPower + adjInx;
	outData[inx++] = regAdd >> 8;
	outData[inx++] = regAdd & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	point.ValX = inData[7] << 8 | inData[8];
	point.ValY = GetOutputDAC();
	AdjustAddPoint(enum75MHzPower + adjInx,point);

	*dataLen = inx;
}

//开始读取入射功率校准数据
void StartReadAdjust(u8 *outData,unsigned char *dataLen,unsigned int regAdd)
{
	unsigned char inx = 0;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址
	outData[inx++] = regAdd >> 8;
	outData[inx++] = regAdd & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	*dataLen = inx;
}

//保存校准数据
void ModBusSaveAdjustData(u8 *inData,u8 *outData,u8 *dataLen,u8 AdjInx)
{
	unsigned char inx = 0;
	unsigned int regAdd = 0;
	DateStructType date;

	if(AdjInx >= enumAdjustNum)
	{
		return;
	}

	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址
	regAdd = REG_SaveAdj_Min + AdjInx;
	outData[inx++] = regAdd >> 8;
	outData[inx++] = regAdd & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	date.Year  = inData[8] << 8 | inData[9];
	date.Month = inData[10];
	date.Day   = inData[11];
	date.Hour  = inData[12];
	date.Min   = inData[13];
	date.Sec   = inData[14];

	SaveAdjustData((enumAdjustIndex)AdjInx,date);

	*dataLen = inx;
}

//xxMHz功率输出控制命令(带校准)
void ModBusCtrlOutPower(u8 *inData,u8 *outData,u8 *dataLen,u8 ctrInx)
{
	char err = 0;
	unsigned char inx = 0;
	unsigned int outDAC = 0x00;
	unsigned int power = 0x00;
	unsigned int regAdd = 0x00;

	if(ctrInx >= 10)
	{
	 	return;
	}

	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址
	regAdd = REG_Ctrl_OutPower_Min + ctrInx;
	outData[inx++] = regAdd >> 8;
	outData[inx++] = regAdd & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	power = inData[7] << 8 | inData[8];//单位10mW

	RefreshOutPowerFreq(ctrInx);
	outDAC = GetAdjustResult(enum75MHzPower + ctrInx,power,&err);
	if(!err)
	{
		//有校准数据，校准成功则输出DAC值
		WriteDAC(outDAC);
	}
	else
	{
		//校准失败则输出功率(按功率等比例输出DAC)
		outData[1] += 0x80 + err;
		OutputPower((float)power / 100);
	}

	*dataLen = inx;
}

//固态源使能命令
void OutputPowerEn(u8 *inData,u8 *outData,unsigned char *dataLen)
{
	unsigned char inx = 0;
	outData[inx++] = SELF_ADDRESS;
	outData[inx++] = CMD_WRITE_VAL;

	//寄存器地址REG_OutputPowerEn
	outData[inx++] = REG_OutputPowerEn >> 8;
	outData[inx++] = REG_OutputPowerEn & 0xFF;

	//寄存器数量1
   	outData[inx++] = 0x00;
	outData[inx++] = 0x01;

	if(inData[8] != 0)
	{
		GPIO_OutLow(enumPTT);	   //低电平为使能
	}
	else
	{
		GPIO_OutHigh(enumPTT);
	}

	*dataLen = inx;
}

/*************************10设置和控制***********************************************/




