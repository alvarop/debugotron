#include <avr/io.h>
#include <util/delay.h>

// Code derived from https://github.com/casperbang/avr-attiny-tutorial

#define RED_LED PA6
#define GREEN_LED PA5

#define DELAY_MS 500

int main(void)
{
    // Set port B output 0 as output
    DDRA = _BV(RED_LED) | _BV(GREEN_LED);

    while (1) {
        PORTB &= ~_BV(RED_LED);
        PORTB |= _BV(GREEN_LED);
        _delay_ms(DELAY_MS);

        PORTB &= ~_BV(GREEN_LED);
        PORTB |= _BV(RED_LED);
        _delay_ms(DELAY_MS);
    }
    return 0;
}
