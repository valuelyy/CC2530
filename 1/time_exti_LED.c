#include <ioCC2530.h>

#include "./hardware/led.h"
#include "./hardware/key.h"
#include "./hardware/delay.h"

#define RED_LED   	 	LED1    		  // red
#define YELLOW_LED 		LED2  		          // yellow
#define GREEN_LED  		LED3   		          // bule


volatile unsigned int timer_count = 0;
volatile unsigned char traffic_state = 0;

void Timer1_Init(void)
 {
    T1CTL = 0x05;  
    T1CC0L = 0xA8; 
    T1CC0H = 0x61;
    T1CCTL0 = 0x04; 
    IEN1 |= 0x02;
    EA = 1;
    T1CTL |= 0x08; 
}

#pragma vector = T1INT_VECTOR
__interrupt void Timer1_Interrupt_Service(void)
 {
    if(T1IFG & 0x01)
    {
        timer_count++;  
        
        switch(traffic_state)
	{
            case 0:  
                if(timer_count >= 300)
		{
                    timer_count = 0;
                    traffic_state = 1;
                    RED_LED = 0;
                    GREEN_LED = 1;
                }break;
            case 1: 
                if(timer_count >= 300)
		{
                    timer_count = 0;
                    traffic_state = 2;
                    GREEN_LED = 0;
                    YELLOW_LED = 1;
                }break;
            
            case 2: 
                if(timer_count >= 20)
		{
                    timer_count = 0;
                    traffic_state = 0;
                    YELLOW_LED = 0;
                    RED_LED = 1;
                }break;
            
            default:
                traffic_state = 0;
                timer_count = 0;
                RED_LED = 1;
                YELLOW_LED = 0;
                GREEN_LED = 0;
                break;
        }
        T1IFG &= ~0x01;
    }
}


void main(void)
 {
    GPIO_Init();
    Timer1_Init();
    while(1) ;
}