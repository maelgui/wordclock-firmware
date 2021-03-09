#ifndef FIRMWARE_TEMPERATURE_H
#define FIRMWARE_TEMPERATURE_H

#define STRUCT_TEMPERATURE_MAGIC 0x3f26

#include <RTClib.h>


typedef struct {
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint16_t value;
} temperature_value_t;

typedef struct {
    temperature_value_t value;
    uint16_t magic;
} temperature_t;


temperature_t temperature(DateTime *date, uint16_t value);


#endif // FIRMWARE_TEMPERATURE_H
