#include "timer1.h"
#include <stdint.h>

// 中断回调函数指针
static Timer1Callback ovfCallback = NULL;
static Timer1Callback ch0Callback = NULL;
static Timer1Callback ch1Callback = NULL;
static Timer1Callback ch2Callback = NULL;

// 定时器1初始化
void Timer1_Init(Timer1Mode mode, Timer1Divider div) {
    // 停止定时器
    T1CTL &= ~0x03;
    
    // 设置模式和分频器
    T1CTL = (T1CTL & ~0xF0) | (mode << 4) | (div << 2);
    
    // 清除所有中断标志
    T1STAT = 0;
    TIMIF &= ~0x40; // 清除溢出中断标志
    
    // 禁用所有通道中断
    T1CCTL0 &= ~0x40;
    T1CCTL1 &= ~0x40;
    T1CCTL2 &= ~0x40;
    T1CCTL3 &= ~0x40;
    T1CCTL4 &= ~0x40;
}

// 设置周期值（模模式和正计数/倒计数模式使用）
void Timer1_SetPeriod(uint16_t period) {
    T1CC0H = period >> 8;
    T1CC0L = period & 0xFF;
}

// 配置定时器1通道
void Timer1_Channel_Config(uint8_t channel, Timer1ChMode mode, uint16_t compareValue) {
    volatile uint8_t *cctl;
    volatile uint8_t *cch;
    volatile uint8_t *ccl;
    
    switch (channel) {
        case 0:
            cctl = &T1CCTL0;
            cch = &T1CC0H;
            ccl = &T1CC0L;
            break;
        case 1:
            cctl = &T1CCTL1;
            cch = &T1CC1H;
            ccl = &T1CC1L;
            break;
        case 2:
            cctl = &T1CCTL2;
            cch = &T1CC2H;
            ccl = &T1CC2L;
            break;
        case 3:
            cctl = (volatile uint8_t *)0x62A3; // T1CCTL3
            cch = (volatile uint8_t *)0x62AD;  // T1CC3H
            ccl = (volatile uint8_t *)0x62AC;  // T1CC3L
            break;
        case 4:
            cctl = (volatile uint8_t *)0x62A4; // T1CCTL4
            cch = (volatile uint8_t *)0x62AF;  // T1CC4H
            ccl = (volatile uint8_t *)0x62AE;  // T1CC4L
            break;
        default:
            return;
    }
    
    // 设置比较值
    *ccl = compareValue & 0xFF;
    *cch = compareValue >> 8;
    
    // 设置通道模式
    *cctl = (*cctl & ~0x07) | (mode & 0x07);
}

// 启用/禁用中断
void Timer1_EnableInterrupt(uint8_t channel, uint8_t enable) {
    switch (channel) {
        case 0:
            if (enable) T1CCTL0 |= 0x40;
            else T1CCTL0 &= ~0x40;
            break;
        case 1:
            if (enable) T1CCTL1 |= 0x40;
            else T1CCTL1 &= ~0x40;
            break;
        case 2:
            if (enable) T1CCTL2 |= 0x40;
            else T1CCTL2 &= ~0x40;
            break;
        case 3:
            if (enable) *((volatile uint8_t *)0x62A3) |= 0x40;
            else *((volatile uint8_t *)0x62A3) &= ~0x40;
            break;
        case 4:
            if (enable) *((volatile uint8_t *)0x62A4) |= 0x40;
            else *((volatile uint8_t *)0x62A4) &= ~0x40;
            break;
        case 0xFF: // 溢出中断
            if (enable) TIMIF |= 0x40;
            else TIMIF &= ~0x40;
            break;
    }
}

// 设置回调函数
void Timer1_SetCallback(Timer1Callback ovfCb, Timer1Callback ch0Cb, 
                       Timer1Callback ch1Cb, Timer1Callback ch2Cb) {
    ovfCallback = ovfCb;
    ch0Callback = ch0Cb;
    ch1Callback = ch1Cb;
    ch2Callback = ch2Cb;
}

// 启动定时器
void Timer1_Start(void) {
    T1CTL |= 0x03; // 运行模式
}

// 停止定时器
void Timer1_Stop(void) {
    T1CTL &= ~0x03; // 停止模式
}

// 获取当前计数值
uint16_t Timer1_GetCount(void) {
    uint8_t low, high;
    low = T1CNTL;
    high = T1CNTH;
    return (high << 8) | low;
}

// 清除计数值
void Timer1_ClearCount(void) {
    T1CNTL = 0;
}

// 定时器1中断服务例程
#pragma vector = T1_VECTOR
__interrupt void Timer1_ISR(void) {
    if (T1STAT & 0x01) { // 通道0中断
        T1STAT &= ~0x01;
        if (ch0Callback) ch0Callback();
    }
    if (T1STAT & 0x02) { // 通道1中断
        T1STAT &= ~0x02;
        if (ch1Callback) ch1Callback();
    }
    if (T1STAT & 0x04) { // 通道2中断
        T1STAT &= ~0x04;
        if (ch2Callback) ch2Callback();
    }
    if (T1STAT & 0x20) { // 通道3中断
        T1STAT &= ~0x20;
        // 通道3回调可以在这里添加
    }
    if (T1STAT & 0x40) { // 通道4中断
        T1STAT &= ~0x40;
        // 通道4回调可以在这里添加
    }
    if (TIMIF & 0x40) { // 溢出中断
        TIMIF &= ~0x40;
        if (ovfCallback) ovfCallback();
    }
}