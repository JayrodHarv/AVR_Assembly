#ifndef PUMP_H
#define PUMP_H

#include <avr/io.h>

// -------------------------------------------------------
// Pump Controller Driver
//
// Controls a 12V peristaltic pump via a 5V relay on PD6.
// Dispense duration is calculated as 1 second per cent
// in the balance at the time of dispensing.
//
// Wiring:
//   PD6 (Pin 6) → Relay IN
//   5V          → Relay VCC
//   GND         → Relay GND
//   Relay NO    → 12V supply (+)
//   Relay COM   → Pump (+)
//   Pump (-)    → 12V supply (-)
//
// Active HIGH = relay energises on HIGH (most common).
// Change PUMP_ACTIVE_STATE to 0 for active LOW modules.
// -------------------------------------------------------

#define PUMP_PIN            PD6
#define PUMP_DDR            DDRD
#define PUMP_PORT           PORTD

#define PUMP_ACTIVE_STATE   0

// Milliseconds of pump runtime per cent in the balance.
// 1000 = 1 second per cent (e.g. 25 cents = 25 seconds).
// Adjust this to calibrate volume per cent for your pump.
#define PUMP_MS_PER_CENT    1000UL

// -------------------------------------------------------
// Public API
// -------------------------------------------------------

void    pump_init(void);
void    pump_on(void);
void    pump_off(void);
uint8_t pump_is_on(void);

/**
 * pump_dispense(balance_cents, stop_condition)
 *
 * Runs the pump for (balance_cents * PUMP_MS_PER_CENT) ms,
 * checking stop_condition once per millisecond. Stops early
 * and returns 0 if stop_condition returns 0. Returns 1 on
 * successful completion of the full duration.
 *
 * Pass NULL for stop_condition to run with no early-stop check.
 *
 * Example:
 *   pump_dispense(25, float_switch_has_liquid);
 *   // Runs for 25,000ms (25s), stops if tank empties
 */
uint8_t pump_dispense(uint32_t balance_cents, uint8_t (*stop_condition)(void));

/**
 * pump_dispense_duration_ms(balance_cents)
 * Returns the calculated dispense duration in milliseconds
 * for a given balance. Useful for displaying a countdown
 * or logging without actually running the pump.
 */
uint32_t pump_dispense_duration_ms(uint32_t balance_cents);

#endif /* PUMP_H */