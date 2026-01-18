#include <ioCC2530.h>

// 硬件端口定义
#define LED_RED     P1_0
#define LED_GREEN   P1_1
#define LED_YELLOW  P1_4
#define SW1         P0_1

// 提示信息
char qdtx1[] = "**交通灯开启\n";
char qdtx2[] = "检测指示灯正常\n进入正常通行管理\n控制方式：上位机（r-正常通行，f-限行，w-通行）\n系统本地（单击按键SW1在正常通行、限行、通行间切换）";
char zlErr[] = "输入指令错\n";
char jrzc[]  = "进入正常模式\n";
char jrxx[]  = "进入限行模式\n";
char jrtx[]  = "进入通行模式\n";
char success[] = "指令执行成功\n";

// 系统状态定义
#define NORMAL_MODE 0
#define FORBID_MODE 1
#define GREEN_MODE  2  // 新增通行模式

// 全局变量
char oldzt = NORMAL_MODE, newzt = NORMAL_MODE;
unsigned int count_t1_IT = 0;
unsigned char temp, RX_flag;
unsigned char blink_state = 0; // 用于绿灯闪烁状态

// 延时函数
void delay(int n) {
    for(int k=0; k<n; k++)
        for(int j=0; j<400; j++);
}

// LED测试函数
void testLed() {
    for (int i=0; i<5; i++) {      
        LED_RED = 0; delay(400); LED_RED = 1; delay(400);
        LED_GREEN = 0; delay(400); LED_GREEN = 1; delay(400);
        LED_YELLOW = 0; delay(400); LED_YELLOW = 1; delay(400);
    }    
}

// 限行模式
void forbid() {
    LED_RED = 0;    // 红灯亮
    LED_GREEN = 1;  // 绿灯灭
    LED_YELLOW = 1; // 黄灯灭
}

// 通行模式（新增）
void green_pass() {
    LED_RED = 1;    // 红灯灭
    LED_GREEN = 0;  // 绿灯亮
    LED_YELLOW = 1; // 黄灯灭
}

// 红灯闪烁函数（用于模式切换提示）
void red_blink() {
    // 先关闭所有灯
    LED_RED = 1;
    LED_GREEN = 1;
    LED_YELLOW = 1;
    
    for(int i=0; i<2; i++) {
        LED_RED = 0; delay(500); // 红灯亮0.5秒
        LED_RED = 1; delay(500); // 红灯灭0.5秒
    }
}

// 绿灯闪烁函数（用于模式切换提示）
void green_blink() {
    // 先关闭所有灯
    LED_RED = 1;
    LED_GREEN = 1;
    LED_YELLOW = 1;
    
    for(int i=0; i<2; i++) {
        LED_GREEN = 0; delay(500); // 绿灯亮0.5秒
        LED_GREEN = 1; delay(500); // 绿灯灭0.5秒
    }
}

// GPIO初始化
void initial_gpio() {
    P1SEL &= ~0x13;        // P1.0, P1.1, P1.4设为GPIO
    P1DIR |= 0x13;         // 设置为输出
    P0DIR &= ~0x02;        // P0.1设为输入
    P0INP &= ~0x02;        // 启用上拉
    P2INP |= 0x20;         // P0口使用上拉
    
    // 初始化所有灯灭
    LED_RED = 1;
    LED_GREEN = 1;
    LED_YELLOW = 1;
}

// 定时器1控制
void start_Timer1() {
    count_t1_IT = 0;
    blink_state = 0;
    T1OVFIM = 1;  // 使能中断
    T1IE = 1;
}

void stop_Timer1() {
    T1OVFIM = 0;  // 禁用中断
    T1IE = 0;
}

void Init_Timer1() {
    T1CC0L = 0x40;   // 计数值低8位
    T1CC0H = 0x9C;   // 计数值高8位
    T1CCTL0 |= 0x04; // 开启比较模式
    T1CTL = 0x06;    // 分频系数8,模模式
}

// 定时器中断服务
#pragma vector = T1_VECTOR
__interrupt void Timer1_Sevice() {
    T1STAT &= ~0x01;  // 清除中断标志
    IRCON = 0;
    
    count_t1_IT++;
    
    // 正常模式下的交通灯控制
    if(newzt == NORMAL_MODE) {
        // 红灯亮1秒 (100个计数单位)
        if(count_t1_IT <= 100) {
            LED_RED = 0;       // 红灯亮
            LED_GREEN = 1;     // 绿灯灭
            LED_YELLOW = 1;    // 黄灯灭
        }
        // 绿灯亮1秒
        else if(count_t1_IT <= 200) {
            LED_RED = 1;       // 红灯灭
            LED_GREEN = 0;     // 绿灯亮
            LED_YELLOW = 1;    // 黄灯灭
        }
        // 绿灯闪烁阶段（200-400计数单位）
        else if(count_t1_IT <= 400) {
            // 每50个计数单位切换一次状态
            if(count_t1_IT % 50 == 0) {
                blink_state = (blink_state + 1) % 4;
                LED_GREEN = (blink_state == 1 || blink_state == 3) ? 0 : 1;
            }
            LED_RED = 1;       // 红灯灭
            LED_YELLOW = 1;    // 黄灯灭
        }
        // 黄灯亮1秒
        else if(count_t1_IT <= 500) {
            LED_RED = 1;       // 红灯灭
            LED_GREEN = 1;     // 绿灯灭
            LED_YELLOW = 0;    // 黄灯亮
        }
        // 循环重置
        else {
            count_t1_IT = 0;
            blink_state = 0;
        }
    }
}

