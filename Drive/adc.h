#ifndef ADC_H
#define ADC_H

#include "drive.h"

//采集通道P1.0 P1.1 P1.2 P1.3 P1.4
typedef enum
{
	ADC0 = 0,
	ADC1,
	ADC2,
	ADC3,
	ADC4,
	ADC_NUM,
};


sfr ADC_CONTR = 0xBC;   //带AD系列
sfr ADC_RES	  = 0xBD;   //带AD系列
sfr ADC_RESL  = 0xBE;   //带AD系列
sfr ADCCFG 	  = 0xDE;

#define ADCTIM 		(*(unsigned char volatile xdata *)0xFEA8)
#define ADCEXCFG 	(*(unsigned char volatile xdata *)0xFEAD)

void ADC_Init(void);
unsigned int ADC_GetResult(unsigned char adcCh);


#endif
