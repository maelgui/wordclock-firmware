#ifndef FIRMWARE_WORDCLOCK_H
#define FIRMWARE_WORDCLOCK_H

#include <avr/io.h>
#include <avr/eeprom.h>


#include <RTClib.h>

#include "settings.h"
#include "display.h"
#include "messages.h"
#include "temperature.h"



class Wordclock
{
public:
    Wordclock();


    void initialize();

    bool isActive();
    void generate();
    void process(Message * msg, Message * response);
    void sendTemperatures();

    void writeTime(DateTime &now);
    void writeTemperature();
    void writeTimer(DateTime &now);

    void copy(volatile uint16_t * matrix);

private:
    // Settings
    Settings settings;

    // Function state
    DateTime timerEnd;

    // Devices
    Display display;
    RTC_DS3231 rtc;

};

#endif // FIRMWARE_WORDCLOCK_H