// 串口通信
void initial_usart() {
    // 1. 选择UART0的备用位置
    PERCFG = (PERCFG & ~0x01) | 0x00; // UART0使用位置1（P0.2和P0.3）
    
    // 2. 配置引脚为外设功能
    P0SEL |= 0x0C; // P0.2(RX)和P0.3(TX)设为外设功能
    
    // 3. 设置波特率
    U0BAUD = 59;   // 19200 bps
    U0GCR = 9;     // BAUD_E值
    
    // 4. 配置UART控制寄存器
    U0UCR = 0x80;  // 清除缓冲器，8位数据，无流控
    U0CSR = 0xC0;  // UART模式，接收使能
    
    // 5. 使能中断
    URX0IE = 1;    // 使能UART0接收中断
}

// 串口发送字符
void usart_tx_char(char data_tx) {
    U0DBUF = data_tx;            // 写入发送缓冲区
    while(!(U0CSR & 0x02));      // 等待发送完成（检查TX_BYTE位）
    U0CSR &= ~0x02;              // 清除TX_BYTE标志
}

// 串口发送字符串
void usart_tx_string(char *data_tx) {
    while(*data_tx) {
        usart_tx_char(*data_tx++);
    }
}

// 串口中断服务
#pragma vector=URX0_VECTOR
__interrupt void UART0_ISR(void) {
    URX0IF = 0; // 清除中断标志
    
    // 读取接收到的数据
    temp = U0DBUF;
    
    // 回显接收到的字符（调试用）
    usart_tx_char(temp);
    
    if(temp != 'r' && temp != 'f' && temp != 'w') {                  
        usart_tx_string(zlErr);
        return; 
    }
    
    oldzt = newzt;
    
    if(temp == 'r') {
        newzt = NORMAL_MODE;
    } else if(temp == 'f') {
        newzt = FORBID_MODE;
    } else if(temp == 'w') {
        newzt = GREEN_MODE;
    }
    
    if(oldzt != newzt) {
        // 根据旧模式执行相应的闪烁提示
        if(oldzt == FORBID_MODE) {
            red_blink(); // 从限行模式切换出来，红灯闪烁两次
        } else if(oldzt == GREEN_MODE) {
            green_blink(); // 从通行模式切换出来，绿灯闪烁两次
        }
        
        // 停止定时器（如果旧模式是正常模式）
        if(oldzt == NORMAL_MODE) {
            stop_Timer1();
        }
        
        if(newzt == NORMAL_MODE) {
            usart_tx_string(jrzc);
            start_Timer1();
        } else if(newzt == FORBID_MODE) {
            usart_tx_string(jrxx);
            forbid();
        } else if(newzt == GREEN_MODE) {
            usart_tx_string(jrtx);
            green_pass();
        }
        usart_tx_string(success); // 发送成功提示
    }
}

// 按键中断
void initial_P0_interrupt() {   
    IEN1 |= 0x20;    // 使能P0中断
    P0IEN |= 0x02;   // 使能P0.1中断
    PICTL |= 0x01;   // 下降沿触发  
}

#pragma vector = P0INT_VECTOR
__interrupt void P0_ISR(void) { 
    if(P0IFG & 0x02) {    
        oldzt = newzt;
        
        // 循环切换模式：正常→限行→通行→正常...
        if(newzt == NORMAL_MODE) {
            newzt = FORBID_MODE;
        } else if(newzt == FORBID_MODE) {
            newzt = GREEN_MODE;
        } else {
            newzt = NORMAL_MODE;
        }
        
        if(oldzt != newzt) {
            // 根据旧模式执行相应的闪烁提示
            if(oldzt == FORBID_MODE) {
                red_blink(); // 从限行模式切换出来，红灯闪烁两次
            } else if(oldzt == GREEN_MODE) {
                green_blink(); // 从通行模式切换出来，绿灯闪烁两次
            }
            
            // 停止定时器（如果旧模式是正常模式）
            if(oldzt == NORMAL_MODE) {
                stop_Timer1();
            }
            
            if(newzt == NORMAL_MODE) {
                usart_tx_string(jrzc);
                start_Timer1();
            } else if(newzt == FORBID_MODE) {
                usart_tx_string(jrxx);
                forbid();
            } else if(newzt == GREEN_MODE) {
                usart_tx_string(jrtx);
                green_pass();
            }
            usart_tx_string(success); // 发送成功提示
        }
    }
    P0IFG = 0;  // 清除中断标志
    P0IF = 0;
}

// 中断优先级设置
void InterruptEn() {
    IP1 = 0x14;  // 中断优先级设置
    IP0 = 0x12;
    EA = 1;      // 使能总中断
}

// 主函数
void main() {
    // 系统时钟设置
    CLKCONCMD &= ~0x7F;     // 32MHz晶振
    while(CLKCONSTA & 0x40); // 等待稳定
    CLKCONCMD &= ~0x47;
    
    // 外设初始化
    initial_gpio();
    Init_Timer1();
    initial_usart();
    initial_P0_interrupt();
    InterruptEn();
    
    // 发送欢迎信息
    usart_tx_string(qdtx1);
    usart_tx_string(qdtx2);
    
    // 启动交通灯（正常模式）
    start_Timer1();
    newzt = NORMAL_MODE;
    
    while(1);  // 主循环
}