#ifndef PUMP_H
#define PUMP_H

#include <avr/io.h>

// -------------------------------------------------------
// Pump Controller Driver
//
// Controls a 12V peristaltic pump via a 5V relay module.
// The relay IN pin is driven by PD6 (Arduino pin 6).
//
// Wiring:
//   PD6 (Pin 6) → Relay IN
//   5V          → Relay VCC
//   GND         → Relay GND
//   Relay NO    → 12V supply (+)
//   Relay COM   → Pump (+)
//   Pump (-)    → 12V supply (-)
//
// Relay type: Active HIGH (relay energises when IN is HIGH)
// If your relay module is active LOW, flip PUMP_ACTIVE_STATE.
// -------------------------------------------------------

#define PUMP_PIN            PD6
#define PUMP_DDR            DDRD
#define PUMP_PORT           PORTD

// Active HIGH = relay energises on HIGH (most common modules)
// Change to 0 if your relay module is active LOW
#define PUMP_ACTIVE_STATE   0

// Default dispense duration in milliseconds
#define PUMP_DISPENSE_MS    3000

// -------------------------------------------------------
// Public API
// -------------------------------------------------------

/**
 * pump_init()
 * Configures PD6 as an output and ensures the pump starts OFF.
 * Call once before the main loop.
 */
void pump_init(void);

/**
 * pump_on()
 * Energises the relay and starts the pump immediately.
 */
void pump_on(void);

/**
 * pump_off()
 * De-energises the relay and stops the pump immediately.
 */
void pump_off(void);

/**
 * pump_is_on()
 * Returns 1 if the pump is currently running, 0 if stopped.
 */
uint8_t pump_is_on(void);

/**
 * pump_dispense(duration_ms, stop_condition)
 *
 * Runs the pump for duration_ms milliseconds, checking
 * stop_condition once per millisecond. If stop_condition
 * returns 0 mid-dispense, the pump is stopped immediately
 * and the function returns 0 (aborted). Returns 1 on
 * successful completion of the full duration.
 *
 * stop_condition is a function pointer — pass NULL to run
 * for the full duration with no early-stop check.
 *
 * Example — run for 3s, stop early if tank empties:
 *   pump_dispense(3000, float_switch_has_liquid);
 *
 * Example — run for 3s with no condition:
 *   pump_dispense(3000, NULL);
 */
uint8_t pump_dispense(uint16_t duration_ms, uint8_t (*stop_condition)(void));

#endif /* PUMP_H */