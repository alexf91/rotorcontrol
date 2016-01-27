#ifndef _FIFO_H_
#define _FIFO_H_

#include <inttypes.h>

typedef volatile struct {
    uint8_t *data;
    uint8_t size;
    uint8_t read;
    uint8_t write;
} fifo_t;

#define FIFO_INIT(name, _size) \
    static uint8_t name##_raw[_size] = {0}; \
    fifo_t name = {&name##_raw[0], _size, 0, 0}

static inline uint8_t fifo_full(fifo_t *fifo) {
    return ((fifo->write + 1 == fifo->read) ||
            (fifo->read == 0 && fifo->write+1 == fifo->size));
}

static inline uint8_t fifo_empty(fifo_t *fifo) {
    return fifo->read == fifo->write;
}

// Return 0 on success
uint8_t fifo_write(fifo_t *fifo, uint8_t byte);

// Return 0 on success
uint8_t fifo_read(fifo_t *fifo, uint8_t *byte);

// Return number of bytes written
uint8_t fifo_write_array(fifo_t *fifo, uint8_t *array, uint8_t size);

// Return number of bytes read
uint8_t fifo_read_array(fifo_t *fifo, uint8_t *array, uint8_t size);


#endif /* _FIFO_H_ */
