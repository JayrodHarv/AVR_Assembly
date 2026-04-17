#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define BTN_PIN  PD3
#define BTN_DDR  DDRD
#define BTN_PORT PORTD

void    button_init(void);
uint8_t button_take_press(void);  // returns 1 if a press is pending, clears flag

#endif