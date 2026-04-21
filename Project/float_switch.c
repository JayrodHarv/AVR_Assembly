#include "float_switch.h"
#include <avr/interrupt.h>
#include <util/delay.h>

// -------------------------------------------------------
// Volatile state flag — written by ISR, read by main loop
// -------------------------------------------------------
static volatile uint8_t liquid_present = 1;   // Assume full at startup

// -------------------------------------------------------
// Internal: read the raw pin state and map to liquid present
// -------------------------------------------------------
static uint8_t read_pin(void) {
    uint8_t pin = (FLOAT_SWITCH_PIND >> FLOAT_SWITCH_PIN) & 0x01;
#if FLOAT_SWITCH_ACTIVE_STATE == 0
    return (pin == 0) ? 1 : 0;     // LOW = active = liquid present
#else
    return (pin == 1) ? 1 : 0;     // HIGH = active = liquid present
#endif
}

// -------------------------------------------------------
// INT1 ISR — fires on any logical change of PD3
//
// Debounce strategy: disable INT1, wait 20ms for the reed
// switch to settle, then take two readings 5ms apart.
// Only update the flag if both readings agree.
// Re-enable INT1 and clear any flag that built up during
// the debounce window before returning.
// -------------------------------------------------------
ISR(INT1_vect) {
    EIMSK &= ~(1 << INT1);         // Disable INT1 during debounce

    _delay_ms(20);                  // Wait for reed switch to settle

    uint8_t r1 = read_pin();
    _delay_ms(5);
    uint8_t r2 = read_pin();

    if (r1 == r2)                   // Both readings agree — accept change
        liquid_present = r1;
    // If readings disagree (still settling), keep the last known state

    EIFR  |= (1 << INTF1);         // Clear any flag queued during debounce
    EIMSK |= (1 << INT1);          // Re-enable INT1
}

// -------------------------------------------------------
// float_switch_init()
// -------------------------------------------------------
void float_switch_init(void) {
    FLOAT_SWITCH_DDR  &= ~(1 << FLOAT_SWITCH_PIN);   // PD3 as input
    // FLOAT_SWITCH_PORT |=  (1 << FLOAT_SWITCH_PIN);   // Enable internal pull-up

    // Trigger INT1 on any logical change (both rising and falling edges)
    EICRA |=  (1 << ISC10);        // ISC11=0, ISC10=1 → any change
    EICRA &= ~(1 << ISC11);

    EIMSK |= (1 << INT1);          // Enable INT1

    // Set the initial state from the actual pin on startup
    liquid_present = read_pin();
}

// -------------------------------------------------------
// float_switch_has_liquid()
// -------------------------------------------------------
uint8_t float_switch_has_liquid(void) {
    return liquid_present;          // Just return the ISR-maintained flag
}