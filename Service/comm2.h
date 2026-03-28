#ifndef COMM2_H
#define COMM2_H

#include "typedef.h"
#include "mem.h"

#define     MAX_LEN_SEND_COMM2    100
#define     MAX_LEN_REC_COMM2     100
#define		REC_FRAME_DELAY		10		//接收数据后，多久后没数据认为接收完成


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


extern void Comm2Init(void);
extern void Comm2Tick(void);
extern void Comm2SendOneDataOK(void);
extern void Comm2RecOneData(unsigned char recData);
extern unsigned char Comm2SendData(unsigned char *dataIn,unsigned char dataLen);
extern unsigned char Comm2GetRecData(unsigned char *dataIn,unsigned char *dataLen);

#endif
