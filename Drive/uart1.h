#ifndef UART1_H
#define UART1_H

#include "config.h"
#include "ioreg.h"
#include "timer.h"
#include "typedef.h"
#include "drive.h"


sfr SCON  = 0x98;
sfr S1BUF = 0x99;
sfr P_SW1 = 0xA2;

/*  SCON  */
sbit SM0  = 0x9F;
sbit SM1  = 0x9E;
sbit SM2  = 0x9D;
sbit REN  = 0x9C;
sbit TB8  = 0x9B;
sbit RB8  = 0x9A;
sbit TI   = 0x99;
sbit RI   = 0x98;

void UART1_SendOneData(unsigned char dataIn);
void UART1_Config(unsigned long baudRate,InVoid_OutVoid fSend,InU8_OutVoid fRec);

#endif
