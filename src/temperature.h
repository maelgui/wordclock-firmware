#ifndef FIRMWARE_TEMPERATURE_H
#define FIRMWARE_TEMPERATURE_H

#define STRUCT_TEMPERATURE_MAGIC 0x3529

#include <RTClib.h>

struct Temperature temperature(DateTime *date, uint16_t value);

struct TemperatureDate {
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
};

struct Temperature {
    TemperatureDate date;
    uint16_t value;
    uint16_t magic;
};

#endif // FIRMWARE_TEMPERATURE_H
