#include "delay.h"

void Delay_ms(int k) 
{
    unsigned int i, j;
    for(i = k; i > 0; i--)
        for(j = 1100; j > 0; j--);
}
