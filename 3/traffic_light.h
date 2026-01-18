#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <ioCC2530.h>

// 宏定义
#define RED_LED P1_0       // 红灯
#define YELLOW_LED P1_1    // 黄灯
#define GREEN_LED P1_2     // 绿灯

// 外部变量声明
extern char data[];
extern char name_string[20];
extern char temp;
extern char RX_flag;
extern char zt;
extern char kzfold;
extern char kzfnew;

// 函数声明
void delay(int n);
void initial_gpio(void);
void initial_usart(void);
void Routine(void);
void Forbid(void);
void usart_tx_string(char *data_tx, int len);
void usart_tx_char(char data_tx);
void ExecuteTheOrder(void);

#endif // TRAFFIC_LIGHT_H