#ifndef TIMER1_H
#define TIMER1_H

#include <ioCC2530.h>

/* 定时器1模式定义 */
#define TIMER1_MODE_FREE_RUNNING 0x00
#define TIMER1_MODE_MODULO       0x01
#define TIMER1_MODE_UP_DOWN      0x02
#define TIMER1_MODE_MODULATED    0x03

/* 时钟分频器定义 */
#define TIMER1_DIV_1   0x00
#define TIMER1_DIV_8   0x01
#define TIMER1_DIV_32  0x02
#define TIMER1_DIV_128 0x03

/* 通道模式定义 */
#define TIMER1_CH_MODE_CAPTURE_RISING  0x01
#define TIMER1_CH_MODE_CAPTURE_FALLING 0x02
#define TIMER1_CH_MODE_CAPTURE_ANY     0x03
#define TIMER1_CH_MODE_COMPARE_SET     0x04
#define TIMER1_CH_MODE_COMPARE_CLEAR   0x05
#define TIMER1_CH_MODE_COMPARE_TOGGLE  0x06

/* 函数声明 */
void Timer1_Init(int mode, int div);
void Timer1_SetPeriod(int period);
void Timer1_Channel0_Config(int mode, int compareValue);
void Timer1_Channel1_Config(int mode, int compareValue);
void Timer1_Channel2_Config(int mode, int compareValue);
void Timer1_EnableInterrupt(int channel, int enable);
void Timer1_Start(void);
void Timer1_Stop(void);
int Timer1_GetCount(void);

#endif