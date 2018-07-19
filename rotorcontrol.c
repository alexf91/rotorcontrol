#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "config.h"

#include "adc.h"
#include "lcd_routines.h"
#include "system.h"
#include "uart.h"

#define BUFSIZE 32

#define AZ_MIN 0
#define AZ_MAX 360
#define EL_MIN 0
#define EL_MAX 180

#define AVERAGE_LENGTH 128

typedef struct {
    uint8_t index;
    uint16_t buffer[AVERAGE_LENGTH];
} average_t;

average_t azimuth_avg = {0};
average_t elevation_avg = {0};

void average_add_sample(average_t *avg, uint16_t sample) {
    avg->buffer[avg->index] = sample;
    avg->index = (avg->index % AVERAGE_LENGTH);
}

uint16_t average_get_value(average_t *avg) {
    uint32_t sum = 0;
    for (uint16_t i = 0; i < AVERAGE_LENGTH; i++) {
        sum += avg->buffer[avg->index];
    }
    return sum / AVERAGE_LENGTH;
}

void azimuth_stop() { ROTOR_PORT &= ~((1 << ROTOR_LEFT) | (1 << ROTOR_RIGHT)); }

void elevation_stop() { ROTOR_PORT &= ~((1 << ROTOR_UP) | (1 << ROTOR_DOWN)); }

void rotator_stop() {
    azimuth_stop();
    elevation_stop();
}

void rotor_start(uint8_t dir) {
    if (dir == ROTOR_RIGHT || dir == ROTOR_LEFT)
        ROTOR_PORT &= ~((1 << ROTOR_RIGHT) | (1 << ROTOR_LEFT));

    if (dir == ROTOR_UP || dir == ROTOR_DOWN)
        ROTOR_PORT &= ~((1 << ROTOR_UP) | (1 << ROTOR_DOWN));

    ROTOR_PORT |= (1 << dir);
}

uint16_t get_azimuth() { return (adc_read(ROTOR_AZ) / AZ_FACTOR); }

uint16_t get_elevation() { return (adc_read(ROTOR_EL) / EL_FACTOR); }

/**
 * Returns true if everything is ok and false otherwise.
 */
bool atoi_safe(const char *nptr, int16_t *i, uint16_t min, uint16_t max) {
    for (uint8_t i = 0; i < strlen(nptr); i++) {
        if (nptr[i] < '0' || nptr[i] > '9') return true;
    }

    int16_t tmp = atoi(nptr);
    if (tmp < min || tmp > max) return 1;

    *i = tmp;
    return false;
}

