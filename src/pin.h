//
// Created by mguillos on 17/12/2019.
//

#ifndef FIRMWARE_PIN_H
#define FIRMWARE_PIN_H

#include <avr/io.h>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1


#define COL_CLOCK_DDR DDRB
#define COL_CLOCK_PORT PORTB
#define COL_CLOCK_PIN 2

#define COL_LATCH_DDR DDRB
#define COL_LATCH_PORT PORTB
#define COL_LATCH_PIN 1

#define COL_DATA_DDR DDRB
#define COL_DATA_PORT PORTB
#define COL_DATA_PIN 0


#define ROW_CLOCK_DDR DDRD
#define ROW_CLOCK_PORT PORTD
#define ROW_CLOCK_PIN 5

#define ROW_LATCH_DDR DDRD
#define ROW_LATCH_PORT PORTD
#define ROW_LATCH_PIN 6

#define ROW_DATA_DDR DDRD
#define ROW_DATA_PORT PORTD
#define ROW_DATA_PIN 7


#define PWM_DDR DDRD
#define PWM_PORT PORTD
#define PWM_PIN 3

#define OE_DDR DDRD
#define OE_PORT PORTD
#define OE_PIN 2


#define LUM_DDR DDRC
#define LUM_PORT PORTC
#define LUM_PIN 0


#endif //FIRMWARE_PIN_H
