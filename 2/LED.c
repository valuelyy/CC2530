#include "led.h"


void Led_Init(void)
{
  P1SEL &= ~0x13;
  P1DIR |= 0x13;
  LED1 = 0;
  LED2 = 0;
  LED3 = 0;
}