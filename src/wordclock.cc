#include "wordclock.h"
#include "version.h"

#include <string.h>
#include <avr/eeprom.h>


// Constants
settings_t EEMEM NonVolatileSettings;
Temperature EEMEM NonVolatileTemperatures[120];
uint8_t EEMEM NonVolatileNextTemperature;

uint32_t minute_words[12] = {
    0,
    DISPLAY_WORD_CINQ2,
    DISPLAY_WORD_DIX2,
    DISPLAY_WORD_ET | DISPLAY_WORD_QUART,
    DISPLAY_WORD_VINGT,
    DISPLAY_WORD_VINGT | DISPLAY_WORD_CINQ2,
    DISPLAY_WORD_ET | DISPLAY_WORD_DEMI,
    DISPLAY_WORD_MOINS | DISPLAY_WORD_VINGT | DISPLAY_WORD_CINQ2,
    DISPLAY_WORD_MOINS | DISPLAY_WORD_VINGT,
    DISPLAY_WORD_MOINS | DISPLAY_WORD_LE | DISPLAY_WORD_QUART,
    DISPLAY_WORD_MOINS | DISPLAY_WORD_DIX2,
    DISPLAY_WORD_MOINS | DISPLAY_WORD_CINQ2,
};

uint32_t hour_words[12] = {
    0,
    DISPLAY_WORD_UNE | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_DEUX | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_TROIS | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_QUATRE | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_CINQ | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_SIX | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_SEPT | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_HUIT | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_NEUF | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_DIX | DISPLAY_WORD_HEURES,
    DISPLAY_WORD_ONZE | DISPLAY_WORD_HEURES,
};


void wordclock_write_time(display_t *display, DateTime now);
void wordclock_write_temperature(display_t *display, uint16_t temperature);
void wordclock_write_timer(display_t *display, DateTime now, DateTime end);
void send_temperatures();


void wordclock_initialise(wordclock_t *w) {
    settings_t *settings = &w->settings;
    eeprom_read_block(settings, &NonVolatileSettings, sizeof(settings_t));
    if (settings->magic != STRUCT_SETTINGS_MAGIC || settings->version != STRUCT_SETTINGS_VERSION) {
        *settings = { 
            .magic = STRUCT_SETTINGS_MAGIC,
            .version = STRUCT_SETTINGS_VERSION,
            .mode = SETTINGS_MODE_ON,
            .function = SETTINGS_FUNCTION_ALTERNATE,
            .rotation = SETTINGS_ROTATION_0,
            .lightThreshold = 100,
            .brightness = 255,
        };

        eeprom_update_block(settings, &NonVolatileSettings, sizeof(settings_t));
    }

    //display_apply_brightness(w->settings.brightness);
}

void wordclock_tick(wordclock_t *w, DateTime now) {
    w->now = now;
    if (now.minute() == 0 && now.second() == 0 && w->last_dht_read_ime > now - TimeSpan(120)) {
        uint8_t i = eeprom_read_byte(&NonVolatileNextTemperature);
        uint32_t tmp = eeprom_read_dword((uint32_t*) &NonVolatileTemperatures[(i-1) % 120]);
        if (DateTime(tmp).hour() != now.hour()) {
            Temperature t = { 
                .date = { .month = now.month(), .day = now.day(), .hour = now.hour(), .minute = now.minute() }, 
                .value = w->last_temperature_read_value,
                .magic = STRUCT_TEMPERATURE_MAGIC, 
            };
            eeprom_update_block(&t, &NonVolatileTemperatures[i], sizeof(Temperature));
            eeprom_update_byte(&NonVolatileNextTemperature, (i+1) % 120);
        }
    }

    if (w->settings.function == SETTINGS_FUNCTION_ALTERNATE) {
        w->actual_function = (now.minute() % 2 == 0) ? SETTINGS_FUNCTION_TEMPERATURE : SETTINGS_FUNCTION_HOUR;
    }
    else {
        w->actual_function = w->settings.function;
    }

}

void wordclock_draw(wordclock_t *w) {
    switch (w->actual_function) {
    case SETTINGS_FUNCTION_HOUR:
        wordclock_write_time(&w->display, w->now);
        break;
    case SETTINGS_FUNCTION_TEMPERATURE:
        wordclock_write_temperature(&w->display, w->last_temperature_read_value);
        break;
    case SETTINGS_FUNCTION_TIMER:
        wordclock_write_timer(&w->display, w->now, w->timer_end);
        break;
    }
}

bool wordclock_need_update(wordclock_t *w, volatile uint16_t *matrix) {
    uint16_t temp[10];
    display_copy(&w->display, temp, w->settings.rotation);

    bool need_update = false;
    for (uint8_t i = 0; i < 10; i++)
    {
        if (matrix[i] != temp[i])
            need_update = true;
    }

    return need_update;    
}


