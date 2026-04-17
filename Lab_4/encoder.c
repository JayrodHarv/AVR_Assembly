#include "encoder.h"

void encoder_init(void) {
    // Set A and B as inputs
    ENC_DDR &= ~((1 << ENC_A_PIN) | (1 << ENC_B_PIN));
}

int8_t encoder_read(void) {
    static uint8_t last_a = 1;
    uint8_t current_a = (ENC_PIN >> ENC_A_PIN) & 1;

    // No change on A — knob hasn't moved
    if (current_a == last_a)
        return 0;

    last_a = current_a;

    // Only act on the falling edge of A.
    if (current_a != 0)
        return 0;

    // Debounce delay
    _delay_ms(ENC_DEBOUNCE_MS);

    // After bounce settles, read B to determine direction:
    // If B is high when A falls the rpg is turned clockwise so increment
    // Otherwise, if B is low when A falls, the rpg is turned counter-clockwise so decrement
    if ((ENC_PIN >> ENC_B_PIN) & 1)
        return 1;   // Clockwise
    else
        return -1;  // Counter-clockwise
}