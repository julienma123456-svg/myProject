#include "mem.h"


//ÄÚ´æ¸´ÖÆ
void MemCopy(unsigned char *goal,unsigned char *source,int dataLen)
{
    int i = 0;
    for(i = 0;i < dataLen;i++)
    {
        *goal = *source;
        goal++;
        source++;
    }
}



