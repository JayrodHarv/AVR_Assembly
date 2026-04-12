#ifndef LCD_H
#define LCD_H

#include <avr/io.h>
#include <util/delay.h>

// Control pins on PORTD
#define LCD_RS_PIN  PD2
#define LCD_E_PIN   PD3

// Data pins DB4–DB7 on PORTB
#define LCD_D4_PIN  PB0
#define LCD_D5_PIN  PB1
#define LCD_D6_PIN  PB2
#define LCD_D7_PIN  PB3

#define LCD_RS_PORT PORTD
#define LCD_E_PORT  PORTD
#define LCD_DATA_PORT PORTB
#define LCD_DATA_DDR  DDRB
#define LCD_CTRL_DDR  DDRD

// HD44780 instruction codes (from Table 6 in datasheet)
#define LCD_CLEAR_DISPLAY   0x01
#define LCD_RETURN_HOME     0x02
#define LCD_ENTRY_MODE      0x06  // Increment cursor, no display shift
#define LCD_DISPLAY_ON      0x0C  // Display on, cursor off, blink off
#define LCD_FUNCTION_SET_4BIT 0x28 // 4-bit, 2 lines, 5x8 dots
#define LCD_SET_DDRAM_ADDR  0x80

void lcd_init(void);
void lcd_send_command(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_print(const char *str);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);

#endif