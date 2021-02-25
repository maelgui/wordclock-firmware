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
#include "messages.pb.h"
#include "wordclock.h"
//#include "version.h"
#include "bluetooth.h"

#include <RTClib.h>
#include <uart.h>
#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>


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

uint64_t uart_getvarint() {
    uint64_t result = 0;
    uint8_t i = 0;
    while (true) {
        while (!uart_available()) {
            _delay_us(80);
        }
        uint8_t c = uart_getc();
        result += (c & 0x7F) << (i*7);
        i++;
        if ((c & 0x80) == 0) {
            break;
        }
    }

    return result;
}

bool uart_getmessage(uint64_t length, Message *msg) {
    uint8_t buffer[256];

    for (uint64_t i = 0; i < length; i++) {
        while (!uart_available()) {
            _delay_us(80);
        }
        buffer[i] = uart_getc();
    }

    pb_istream_t stream = pb_istream_from_buffer(buffer, length);
    bool status = pb_decode(&stream, &Message_msg, msg);

    return status;
}

void loop(Wordclock *wordclock) {

    if (uart_available()) {
        Message msg = Message_init_zero;

        uint64_t length = uart_getvarint();

        bool status = uart_getmessage(length, &msg);

        Message response = Message_init_zero;

        if (status) {
            wordclock->process(&msg, &response);
        }
        else {
            response.key = Message_Key_ERROR;
            response.has_value = false;
        }

        send_msg(&response);
    }    

    wordclock->generate();
    screen.active = wordclock->isActive();
    wordclock->copy(screen.matrix);


    _delay_ms(300);
}

