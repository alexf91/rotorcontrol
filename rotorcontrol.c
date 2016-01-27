#include "config.h"
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lcd_routines.h"
#include "uart.h"
#include "system.h"
#include "adc.h"

#define LCD_SLICE 0

#define BUFSIZE 32

#define TOLERANCE 2
#define AZ_MIN 0
#define AZ_MAX 360
#define EL_MIN 0
#define EL_MAX 180

void rotator_stop() {
    ROTOR_PORT &= ~((1<<ROTOR_UP) | (1<<ROTOR_DOWN) | (1<<ROTOR_LEFT) | (1<<ROTOR_RIGHT));
}

void azimuth_stop() {
    ROTOR_PORT &= ~((1<<ROTOR_LEFT) | (1<<ROTOR_RIGHT));
}

void elevation_stop() {
    ROTOR_PORT &= ~((1<<ROTOR_UP) | (1<<ROTOR_DOWN));
}

void rotor_start(uint8_t dir) {
    ROTOR_PORT |= (1<<dir);
}

uint16_t get_azimuth() {
    return (adc_read(ROTOR_AZ) / AZ_FACTOR);
}

uint16_t get_elevation() {
    return (adc_read(ROTOR_EL) / EL_FACTOR);
}

/**
 * Returns 0 if everything is ok, 1 otherwise
 */
uint8_t atoi_save(const char *nptr, int16_t *i, uint16_t min, uint16_t max) {
    for(uint8_t i=0; i<strlen(nptr); i++) {
        if(nptr[i] < '0' || nptr[i] > '9')
            return 1;
    }

    int16_t tmp = atoi(nptr);
    if(tmp < min || tmp > max)
        return 1;

    *i = tmp;
    return 0;
}

void rotator_control() {
    static char buffer[BUFSIZE] = {0};
    static uint8_t size = 0;

    static uint16_t az_target = 0;
    static uint16_t el_target = 0;

    static uint8_t  az_running = 0;
    static uint8_t  el_running = 0;

    // Look if a command is in the buffer
    char c;
    uint8_t new_command = 0;
    while(uart_read_noblock(&c, 1) == 1) {
        if(c == '\r') {
            new_command = (size > 0);
        }
        else if(size < BUFSIZE) {
            buffer[size] = c;
            size++;
        }
        else {
            // Just throw everything away before we overflow
            size = 0;
        }
    }
    
    char uart_buffer[BUFSIZE];
    uint16_t tmp;
    if(new_command) {
        if(buffer[0] == 'S' && size == 1) {
            rotator_stop();
            az_running = 0;
            el_running = 0;
            uart_puts("\r\n");
        }
        else if(buffer[0] == 'W' && size == 8) {
            buffer[4] = '\0';
            buffer[8] = '\0';
            
            if(atoi_save(&buffer[1], &az_target, AZ_MIN, AZ_MAX) == 0 &&
               atoi_save(&buffer[5], &el_target, EL_MIN, EL_MAX) == 0) {

                az_running = 1;
                el_running = 1;
                uart_puts("\r\n");
            }
            else {
                uart_puts("? >");
            }
        }
        else if(buffer[0] == 'M' && size == 4) {
            buffer[4] = '\0';

            if(atoi_save(&buffer[1], &az_target, AZ_MIN, AZ_MAX) == 0) {
                az_running = 1;
                uart_puts("\r\n");
            }
            else {
                uart_puts("? >");
            }
        }
        else if(buffer[0] == 'C' && buffer[1] == '2') {
            uint16_t az = get_azimuth();
            uint16_t el = get_elevation();
            sprintf(uart_buffer, "+0%03u+0%03u\r", az, el);
            uart_puts(uart_buffer);
        }
        else if(buffer[0] == 'R' && size == 1) {
            rotor_start(ROTOR_RIGHT);
            uart_puts("\r\n");
        }
        else if(buffer[0] == 'L' && size == 1) {
            rotor_start(ROTOR_LEFT);
            uart_puts("\r\n");
        }
        else if(buffer[0] == 'U' && size == 1) {
            rotor_start(ROTOR_UP);
            uart_puts("\r\n");
        }
        else if(buffer[0] == 'D' && size == 1) {
            rotor_start(ROTOR_DOWN);
            uart_puts("\r\n");
        }
        else if(buffer[0] == 'A' && size == 1) {
            azimuth_stop();
            uart_puts("\r\n");
        }
        else if(buffer[0] == 'E' && size == 1) {
            elevation_stop();
            uart_puts("\r\n");
        }
        else if(buffer[0] == 'C' && size == 1) {
            uint16_t az = get_azimuth();
            sprintf(uart_buffer, "+0%03u\r", az);
            uart_puts(uart_buffer);
        }
        else if(buffer[0] == 'B' && size == 1) {
            uint16_t el = get_elevation();
            sprintf(uart_buffer, "+0%03u\r", el);
            uart_puts(uart_buffer);
        }
        else {
            uart_puts("? >");
        }

        size = 0;
        new_command = 0;
    }
    
    if(az_running) {
        int16_t az_diff = az_target - get_azimuth();

        if(az_diff > 0 && abs(az_diff) > TOLERANCE) {
            rotor_start(ROTOR_RIGHT);
        }
        else if(az_diff < 0 && abs(az_diff) > TOLERANCE) {
            rotor_start(ROTOR_LEFT);
        }
        else {
            azimuth_stop();
            az_running = 0;
        }
    }

    if(el_running) {
        int16_t el_diff = el_target - get_elevation();

        if(el_diff > 0 && abs(el_diff) > TOLERANCE) {
            rotor_start(ROTOR_UP);
        }
        else if(el_diff < 0 && abs(el_diff) > TOLERANCE) {
            rotor_start(ROTOR_DOWN);
        }
        else {
            elevation_stop();
            el_running = 0;
        }
    }

}

int main(void) {
    char display_buffer[16];
    uint8_t local_systick;

    LED_DDR |= (1<<LED);

    ROTOR_DDR |= (1<<ROTOR_UP) | (1<<ROTOR_DOWN) | (1<<ROTOR_LEFT) | (1<<ROTOR_RIGHT);

    uart_init();
    systick_init();
    adc_init(((1<<ROTOR_AZ) | (1<<ROTOR_EL)));

    lcd_init();
    lcd_clear();

    sei();
    while(1) {
        local_systick = systick_counter();

        if(local_systick == LCD_SLICE) {
            lcd_clear();
            sprintf(display_buffer, "AZ: %u", get_azimuth());
            lcd_string(display_buffer);
            lcd_setcursor(0, 2);
            sprintf(display_buffer, "EL: %u", get_elevation());
            lcd_string(display_buffer);
        }
        else {
            rotator_control();
        }
    }
}
