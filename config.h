#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <avr/io.h>

#define F_CPU       16000000UL
#define F_SYSTICK   1000UL

/* UART definitions, used in uart.c */
#define UART_RX_FIFO_SIZE 64
#define UART_TX_FIFO_SIZE 64
#define UART_BAUDRATE 115200UL

/* Pinout */
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED         5

#define ROTOR_DDR   DDRC
#define ROTOR_PORT  PORTC
#define ROTOR_PIN   PINC
#define ROTOR_AZ    0   // ADC input channel 0
#define ROTOR_EL    1   // ADC input channel 1
#define ROTOR_RIGHT 2
#define ROTOR_LEFT  3
#define ROTOR_UP    4
#define ROTOR_DOWN  5

/* Degree - ADC relation */
#define EL_FACTOR   5
#define AZ_FACTOR   2

#endif /* _CONFIG_H_ */
