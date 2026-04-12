#define F_CPU 16000000UL // 16 MHz ATmega328P

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "lcd.h"
#include "encoder.h"
#include "fan.h"

#define RPM_UPDATE_INTERVAL_MS 500

int main(void) {
    lcd_init();
    encoder_init();
    fan_init();

    int8_t value   = 1;
    int8_t MIN_VAL = 1;
    int8_t MAX_VAL = 12;

    uint16_t elapsed_ms = 0;

    lcd_set_cursor(0, 0);
    lcd_print("Duty:   25%");

    lcd_set_cursor(1, 0);
    lcd_print("RPM:      0");

    while (1) {
        int8_t direction = encoder_read();

        if (direction != 0) {
            value += direction;

            // Clamp between 1 and 12
            if (value < MIN_VAL) value = MIN_VAL;
            if (value > MAX_VAL) value = MAX_VAL;

            fan_set_speed(value);

            char duty_buf[5];
            snprintf(duty_buf, sizeof(duty_buf), "%3d%%", fan_get_duty_pct());
            lcd_set_cursor(0, 7);
            lcd_print(duty_buf);
        }

        _delay_ms(1);
        elapsed_ms++;

        if (elapsed_ms >= RPM_UPDATE_INTERVAL_MS) {
            elapsed_ms = 0;

            char rpm_buf[6];
            snprintf(rpm_buf, sizeof(rpm_buf), "%5u", fan_get_rpm());
            lcd_set_cursor(1, 5);
            lcd_print(rpm_buf);
        }
    }
}