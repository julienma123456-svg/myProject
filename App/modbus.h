#include "typedef.h"
#include "comm1.h"
#include "modbusCRC.h"
#include "measure.h"
#include "version.h"
#include "dac7311.h"
#include "debug.h"
#include "config.h"
#include "adjust.h"

#define		SELF_ADDRESS		0x01
#define		CMD_READ_VAL		0x03
#define		CMD_WRITE_VAL		0x10

//获取信息
#define		REG_ReadInfo_SwAdjust		0x100	//获取版本和校准信息
#define		REG_ReadInfo_Run			0x200	//获取运行信息
#define		REG_ReadInfo_AD				0x300	//获取AD值信息

//保存校准数据
#define		REG_SaveAdj_Min		0x400	//开始地址
#define		REG_SaveAdj_Max		0x40D  	//结束地址

//输出功率控制(校准)
#define		REG_Ctrl_OutPower_Min		0x440	//开始地址
#define		REG_Ctrl_OutPower_Max		0x449	//结束地址

//开始校准
#define		REG_StartAdj_Min		0x500	//开始地址
#define		REG_StartAdj_Max		0x50D	//结束地址

//保存一个校准点
#define		REG_AddAdj_Min			0x540	//开始地址
#define		REG_AddAdj_InPower1		0x540	
#define		REG_AddAdj_InPower2		0x541	
#define		REG_AddAdj_InPower3		0x542	
#define		REG_AddAdj_RefPower		0x543
#define		REG_AddAdj_75MHzPower	0x544	
#define		REG_AddAdj_Max			0x54D	//结束地址

//开始读取校准
#define		REG_ReadAdj_Min			0x600	//开始地址
#define		REG_ReadAdj_Max			0x60D	//结束地址

//读取一个校准点
#define		REG_GetOneAdj_Min		0x640  	//开始地址
#define		REG_GetOneAdj_Max		0x64D  	//结束地址

//控制和设置
#define		REG_OutputPowerEn			0x700	//固态源输出使能
#define		REG_Ctrl_OutPower			0x701	//控制信号(不校准的控制)
#define		REG_Ctrl_OutPowerDAC		0x702	//功率DAC输出控制

//读取EEPROM数据
#define		REG_ReadEEData				0x1000	//读取EEPROM数据


#ifdef	DEBUG_ENABLE
#define		DEBUG_COM1
#endif

void ModBusService(void);




