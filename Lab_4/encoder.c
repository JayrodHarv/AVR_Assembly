#include "encoder.h"

void encoder_init(void) {
    // Set A and B as inputs
    // ENC_DDR &= ~((1 << ENC_A_PIN) | (1 << ENC_B_PIN));

    // Enable internal pull-ups as a backup to the external 10kΩ resistors.
    // The external resistors recommended by the datasheet are the primary
    // pull-ups — these just add a safety net.
    // ENC_PORT |= (1 << ENC_A_PIN) | (1 << ENC_B_PIN);
}

int8_t encoder_read(void) {
    static uint8_t last_a = 1;
    uint8_t current_a = (ENC_PIN >> ENC_A_PIN) & 1;

    // No change on A — knob hasn't moved
    if (current_a == last_a)
        return 0;

    last_a = current_a;

    // Only act on the falling edge of A (HIGH to LOW transition).
    // The EVE-GA1F2012B produces one falling edge per detent click.
    if (current_a != 0)
        return 0;

    // Wait for bounce to settle before reading B.
    // Datasheet specifies 3ms max bounce — we wait 4ms to be safe.
    _delay_ms(ENC_DEBOUNCE_MS);

    // After bounce settles, read B to determine direction:
    // B HIGH when A falls → clockwise     → increment
    // B LOW  when A falls → anticlockwise → decrement
    if ((ENC_PIN >> ENC_B_PIN) & 1)
        return 1;   // Clockwise
    else
        return -1;  // Counter-clockwise
}