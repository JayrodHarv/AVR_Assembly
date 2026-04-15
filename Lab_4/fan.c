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
    // ── PWM setup (Timer0, Fast PWM, 80kHz) ──────────────────────────────
    FAN_PWM_DDR |= (1 << FAN_PWM_PIN);

    // Set OCR0A as TOP for custom frequency (Mode 7: Fast PWM, WGM = 0b111)
    OCR0A = FAN_PWM_TOP;

    // COM0B1=1: non-inverting output on OC0B
    // WGM01=1, WGM00=1: Fast PWM (bottom two bits of WGM)
    TCCR0A = (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);

    // WGM02=1: top bit of WGM, selects OCR0A as TOP
    // CS00=1: prescaler = 1 (no prescaling)
    TCCR0B = (1 << WGM02) | (1 << CS00);

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

    fan_set_duty(25);
}

void fan_set_duty(uint8_t duty_pct) {
    // Clamp to valid range just in case
    if (duty_pct < 1)   duty_pct = 1;
    if (duty_pct > 100) duty_pct = 100;

    _duty_pct = duty_pct;
    OCR0B = ((uint16_t)_duty_pct * FAN_PWM_TOP) / 100;
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