void rotator_control() {
    static char cmdbuf[BUFSIZE] = {0};
    static uint8_t size = 0;

    static int16_t az_target = 0;
    static int16_t el_target = 0;

    static bool az_running = false;
    static bool el_running = false;

    // Look if a command is in the buffer
    bool new_command = false;
    unsigned int c = uart_getc();
    while ((c & 0xFF00) == 0) {
        if (c == '\r') {
            cmdbuf[size] = '\0';
            new_command = (size > 0);
            break;
        } else if (size < BUFSIZE) {
            cmdbuf[size++] = c;
        } else {
            // Just throw everything away before we overflow
            size = 0;
        }
        c = uart_getc();
    }

    char uart_buffer[BUFSIZE];
    if (new_command) {
        if (cmdbuf[0] == 'S' && size == 1) {
            rotator_stop();
            az_running = false;
            el_running = false;
            uart_puts("\r\n");
        } else if (cmdbuf[0] == 'W' && size == 8) {
            cmdbuf[4] = '\0';
            cmdbuf[8] = '\0';

            if (atoi_safe(&cmdbuf[1], &az_target, AZ_MIN, AZ_MAX) == 0 &&
                atoi_safe(&cmdbuf[5], &el_target, EL_MIN, EL_MAX) == 0) {
                az_running = true;
                el_running = true;
                uart_puts("\r\n");
            } else {
                uart_puts("? >");
            }
        } else if (cmdbuf[0] == 'M' && size == 4) {
            cmdbuf[4] = '\0';

            if (atoi_safe(&cmdbuf[1], &az_target, AZ_MIN, AZ_MAX) == 0) {
                az_running = true;
                uart_puts("\r\n");
            } else {
                uart_puts("? >");
            }
        } else if (cmdbuf[0] == 'C' && cmdbuf[1] == '2') {
            uint16_t az = average_get_value(&azimuth_avg);
            uint16_t el = average_get_value(&elevation_avg);
            sprintf(uart_buffer, "+0%03u+0%03u\r\n", az, el);
            uart_puts(uart_buffer);
        } else if (cmdbuf[0] == 'R' && size == 1) {
            az_target = AZ_MAX;
            az_running = true;
            uart_puts("\r\n");
        } else if (cmdbuf[0] == 'L' && size == 1) {
            az_target = AZ_MIN;
            az_running = true;
            uart_puts("\r\n");
        } else if (cmdbuf[0] == 'U' && size == 1) {
            el_target = EL_MAX;
            el_running = true;
            uart_puts("\r\n");
        } else if (cmdbuf[0] == 'D' && size == 1) {
            el_target = EL_MIN;
            el_running = true;
            uart_puts("\r\n");
        } else if (cmdbuf[0] == 'A' && size == 1) {
            azimuth_stop();
            az_running = false;
            uart_puts("\r\n");
        } else if (cmdbuf[0] == 'E' && size == 1) {
            elevation_stop();
            el_running = false;
            uart_puts("\r\n");
        } else if (cmdbuf[0] == 'C' && size == 1) {
            uint16_t az = average_get_value(&azimuth_avg);
            sprintf(uart_buffer, "+0%03u\r\n", az);
            uart_puts(uart_buffer);
        } else if (cmdbuf[0] == 'B' && size == 1) {
            uint16_t el = get_elevation();
            sprintf(uart_buffer, "+0%03u\r\n", el);
            uart_puts(uart_buffer);
        } else {
            uart_puts("? >");
        }

        size = 0;
        new_command = false;
    }

    if (az_running) {
        int16_t az_diff = az_target - average_get_value(&azimuth_avg);

        if (az_diff > 0) {
            rotor_start(ROTOR_RIGHT);
        } else if (az_diff < 0) {
            rotor_start(ROTOR_LEFT);
        } else {
            azimuth_stop();
            az_running = false;
        }
    }

    if (el_running) {
        int16_t el_diff = el_target - average_get_value(&elevation_avg);

        if (el_diff > 0) {
            rotor_start(ROTOR_UP);
        } else if (el_diff < 0) {
            rotor_start(ROTOR_DOWN);
        } else {
            elevation_stop();
            el_running = false;
        }
    }
}

int main(void) {
    char display_buffer[16];
    uint8_t local_systick;
    uint8_t last_systick = 0;

    LED_DDR |= (1 << LED);

    ROTOR_DDR |= (1 << ROTOR_UP) | (1 << ROTOR_DOWN) | (1 << ROTOR_LEFT) |
                 (1 << ROTOR_RIGHT);

    uart_init(UART_BAUD_SELECT(UART_BAUDRATE, F_CPU));
    systick_init();
    adc_init(((1 << ROTOR_AZ) | (1 << ROTOR_EL)));

    lcd_init();
    lcd_clear();

    sei();
    while (1) {
        local_systick = systick_counter();

        if (local_systick != last_systick) {
            average_add_sample(&azimuth_avg, get_azimuth());
            average_add_sample(&elevation_avg, get_elevation());

            if (local_systick == 0) {
                lcd_clear();
                sprintf(display_buffer, "AZ: %3u",
                        average_get_value(&azimuth_avg));
                lcd_string(display_buffer);
                lcd_setcursor(0, 2);
                sprintf(display_buffer, "EL: %3u",
                        average_get_value(&elevation_avg));
                lcd_string(display_buffer);
            } else {
                rotator_control();
            }
        }
        last_systick = local_systick;
    }
}
