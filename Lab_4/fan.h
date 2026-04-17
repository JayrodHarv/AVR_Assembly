#ifndef FAN_H
#define FAN_H

#include <avr/io.h>
#include <avr/interrupt.h>

// PWM pin: OC0B = PD5 = Arduino D5
#define FAN_PWM_PIN  PD5
#define FAN_PWM_DDR  DDRD

// Tachometer pin
#define FAN_TACHO_PIN  PD2
#define FAN_TACHO_DDR  DDRD
#define FAN_TACHO_PORT PORTD

// Speed range mapping
#define FAN_DUTY_MIN_PCT  1
#define FAN_DUTY_MAX_PCT  100

// ICR0 top value for 80kHz
#define FAN_PWM_TOP  199

// 4 poles = 4 FG pulses per revolution
#define FAN_PULSES_PER_REV 4

void fan_init(void);
void fan_set_duty(uint8_t duty_pct);
uint8_t  fan_get_duty_pct(void);
uint16_t fan_get_rpm(void);
void fan_toggle(void);
uint8_t fan_is_on(void);

#endif