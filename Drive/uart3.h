#ifndef UART3_H
#define UART3_H

#include "config.h"
#include "ioreg.h"
#include "timer.h"
#include "typedef.h"
#include "drive.h"

#define SELECT_P0	  //Rx:P0.0,Tx:P0.1
// #define SELECT_P5	  //Rx:P5.0,Tx:P5.1

typedef struct UART3Mng
{
    InVoid_OutVoid funSendOneDataOk;
    InU8_OutVoid funRecOneData;
}uart3_mng_struct;


//����3�Ĵ���
sfr S3CON = 0xAC;
sfr S3BUF = 0xAD;


void UART3_SendOneData(unsigned char dataIn);
void UART3_Config(unsigned long baudRate);
void Uart3RegresiterCallback(InVoid_OutVoid fSend,InU8_OutVoid fRec);
#endif
