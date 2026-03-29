#ifndef COMM2_H
#define COMM2_H

#include "typedef.h"
#include "mem.h"
#include <stdio.h>
#define     MAX_LEN_SEND_COMM2    100
#define     MAX_LEN_REC_COMM2     100
#define		REC_FRAME_DELAY		10		


typedef struct tagComm2Struct
{
    enumCommState ComState;
    unsigned char SendInx;
    unsigned char SendNum;
    unsigned char SendArray[MAX_LEN_SEND_COMM2];
    unsigned char RecInx;
    unsigned char RecCount;
    unsigned char RecArray[MAX_LEN_REC_COMM2];
	unsigned char IdleCount;
}Comm2StructType;


void Comm2Init(void);
void Comm2Tick(void);
void Comm2SendOneDataOK(void);
void Comm2RecOneData(unsigned char recData);
unsigned char Comm2SendData(unsigned char *dataIn,unsigned char dataLen);
unsigned char Comm2GetRecData(unsigned char *dataIn,unsigned char *dataLen);
#endif
