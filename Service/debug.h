#ifndef DEBUG_H
#define DEBUG_H

#include "typedef.h"
#include "comm2.h"
#include "adc.h"
#include "version.h"
#include <stdio.h>
#include <string.h>

#define CHANGE_UART_BECAUSEOF_UART3ERR  1
#define MOCK_DATA_FOR_TEST 1


extern char pstring[256];

void PrintfArray(unsigned char *dataIn,unsigned char dataLen);
void PrintfAD_Result(void);
void Trap(void);
#endif // DEBUG_H