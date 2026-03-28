#ifndef     ADJUST_H
#define     ADJUST_H

#include "typedef.h"
#include "eeprom.h"
#include "modbusCRC.h"

#ifdef	DEBUG_ENABLE
#define		DEBUG_ADJUST
#endif

#define		EE_SIZE_ADJ			0x80

//#define		DEBUG_ADJUST

//EEPROM地址空间(从第一K开始用，14个校准数据共使用：14*256=3.5K)
#define		EE_ADD_InPower			0x400
#define		EE_ADD_InPower2			0x500
#define		EE_ADD_InPower3			0x600
#define		EE_ADD_RefPower			0x700
#define		EE_ADD_75MPower			0x800	 //每5M一个校准，75MHz为75~80的缩写
#define		EE_ADD_80MPower			0x900
#define		EE_ADD_85MPower			0xA00
#define		EE_ADD_90MPower			0xB00
#define		EE_ADD_95MPower			0xC00
#define		EE_ADD_100MPower		0xD00
#define		EE_ADD_105MPower		0xE00
#define		EE_ADD_110MPower		0xF00
#define		EE_ADD_115MPower		0x1000
#define		EE_ADD_120MPower		0x1100

typedef enum
{
	enumInputPower,			//75~90MHz入射功率校准
	enumInputPower2,	 	//90~110MHz入射功率校准
	enumInputPower3,		//110~125MHz入射功率校准
	enumRefPower,
	enum75MHzPower,			//每5M一个校准，75MHz为75~80的缩写
	enum80MHzPower,
	enum85MHzPower,
	enum90MHzPower,
	enum95MHzPower,
	enum100MHzPower,
	enum105MHzPower,
	enum110MHzPower,
	enum115MHzPower,
	enum120MHzPower,	   	//每5M一个校准，校准范围为75~125MHz
	enumAdjustNum,
}enumAdjustIndex;

typedef struct tagAdjustManage
{
	unsigned char  IsAdjust[enumAdjustNum];
	unsigned char  ReadInx[enumAdjustNum];
	unsigned char  WriteInx[enumAdjustNum];
	DateStructType AdjustDate[enumAdjustNum];
}AdjustManageType;

//校准数据结构体，需要保存至EEPROM，大小为128字节
//(从EEPROM最小擦除单位为512来看，256利用率高，但从RAM考虑不宜定义256这么大)
typedef struct tagAdjustData
{
	unsigned char 	Flag;			//1字节
	DateStructType 	AdjustDate;		//7字节
	unsigned char 	Res1[10];		//10字节
	AdjustPointType Point[20];		//80字节
	unsigned char 	Res2[28];		//28字节
	unsigned int 	CRC16;			//2字节
}AdjustDataType;

unsigned char AdjustIsFinish(enumAdjustIndex index);
unsigned char AdjustGetDate(enumAdjustIndex index,DateStructType *date);

void AdjustStart(enumAdjustIndex index);
void AdjustStartRead(enumAdjustIndex index);
char AdjustAddPoint(enumAdjustIndex index,AdjustPointType point);
char AdjustReadPoint(enumAdjustIndex index,AdjustPointType *point);
void SaveAdjustData(enumAdjustIndex index,DateStructType date);
void LoadAdjustData(void);
float GetAdjustResult(enumAdjustIndex index,unsigned int input,char *err);

#endif
