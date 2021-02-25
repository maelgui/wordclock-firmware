#include "wordclock.h"
#include "bluetooth.h"


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
            Temperature t = { .timestamp = now.unixtime(), .magic = STRUCT_TEMPERATURE_MAGIC, .value = rtc.getTemperatureBytes() };
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

    if (msg->has_value) {
        bool settingsChanges = false;
        
        switch(msg->key) {
        case Message_Key_MODE:
            settings.mode = static_cast<Mode>(msg->value);
            settingsChanges = true;
            break;
        case Message_Key_FUNCTION:
            settings.function = static_cast<Function>(msg->value);
            settingsChanges = true;
            break;
        case Message_Key_TIME:
            rtc.adjust(DateTime(msg->value));
            break;
        case Message_Key_TIMER:
            if (msg->value == 0) {
                settings.function = Function::HOUR;
            }
            else {
                timerEnd = rtc.now() + TimeSpan(msg->value);
                settings.function = Function::TIMER;
            }
            break;
        case Message_Key_BRIGHTNESS:
            settings.brightness = msg->value;
            display.applyBrightness();
            settingsChanges = true;
            break;
        case Message_Key_THRESHOLD:
            settings.lightThreshold = msg->value;
            settingsChanges = true;
            break;
        case Message_Key_ROTATION:
            settings.rotation = static_cast<Rotation>(msg->value);
            settingsChanges = true;
            break;
        default:
            break;
        }

        if (settingsChanges) {
            eeprom_update_block(&settings, &NonVolatileSettings, sizeof(Settings));
        }

    }

    response->key = msg->key;
    response->has_value = true;
    
    switch(msg->key) {
    case Message_Key_MODE:
        response->value = static_cast<uint64_t>(settings.mode);
        break;
    case Message_Key_FUNCTION:
        response->value = static_cast<uint64_t>(settings.function);
        break;
    case Message_Key_TIME:
        response->value = rtc.now().unixtime();
        break;
    case Message_Key_TEMPERATURE:
        response->value = ((uint64_t) rtc.now().unixtime() << 32) + rtc.getTemperatureBytes() ;
        break;
    case Message_Key_TEMPERATURES:
        sendTemperatures();

        response->key = Message_Key_OK;
        response->has_value = false;
        break;
    case Message_Key_TIMER:
        response->value = (timerEnd - rtc.now()).totalseconds();
        break;
    case Message_Key_BRIGHTNESS:
        response->value = settings.brightness;
        break;
    case Message_Key_ROTATION:
        response->value = static_cast<uint64_t>(settings.rotation);
        break;
    case Message_Key_THRESHOLD:
        response->value = settings.lightThreshold;
        break;
    default:
        response->has_value = false;
        response->key = Message_Key_ERROR;
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
                .has_value = true,
                .value = ((uint64_t) tmp.timestamp << 32) + tmp.value,
                .key = Message_Key_TEMPERATURES
            };
            send_msg(&response);
        }
    }
}