bool wordclock_screen_update(wordclock_t *w, volatile uint16_t *matrix) {
    bool is_active = false;
    if (w->settings.mode == SETTINGS_MODE_AMBIENT) {
        is_active = w->ambient_light > w->settings.lightThreshold;
    }
    else if (w->settings.mode == SETTINGS_MODE_ON) {
        is_active = true;
    }

    display_copy(&w->display, matrix, w->settings.rotation);
    
    return is_active;

}

void wordclock_write_time(display_t *display, DateTime now) {
    display_time_t time = 0;
    uint8_t minute = now.minute();
    uint8_t hour = now.hour();

    if(minute >= 35)
        hour = (hour + 1) % 24;

    time |= DISPLAY_WORD_IL | DISPLAY_WORD_EST;

    time |= minute_words[minute / 5];

    if (hour == 0)
        time |= DISPLAY_WORD_MINUIT;
    else if (hour == 12)
        time |= DISPLAY_WORD_MIDI;
    else
        time |= hour_words[hour % 12];

    display_write_time(display, time);
}


void wordclock_write_temperature(display_t *display, uint16_t temperature) {
    display_write_number(display, (temperature + 5) / 10);
    display->line[0] = 512;
}


void wordclock_write_timer(display_t *display, DateTime now, DateTime end) {
    TimeSpan remaining = end - now;

    display_write_number(display, remaining.seconds() > 0 ? remaining.seconds() : 0);
    for (uint8_t k = 0; k < remaining.minutes(); k++) {
        display->line[0] = display->line[0] * 2 + 1;
    }

    if (remaining.totalseconds() < 0 && remaining.totalseconds() % 2)
    {
        display_clear(display);
    }

}


void wordclock_process_message(wordclock_t *w, Message *msg, Message *res) {

    if (msg->length != 0) {
        bool settingsChanged = false;
        
        switch(msg->command) {
        case Command_MODE:
            w->settings.mode = (settings_mode_t) msg->message[0];
            settingsChanged = true;
            break;
        case Command_FUNCTION:
            w->settings.function = (settings_function_t) *msg->message;
            settingsChanged = true;
            break;
        case Command_TIMER:
            if (*msg->message == 0) {
                w->settings.function = SETTINGS_FUNCTION_HOUR;
            }
            else {
                w->timer_end = w->now + TimeSpan(msg->message[0] * 60 + msg->message[1]);
                w->settings.function = SETTINGS_FUNCTION_TIMER;
            }
            break;
        case Command_BRIGHTNESS:
            w->settings.brightness = *msg->message;
            display_apply_brightness(w->settings.brightness);
            settingsChanged = true;
            break;
        case Command_THRESHOLD:
            w->settings.lightThreshold = *msg->message;
            settingsChanged = true;
            break;
        case Command_ROTATION:
            w->settings.rotation = (settings_rotation_t) *msg->message;
            settingsChanged = true;
            break;
        default:
            break;
        }

        if (settingsChanged) {
            eeprom_update_block(&w->settings, &NonVolatileSettings, sizeof(settings_t));
        }

    }

    res->error = Error_NONE;
    res->command = msg->command;
    res->length = command_length[res->command];
    
    switch(msg->command) {
    case Command_VERSION:
        //res->message[0] = FIRMWARE_VERSION_MAJOR;
        break;
    case Command_MODE:
        res->message[0] = (uint8_t) w->settings.mode;
        break;
    case Command_FUNCTION:
        res->message[0] = (uint8_t) w->settings.function;
        break;
    case Command_TIME:
        res->message[0] = w->now.rawYear();
        res->message[1] = w->now.month();
        res->message[2] = w->now.day();
        res->message[3] = w->now.hour();
        res->message[4] = w->now.minute();
        res->message[5] = w->now.second();
        break;
    case Command_TEMPERATURE:
        memcpy(res->message, &w->last_temperature_read_value, 2);
        break;
    case Command_HUMIDITY:
        memcpy(res->message, &w->last_humidity_read_value, 2);
        break;
    case Command_LIGHT:
        res->message[0] = w->ambient_light;
        break;
    case Command_TEMPERATURES:
        send_temperatures();
        // TODO: response
        break;
    case Command_TIMER:
    {
        uint16_t remaining = (w->timer_end - w->now).totalseconds();
        memcpy(res->message, &remaining, 2);
    }
        break;
    case Command_BRIGHTNESS:
        res->message[0] = w->settings.brightness;
        break;
    case Command_ROTATION:
        res->message[0] = (uint8_t) w->settings.rotation;
        break;
    case Command_THRESHOLD:
        res->message[0] = w->settings.lightThreshold;
        break;
    default:
        res->error = Error_UNKNOWN_COMMAND;
        res->command = Command_ERROR;
        res->length = 0;
        break;
    }
}

void send_temperatures() {
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
