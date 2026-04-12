#ifndef ENCODER_H
#define ENCODER_H

#include <avr/io.h>
#include <util/delay.h>

// ── Panasonic EVE-GA1F2012B Rotary Encoder ────────────────────────────────────
// 12 pulses per revolution, 12 detents, no push switch
// Quadrature (incremental) output, 3-pin: A, B, COM
// COM connects to GND
// A and B require 10kΩ pull-ups to 5V and 0.01µF caps to GND
// (per Panasonic datasheet recommended circuit)

// ── Encoder pins (PORTD) ──────────────────────────────────────────────────────
#define ENC_A_PIN  PD4   // Signal A (CLK)
#define ENC_B_PIN  PD5   // Signal B (DT)
#define ENC_PORT   PORTD
#define ENC_PIN    PIND  // PIND is used to READ pin states on PORTD
#define ENC_DDR    DDRD

// ── Bounce time per datasheet is 3ms max. We use 4ms to be safe. ──────────────
#define ENC_DEBOUNCE_MS 4

// ── Encoder functions ─────────────────────────────────────────────────────────
void   encoder_init(void);
int8_t encoder_read(void);

#endif