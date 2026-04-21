#include "button.h"

void btn_init(void) {
    BTN_DDR  &= ~(1 << BTN_PIN);   // Input
    BTN_PORT |=  (1 << BTN_PIN);   // Internal pull-up
}

// Returns 1 on a confirmed button press (falling edge with debounce)
uint8_t btn_pressed(void) {
    if (BTN_PIND & (1 << BTN_PIN)) return 0;   // HIGH = not pressed, bail early

    _delay_ms(20);                               // Debounce

    if (BTN_PIND & (1 << BTN_PIN)) return 0;   // Gone HIGH again = bounce, ignore

    // Wait for release before returning so one press = one event
    while (!(BTN_PIND & (1 << BTN_PIN)));

    return 1;
}