#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>
#include <util/delay.h>

// Pin definition
#define BTN_PIN     PD4
#define BTN_DDR     DDRD
#define BTN_PORT    PORTD
#define BTN_PIND    PIND

void btn_init(void);
uint8_t btn_pressed(void);

#endif