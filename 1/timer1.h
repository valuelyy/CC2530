#ifndef TIMER1_H
#define TIMER1_H

#include <ioCC2530.h>

// 定时器1模式定义
typedef enum {
    TIMER1_MODE_FREE_RUNNING = 0x00,  // 自由运行模式
    TIMER1_MODE_MODULO       = 0x01,  // 模模式
    TIMER1_MODE_UP_DOWN      = 0x02,  // 正计数/倒计数模式
    TIMER1_MODE_MODULATED    = 0x03   // 调制模式（用于IR生成）
} Timer1Mode;

// 时钟分频器定义
typedef enum {
    TIMER1_DIV_1   = 0x00,  // 1分频
    TIMER1_DIV_8   = 0x01,  // 8分频
    TIMER1_DIV_32  = 0x02,  // 32分频
    TIMER1_DIV_128 = 0x03   // 128分频
} Timer1Divider;

// 通道模式定义（输入捕获/输出比较）
typedef enum {
    TIMER1_CH_MODE_CAPTURE_RISING  = 0x01,  // 上升沿捕获
    TIMER1_CH_MODE_CAPTURE_FALLING = 0x02,  // 下降沿捕获
    TIMER1_CH_MODE_CAPTURE_ANY     = 0x03,  // 任何边沿捕获
    TIMER1_CH_MODE_COMPARE_SET     = 0x04,  // 比较模式：设置输出
    TIMER1_CH_MODE_COMPARE_CLEAR   = 0x05,  // 比较模式：清除输出
    TIMER1_CH_MODE_COMPARE_TOGGLE  = 0x06   // 比较模式：切换输出
} Timer1ChMode;

// 中断回调函数类型
typedef void (*Timer1Callback)(void);

// 函数声明
void Timer1_Init(Timer1Mode mode, Timer1Divider div);
void Timer1_SetPeriod(uint16_t period);
void Timer1_Channel_Config(uint8_t channel, Timer1ChMode mode, uint16_t compareValue);
void Timer1_EnableInterrupt(uint8_t channel, uint8_t enable);
void Timer1_SetCallback(Timer1Callback ovfCallback, Timer1Callback ch0Callback, 
                       Timer1Callback ch1Callback, Timer1Callback ch2Callback);
void Timer1_Start(void);
void Timer1_Stop(void);
uint16_t Timer1_GetCount(void);
void Timer1_ClearCount(void);

#endif // TIMER1_H