#include <reg51.h>
#include <math.h>



#define uchar unsigned char
#define uint unsigned int



#define LCD_numOfCols 16
#define LCD_numOfRows 2

sbit LCD_RS = P3^2; 							
sbit LCD_RW = P3^3; 							
sbit LCD_EN = P3^4; 							
#define LCD_DATA P2



void delay_ms(uint ms);
void lcd_sendCmd(uchar command);
void lcd_sendCmd(uchar command);
void lcd_clear(void);
void lcd_init(void);
void lcd_movCur(uchar row, uchar column);
void lcd_blinkCur(void);
void lcd_off_blinkCur(void);
void lcd_sendStr(uchar* string);
void lcd_sendInt(uint value);
void lcd_sendFloat(float value, uint precision);



void delay_ms(uint ms)								// delay in ms					
{
	uint i, j;
	for (i = 0; i < ms; i++)
		for (j = 0; j < 123; j++); 					
}



void lcd_sendCmd(uchar command)						// send cmd
{
	LCD_RW = 0; 									// select write mode
	LCD_RS = 0; 									// select cmd reg
	LCD_DATA = command; 							
	LCD_EN = 1;										// high to low transition -> enable
	LCD_EN = 0;										
	delay_ms(1);
}



void lcd_sendData(uchar Data)						// send data
{
	LCD_RW = 0; 									
	LCD_RS = 1; 									// select data reg
	LCD_DATA = Data; 								
	LCD_EN = 1;										
	LCD_EN = 0;										
	delay_ms(1);
}



void lcd_clear()									// clear screen
{
	lcd_sendCmd(0x01);									
	delay_ms(2); 								
}



void lcd_init()										// init LCD
{
	LCD_RS = 0;										
	LCD_RW = 0;										
	delay_ms(50); 									// until LCD turn on > 40ms
	lcd_sendCmd(0x38); 								// 8-bits, 2 lines, 5x8 pixel
	delay_ms(1);
	lcd_sendCmd(0x0C); 								// display on, cursor off
	delay_ms(1);
	lcd_sendCmd(0x06); 								// inc cursor
	delay_ms(1);
	lcd_clear();
}



void lcd_movCur(uchar row, uchar column)			// move cursor by DDRAM address
{
	switch(row)
	{
		case 1:
			lcd_sendCmd(0x80 + (column - 1));		// 0x00 | 0x80	 		
			break;
		case 2: 
			lcd_sendCmd(0xC0 + (column - 1));		// 0x40 | 0x80
			break;
		default: 
			break;
	}
}



void lcd_blinkCur()
{
	lcd_sendCmd(0x0F);
	delay_ms(1);
}



void lcd_off_blinkCur()
{
	lcd_sendCmd(0x0E);
	delay_ms(1);
}



void lcd_sendStr(uchar *string)						// send string
{
	while(*string)									
		lcd_sendData(*string++);
}



void lcd_sendInt(uint value)						// send integer number
{
	uchar buffer[16], i = 0;

	do {
		buffer[i++] = (value % 10) + '0'; 			// convert number to character
		value /= 10;							
	} while(value);
	
	while(i)										
		lcd_sendData(buffer[--i]);
}



void lcd_sendFloat(float value, uint precision)		// send float number
{
	uint integer, fractional;
										
	integer = (uint)value;						
	fractional = (value - integer) * pow(10, precision);		
	
	lcd_sendInt(integer);								
	lcd_sendData('.');									
	lcd_sendInt(fractional);								
}