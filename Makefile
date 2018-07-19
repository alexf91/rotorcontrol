PROJECT = rotorcontrol

MCU = atmega328p

CC = avr-gcc
AR = avr-ar
OBJCOPY = avr-objcopy
SIZE = avr-size

CFLAGS = -Os -std=gnu11 -mmcu=$(MCU) -Wall -I/usr/avr/include -DF_CPU=16000000UL
OBJECTS = rotorcontrol.o system.o uart.o lcd_routines.o adc.o

LDFLAGS = -mmcu=$(MCU)

PROG = avrdude
PROGFLAGS = -p $(MCU) -P usb -c avrispmkii -e

all: $(PROJECT).hex

%.o: %.c
	$(CC) $(CFLAGS) -c $<

$(PROJECT).elf: $(OBJECTS) $(LIBS)
	$(CC) -o $(PROJECT).elf $(OBJECTS) $(LDFLAGS)

$(PROJECT).hex: $(PROJECT).elf
	$(OBJCOPY) -R .eeprom -O ihex $< $@

install: $(PROJECT).hex
	$(PROG) $(PROGFLAGS) -U flash:w:$<

size: $(PROJECT).elf
	$(SIZE) --format=avr --mcu=$(MCU) $<

clean:
	rm -f $(OBJECTS) $(PROJECT).elf $(PROJECT).hex


.PHONY: size install clean
