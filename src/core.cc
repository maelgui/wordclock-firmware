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
#include "messages.h"

#include <RTClib.h>
#include <uart.h>
#include <DHT22.h>


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

// Wordclock global
wordclock_t clock;
// Sensors
RTC_DS3231 rtc;
DHT22 dht(PC1);



void setup() {

    // --------------------
    // Matrix pin configuration
    // --------------------

    // set col pin output mode
    COL_CLOCK_DDR |= _BV(COL_CLOCK_PIN);
    COL_LATCH_DDR |= _BV(COL_LATCH_PIN);
    COL_DATA_DDR |= _BV(COL_DATA_PIN);

    // set row pin output mode
    ROW_CLOCK_DDR |= _BV(ROW_CLOCK_PIN);
    ROW_LATCH_DDR |= _BV(ROW_LATCH_PIN);
    ROW_DATA_DDR |= _BV(ROW_DATA_PIN);

    OE_DDR |= _BV(OE_PIN);


    // --------------------
    // Luminosity sensor configuration
    // --------------------

    LUM_DDR &= ~_BV(LUM_PIN);
    ADMUX |= _BV(REFS0) | _BV(ADLAR);
    ADCSRA |= _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);


    // --------------------
    // PWM (Matrix luminosity)
    // --------------------

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


    // --------------------
    // Timer interrupt
    // --------------------

    // Interrupt
    TCNT1 = 0; //start value
    //OCR1A = 8000;
    OCR1A = 24000;
    TCCR1B |= _BV(WGM12) | _BV(CS10); // prescaler + CTC mode
    TIMSK1 |= _BV(OCIE1A);


    // --------------------
    // Pin change interrupt
    // --------------------

    // set PCINT9 to trigger an interrupt on state change
    PCICR |= (1 << PCIE1);
    // set PCIE1 to enable PCMSK1 scan
    PCMSK1 |= (1 << PCINT9);


    // --------------------
    // Enable interrupt
    // --------------------

    sei();


    // --------------------
    // Device initialisation
    // --------------------

    uart_init( UART_BAUD_SELECT(BAUD,F_CPU) ); 

    if (! rtc.begin()) {
        while (1) {}
    }

    if (rtc.lostPower()) {
        rtc.adjust(DateTime(__DATE__, __TIME__));
    }


    // --------------------
    // Wordclock start
    // --------------------

    wordclock_initialise(&clock);
}




ISR (TIMER1_COMPA_vect, ISR_NOBLOCK) {
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

ISR (PCINT1_vect)
{
    if (!(PINC & (1<<PC1))) {
        dht.onFallingEdge();
    }  
}


void loop() {

    // --------------------
    // Process incoming message
    // --------------------

    if (uart_available()) {
        Message msg, res;

        bool status = receive_msg(&msg);

        if (status) {
            if (msg.command == Command_TIME) {
                rtc.adjust(DateTime(
                    msg.message[0],
                    msg.message[1],
                    msg.message[2],
                    msg.message[3],
                    msg.message[4],
                    msg.message[5]
                ));
                res.error = Error_NONE;
                res.command = Command_TIME;
                res.length = 0;
            }
            else {
                wordclock_process_message(&clock, &msg, &res);
            }
        }
        else {
            res.error = Error_BAD_PACKET;
        }

        send_msg(&res);
    }    


    // --------------------
    // Wordclock tick and draw
    // --------------------

    wordclock_tick(&clock, rtc.now());
    screen.active = clock.is_active;
    if (wordclock_need_update(&clock, screen.matrix))
        wordclock_screen_update(&clock, screen.matrix);


    // --------------------
    // Light Sensor
    // --------------------

    // Start reading
    ADCSRA |= _BV(ADSC);
    // Wait until the ADSC bit has been cleared
    while(ADCSRA & _BV(ADSC));
    clock.ambient_light = ADCH;


    // --------------------
    // DHT22 sensor
    // --------------------

    if (dht.state() == DHT22::Done && dht.lastResult() == DHT22::Ok) {
        clock.last_dht_read_time = rtc.now();
        clock.last_temperature_read_value = dht.getTemp();
        clock.last_humidity_read_value = dht.getHumidity();
    }
    else if ((dht.state() == DHT22::Done || dht.state() == DHT22::Invalid)) {
        dht.startRead();
    }
    else if (clock.last_dht_read_time > rtc.now() - TimeSpan(DHT_VALIDITY_LIMIT)) {
        clock.last_dht_read_time = rtc.now();
        clock.last_temperature_read_value = rtc.getTemperature() * 10;
        clock.last_humidity_read_value = -1;
    }

}
