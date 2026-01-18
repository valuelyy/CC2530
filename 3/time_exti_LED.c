#include <ioCC2530.h>
#include "timer1.h"
#include "led.h"

// LED引脚定义
#define RED_LED     P1_0    // 红灯
#define YELLOW_LED  P1_1    // 黄灯
#define GREEN_LED   P1_2    // 绿灯

// 全局变量
volatile unsigned int timer_count = 0;    // 定时器计数
volatile unsigned char traffic_state = 0; // 交通灯状态 (0:红灯, 1:绿灯, 2:黄灯)

// 定时器1中断服务函数
#pragma vector = T1_VECTOR
__interrupt void Timer1_ISR(void)
{
    // 检查通道0比较中断
    if(T1CTL & 0x04) 
    {
        // 清除通道0中断标志
        T1CTL &= ~0x04;
        
        // 每10ms增加一次计数
        timer_count++;
        
        // 交通灯状态机
        switch(traffic_state)
        {
            case 0:  // 红灯亮30秒
                if(timer_count >= 300)  // 300 * 10ms = 3秒（测试用）
                {
                    timer_count = 0;
                    traffic_state = 1;
                    RED_LED = 0;     // 红灯灭
                    GREEN_LED = 1;   // 绿灯亮
                }
                break;
                
            case 1:  // 绿灯亮30秒
                if(timer_count >= 300)  // 300 * 10ms = 3秒
                {
                    timer_count = 0;
                    traffic_state = 2;
                    GREEN_LED = 0;   // 绿灯灭
                    YELLOW_LED = 1;  // 黄灯亮
                }
                break;
                
            case 2:  // 黄灯亮3秒
                if(timer_count >= 30)  // 30 * 10ms = 0.3秒
                {
                    timer_count = 0;
                    traffic_state = 0;
                    YELLOW_LED = 0;  // 黄灯灭
                    RED_LED = 1;     // 红灯亮
                }
                break;
                
            default:  // 异常处理
                traffic_state = 0;
                timer_count = 0;
                RED_LED = 1;
                YELLOW_LED = 0;
                GREEN_LED = 0;
                break;
        }
    }
}

// 在main函数中修正定时器配置
void main(void)
{
    // 初始化LED
    Led_Init();
    
    // 初始状态：红灯亮
    RED_LED = 1;
    YELLOW_LED = 0;
    GREEN_LED = 0;
    
    // 初始化定时器1 - 模模式, 128分频
    Timer1_Init(1, 3);  // 模模式(1), 128分频(3)
    
    // 设置定时器周期为10ms 
    // 假设系统时钟32MHz，128分频后为250kHz，周期4us
    // 10ms/4us = 2500
    Timer1_SetPeriod(2500);
    
    // 配置通道0为比较模式，比较值等于周期值
    Timer1_Channel0_Config(4, 2500);  // 比较模式(4)
    
    // 启用通道0中断
    Timer1_EnableInterrupt(0, 1);
    
    // 启用全局中断
    IEN0 |= 0x80;  // EA = 1
    
    // 启动定时器
    Timer1_Start();
    
    // 主循环
    while(1)
    {
        // 进入低功耗模式，等待中断唤醒
        PCON |= 0x01; // 进入空闲模式
    }
}