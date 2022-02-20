#include <avr/io.h>
#include <util/delay.h>

#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

// Input CC1/CC2
#define CC1 PA6
#define CC2 PA5

// Output CC1/CC2 Rpu's
#define CC1_R1 PB0 // 10 kOhm
#define CC1_R2 PB1 // 22 kOhm
#define CC2_R1 PA1 // 22 kOhm
#define CC2_R2 PA0 // 56 kOhm

typedef enum {
    USB_500MA,
    USB_1500MA,
    USB_3000MA,
} USBCurrent_t;

// Current | CC1/CC2 In | CC1 Rpu | CC2 Rpu
// 500mA   |    0.42V   | 10 kOhm | 22 kOhm
// 1.5A    |    0.94V   | 22 kOhm | 56 kOhm
// 3.0A    |    1.69V   | 10 kOhm | 22 kOhm

void set_usb_current(USBCurrent_t current) {
    switch(current) {
        case USB_500MA: {
            PORTB |= _BV(CC1_R1);
            PORTB &= ~_BV(CC1_R2);
            PORTA |= _BV(CC2_R1);
            PORTA &= ~_BV(CC2_R2);
            break;
        }
        case USB_1500MA: {
            PORTB &= ~_BV(CC1_R1);
            PORTB |= _BV(CC1_R2);
            PORTA &= ~_BV(CC2_R1);
            PORTA |= _BV(CC2_R2);
            break;
        }
        case USB_3000MA: {
            PORTB |= _BV(CC1_R1);
            PORTB &= ~_BV(CC1_R2);
            PORTA &= ~_BV(CC2_R1);
            PORTA |= _BV(CC2_R2);
            break;
        }
    }
}

uint16_t adc_sample_mv(uint8_t channel) {
    ADMUX = channel;
    ADCSRA |= (1 << ADSC); // Start conversion

    // Wait for conversion to complete
    while(ADCSRA & (1 << ADSC));

    uint16_t raw_adc_val = ADC;
    uint32_t adc_mv = ((uint32_t)raw_adc_val * 5000)/1024;

    return adc_mv;
}

// CC1/CC2 voltages for various USB C currents
#define USB_CC_MV_500MA 200
#define USB_CC_MV_1500MA 660
#define USB_CC_MV_3000MA 1230

USBCurrent_t get_cc_current(void) {
    // Read CC1/CC2 values for input current detection
    uint16_t cc1 = adc_sample_mv(CC1);
    uint16_t cc2 = adc_sample_mv(CC2);

    cc1 = MAX(cc1, cc2);

    USBCurrent_t curent;
    if (cc1 <= USB_CC_MV_1500MA) {
        curent = USB_500MA;
    } else if (cc1 <= USB_CC_MV_3000MA) {
        curent = USB_1500MA;
    } else {
        curent = USB_3000MA;
    }

    return curent;
}

int main(void)
{
    // Set port B output 0 as output
    DDRA = _BV(CC2_R1) | _BV(CC2_R2);
    DDRB = _BV(CC1_R1) | _BV(CC1_R2);

    // ADC Enable with / 16 prescaler (62.5kHz)
    ADCSRA = (1 << ADEN) | 3;

    // Disable digital inputs on ADC pins
    DIDR0 |= _BV(CC1) | _BV(CC2);

    // Default to 500mA charge current to start
    set_usb_current(USB_500MA);

    while (1) {
        set_usb_current(get_cc_current());
        _delay_ms(1000);
    }
    return 0;
}
