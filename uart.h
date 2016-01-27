#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>

/**
 * @brief Initialize UART according to the settings in config.h
 */
extern void uart_init(void);

/**
 * @brief Write 'size' bytes to UART. Blocks until all
 * bytes are written to the fifo.
 * @returns Number of bytes written
 */
extern uint8_t uart_write(uint8_t *data, uint8_t size);

/**
 * @brief Write a string to UART. Blocks until all bytes
 * are written to the fifo.
 */
extern uint8_t uart_puts(char *str);

/**
 * @brief Write size bytes to UART. Can return even if not
 * all bytes are written to the fifo.
 * @returns Number of bytes written
 */
extern uint8_t uart_write_noblock(uint8_t *data, uint8_t size);


/**
 * @brief Read 'size' bytes from UART. Blocks until 'size'
 * bytes are read from the fifo.
 * @returns Number of bytes read
 */
extern uint8_t uart_read(uint8_t *data, uint8_t size);

/**
 * @brief Read at most 'size' bytes from UART.
 * @returns Number of bytes read
 */
extern uint8_t uart_read_noblock(uint8_t *data, uint8_t size);


/**
 * @brief Read at most 'size' bytes until delimiter is found.
 * @returns Number of bytes read
 */
extern uint8_t uart_read_until(uint8_t *data, uint8_t size, uint8_t delimiter);

#endif /* _UART_H_ */
