/****************************************************************************
* 文 件 名: main.c
* 描    述: 人进入其感应范围模块输出高电平,点亮LED1，人离开感应范围LED1熄灭，
*           P0.4口为HC-SR501传感器的输入端  
****************************************************************************/
#include <ioCC2530.h>

typedef unsigned char uchar;
typedef unsigned int  uint;

#define LED1     P1_0        //定义P1.0口为LED1控制端
#define DATA_PIN P0_4        //定义P0.4口为传感器的输入端
#define uint8  unsigned char
/****************************************************************************
* 名    称: DelayMS()
* 功    能: 以毫秒为单位延时 16M时约为535,系统时钟不修改默认为16M
* 入口参数: msec 延时参数，值越大，延时越久
* 出口参数: 无
****************************************************************************/
void DelayMS(uint msec)
{ 
    uint i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<535; j++);
}

/****************************************************************************
* 名    称: InitGpio()
* 功    能: 设置LED灯和P0.4相应的IO口
* 入口参数: 无
* 出口参数: 无
****************************************************************************/
void InitLed(void)
{
    P1DIR |= 0x01;           //P1.0定义为输出口
    P0SEL = 0x00;
    P0DIR &= ~0x10;          //P0.4定义为输入口    
    P2INP |= 0x20;
    
    
    P0DIR |= 0x80;     //蜂鸣器引脚　     
}

//检测人体传感器是否有人
//人体传感器接在P04上
//一旦检测到有人，10次检测内都认为有人
//返回1表示有人，0表示无人
uint8 GetPeople()
{
    static uint8 peopleSencond=0;
    uint8 people=0;
    
    if(DATA_PIN==1)
    {
        peopleSencond=10;//一旦检测到有人，10次检测内都认为有人
        people=1;//检测到有人
    }
    else
    {
        if(peopleSencond>0)
        {
            peopleSencond--;
            people=1;//倒计时结束前都认为有人
        }
        else
        {
            peopleSencond=0;
            people=0;//没有人
        }
    }
    
    return people;
}

void main(void)
{      
    InitLed();               //设置LED灯和P0.4相应的IO口
    
    while(1)                 //无限循环
    {
        if(GetPeople() == 1)
        {
            LED1 = 0;    //有人时LED1亮
            P0_7=0;  //有人的时候，蜂鸣器报警,方便验证
        }    	
        else
        {
            LED1=1;          //无人时LED1熄灭
            P0_7=1;          //无人的时候，蜂鸣器不报警,方便验证
        }
        DelayMS(500);
    }
    
}
