#include "drive.h"
#include "measure.h"

//DAC最大值4095，对应最大功率400W
#define		MAX_VAL_DAC		4095	
#define		MAX_VAL_POWER	200

void  WriteDAC(unsigned int dacData);
unsigned int GetOutputDAC(void);
void OutputPower(float power);
void DAC7311Tick(void);


