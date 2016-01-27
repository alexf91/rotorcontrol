#ifndef _ADC_H_
#define _ADC_H_

#include <avr/io.h>

void adc_init(uint8_t channels);

uint16_t adc_read(uint8_t channel);


#endif /* _ADC_H_ */
