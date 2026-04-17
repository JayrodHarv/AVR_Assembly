#ifndef ENCODER_H
#define ENCODER_H

#include <avr/io.h>
#include <util/delay.h>

#define ENC_A_PIN  PD6   // Signal A
#define ENC_B_PIN  PD7   // Signal B
#define ENC_PORT   PORTD
#define ENC_PIN    PIND
#define ENC_DDR    DDRD

// Debounce delay (greater than 3ms according to datasheet)
#define ENC_DEBOUNCE_MS 4

// Encoder functions
void   encoder_init(void);
int8_t encoder_read(void);

#endif