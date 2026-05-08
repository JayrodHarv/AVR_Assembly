#ifndef FLOAT_SWITCH_H
#define FLOAT_SWITCH_H

#include <avr/io.h>

// Pin definition
#define FLOAT_SWITCH_PIN    PD3
#define FLOAT_SWITCH_PORT   PORTD
#define FLOAT_SWITCH_DDR    DDRD
#define FLOAT_SWITCH_PIND   PIND

// The pin state that means "liquid is present".
// LOW (0) = switch closed = float up = liquid present (default NO wiring).
#define FLOAT_SWITCH_ACTIVE_STATE   0

void float_switch_init(void);
uint8_t float_switch_has_liquid(void);

#endif