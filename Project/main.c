#define F_CPU 16000000UL // 16 MHz ATmega328P

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "coin_acceptor.h"
#include "float_switch.h"
#include "button.h"
#include "pump.h"

// -------------------------------------------------------
// State machine
// -------------------------------------------------------
typedef enum {
    INITIAL_STATE,      // No coins have been inserted yet, show welcome message
    COIN_COLLECTING_STATE,
    DISPENSING_STATE,
    THANKS_STATE,
    TANK_EMPTY_STATE,
    NO_FUNDS_STATE
} MachineState;
 
// The one global state variable
volatile MachineState state = INITIAL_STATE;

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

    // Clear the display first
    lcd_clear();

    switch(s) {
        case INITIAL_STATE:
            lcd_set_cursor(0, 0);
            lcd_print("Welcome!");
            lcd_set_cursor(1, 0);
            lcd_print("Insert coins...");
            break;
        
        case COIN_COLLECTING_STATE:
            lcd_set_cursor(0, 0);
            lcd_print("Balance: ");
            format_dollars(balance, buf);
            lcd_print(buf);
            lcd_set_cursor(1, 0);
            lcd_print("Press button");
            break;

        case DISPENSING_STATE:
            lcd_set_cursor(0, 0);
            lcd_print("Dispensing...");
            lcd_set_cursor(1, 0);
            lcd_print("Balance: ");
            format_dollars(balance, buf);
            lcd_print(buf);
            break;

        case THANKS_STATE:
            lcd_set_cursor(0, 0);
            lcd_print("Thank you!");
            lcd_set_cursor(1, 0);
            lcd_print("Enjoy your drink");
            break;

        case TANK_EMPTY_STATE:
            lcd_set_cursor(0, 0);
            lcd_print("** Tank empty **");
            lcd_set_cursor(1, 0);
            lcd_print("Please refill");
            break;

        case NO_FUNDS_STATE:
            lcd_set_cursor(0, 0);
            lcd_print("** No funds! **");
            lcd_set_cursor(1, 0);
            lcd_print("Insert coins...");
            break;
    }
}

int main(void) {
    lcd_init();
    coin_acceptor_init();
    float_switch_init();
    btn_init();
    pump_init();
    sei(); // Enable global interrupts

    // Determine initial state from float switch
    state = float_switch_has_liquid() ? INITIAL_STATE : TANK_EMPTY_STATE;

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

        // If at any point the float switch indicates no liquid, we must go to TANK_EMPTY_STATE
        if (!float_switch_has_liquid()) {
            state = TANK_EMPTY_STATE;
        } else if (state == TANK_EMPTY_STATE) {
            // If we were in TANK_EMPTY_STATE but liquid has returned, go back to the appropriate state based on balance
            if (balance > 0) {
                state = COIN_COLLECTING_STATE;
            } else {
                state = INITIAL_STATE;
            }
        }

        // --------------------------------------------------
        // State machine
        // --------------------------------------------------

        switch (state) {
 
            case INITIAL_STATE:
                // No coins yet. If we get any, move to COIN_COLLECTING.
                if (balance > 0) {
                    state = COIN_COLLECTING_STATE;
                }
                else if (btn_pressed()) {
                    // Button press with no coins — show error message briefly
                    state = NO_FUNDS_STATE;
                }
                break;
 
            case COIN_COLLECTING_STATE:
                // Coins have been inserted. If button pressed, try to spend.
                // Coin isertions handled by interrupt
                if (btn_pressed()) {
                    state = DISPENSING_STATE;
                }
                break;

            case DISPENSING_STATE:
                pump_on();
 
                while (coin_get_balance() > 0) {
                    uint8_t tank_ok = 1;
 
                    // Wait out one cent's pump time
                    for (uint32_t ms = 0; ms < PUMP_MS_PER_CENT; ms++) {
                        _delay_ms(1);
 
                        if (!float_switch_has_liquid()) {
                            tank_ok = 0;
                            break;      // Exit inner loop immediately
                        }
                    }
 
                    if (!tank_ok) {
                        // Tank emptied mid-cent — stop pump,
                        // keep balance (cent not yet decremented)
                        pump_off();
                        state = TANK_EMPTY_STATE;
                        goto dispense_exit;
                    }
 
                    // Full cent dispensed successfully — consume it
                    coin_decrement();
 
                    // Redraw LCD with updated balance
                    lcd_draw(DISPENSING_STATE, coin_get_balance());
                    last_balance = coin_get_balance();
                }

                pump_off();
                state = INITIAL_STATE;
                dispense_exit:
                break;
            
            case THANKS_STATE:
                // Show thank you message for a few seconds, then return to initial state
                _delay_ms(5000);
                state = INITIAL_STATE;
                break;

            case NO_FUNDS_STATE:
                // Wait here until the error condition clears (handled above in the main loop)
                _delay_ms(5000); // Show error message for 5 seconds
                state = INITIAL_STATE; // After showing error, go back to initial state
                break;
            
            case TANK_EMPTY_STATE:
                // Wait here until the tank is refilled (handled above in the main loop)
                break;
        }

    }
}