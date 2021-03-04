#include "DHT22.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include "../UART/uart.h"

volatile uint64_t timer0_overflow_count = 0;

// uint64_t micros() {
//     return((timer0_overflow_count << 8) + TCNT0)*(64/16);
// }

uint64_t micros() {
    uint64_t m;
    uint8_t oldSREG = SREG, t;
     
    cli();
    m = timer0_overflow_count;
    t = TCNT0;
 
    if ((TIFR0 & _BV(TOV0)) && (t < 255))
        m++;
 
    SREG = oldSREG;
     
    return ((m << 8) + t) * (64 / (F_CPU/1000000));
}


namespace {
namespace Trigger {
    const uint16_t low = 2000;
    const uint16_t high = 20;
}
namespace WakeUp {
    const uint16_t min = 115;
    const uint16_t max = 190;
}
namespace Bit {
    const uint16_t min = 60;
    const uint16_t threshold = 100;
    const uint16_t max = 145;
}
}

DHT22::DHT22(int pin):
    _pin(pin),
    _state(Invalid),
    _result(None),
    _humidity(0),
    _temp(0)
{
    // --------------------
    // Timer 0
    // --------------------

    // Set the Timer Mode to CTC
    //TCCR0A |= (1 << WGM01);

    // Set the value that you want to count to
    //OCR0A = 128;

    // Enable interrupt
    TIMSK0 |= (1 << TOIE0);

    // Set no prescaler and start the timer
    TCCR0B |= (1 << CS01) | (1 << CS00);

    


}

ISR (TIMER0_OVF_vect) {
    timer0_overflow_count += 1;
}




bool DHT22::startRead() {
    if (_state == Invalid || _state == Done) {
        for (uint8_t i=0; i< sizeof(_data); i++) {
            _data[i] = 0;
        }
        _bit = 7;
        _byte = 0;

        // Trigger the sensor

        //prepare correct port and pin of DHT sensor
        DDRC |= (1 << _pin); //output
        PORTC |= (1 << _pin); //high
        _delay_ms(100);

        //begin send request
        PORTC &= ~(1 << _pin); //low
        _delay_us(Trigger::low);
        PORTC |= (1 << _pin); //high
        _delay_us(Trigger::high);
        DDRC &= ~(1 << _pin); //input


        _lastEdge = micros();
        _state = WakingUp;
        return true;
    }
    return false;
}

DHT22::Result DHT22::blockingRead() {
    startRead();
    while((_state != Done && _state != Invalid)){}
    return lastResult();
}

uint16_t DHT22::onFallingEdge() {
    unsigned long now = micros();
    uint16_t elapsed = now - _lastEdge;
    _lastEdge = now;
    switch(_state) {
    case WakingUp:
        if(elapsed >= WakeUp::min && elapsed <= WakeUp::max) {
            _state = Reading;
        } else {
            _result = WakeUpError;
            _state = Invalid;
        }
        break;
    case Reading:
        if(elapsed >= Bit::min && elapsed <= Bit::max) {
            if(elapsed > Bit::threshold)  _data[_byte] |= (1 << _bit);
            if (_bit == 0) {
                _bit = 7;
                if(++_byte == sizeof(_data)) {
                    uint8_t sum = _data[0] + _data[1] + _data[2] + _data[3];
                    if (_data[4] != sum) {
                        _result = ChecksumMismatch;
                        _state = Invalid;
                    } else {
                        _humidity = (_data[0] << 8) + _data[1];
                        _temp = ((_data[2] & 0x7F) << 8) +  _data[3];
                        if (_data[2] & 0x80) _temp = -_temp;
                        _result = Ok;
                        _state = Done;
                    }
                    break;
                }
            }
            else _bit--;
        } else {
            _result = DataError;
            _state = Invalid;
        }
        break;
    default:
        break;
    }
    return elapsed;
}

DHT22::Status DHT22::state() {
    return _state;
}

DHT22::Result DHT22::lastResult() {
    return _result;
}

int16_t DHT22::getTemp() {
    return _temp;
}

uint16_t DHT22::getHumidity() {
    return _humidity;
}
