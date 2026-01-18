#include <ioCC2530.h>
#define LED_RED (P1_0)     //交通灯端口定义
#define LED_GREEN (P1_1)  
char data[]="交通灯模式控制系统系统默认运行于正常指挥状态---";
char name_string[20];
char kzfold,kzfnew,zt;
unsigned char temp,RX_flag,counter=0;
void delay(int n)
{
  unsigned int j,k;
  for(k=0;k<n;k++)
    for(j=0;j<500;j++);
}
void initial_gpio()
{
  P1SEL &= ~0xFF;         //设置P1_0口和P1_2为通用I/O口
  P1DIR |= 0x03;          //设置P1_0、P1_1口为输出口  
  LED_RED=0;  LED_GREEN=0;  //熄灭 所有指示灯  
  }
void initial_usart()
{
  CLKCONCMD &=~0x7F;    //晶振设置为32MHz
  while(CLKCONSTA&0x40);//等待晶振稳定
  CLKCONCMD &= ~0x47;   //0100 0111  -> 1011 1000
  PERCFG&=~0xFF;
   P0SEL|=0x3C;     //P0 2345     0011 1100
  P2DIR &= ~0xC0;  //0011 1111
 U0GCR = 9;       //BAUD_E  波特率19200
  U0BAUD=59;       //BAUD_M
  U0UCR |= 0x80;    //禁止流控，8位数据，清除缓冲器
   U0CSR|=0x80;      //uart 模式 1100 0000
  //U0CSR|=0xC0;      //uart 模式 1100 0000
  UTX0IF=0;
 IEN0=0x84;       //使能中断 具有接收能力
}
//Routine-定义红绿灯交替点亮，指挥车辆正常通行
void Routine() 
{
     LED_RED=1;  LED_GREEN=0;   
     delay(1500); 
     LED_RED=0;  LED_GREEN=0;   
     delay(400); 
     LED_RED=0;  LED_GREEN=1;   
     delay(1500); 
     LED_RED=0;  LED_GREEN=0;
     delay(400); 
 }
void Forbid()
{
    LED_RED=1;  LED_GREEN=0; 
     delay(1000); 
}
void usart_tx_string(char *data_tx,int len)
{
  unsigned int j;
  for(j=0;j<len;j++)
  {
    U0DBUF = *data_tx++;
    while(UTX0IF==0);  //等待发送完成
    UTX0IF=0;         //清除中断标志位
  }
  U0CSR|=0xC0; 
}
void usart_tx_char(char data_tx)
{
     U0DBUF = data_tx;
    while(UTX0IF==0);  //等待发送完成
    UTX0IF=0;         //清除中断标志位
    U0CSR|=0xC0; 
}
#pragma vector=URX0_VECTOR
__interrupt void UART0_ISR(void)
{
  URX0IF=0;
  temp=U0DBUF;
  RX_flag=1;
}

void main()
{
  //初始化串口
    initial_gpio();
    initial_usart();
    //U0CSR|=0xC0;      //uart 模式 1100 0000  收
  // U0CSR|=0x80;      //uart 模式 1100 0000  发
  Routine(); 
  //Forbid();
  
  U0CSR|=0x80;      
  usart_tx_string(data,sizeof(data)); //输出系统提示

  zt='r';
  kzfold='f';
  kzfnew='r';
  while(1)
  {
    switch (zt)
    {
    case 'r': Routine();break;
    case 'f':Forbid();break;
    }
    if(RX_flag == 1)
    {
      RX_flag=0;
      kzfnew=temp;  
     }
     U0CSR|=0x80;      //uart 模式 1100 0000  发使能
     if (kzfnew!='r' && kzfnew!='f')
      {                  
        usart_tx_string("输入指令错----",sizeof("输入指令错----"));
      }
      else if (kzfnew!=kzfold)
      {
        if (kzfnew =='r')
        {
          usart_tx_string("将切换至正常模式----",sizeof("将切换至正常模式----"));
          kzfold=kzfnew;
          zt='r';
        }
        else if(kzfnew=='f')
        {
          usart_tx_string("将切换至限行模式----",sizeof("将切换至限行模式----"));
          kzfold=kzfnew;
           zt='f';
        }
      }
       U0CSR|=0xC0;      //uart 模式 1100 0000  收使能
    }
}