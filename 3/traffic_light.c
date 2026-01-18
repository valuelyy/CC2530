#include "traffic_light.h"

// 全局变量定义
char data[] = "交通灯模式控制系统系统默认运行于正常指挥状态---";
char name_string[20];
char temp;
char RX_flag = 0;
char zt;
char kzfold;
char kzfnew;

// 延时函数
void delay(int n) {
    unsigned int j, k;
    for (k = 0; k < n; k++)
        for (j = 0; j < 500; j++);
}

// GPIO初始化
void initial_gpio(void) {
    P1DIR |= 0x07;      // P1.0、P1.1、P1.2 设置为输出
    RED_LED = 0;
    YELLOW_LED = 0;
    GREEN_LED = 0;
}

// 串口初始化
void initial_usart(void) {
    CLKCONCMD &= ~0x7F;     // 晶振设置为32MHz
    while (CLKCONSTA & 0x40); // 等待晶振稳定
    CLKCONCMD &= ~0x47;      // 0100 0111 -> 1011 1000
    
    PERCFG &= ~0xFF;        // 外设控制
    P0SEL |= 0x3C;          // P0.2、P0.3、P0.4、P0.5 设置为外设功能
    P2DIR &= ~0xC0;         // 0011 1111
    
    U0GCR = 9;              // BAUD_E 波特率19200
    U0BAUD = 59;            // BAUD_M
    U0UCR |= 0x80;          // 禁止流控，8位数据，清除缓冲器
    U0CSR |= 0x80;          // UART模式
    UTX0IF = 0;
    IEN0 = 0x84;            // 使能中断，具有接收能力
}

// 正常交通模式
void Routine(void) {
    // 红灯阶段
    RED_LED = 1;
    GREEN_LED = 0;
    YELLOW_LED = 0;
    delay(1000);           // 红灯亮5秒
    
    // 绿灯阶段
    RED_LED = 0;
    GREEN_LED = 1;
    delay(800);            // 绿灯亮4秒
    
    // 黄灯阶段
    GREEN_LED = 0;
    YELLOW_LED = 1;
    delay(200);            // 黄灯亮1秒
    
    YELLOW_LED = 0;
}

// 限行模式
void Forbid(void) {
    // 红灯常亮，禁止通行
    RED_LED = 1;
    GREEN_LED = 0;
    YELLOW_LED = 0;
}

// 串口发送字符串
void usart_tx_string(char *data_tx, int len) {
    unsigned int j;
    for (j = 0; j < len; j++) {
        U0DBUF = *data_tx++;
        while (UTX0IF == 0);  // 等待发送完成
        UTX0IF = 0;           // 清除中断标志位
    }
    U0CSR |= 0xC0;
}

// 串口发送单个字符
void usart_tx_char(char data_tx) {
    U0DBUF = data_tx;
    while (UTX0IF == 0);      // 等待发送完成
    UTX0IF = 0;               // 清除中断标志位
    U0CSR |= 0xC0;
}

// 命令执行函数
void ExecuteTheOrder(void) {
    if (RX_flag == 1) {
        RX_flag = 0;
        
        switch (temp) {
            case '1': // 切换到正常模式
                kzfnew = 'r';
                usart_tx_string("切换到正常模式\n", 15);
                break;
            case '2': // 切换到限行模式
                kzfnew = 'f';
                usart_tx_string("切换到限行模式\n", 15);
                break;
            case 's': // 状态查询
                if (zt == 'r') {
                    usart_tx_string("当前模式：正常模式\n", 18);
                } else {
                    usart_tx_string("当前模式：限行模式\n", 18);
                }
                break;
            default:
                usart_tx_string("无效指令，请发送1(正常)或2(限行)\n", 30);
                break;
        }
        
        // 模式切换处理
        if (kzfold != kzfnew) {
            kzfold = kzfnew;
            zt = kzfnew;
            
            if (zt == 'r') {
                usart_tx_string("交通灯进入正常指挥状态\n", 22);
            } else {
                usart_tx_string("交通灯进入限行状态\n", 18);
            }
        }
    }
}

// 串口中断服务函数
#pragma vector = URX0_VECTOR
__interrupt void UART0_ISR(void) {
    URX0IF = 0;
    temp = U0DBUF;
    RX_flag = 1;
}