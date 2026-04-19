#ifndef coin_acceptor_H
#define coin_acceptor_H

#include <avr/io.h>

// -------------------------------------------------------
// HX-916 Coin Acceptor Driver — Pulse Mode
//
// The HX-916 White (COIN) wire is wired to Arduino Pin 2
// (PD2 / INT0). Each coin insertion triggers one or more
// falling-edge pulses. The number of pulses per coin is
// set by the P value programmed into the HX-916.
//
// This driver uses:
//   INT0  (PD2, pin 2) — counts incoming pulses via ISR
//   Timer1             — detects when pulses have stopped
//                        (coin insertion complete)
//
// Wiring (pulse mode):
//   HX-916 White (COIN) → Arduino Pin 2 (INT0 / PD2)
//   HX-916 Green (GND)  → Arduino GND + 12V supply GND
//   HX-916 Red   (12V)  → External 12V supply (+)
//   HX-916 Gray         → Not connected
//   10kΩ resistor       → Between Pin 2 and 5V (external pull-up)
//   NO/NC switch        → Set to NO (Normally Open)
// -------------------------------------------------------

// -------------------------------------------------------
// Pulse-to-value configuration
//
// Set PULSE_VALUE_CENTS to the value of ONE pulse in cents.
// Then program each coin's P value on the HX-916 to emit
// that many pulses. Examples:
//
//   PULSE_VALUE_CENTS = 1  → P=1 for 1c, P=5 for 5c, P=25 for 25c
//   PULSE_VALUE_CENTS = 5  → P=1 for 5c, P=2 for 10c, P=5 for 25c
//
// The simplest setup: PULSE_VALUE_CENTS=1, and program each
// coin to emit its exact cent value as the pulse count.
// -------------------------------------------------------
#define PULSE_VALUE_CENTS   1       // Value of each pulse in cents

/**
 * coin_pulse_init()
 * Configures INT0 (PD2) for falling-edge interrupts and
 * Timer1 in normal mode for timeout detection.
 * Call once before sei().
 */
void coin_acceptor_init(void);

/**
 * coin_get_balance()
 * Returns the current cent balance.
 * Safe to call from the main loop at any time.
 */
uint32_t coin_get_balance(void);

/**
 * coin_spend_ready()
 * Returns 1 if the spend button was pressed since last checked.
 * Clears the internal flag when called — call only once per loop.
 */
// uint8_t hx916_spend_ready(void);
 
/**
 * coin_spend()
 * Deducts SPEND_AMOUNT_CENTS from the balance if funds allow.
 * Returns 1 on success, 0 if balance is insufficient.
 */
uint8_t coin_spend(void);

#endif