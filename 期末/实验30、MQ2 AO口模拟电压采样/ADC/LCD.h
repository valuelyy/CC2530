#ifndef _LCD_H_
#define _LCD_H_


#include "codetab.h"

#define XLevelL        0x00
#define XLevelH        0x10
#define XLevel         ((XLevelH&0x0F)*16+XLevelL)
#define Max_Column     128
#define Max_Row        64
#define Brightness     0xCF 
#define X_WIDTH        128
#define Y_WIDTH        64


typedef int            INT;
typedef const char*    LPCSTR;
typedef unsigned char BYTE;
typedef int	           LENGTH;

//英文编码为0~127，所以大于127的中文
#define IS_CHINESE(x)       (((BYTE)(x))>(BYTE)0x7f )

void DelayMS(unsigned int msec)
{ 
    unsigned int i,j;
    
    for (i=0; i<msec; i++)
        for (j=0; j<600; j++);
}

/*********************LCD 延时1ms************************************/
void LCD_DLY_ms(unsigned int ms)
{                         
    unsigned int a;
    while(ms)
    {
        a=1800;
        while(a--);
        ms--;
    }
    return;
}
/*********************LCD写数据************************************/ 
void LCD_WrDat(unsigned char dat)     
{
    unsigned char i=8, temp=0;
    LCD_DC=1;  
    for(i=0;i<8;i++) //发送一个八位数据 
    {
        LCD_SCL=0;  
        
        temp = dat&0x80;
        if (temp == 0)
        {
            LCD_SDA = 0;
        }
        else
        {
            LCD_SDA = 1;
        }
        LCD_SCL=1;             
        dat<<=1;    
    }
}
/*********************LCD写命令************************************/                                        
void LCD_WrCmd(unsigned char cmd)
{
    unsigned char i=8, temp=0;
    LCD_DC=0;
    for(i=0;i<8;i++) //发送一个八位数据 
    { 
        LCD_SCL=0; 
       
        temp = cmd&0x80;
        if (temp == 0)
        {
            LCD_SDA = 0;
        }
        else
        {
            LCD_SDA = 1;
        }
        LCD_SCL=1;
        cmd<<=1;;        
    }     
}
/*********************LCD 设置坐标************************************/
void LCD_Set_Pos(unsigned char x, unsigned char y) 
{ 
    if(x>127) return;
    if(y>7) return;
    
    unsigned char lower_start_column_address = (((x&0xf)));   // 低列地址, 一共占4个位, 一共16个地址
    unsigned char upper_start_column_address = (((x>>4)&0x7));// 高列地址, 一共占3个位, 一共8个地址
    // 一行地址最大值为127,也就是128个点, (upper_start_column_address+1)*(lower_start_column_address+1) 最大值为128
   
    LCD_WrCmd(0xb0+y);
    LCD_WrCmd(lower_start_column_address);              // 低列地址, 一共占4个位, 一共16个地址
    LCD_WrCmd(upper_start_column_address | 0x10);       // 高列地址, 一共占3个位, 一共8个地址
} 
/*********************LCD全屏************************************/
void LCD_Fill(unsigned char bmp_dat) 
{
    unsigned char y,x;
    for(y=0;y<8;y++)
    {
        //LCD_WrCmd(0xb0+y);
        //LCD_WrCmd(0x00);
        //LCD_WrCmd(0x10);
        LCD_Set_Pos(0, y);
        for(x=0;x<X_WIDTH;x++)
            LCD_WrDat(bmp_dat);
    }
}
/*********************LCD复位************************************/
void LCD_Clear(void)
{
    unsigned char y,x;    
    for(y=0;y<8;y++)
    {
        //LCD_WrCmd(0xb0+y);
        //LCD_WrCmd(0x00);
        //LCD_WrCmd(0x10); 
        LCD_Set_Pos(0, y);
        for(x=0;x<X_WIDTH;x++)
            LCD_WrDat(0);
    }
}
/*********************LCD初始化************************************/
void LCD_Init(void)     
{
  IO_INIT(); //IO口初始化
  P0SEL &= 0xFE; //让P0.0为普通IO口，
  P0DIR |= 0x01; //让P0.0为为输出

  P1SEL &= 0x73; //让 P1.2 P1.3 P1.7为普通IO口
  P1DIR |= 0x8C; //把 P1.2 P1.3 1.7设置为输出

  LCD_SCL=1;
  LCD_RST=0;
  LCD_DLY_ms(50);
  LCD_RST=1;      //从上电到下面开始初始化要有足够的时间，即等待RC复位完毕   
  LCD_WrCmd(0xae);//--turn off oled panel
  LCD_WrCmd(0x00);//---set low column address
  LCD_WrCmd(0x10);//---set high column address
  LCD_WrCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  LCD_WrCmd(0x81);//--set contrast control register
  LCD_WrCmd(0xcf); // Set SEG Output Current Brightness
  LCD_WrCmd(0xa1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
  LCD_WrCmd(0xc8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
  LCD_WrCmd(0xa6);//--set normal display
  LCD_WrCmd(0xa8);//--set multiplex ratio(1 to 64)
  LCD_WrCmd(0x3f);//--1/64 duty
  LCD_WrCmd(0xd3);//-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
  LCD_WrCmd(0x00);//-not offset
  LCD_WrCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  LCD_WrCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
  LCD_WrCmd(0xd9);//--set pre-charge period
  LCD_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  LCD_WrCmd(0xda);//--set com pins hardware configuration
  LCD_WrCmd(0x12);
  LCD_WrCmd(0xdb);//--set vcomh
  LCD_WrCmd(0x40);//Set VCOM Deselect Level
  LCD_WrCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
  LCD_WrCmd(0x02);//
  LCD_WrCmd(0x8d);//--set Charge Pump enable/disable
  LCD_WrCmd(0x14);//--set(0x10) disable
  LCD_WrCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
  LCD_WrCmd(0xa6);// Disable Inverse Display On (0xa6/a7) 
  LCD_WrCmd(0xaf);//--turn on oled panel
  LCD_Fill(0);  //初始清屏
  LCD_Set_Pos(0,0);
} 

/***************功能描述：显示6*8一组标准ASCII字符串    显示的坐标（x,y），y为页范围0～7****************/
void LCD_P6x8Str(unsigned char x, unsigned char y,unsigned char ch[])
{
    unsigned char c=0,i=0,j=0;      
    while (ch[j]!='\0')
    {    
        c =ch[j]-32;
        if(x>126)
        {
          break;
          x=0;
          y++;
        }
        
        LCD_Set_Pos(x,y);    
        
        for(i=0;i<6;i++)     
        {
            LCD_WrDat(F6x8[c][i]);  
        }
        x+=6;
        j++;
    }
}

/*******************功能描述：显示8*16一组标准ASCII字符串     显示的坐标（x,y），y为页范围0～7****************/
void LCD_P8x16Str(unsigned char x, unsigned char y,unsigned char ch[])
{
    unsigned char c=0,i=0,j=0;
    while (ch[j]!='\0')
    {    
        c =ch[j]-32;
        if(x>120)
        {
          x=0;
          y++;
        }
        
        LCD_Set_Pos(x,y);    
        
        for(i=0;i<8;i++)
        {
          LCD_WrDat(F8X16[c*16+i]);
        }
        
        LCD_Set_Pos(x,y+1);
        
        for(i=0;i<8;i++)     
        {
            LCD_WrDat(F8X16[c*16+i+8]);  
        }
        
        x+=8;
        j++;
    }
}

/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void LCD_DrawBmp(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1, unsigned char* BMP)
{     
    unsigned int j=0;
    unsigned char x,y;
    
    if(y1%8==0) y=y1/8;      
    else y=y1/8+1;
    for(y=y0;y<y1;y++)
    {
        LCD_Set_Pos(x0,y);                
        for(x=x0;x<x1;x++)
        {      
            LCD_WrDat(BMP[j++]);            
        }
    }
} 

/*****************功能描述：显示16*16点阵  显示的坐标（x,y），y为页范围0～7****************************/
void LCD_P16x16Ch(unsigned char x, unsigned char y, unsigned char* chinese)
{
    unsigned char wm=0;
    unsigned char* addr=0;
    if(chinese==0) return;
    addr=getChineseCode(chinese);

    LCD_Set_Pos(x , y);
    for(wm = 0;wm < 16;wm++)  //             
    {
        LCD_WrDat(addr[wm]);    
    }      
    LCD_Set_Pos(x,y + 1); 
    for(wm = 0;wm < 16;wm++) //         
    {
        LCD_WrDat(addr[wm+16]);
    }           
}

/*****************功能描述：显示16*16点阵  显示的坐标（x,y），y为页范围0～7****************************/
void LCD_TextOut(unsigned char x, unsigned char y, unsigned char* str)
{
    unsigned char len=0;
    unsigned char i=0,j=0,k=0;
    unsigned char* addr=0;

    if(str==0) return;
 
    len=strlen(str);

    
    for(i=0; i<len; )
    {
        if(IS_CHINESE(str[i]))
        {
            LCD_P16x16Ch(x, y, str+i);
            i+=2;
            x+=16;
        }
        else
        {
            LCD_P8x16Str(x,y,str+i);
            i++;
            x+=8;
        }

        if(x>=X_WIDTH) return;
    }        
}

#endif