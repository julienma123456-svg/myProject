#ifndef COMM1_H
#define COMM1_H

#include "typedef.h"
#include "mem.h"

#define     MAX_LEN_SEND_COMM1    256
#define     MAX_LEN_REC_COMM1     256
#define		REC_FRAME_DELAY		10		//接收数据后，多久后没数据认为接收完成

typedef struct tagComm1Struct
{
    enumCommState ComState;
    unsigned char SendInx;
    unsigned char SendNum;
    unsigned char SendArray[MAX_LEN_SEND_COMM1];
    unsigned char RecInx;
    unsigned char RecCount;
    unsigned char RecArray[MAX_LEN_REC_COMM1];
	unsigned char IdleCount;
}Comm1StructType;


extern void Comm1Init(void);
extern void Comm1Tick(void);
extern void Comm1SendOneDataOK(void);
extern void Comm1RecOneData(unsigned char recData);
extern unsigned char Comm1SendData(unsigned char *dataIn,unsigned char dataLen);
extern unsigned char Comm1GetRecData(unsigned char *dataIn,unsigned char *dataLen);


#endif
