#ifndef PUMP_H
#define PUMP_H

#include <avr/io.h>

#define PUMP_PIN            PD6
#define PUMP_DDR            DDRD
#define PUMP_PORT           PORTD

#define PUMP_ACTIVE_STATE   0

// Milliseconds of pump runtime per cent in the balance.
// 1000 = 1 second per cent (e.g. 25 cents = 25 seconds).
#define PUMP_MS_PER_CENT    1000UL

void    pump_init(void);
void    pump_on(void);
void    pump_off(void);
uint8_t pump_is_on(void);

uint8_t pump_dispense(uint32_t balance_cents, uint8_t (*stop_condition)(void));
uint32_t pump_dispense_duration_ms(uint32_t balance_cents);

#endif