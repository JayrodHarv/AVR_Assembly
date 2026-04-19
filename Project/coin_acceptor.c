#include "coin_acceptor.h"
#include <avr/interrupt.h>
#include <util/delay.h>

// -------------------------------------------------------
// Shared state — volatile because written inside ISRs
// -------------------------------------------------------

// Running cent balance — incremented directly by INT0 ISR
static volatile uint32_t balance_cents = 0;

// -------------------------------------------------------
// INT0 ISR — fires on each falling edge from HX-916
// One pulse = PULSE_VALUE_CENTS cents. That's it.
// -------------------------------------------------------
ISR(INT0_vect) {
    EIMSK &= ~(1 << INT0);          // Disable INT0 immediately

    _delay_ms(5); // Wait for bounce to settle

    // If still low after debounce, increment balance_cents
    if ((PIND & (1 << PIND2)) == 0) {
        balance_cents += PULSE_VALUE_CENTS;
    }

    // Now clear any flag that accumulated while we waited
    EIFR |= (1 << INTF0);

    // Safe to re-enable — line is confirmed idle
    EIMSK |= (1 << INT0);
}

// -------------------------------------------------------
// INT1 ISR — fires when spend button is pressed
// Hardware debounce via external 10kΩ pull-up is recommended.
// We do a brief software debounce here too just in case.
// -------------------------------------------------------
// ISR(INT1_vect) {
//     // Disable INT1 temporarily to ignore bounce
//     EIMSK &= ~(1 << INT1);

//     spend_flag = 1;

//     // Small delay to let button settle — safe inside ISR
//     // because this is the only interrupt doing this
//     _delay_ms(50);

//     // Clear any spurious INT1 flag that fired during debounce
//     EIFR |= (1 << INTF1);

//     // Re-enable INT1
//     EIMSK |= (1 << INT1);
// }

// -------------------------------------------------------
// coin_pulse_init()
// -------------------------------------------------------
void coin_acceptor_init(void) {

    // --- INT0 (PD2, Pin 2) — HX-916 coin pulse input ---
    DDRD  &= ~(1 << DDD2);     // PD2 as input
    PORTD &= ~(1 << PD2);      // No internal pull-up (use external 10kΩ)

    // Falling edge trigger (NO mode: line pulled HIGH, pulses LOW)
    EICRA |=  (1 << ISC01);
    EICRA &= ~(1 << ISC00);

    EIMSK |= (1 << INT0);      // Enable INT0

    // --- INT1 (PD3, Pin 3) — Spend button input ---
    // DDRD  &= ~(1 << DDD3);     // PD3 as input
    // PORTD &= ~(1 << PD3);      // No internal pull-up (use external 10kΩ)

    // Falling edge trigger (button pulls line LOW when pressed)
    // EICRA |=  (1 << ISC11);
    // EICRA &= ~(1 << ISC10);

    // EIMSK |= (1 << INT1);      // Enable INT1
}

// -------------------------------------------------------
// coin_get_balance()
// -------------------------------------------------------
uint32_t coin_get_balance(void) {
    // balance_cents is uint32_t — not atomic on 8-bit AVR.
    // Disable interrupts briefly to read all 4 bytes consistently.
    uint32_t val;
    cli();
    val = balance_cents;
    sei();
    return val;
}

// -------------------------------------------------------
// coin_spend_ready()
// -------------------------------------------------------
// uint8_t coin_spend_ready(void) {
//     if (spend_flag) {
//         spend_flag = 0;     // Clear flag — caller must handle the spend
//         return 1;
//     }
//     return 0;
// }

// -------------------------------------------------------
// coin_spend()
// -------------------------------------------------------
uint8_t coin_spend(void) {
    cli();
    if (balance_cents > 0) {
        balance_cents = 0; // Spend full balance
        sei();
        return 1;   // Success
    }
    sei();
    return 0;       // Insufficient funds
}