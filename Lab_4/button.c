#include "button.h"

static volatile uint8_t _btn_pressed = 0;

ISR(INT1_vect) {
    _btn_pressed = 1;
}

void button_init(void) {
    BTN_DDR  &= ~(1 << BTN_PIN);   // set as input
    BTN_PORT |=  (1 << BTN_PIN);   // enable internal pull-up

    // Trigger on falling edge (ISC11=1, ISC10=0)
    EICRA |= (1 << ISC11);
    EICRA &= ~(1 << ISC10);

    // Enable INT1
    EIMSK |= (1 << INT1);
}

uint8_t button_take_press(void) {
    if (_btn_pressed) {
        _btn_pressed = 0;
        return 1;
    }
    return 0;
}