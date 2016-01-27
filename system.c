#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "system.h"

static volatile uint32_t _systick_counter;

ISR(TIMER0_COMPA_vect) {
    _systick_counter++;
}

void systick_init(void) {
  TCCR0A = (1<<WGM01); // CTC Modus
  TCCR0B |= (1<<CS01) | (1<<CS00); // Prescaler 64
  OCR0A = OCR_VALUE;
 
  TIMSK0 |= (1<<OCIE0A);

  _systick_counter = 0;
}

uint32_t systick_counter(void) {
    return _systick_counter;
}

void system_reset(void) {
    wdt_enable(WDTO_15MS);
    while(1);
}

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

void wdt_init(void) {
    MCUSR = 0;
    wdt_disable();
}
