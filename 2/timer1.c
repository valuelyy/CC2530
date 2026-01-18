#include "timer1.h"

/* 定时器1初始化 */
void Timer1_Init(int mode, int div) {
    /* 停止定时器 */
    T1CTL &= 0xFC;
    
    /* 设置模式和分频器 */
    T1CTL = (T1CTL & 0x0F) | ((mode << 4) | (div << 2));
    
    /* 清除中断标志 */
    T1STAT = 0;
    TIMIF &= 0xBF;
}

/* 设置周期值 */
void Timer1_SetPeriod(int period) {
    T1CC0H = (char)(period >> 8);
    T1CC0L = (char)(period & 0xFF);
}

/* 配置通道0 */
void Timer1_Channel0_Config(int mode, int compareValue) {
    T1CC0L = (char)(compareValue & 0xFF);
    T1CC0H = (char)(compareValue >> 8);
    T1CCTL0 = (T1CCTL0 & 0xF8) | (mode & 0x07);
}

/* 配置通道1 */
void Timer1_Channel1_Config(int mode, int compareValue) {
    T1CC1L = (char)(compareValue & 0xFF);
    T1CC1H = (char)(compareValue >> 8);
    T1CCTL1 = (T1CCTL1 & 0xF8) | (mode & 0x07);
}

/* 配置通道2 */
void Timer1_Channel2_Config(int mode, int compareValue) {
    T1CC2L = (char)(compareValue & 0xFF);
    T1CC2H = (char)(compareValue >> 8);
    T1CCTL2 = (T1CCTL2 & 0xF8) | (mode & 0x07);
}

/* 启用/禁用中断 */
void Timer1_EnableInterrupt(int channel, int enable) {
    switch (channel) {
        case 0:
            if (enable) {
                T1CCTL0 |= 0x40;
            } else {
                T1CCTL0 &= 0xBF;
            }
            break;
            
        case 1:
            if (enable) {
                T1CCTL1 |= 0x40;
            } else {
                T1CCTL1 &= 0xBF;
            }
            break;
            
        case 2:
            if (enable) {
                T1CCTL2 |= 0x40;
            } else {
                T1CCTL2 &= 0xBF;
            }
            break;
            
        case 0xFF: /* 溢出中断 */
            if (enable) {
                TIMIF |= 0x40;
            } else {
                TIMIF &= 0xBF;
            }
            break;
            
        default:
            break;
    }
}

/* 启动定时器 */
void Timer1_Start(void) {
    T1CTL |= 0x03;
}

/* 停止定时器 */
void Timer1_Stop(void) {
    T1CTL &= 0xFC;
}

/* 获取当前计数值 */
int Timer1_GetCount(void) {
    int count_value;
    char low_byte, high_byte;
    
    low_byte = T1CNTL;
    high_byte = T1CNTH;
    count_value = ((int)high_byte << 8) | low_byte;
    
    return count_value;
}