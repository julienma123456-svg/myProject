#ifndef IO_H
#define IO_H

#include "ioreg.h"
#include "typedef.h"

typedef enum
{
	// ‰≥ˆ
	enum485CTRL,
	enumSCLK,
	enumSYNC,
	enumDIN,
	enumLED,
	enumLED1,
	enumLED2,
	enumLED3,
	enumLED4,
	enumPTT,
	enumFREGSWONFF,
	// ‰»Î
	enumALARM_T,
}enumGPIOName;

void GPIO_Init(void);
void GPIO_OutHigh(enumGPIOName gpioName);
void GPIO_OutLow(enumGPIOName gpioName);
unsigned char GPIO_GetIn(enumGPIOName gpioName);

#endif
