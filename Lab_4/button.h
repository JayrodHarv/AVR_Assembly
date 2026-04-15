#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>
#include <avr/interrupt.h>

// ── Toggle button (INT1 on D3) ────────────────────────────────────────────────
// Wiring: one leg to GND, other leg to D3
// Internal pull-up used, so pin idles HIGH and pulls LOW on press
#define BTN_PIN  PD3
#define BTN_DDR  DDRD
#define BTN_PORT PORTD

void    button_init(void);
uint8_t button_take_press(void);  // returns 1 if a press is pending, clears flag

#endif