#ifndef DRIVE_H
#define DRIVE_H

#include "timer0.h"
#include "uart3.h"
#include "io.h"
#include "adc.h"
#include "uart1.h"
#include "watchdog.h"
#include "eeprom.h"

sbit EA   = 0xAF;

sfr IE    = 0xA8;
sfr IE2   = 0xAF;
sfr P_SW2 = 0xBA;

/*  IE   */
//sbit EA   = 0xAF;
sbit ES   = 0xAC;
sbit ET1  = 0xAB;
sbit EX1  = 0xAA;
//sbit ET0  = 0xA9;
sbit EX0  = 0xA8;

void EnableInt(void);
//void DisableInt(void);
void DriveInit(void);

void EnableXdata(void);
void DisableXdata(void);

#endif
