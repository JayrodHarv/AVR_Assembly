#define F_CPU 16000000UL // 16 MHz ATmega328P

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "lcd.h"
#include "encoder.h"

int main(void) {
    lcd_init();
    encoder_init();

    int8_t value   = 1;
    int8_t MIN_VAL = 1;
    int8_t MAX_VAL = 12;

    // Static label on line 1
    lcd_set_cursor(0, 0);
    lcd_print("Value:");

    // Initial value on line 2
    lcd_set_cursor(1, 0);
    lcd_print(" 1");

    while (1) {
        int8_t direction = encoder_read();

        if (direction != 0) {
            value += direction;

            // Clamp between 1 and 12
            if (value < MIN_VAL) value = MIN_VAL;
            if (value > MAX_VAL) value = MAX_VAL;

            // Print value padded to 2 chars wide to cleanly
            // overwrite previous value without screen flicker
            char buf[3];
            snprintf(buf, sizeof(buf), "%2d", value);
            lcd_set_cursor(1, 0);
            lcd_print(buf);
        }

        // Small polling delay — the hardware caps and resistors handle
        // most of the debounce, so we only need a light delay here
        _delay_ms(1);
    }
}