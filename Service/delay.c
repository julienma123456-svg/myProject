#include "delay.h"

void delay_ms(char ms)
{
     int i;
     do{
          i = MAIN_Fosc / 10000;
          while(--i);   //10T per loop
     }while(--ms);
}

//上电闪烁指示
void PowerOnFlash(void)
{
	char i = 0;
	for(i = 0;i < 10;i++)
	{
	 	GPIO_OutHigh(enumLED);
		delay_ms(150);
		GPIO_OutLow(enumLED);
		delay_ms(50);
	}
}
