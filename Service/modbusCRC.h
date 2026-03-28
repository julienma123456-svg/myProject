#include "typedef.h"

//选择是否使用查表法快速计算(需要消耗512Byte RAM)
#define		CAL_CRC_FAST

unsigned int ModBus_CalCRC16(const unsigned char *dataIn, unsigned int length);
unsigned char CheckArray(unsigned char *dataIn, unsigned int length);
unsigned char ArrayAddCheck(unsigned char *dataIn, unsigned int length);
