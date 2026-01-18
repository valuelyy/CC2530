#include "key.h"
#include "ioCC2530.h"

void Key_Init(void)
{
  
  P1DIR &= ~0x04;
  P1SEL &= ~0x04;
  P1INP &= ~0x04;
}

void Key_EXTI_Init(void)
{
  
  Key_Init();
  
  IRCON &= ~0x20;
  IRCON2 &= ~0x01;
  
  P0IEN |= 0x02;//中断屏蔽
  P2IEN |= 0x01;  
  
  IEN1 |= 0x20;
  IEN2 |= 0x02;
  IEN0 |= 0x80;
  
  IRCON |= 0x20;
  IRCON2 |= 0x01;
  
  PICTL &= ~0x02;//触发沿
  P1IFG &= ~0x01;
}


//IEN1    0x20
//IEN2 0x02 端口中断使能

//IRCON 0x20       中断标志
//IRCON2 0x01

//IP1
//IP0         中断优先级