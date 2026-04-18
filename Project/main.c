#define F_CPU 16000000UL // 16 MHz ATmega328P

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "HX916.h"

static void format_dollars(uint32_t cents, char *buf) {
    uint32_t dollars = cents / 100;
    uint8_t  rem     = (uint8_t)(cents % 100);
    uint8_t  i       = 0;
 
    buf[i++] = '$';
 
    if (dollars == 0) {
        buf[i++] = '0';
    } else {
        char tmp[6];
        uint8_t d = 0;
        uint32_t v = dollars;
        while (v > 0) { tmp[d++] = '0' + (uint8_t)(v % 10); v /= 10; }
        while (d)      { buf[i++] = tmp[--d]; }
    }
 
    buf[i++] = '.';
    buf[i++] = '0' + (rem / 10);
    buf[i++] = '0' + (rem % 10);
    buf[i]   = '\0';
}

static void lcd_print_padded(const char *str, uint8_t width) {
    uint8_t len = 0;
    const char *p = str;
    while (*p++) len++;         // Measure string length
 
    lcd_print(str);
 
    while (len < width) {       // Pad remainder with spaces
        lcd_send_data(' ');
        len++;
    }
}

static void lcd_update(uint32_t balance) {
    char buf[9];
 
    // Row 0 — balance
    lcd_set_cursor(0, 0);
    lcd_print("Bal: ");
    format_dollars(balance, buf);
    lcd_print_padded(buf, 11);      // 16 - 5 ("Bal: ") = 11 chars remaining
}

int main(void) {
    lcd_init();
    hx916_init();
    sei(); // Enable global interrupts

    lcd_set_cursor(0, 0);
    lcd_print("Balance:  0");

    uint32_t last_balance = 0xFFFFFFFF;     // Force first redraw

    while (1) {

        uint32_t balance = hx916_get_balance();

        if (balance != last_balance) {
            last_balance = balance;
            lcd_update(balance);
        }

    }
}