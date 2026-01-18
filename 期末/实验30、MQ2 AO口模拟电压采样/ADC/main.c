#include <ioCC2530.h>
#include <string.h>
#include "adc.h"
#include "LCD.h"
#define uint16 unsigned int
#define uint8 unsigned char



void InitUart0(void)
{
    PERCFG = 0x00;           //外设控制寄存器 USART 0的IO位置:0为P0口位置1 
    P0SEL = 0x0c;            //P0_2,P0_3用作串口（外设功能）
    P2DIR &= ~0xC0;          //P0优先作为UART0
    
    U0CSR |= 0x80;           //设置为UART方式
    U0GCR |= 11;				       
    U0BAUD |= 216;           //波特率设为115200
    UTX0IF = 0;              //UART0 TX中断标志初始置位0
    U0CSR |= 0x40;           //允许接收 
    IEN0 |= 0x84;            //开总中断允许接收中断  
}

void Uart0SendString(char *Data, int len)
{
  uint16 i;
  for(i=0; i<len; i++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}

void InitClockTo32M(void)
{   
    CLKCONCMD &= ~0x40;              //设置系统时钟源为 32MHZ晶振
    while(CLKCONSTA & 0x40);         //等待晶振稳定 
    CLKCONCMD &= ~0x47;              //设置系统主时钟频率为 32MHZ
}

uint16 readV(uint8 channal,uint8 resolution)
{
    uint16 value ;

    APCFG |= 1 << channal ; //注意这里是设置ADC输入通道！！
    ADC_ENABLE_CHANNEL(channal);      //使能ADC的采样通道
  
    ADC_SINGLE_CONVERSION(ADC_REF_AVDD | resolution | channal);//片上3.3V参考电压,12位，6通道
    ADC_SAMPLE_SINGLE(); //开始转换
    
    //等待转换完成
    while (0==(ADCCON1 & 0x80));
  
//    ADCCFG &= (0x40 ^ 0xFF);
    
    value = ADCL ;
    value |= ((uint16) ADCH) << 8 ;//这里注意一下
  
    if(resolution == ADC_7_BIT)//7位分辨率
    {
        value >>= 9 ;
    }
    else if(resolution == ADC_9_BIT) //9位分辨率
    {
        value >>= 7 ;
    }
    else if(resolution == ADC_10_BIT) //10位分辨率
    {
        value >>= 6 ;
    }
    else if(resolution == ADC_12_BIT) //12位分辨率
    {
        value >>= 4;
    }

    return value;
}

void main( void )
{
    uint16 adc=0;//adc采样值
    uint16 vol=0; //adc采样电压  
    uint8 buff[10]=0; //adc采样字符串
    
    
    //初始化
    InitClockTo32M();
    InitUart0();
    LCD_Init();
    LCD_P6x8Str(0, 2, "MQ2 AO VOLTAGE");
    
    while(1)
    {
        adc = readV(ADC_AIN6,ADC_12_BIT) ;//通道6, 12位分辨率
        
        //12位的分辨率最大为2048
        if(adc>2048) continue;
        
        //mq2 Ao口输出电压
        vol=(uint16)(((float)adc/2048.0)*3300);
        //vol=(uint16)((float)(adc*3300))/2048.0;
                
        //变成可视的字符输出
        sprintf(buff, "v:%04d mv, adc:%04d", vol,adc);
        
        //串口输出
        Uart0SendString(buff, strlen(buff));
        Uart0SendString("\r\n", 2);

        //lcd 显示
        LCD_P6x8Str(0, 4, buff);

        DelayMS(2000);
    }
}
