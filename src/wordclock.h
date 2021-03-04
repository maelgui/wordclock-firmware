#ifndef FIRMWARE_WORDCLOCK_H
#define FIRMWARE_WORDCLOCK_H



#include <RTClib.h>

#include "settings.h"
#include "display.h"
#include "messages.h"
#include "temperature.h"

typedef struct {
    // Settings
    settings_t settings;
    // State
    DateTime now;
    settings_function_t actual_function;
    DateTime timer_end;
    // Sensors
    DateTime last_dht_read_ime;
    uint16_t last_temperature_read_value;
    uint16_t last_humidity_read_value;
    uint8_t ambient_light;
    // Display
    display_t display;
    bool is_active;
    bool force_update;
} wordclock_t;


void wordclock_initialise(wordclock_t *w);
void wordclock_tick(wordclock_t *w, DateTime now);

bool wordclock_need_update(wordclock_t *w, volatile uint16_t *matrix);
void wordclock_screen_update(wordclock_t *w, volatile uint16_t *matrix);

void wordclock_process_message(wordclock_t *w, Message *msg, Message *res);


#endif // FIRMWARE_WORDCLOCK_H