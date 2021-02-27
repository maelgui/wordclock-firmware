#include "wordclock.h"

#include <string.h>


Settings EEMEM NonVolatileSettings;
Temperature EEMEM NonVolatileTemperatures[120];
uint8_t EEMEM NonVolatileNextTemperature;


Wordclock::Wordclock() : display(&settings) {
    eeprom_read_block(&settings, &NonVolatileSettings, sizeof(Settings));
    if (settings.magic != STRUCT_SETTINGS_MAGIC || settings.version != STRUCT_SETTINGS_VERSION) {
        settings = { 
            .magic = STRUCT_SETTINGS_MAGIC,
            .version = STRUCT_SETTINGS_VERSION,
            .mode = Mode::ON,
            .function = Function::ALTERNATE,
            .rotation = Rotation::ROT_0,
            .lightThreshold = 100,
            .brightness = 255,
        };

        eeprom_update_block(&settings, &NonVolatileSettings, sizeof(Settings));
    }
}

bool Wordclock::isActive() {
    if (settings.mode == Mode::AMBIENT) {
        ADCSRA |= _BV(ADSC);

        // Wait until the ADSC bit has been cleared
        while(ADCSRA & _BV(ADSC));

        uint8_t ambient = ADCH;

        return ambient > settings.lightThreshold;
    }
    else if (settings.mode == Mode::ON) {
        return true;
    }
    
    return false;
}

void Wordclock::initialize() {
    if (! rtc.begin()) {
        while (1) {}
    }

    if (rtc.lostPower()) {
        rtc.adjust(DateTime(__DATE__, __TIME__));
    }

    display.applyBrightness();
}

void Wordclock::generate() {
    display.clear();

    DateTime now = rtc.now();

    if (now.minute() == 0 && now.second() == 0) {
        uint8_t i = eeprom_read_byte(&NonVolatileNextTemperature);
        uint32_t tmp = eeprom_read_dword((uint32_t*) &NonVolatileTemperatures[(i-1) % 120]);
        if (DateTime(tmp).hour() != now.hour()) {
            Temperature t = { 
                .date = { .month = now.month(), .day = now.day(), .hour = now.hour(), .minute = now.minute() }, 
                .value = rtc.getTemperatureBytes(),
                .magic = STRUCT_TEMPERATURE_MAGIC, 
            };
            eeprom_update_block(&t, &NonVolatileTemperatures[i], sizeof(Temperature));
            eeprom_update_byte(&NonVolatileNextTemperature, (i+1) % 120);
        }
    }

    switch (settings.function) {
    case Function::HOUR:
        writeTime(now);
        break;
    case Function::TEMPERATURE:
        writeTemperature();
        break;
    case Function::TIMER:
        writeTimer(now);
        break;
    case Function::ALTERNATE:
        if (now.minute() % 2 == 0) {
            writeTime(now);
        }
        else {
            writeTemperature();
        }
    }
}

void Wordclock::writeTime(DateTime &now) {
    display.writeTime(now.hour(), now.minute());
}

void Wordclock::writeTemperature() {
    display.writeNumber(rtc.getTemperature());
    display[0] = 512;
}

void Wordclock::writeTimer(DateTime &now) {
    TimeSpan remaining = timerEnd - now;

    display.writeNumber(remaining.seconds() > 0 ? remaining.seconds() : 0);
    for (uint8_t k = 0; k < remaining.minutes(); k++) {
        display[0] = display[0] * 2 + 1;
    }

    if (remaining.totalseconds() < 0 && remaining.totalseconds() % 2)
    {
        display.clear();
    }
}

void Wordclock::copy(volatile uint16_t * matrix) {
    display.copy(matrix);
}

void Wordclock::process(Message *msg, Message *response) {

    DateTime now = rtc.now();

    if (msg->length != 0) {
        bool settingsChanged = false;
        
        switch(msg->command) {
        case Command_MODE:
            settings.mode = (Mode) msg->message[0];
            settingsChanged = true;
            break;
        case Command_FUNCTION:
            settings.function = (Function) *msg->message;
            settingsChanged = true;
            break;
        case Command_TIME:
            rtc.adjust(DateTime(
                msg->message[0],
                msg->message[1],
                msg->message[2],
                msg->message[3],
                msg->message[4],
                msg->message[5]
            ));
            break;
        case Command_TIMER:
            if (*msg->message == 0) {
                settings.function = Function::HOUR;
            }
            else {
                timerEnd = rtc.now() + TimeSpan(*msg->message);
                settings.function = Function::TIMER;
            }
            break;
        case Command_BRIGHTNESS:
            settings.brightness = *msg->message;
            display.applyBrightness();
            settingsChanged = true;
            break;
        case Command_THRESHOLD:
            settings.lightThreshold = *msg->message;
            settingsChanged = true;
            break;
        case Command_ROTATION:
            settings.rotation = (Rotation) *msg->message;
            settingsChanged = true;
            break;
        default:
            break;
        }

        if (settingsChanged) {
            eeprom_update_block(&settings, &NonVolatileSettings, sizeof(Settings));
        }

    }

    response->command = msg->command;
    response->length = command_length[response->command];
    
    switch(msg->command) {
    case Command_MODE:
        response->message[0] = static_cast<uint8_t>(settings.mode);
        break;
    case Command_FUNCTION:
        response->message[0] = (uint8_t) settings.function;
        break;
    case Command_TIME:
        response->message[0] = now.rawYear();
        response->message[1] = now.month();
        response->message[2] = now.day();
        response->message[3] = now.hour();
        response->message[4] = now.minute();
        response->message[5] = now.second();
        break;
    case Command_TEMPERATURE:
    {
        Temperature t = temperature(&now, rtc.getTemperatureBytes());
        memcpy(response->message, &t, 6);
    }
        break;
    case Command_TEMPERATURES:
        sendTemperatures();
        // TODO: response
        break;
    case Command_TIMER:
    {
        uint16_t remaining = (timerEnd - rtc.now()).totalseconds();
        memcpy(response->message, &remaining, 2);
    }
        break;
    case Command_BRIGHTNESS:
        response->message[0] = settings.brightness;
        break;
    case Command_ROTATION:
        response->message[0] = (uint8_t) settings.rotation;
        break;
    case Command_THRESHOLD:
        response->message[0] = settings.lightThreshold;
        break;
    default:
        response->error = Error_UNKNOWN_COMMAND;
        break;
    }
}

void Wordclock::sendTemperatures() {
    for (uint8_t i = 0; i < 120; i++)
    {
        Temperature tmp;
        eeprom_read_block(&tmp, &NonVolatileTemperatures[i], sizeof(Temperature));
        if (tmp.magic == STRUCT_TEMPERATURE_MAGIC) {
            Message response = { 
                .error = Error_NONE,
                .command = Command_TEMPERATURES,
                .length = sizeof(Temperature),
            };
            memcpy(response.message, &tmp, 6);
            send_msg(&response);
        }
    }
}
