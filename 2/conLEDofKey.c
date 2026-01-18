#include "ioCC2530.h"
#include "led.h"
#include "key.h"
#include "delay.h"

int INT_Flag = 0;

void main(void)
{
  Key_Init();
  Led_Init();

  while(1)
  {
    if(INT_Flag == 1)
    {
      LED1 =~LED1;
      LED2 =~LED2;
      Delay_ms(500);
      INT_Flag = 1;
    }
  }
}

#pragma vector = P0INT_VECTOR
__interrupt void P1_INT_Service(void)
{
     if( P0IFG & 0x02)
    {
       //EA = 0;
       P1IFG &= ~0x02;
    }
    INT_Flag = 1;
}