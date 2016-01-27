#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "config.h"
#include "uart.h"
#include "fifo.h"

FIFO_INIT(rx_fifo, UART_RX_FIFO_SIZE);
FIFO_INIT(tx_fifo, UART_TX_FIFO_SIZE);

ISR(USART_UDRE_vect) {
    if(fifo_empty(&tx_fifo)) {
        UCSR0B &= ~(1<<UDRIE0);
    }
    else {
        uint8_t byte;
        fifo_read(&tx_fifo, &byte);
        UDR0 = byte;
    }
}

ISR(USART_RX_vect) {
    fifo_write(&rx_fifo, UDR0);
}


void uart_init(void) {
    UCSR0A = (1<<U2X0);
    UCSR0B = (1<<RXCIE0) | (1<<RXEN0) | (1<<TXEN0);
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

    uint16_t ubrr = (F_CPU / (8UL * UART_BAUDRATE)) - 1;

    UBRR0H = (uint8_t) (ubrr >> 8);
    UBRR0L = (uint8_t) (ubrr & 0xFF);
}

uint8_t uart_write(uint8_t *data, uint8_t size) {
    uint8_t  left = size;
    while(left != 0) {
        left -= fifo_write_array(&tx_fifo, &data[size-left], left);
        UCSR0B |= (1<<UDRIE0);
    }
    return size;
}

uint8_t uart_puts(char *str) {
    uint8_t size = strlen(str);
    return uart_write((uint8_t *)str, size);
}

uint8_t uart_write_noblock(uint8_t *data, uint8_t size) {
    uint8_t sent = fifo_write_array(&tx_fifo, data, size);
    UCSR0B |= (1<<UDRIE0);
    return sent;
}

uint8_t uart_read(uint8_t *data, uint8_t size) {
    uint8_t left = size;
    while(left != 0) {
        left -= fifo_read_array(&rx_fifo, &data[size-left], left);
    }
    return size;
}

uint8_t uart_read_noblock(uint8_t *data, uint8_t size) {
    return fifo_read_array(&rx_fifo, data, size);
}

uint8_t uart_read_until(uint8_t *data, uint8_t size, uint8_t delimiter) {
    uint8_t i = 0;
    uint8_t c = ~delimiter;
    while(i < size && c != delimiter) {
        uart_read(&c, 1);
        data[i] = c;
        i++;
    }
    data[i-1] = '\0';
    return i;
}
