#ifndef coin_acceptor_H
#define coin_acceptor_H

#include <avr/io.h>

#define PULSE_VALUE_CENTS   1       // Value of each pulse in cents

void coin_acceptor_init(void);
uint32_t coin_get_balance(void);
void coin_decrement(void);

#endif