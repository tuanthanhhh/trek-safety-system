/** 
Edit by modify: Ngoc Hang 
**/

#ifndef I2C_LCD_H
#define I2C_LCD_H

#include "stm32f4xx_hal.h"

extern char lcd_buff[16];

void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_clear_display (void);	//clear display lcd

void lcd_goto_XY (int row, int col); //set proper location on screen

#endif /* I2C_LCD_H */
