#ifndef FIRMWARE_DISPLAY_H
#define FIRMWARE_DISPLAY_H

#include <stdlib.h>
#include <inttypes.h>

#include "settings.h"

class Display
{
public:
    Display();
    Display(Settings *settings);

    void writeTime(uint8_t hour, uint8_t minute);
    void writeNumber(uint8_t n);
    void clear();
    void copy(volatile uint16_t * dst);

    void applyBrightness();

    uint16_t& operator[] (size_t i);

private:
    uint16_t matrix[10];
    Settings *settings;
};

#endif // FIRMWARE_DISPLAY_H