#define F_CPU 16000000UL // 16 MHz ATmega328P

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "coin_acceptor.h"
#include "float_switch.h"

/ -------------------------------------------------------
// State machine
// -------------------------------------------------------
typedef enum {
    STATE_IDLE,         // Waiting for coins or button press
    STATE_TANK_EMPTY,   // Float switch low — block all spending
    STATE_DISPENSING,   // Pump running
    STATE_NO_FUNDS      // Button pressed with insufficient balance
} MachineState;
 
// The one global state variable
volatile MachineState state = STATE_IDLE;

static void format_dollars(uint32_t cents, char *buf) {
    uint32_t dollars = cents / 100;
    uint8_t  rem     = (uint8_t)(cents % 100);
    uint8_t  i       = 0;
 
    buf[i++] = '$';
 
    if (dollars == 0) {
        buf[i++] = '0';
    } else {
        char tmp[6];
        uint8_t d = 0;
        uint32_t v = dollars;
        while (v > 0) { tmp[d++] = '0' + (uint8_t)(v % 10); v /= 10; }
        while (d)      { buf[i++] = tmp[--d]; }
    }
 
    buf[i++] = '.';
    buf[i++] = '0' + (rem / 10);
    buf[i++] = '0' + (rem % 10);
    buf[i]   = '\0';
}

static void lcd_print_padded(const char *str, uint8_t width) {
    uint8_t len = 0;
    const char *p = str;
    while (*p++) len++;         // Measure string length
 
    lcd_print(str);
 
    while (len < width) {       // Pad remainder with spaces
        lcd_send_data(' ');
        len++;
    }
}

// Draw both LCD rows to match the current state
static void lcd_draw(MachineState s, uint32_t balance) {
    char buf[9];
 
    // Row 0 — always shows balance
    lcd_set_cursor(0, 0);
    lcd_print("Bal: ");
    format_dollars(balance, buf);
    lcd_print_padded(buf, 11);
 
    // Row 1 — reflects current state
    lcd_set_cursor(1, 0);
    switch (s) {
        case STATE_IDLE:
            // lcd_print("Spend ");
            // format_dollars(SPEND_AMOUNT_CENTS, buf);
            // lcd_print_padded(buf, 10);
            break;
        case STATE_TANK_EMPTY:
            lcd_print_padded("** Tank empty **", 16);
            break;
        case STATE_DISPENSING:
            lcd_print_padded("  Dispensing... ", 16);
            break;
        case STATE_NO_FUNDS:
            lcd_print_padded("  No funds!     ", 16);
            break;
    }
}

int main(void) {
    lcd_init();
    coin_acceptor_init();
    sei(); // Enable global interrupts

    // Determine initial state from float switch
    state = float_switch_has_liquid() ? STATE_IDLE : STATE_TANK_EMPTY;

    uint32_t balance = coin_get_balance();
    uint32_t last_balance = 0xFFFFFFFF;         // Force first LCD draw
    MachineState last_state = (MachineState)0xFF;

    while (1) {

        uint32_t balance = coin_get_balance();

        // Redraw LCD only when something actually changed
        if (state != last_state || balance != last_balance) {
            lcd_draw(state, balance);
            last_state   = state;
            last_balance = balance;
        }

        // --------------------------------------------------
        // State machine
        // --------------------------------------------------
        switch (state) {
 
            case STATE_IDLE:
            // Waiting for a coin or button press.
            // Transition to TANK_EMPTY if liquid is lost.
            // Transition to DISPENSING or NO_FUNDS on button.
            // ----------------------------------------------
                if (!float_switch_has_liquid()) {
                    state = STATE_TANK_EMPTY;
                    break;
                }
 
                // if (coin_spend_ready()) {
                //     if (coin_spend()) {
                //         state = STATE_DISPENSING;
                //     } else {
                //         state = STATE_NO_FUNDS;
                //     }
                // }
                break;
 
            case STATE_TANK_EMPTY:
            // Blocked — do nothing until liquid returns.
            // Button presses are silently consumed here so
            // the spend_ready flag doesn't queue up.
            // ----------------------------------------------
                // coin_spend_ready();     // Drain flag — ignore button
 
                if (float_switch_has_liquid()) {
                    state = STATE_IDLE;
                }
                break;

            case STATE_DISPENSING:
            // Pump is running. Run for the set duration
            // then return to IDLE. If the tank empties mid-
            // dispense, stop the pump and go to TANK_EMPTY.
            // ----------------------------------------------
                // pump_on();
 
                // for (uint16_t ms = 0; ms < 3000; ms++) {
                //     _delay_ms(1);
 
                //     if (!float_switch_has_liquid()) {
                //         pump_off();
                //         state = STATE_TANK_EMPTY;
                //         goto dispense_done;
                //     }
                // }
 
                // pump_off();
                // state = STATE_IDLE;
                // dispense_done:
                break;
 
            // ----------------------------------------------
            case STATE_NO_FUNDS:
            // Show the message briefly, then return to IDLE.
            // Coins inserted during this delay still count —
            // the INT0 ISR runs freely in the background.
            // ----------------------------------------------
                _delay_ms(1500);
                state = STATE_IDLE;
                break;
        }

    }
}