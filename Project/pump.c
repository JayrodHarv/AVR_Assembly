#include "pump.h"
#include <util/delay.h>

// -------------------------------------------------------
// Internal state
// -------------------------------------------------------
static uint8_t pump_running = 0;

// -------------------------------------------------------
// Internal: write the correct pin level based on
// PUMP_ACTIVE_STATE so callers never think about polarity
// -------------------------------------------------------
static void set_relay(uint8_t on) {
#if PUMP_ACTIVE_STATE == 1
    if (on) PUMP_PORT |=  (1 << PUMP_PIN);
    else    PUMP_PORT &= ~(1 << PUMP_PIN);
#else
    if (on) PUMP_PORT &= ~(1 << PUMP_PIN);
    else    PUMP_PORT |=  (1 << PUMP_PIN);
#endif
}

// -------------------------------------------------------
// pump_init()
// -------------------------------------------------------
void pump_init(void) {
    PUMP_DDR |= (1 << PUMP_PIN);    // PD6 as output
    set_relay(0);                    // Ensure pump starts OFF
    pump_running = 0;
}

// -------------------------------------------------------
// pump_on()
// -------------------------------------------------------
void pump_on(void) {
    set_relay(1);
    pump_running = 1;
}

// -------------------------------------------------------
// pump_off()
// -------------------------------------------------------
void pump_off(void) {
    set_relay(0);
    pump_running = 0;
}

// -------------------------------------------------------
// pump_is_on()
// -------------------------------------------------------
uint8_t pump_is_on(void) {
    return pump_running;
}

// -------------------------------------------------------
// pump_dispense()
// -------------------------------------------------------
uint8_t pump_dispense(uint16_t duration_ms, uint8_t (*stop_condition)(void)) {
    pump_on();

    for (uint16_t ms = 0; ms < duration_ms; ms++) {
        _delay_ms(1);

        // Check stop condition once per millisecond if provided
        if (stop_condition != 0 && !stop_condition()) {
            pump_off();
            return 0;       // Aborted early
        }
    }

    pump_off();
    return 1;               // Completed full duration
}