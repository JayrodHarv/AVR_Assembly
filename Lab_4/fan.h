#ifndef FAN_H
#define FAN_H

#include <avr/io.h>
#include <avr/interrupt.h>

// ── Delta EFB0412VHD-SP05 Fan ─────────────────────────────────────────────────
// 12V DC brushless axial fan, PWM speed control via yellow wire
// PWM frequency: preferred 25kHz (datasheet section 11)
// Duty cycle:    0% = stop, 100% = 9000 RPM max
// Min start duty: 25% (datasheet section 12)
//
// Wiring:
//   Red    → Arduino Vin
//   Black  → GND (shared with Arduino GND)
//   Yellow → Arduino D9 (PWM output, OC1A)
//   Blue   → unconnected (tachometer, unused here)

// ── PWM pin: OC1A = PB1 = Arduino D9 ─────────────────────────────────────────
// We use Timer1 in Fast PWM mode at 25kHz
// At F_CPU = 16MHz: prescaler=1, TOP = ICR1 = 639
//   f_PWM = 16,000,000 / (1 * (639+1)) = 25,000 Hz exactly
#define FAN_PWM_PIN  PB1
#define FAN_PWM_DDR  DDRB

// Tachometer pin
#define FAN_TACHO_PIN  PD2
#define FAN_TACHO_DDR  DDRD
#define FAN_TACHO_PORT PORTD

// ── Speed range mapping ───────────────────────────────────────────────────────
// Encoder value 1–12 maps to duty cycle 25%–100%
// Below 25% the fan cannot reliably start from rest (datasheet minimum)
#define FAN_DUTY_MIN_PCT  25
#define FAN_DUTY_MAX_PCT  100

// ICR1 TOP value for 25kHz at 16MHz with prescaler=1
#define FAN_PWM_TOP  639

// 4 poles = 4 FG pulses per revolution
#define FAN_PULSES_PER_REV 4

void fan_init(void);
void fan_set_speed(uint8_t encoder_value);  // accepts 1–12
uint8_t  fan_get_duty_pct(void);
uint16_t fan_get_rpm(void);

#endif