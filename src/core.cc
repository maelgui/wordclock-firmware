#define BAUD 9600UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "pin.h"
#include "core.h"
#include "wordclock.h"
//#include "version.h"
#include "messages.h"

#include <RTClib.h>
#include <uart.h>


struct Screen {
    bool active;
    uint16_t matrix[10];
    uint8_t i;
};

volatile Screen screen = { 
    .active = true,
    .matrix = {1},
    .i = 0,
};

void setup(Wordclock *wordclock) {
    // set col pin output mode
    COL_CLOCK_DDR |= _BV(COL_CLOCK_PIN);
    COL_LATCH_DDR |= _BV(COL_LATCH_PIN);
    COL_DATA_DDR |= _BV(COL_DATA_PIN);

    // set row pin output mode
    ROW_CLOCK_DDR |= _BV(ROW_CLOCK_PIN);
    ROW_LATCH_DDR |= _BV(ROW_LATCH_PIN);
    ROW_DATA_DDR |= _BV(ROW_DATA_PIN);

    OE_DDR |= _BV(OE_PIN);

    LUM_DDR &= ~_BV(LUM_PIN);
    ADMUX |= _BV(REFS0) | _BV(ADLAR);
    ADCSRA |= _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);

    // PWM
    //PWM_PORT &= ~_BV(PWM_PIN);
    // Set PWM pin as output
    PWM_DDR |= _BV(PWM_PIN);
    // Set duty cycle
    OCR2B = 255;
    // Set inverted mode
    TCCR2A |= _BV(COM2B1) | _BV(COM2B0);
    // Set fast PWM Mode
    TCCR2A |= _BV(WGM21) | _BV(WGM20);

    // Enable counter with no prescaler
    TCCR2B |= _BV(CS20);

    // Interrupt
    TCNT1 = 0; //start value
    //OCR1A = 8000;
    OCR1A = 16000;
    TCCR1B |= _BV(WGM12) | _BV(CS10); // prescaler + CTC mode
    TIMSK1 |= _BV(OCIE1A);
    sei();


    _delay_ms(3000); // wait for console opening

    uart_init( UART_BAUD_SELECT(BAUD,F_CPU) ); 

    wordclock->initialize();
}



ISR (TIMER1_COMPA_vect) {
    // Désactivation de l'affichage
    OE_PORT |= _BV(OE_PIN);

    if (screen.active) {
        // Change lines
        ROW_LATCH_PORT &= ~_BV(ROW_LATCH_PIN);
        ROW_CLOCK_PORT &= ~_BV(ROW_CLOCK_PIN);
        if (screen.i == 0) {
            ROW_DATA_PORT |= _BV(ROW_DATA_PIN);
        } else {
            ROW_DATA_PORT &= ~_BV(ROW_DATA_PIN);
        }
        ROW_CLOCK_PORT |= _BV(ROW_CLOCK_PIN);
        ROW_LATCH_PORT |= _BV(ROW_LATCH_PIN);

        // Print matrix
        COL_LATCH_PORT &= ~_BV(COL_LATCH_PIN);
        for (int8_t j = 9; j >= 0; j--) {
            COL_CLOCK_PORT &= ~_BV(COL_CLOCK_PIN);
            if (screen.matrix[screen.i] & 0x01 << j) {
                COL_DATA_PORT |= _BV(COL_DATA_PIN);
            } else {
                COL_DATA_PORT &= ~_BV(COL_DATA_PIN);
            }
            COL_CLOCK_PORT |= _BV(COL_CLOCK_PIN);
        }
        COL_LATCH_PORT |= _BV(COL_LATCH_PIN);

        // Réactivation de l'affichage
        OE_PORT &= ~_BV(OE_PIN);

        // Increment line number
        screen.i = (screen.i == 9) ? 0 : screen.i + 1;
    }
}


void loop(Wordclock *wordclock) {

    if (uart_available()) {
        Message message, response;

        bool status = receive_msg(&message);

        if (status) {
            wordclock->process(&message, &response);
        }
        else {
            response.error = Error_BAD_PACKET;
        }

        send_msg(&response);
    }    

    wordclock->generate();
    screen.active = wordclock->isActive();
    wordclock->copy(screen.matrix);


    _delay_ms(300);
}

