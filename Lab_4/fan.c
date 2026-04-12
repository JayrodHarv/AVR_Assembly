#include "fan.h"

// ── Shared state between ISR and main ────────────────────────────────────────
// volatile tells the compiler these can change outside normal program flow
// (i.e. inside the interrupt) so it must never cache them in a register
static volatile uint16_t _pulse_count = 0;  // pulses counted by ISR
static uint8_t _duty_pct = FAN_DUTY_MIN_PCT; // current duty cycle

// ── Tachometer interrupt service routine ─────────────────────────────────────
// Fires on every falling edge of the FG (blue wire) signal.
// Open-collector output idles HIGH via pull-up and pulls LOW each pulse,
// so we count falling edges.
ISR(INT0_vect) {
    _pulse_count++;
}

void fan_init(void) {
    // ── PWM setup (Timer1, Fast PWM, 25kHz) ──────────────────────────────
    FAN_PWM_DDR |= (1 << FAN_PWM_PIN);

    ICR1   = FAN_PWM_TOP;
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13)  | (1 << WGM12) | (1 << CS10);

    // ── Tachometer setup (INT0 on D2) ────────────────────────────────────
    // Set D2 as input
    FAN_TACHO_DDR  &= ~(1 << FAN_TACHO_PIN);
    // Enable internal pull-up as backup to the external 10kΩ resistor
    FAN_TACHO_PORT |=  (1 << FAN_TACHO_PIN);

    // Configure INT0 to trigger on falling edge
    // ISC01=1, ISC00=0 → falling edge of INT0 generates interrupt
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);

    // Enable INT0
    EIMSK |= (1 << INT0);

    // Enable global interrupts
    sei();

    fan_set_speed(1);
}

void fan_set_speed(uint8_t encoder_value) {
    _duty_pct = FAN_DUTY_MIN_PCT +
                ((uint16_t)(encoder_value - 1) *
                (FAN_DUTY_MAX_PCT - FAN_DUTY_MIN_PCT)) / 11;

    OCR1A = ((uint32_t)_duty_pct * FAN_PWM_TOP) / 100;
}

uint8_t fan_get_duty_pct(void) {
    return _duty_pct;
}

uint16_t fan_get_rpm(void) {
    // To calculate RPM we snapshot the pulse count over a fixed
    // time window (500ms here), then scale to RPM:
    //
    //   pulses_per_min = pulses_in_500ms * 2      (500ms → 1 minute = ×120)
    //   rpm = pulses_per_min / FAN_PULSES_PER_REV
    //
    // Combined: rpm = (pulse_count * 120) / FAN_PULSES_PER_REV
    //
    // We disable interrupts briefly just to take a clean atomic
    // snapshot of _pulse_count before resetting it, then re-enable.

    cli();
    uint16_t count = _pulse_count;
    _pulse_count   = 0;
    sei();

    // Scale count from 500ms window to RPM
    // Multiply first to preserve precision before dividing
    return ((uint32_t)count * 120) / FAN_PULSES_PER_REV;
}