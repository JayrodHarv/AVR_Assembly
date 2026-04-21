#ifndef FLOAT_SWITCH_H
#define FLOAT_SWITCH_H

#include <avr/io.h>

// -------------------------------------------------------
// Float Switch Driver
//
// Reads a passive reed-contact float switch on PD4 (Pin 4).
// Uses the AVR's internal pull-up — no external resistor needed.
//
// Wiring:
//   One leg of float switch → Arduino Pin 4 (PD4)
//   Other leg               → GND
//
// Logic (NO orientation — open when float is DOWN):
//   Float UP   (tank has liquid) → switch closes → pin reads LOW  → liquid present
//   Float DOWN (tank empty)      → switch opens  → pin reads HIGH → no liquid
//
// If your switch is mounted in NC orientation and reads the
// opposite, flip FLOAT_SWITCH_ACTIVE_STATE below.
// -------------------------------------------------------

// Pin definition
#define FLOAT_SWITCH_PIN    PD3
#define FLOAT_SWITCH_PORT   PORTD
#define FLOAT_SWITCH_DDR    DDRD
#define FLOAT_SWITCH_PIND   PIND

// The pin state that means "liquid is present".
// LOW (0) = switch closed = float up = liquid present (default NO wiring).
// Change to 1 if your switch reads the opposite way.
#define FLOAT_SWITCH_ACTIVE_STATE   0

// -------------------------------------------------------
// Public API
// -------------------------------------------------------

/**
 * float_switch_init()
 * Configures PD4 as an input with the internal pull-up enabled.
 * Call once before the main loop.
 */
void float_switch_init(void);

/**
 * float_switch_has_liquid()
 * Returns 1 if liquid is present (float is up), 0 if tank is empty.
 * Safe to call from the main loop at any time.
 */
uint8_t float_switch_has_liquid(void);

#endif /* FLOAT_SWITCH_H */