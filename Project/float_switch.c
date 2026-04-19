#include "float_switch.h"
#include <util/delay.h>

// -------------------------------------------------------
// float_switch_init()
// -------------------------------------------------------
void float_switch_init(void) {
    FLOAT_SWITCH_DDR  &= ~(1 << FLOAT_SWITCH_PIN);   // PD4 as input
    FLOAT_SWITCH_PORT |=  (1 << FLOAT_SWITCH_PIN);   // Enable internal pull-up
}

// -------------------------------------------------------
// float_switch_has_liquid()
//
// Reads the pin twice with a small delay between reads.
// This guards against reed switch chatter — a floating
// magnet can cause the switch to briefly open and close
// as it settles, so we only trust a reading that is
// stable across two samples.
// -------------------------------------------------------
uint8_t float_switch_has_liquid(void) {
    uint8_t reading1 = (FLOAT_SWITCH_PIND >> FLOAT_SWITCH_PIN) & 0x01;
    _delay_ms(5);
    uint8_t reading2 = (FLOAT_SWITCH_PIND >> FLOAT_SWITCH_PIN) & 0x01;

    // Only trust the reading if both samples agree
    if (reading1 != reading2) {
        // Switch is mid-chatter — return last known safe state (assume liquid
        // present to avoid spuriously blocking the spend button)
        return 1;
    }

    uint8_t pin_state = reading1;

#if FLOAT_SWITCH_ACTIVE_STATE == 0
    return (pin_state == 0) ? 1 : 0;   // LOW = active = liquid present
#else
    return (pin_state == 1) ? 1 : 0;   // HIGH = active = liquid present
#endif
}