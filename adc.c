#include "adc.h"
#include <avr/interrupt.h>
#include <avr/io.h>

static volatile uint16_t channels[8] = {0};
static volatile uint8_t enabled_channels = 0x00;
static volatile uint8_t current_channel = 0;

static uint8_t next_channel() {
    const uint8_t start = (current_channel + 1) & 0x07;
    for (uint8_t i = start; i < 8; i++) {
        if (enabled_channels & (1 << i)) {
            return i;
        }
    }
    return 0;
}

ISR(ADC_vect) {
    channels[current_channel] = ADC;
    current_channel = next_channel();
    ADMUX &= ~0x0F;
    ADMUX |= current_channel;
}

void adc_init(uint8_t channels) {
    if (!channels) return;

    enabled_channels = channels;

    uint8_t channel = next_channel();

    ADMUX = (1 << REFS0) | channel;
    ADCSRA = (1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (0x07 << ADPS0);
    ADCSRB = (0x03 << ADTS0);

    ADCSRA |= (1 << ADSC);
}

uint16_t adc_read(uint8_t channel) {
    return channels[channel];
}
