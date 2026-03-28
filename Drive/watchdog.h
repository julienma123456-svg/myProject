#include "typedef.h"

sfr WDT_CONTR = 0xC1;

typedef enum
{
	enumReset65ms = 0,
	enumReset131ms = 1,
	enumReset262ms = 2,
	enumReset524ms = 3,
	enumReset1050ms = 4,
	enumReset2100ms = 5,
	enumReset4200ms = 6,
	enumReset8390ms = 7,
}enumWDTResetTime;

void WatchDogInit(enumWDTResetTime resetTime);
void FeedWatchDog(void);

