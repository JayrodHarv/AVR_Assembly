#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    // Set PB5 (pin 13) as output
    DDRB |= (1 << PB5);

    while (1)
    {
        PORTB |= (1 << PB5);   // LED ON
        _delay_ms(500);

        PORTB &= ~(1 << PB5);  // LED OFF
        _delay_ms(500);
    }
}