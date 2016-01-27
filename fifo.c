#include "fifo.h"

uint8_t fifo_write(fifo_t *fifo, uint8_t byte) {
    if(fifo_full(fifo))
        return 1;

    fifo->data[fifo->write] = byte;
    fifo->write++;
    if(fifo->write >= fifo->size)
        fifo->write = 0;
    return 0;
}

uint8_t fifo_read(fifo_t *fifo, uint8_t *byte) {
    if(fifo_empty(fifo))
        return 1;

    *byte = fifo->data[fifo->read];
    fifo->read++;
    if(fifo->read >= fifo->size)
        fifo->read = 0;

    return 0;
}

uint8_t fifo_read_array(fifo_t *fifo, uint8_t *array, uint8_t size) {
    uint8_t i = 0;
    uint8_t c;
    while(i < size && fifo_read(fifo, &c) == 0) {
        array[i] = c;
        i++;
    }

    return i;
}

uint8_t fifo_write_array(fifo_t *fifo, uint8_t *array, uint8_t size) {
    uint8_t i = 0;
    
    while(i < size && fifo_write(fifo, array[i]) == 0) {
        i++;
    }

    return i;
}
