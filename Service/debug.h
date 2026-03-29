#ifndef DEBUG_H
#define DEBUG_H

#include "typedef.h"
#include "comm2.h"
#include "adc.h"
#include "version.h"

void PrintfArray(unsigned char *dataIn,unsigned char dataLen);
void PrintfAD_Result(void);

#endif // DEBUG_H