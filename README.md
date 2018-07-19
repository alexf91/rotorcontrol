# Controller for Yaesu G-5500 Antenna Rotator

The controller uses a Arduino Nano board with a LCD display and several inputs
and outputs connected to the manual control unit of the rotator. It essentially
replaces the Yaesu GS-232 and implements a subset of its commands.

The implemented commands are sufficient for operation using `rotctld`. It
should work with most amateur radio related projects for Linux.

## Setup

Connect the display and the control unit to the Microcontroller and set the
pins in `config.h` and `lcd.h`.

## Usage

```
rotctl -m601 -s 9600 -r /dev/ttyUSB0
```
