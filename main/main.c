#include <reg51.h>
#include "LCD_16x2.h"
#include "Keypad_4x4.h"
#include <string.h>



#define MAX_SIZE 24
#define MAX_RESULT 2147483647
#define MIN_RESULT -2147483647

uchar bufferDisplay[LCD_numOfCols] = "                ";
uchar bufferCalc[MAX_SIZE];
uchar numOfDisplayedChars = 0, numOfChars = 0, pressedEqualKey = 0;



void reset() 
{
	lcd_clear();
	memcpy(bufferDisplay, "                ", LCD_numOfCols);
	memset(bufferCalc, 0, MAX_SIZE);
	numOfDisplayedChars = 0;
	numOfChars = 0;
	pressedEqualKey = 0;
}



int calculate()
{
	long result = 0, operand = 0, operand1 = 0; 
	uchar operator = ' '; 
	char priorityCalcFlag = 0, lastIndexOfSumSubSign = -1, notCompleteCalcFlag = 0, i;
	
	for (i = 0; i < numOfChars; i++) {		
		/* ------------------------------ handle number ------------------------------ */
		if (bufferCalc[i] >= '0' && bufferCalc[i] <= '9') 
		{			
			/* prioritize to calculate * or / before */
			if ((i != 0) && (bufferCalc[i-1] == 'x' || bufferCalc[i-1] == 0xFD))
				priorityCalcFlag = 1;
			
			/* not complete calculation previously */
			if (notCompleteCalcFlag == 1 ) 
			{
				operand = operand1;
				if (operator == '-' || operator == 0xFD)
				{
					operand1 = result;
					result = operand;
					operand = operand1;
				}
				operand1 = 0;
				notCompleteCalcFlag = 0;
				priorityCalcFlag = 0;
			} 
			/* convert sequence to number  */	 	
			else 
			{
				operand += (bufferCalc[i] - '0');
				while ((i != numOfChars-1) && (bufferCalc[i+1] >= '0') && (bufferCalc[i+1] <= '9'))
				{
					operand *= 10;
					operand += (bufferCalc[++i] - '0');	
				}
			}
			
			/* if number is before (* or /) and is after (+ or -) */
			if ((i != numOfChars-1) && (bufferCalc[i+1] == 'x' || bufferCalc[i+1] == 0xFD) && (priorityCalcFlag == 0)) 
			{
				operand1 = result;	// store calculated result before
				operator = ' ';
			}							
			
			/* calculate */				
			if (operator == '+') 
			{
				result += operand;
			}				
			else if (operator == '-') 
			{
				result -= operand;
			}													
			else if (operator == 'x') 
			{
				result *= operand;
			}				
			else if (operator == 0xFD) 
			{
			 	if (operand == 0)				// systax error
					return -1;
				result /= operand;		
			}				
			else
				result = operand;
			
  			/* calculate result until now*/
			if (priorityCalcFlag == 1) {			   				
				if (bufferCalc[i+1] != 'x' && bufferCalc[i+1] != 0xFD && lastIndexOfSumSubSign != -1) 
				{
					operator = bufferCalc[lastIndexOfSumSubSign];
					notCompleteCalcFlag = 1;
					i--;									
				}
				priorityCalcFlag = 0;
			}	
		} 

	   /* ------------------------------ handle operator ------------------------------ */
		else 
		{													
		  	if (i == 0 || bufferCalc[i+1] < '0' || bufferCalc[i+1] > '9')	// systax error
				return -1;

			if (bufferCalc[i] == '+' || bufferCalc[i] == '-')
				lastIndexOfSumSubSign = i;

			operator = bufferCalc[i];			
		}	
		
		operand = 0;							
	}		
   	
	/* ------------------------------------ handle result ---------------------------------- */
	if (result == 0)
	   	bufferDisplay[--numOfDisplayedChars] = '0';
	else {
		if (result < 0)
			operand = result * -1;
		else
			operand = result;
		
	   	while (operand != 0) {
			bufferDisplay[--numOfDisplayedChars] = (operand % 10 + '0');
			operand /= 10;
		}
	   	if (result < 0)
			bufferDisplay[--numOfDisplayedChars] = '-';	
	}			 	
	return 0;
}



void main()
{ 	
    uchar col, row, i;
	
	lcd_init();
	
    while(1)
    {   		
		/* scan column to identify key */
		col = keypad_scanColumn();
		
		/* ----------------------------- when key is pressed  --------------------------------- */
		if (col != -1) 									
		{								
			/* handle display full line */
			if (numOfDisplayedChars == LCD_numOfCols-1) {		
				bufferDisplay[0] = 0x7F;					// symbol '<-'
				for (i = 1; i < LCD_numOfCols-2; i++)
					bufferDisplay[i] = bufferDisplay[i+1];			
				--numOfDisplayedChars;
			}

			/* scan row to identify key */
		 	row = keypad_scanRow(); 
			
			/* handle 'ON/C' key */	
			if (keys[row][col] == ' ') {				
				reset();
			}

			/* handle '=' key */
			else if (keys[row][col] == '=') {	   				
				numOfDisplayedChars = 16;
				memcpy(bufferDisplay, "                ", LCD_numOfCols);
				
				i = calculate();
				if (i == -1) {
					reset();
				   	lcd_movCur(1, 3);
					lcd_sendStr("Syntax ERROR");	
				}
//				else if (i == -2) {
//					reset();
//				   	lcd_movCur(1, 2);
//					lcd_sendStr("Overflow ERROR");
//				}
//				else if (i == -3) {
//					reset();
//				   	lcd_movCur(1, 2);
//					lcd_sendStr("Underflow ERROR");
//				}
				else {
					lcd_movCur(2, 1);
					lcd_sendStr(bufferDisplay);	
				}

				numOfDisplayedChars = 16;
				pressedEqualKey = 1;		
			}  

			/* handle other keys */
			else {													  	 
				// when '=' is pressed previously, press other keys to clear and enter again 						   
			 	if (pressedEqualKey == 1)				
				  	reset();	
			
				if (numOfChars < MAX_SIZE) {
					// store data																		
					bufferDisplay[numOfDisplayedChars++] = keys[row][col];
					bufferCalc[numOfChars++] = keys[row][col];

					// display on LCD
					lcd_movCur(1, 1);					
					lcd_sendStr(bufferDisplay);
				}
				else 
				   	numOfDisplayedChars = LCD_numOfCols - 1;	// exceed threshold
			}	 							
		}
		
		/* ------------------------------handle blink cursor ------------------------------ */
		lcd_movCur(1, numOfDisplayedChars+1);  				
		if (pressedEqualKey == 1)
			lcd_off_blinkCur();
		else
			lcd_blinkCur();

		delay_ms(100);
    }
}