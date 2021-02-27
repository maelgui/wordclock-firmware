#include "temperature.h"


struct Temperature temperature(DateTime *date, uint16_t value) {
    return Temperature {
        .date = { .month = date->month(), .day = date->day(), .hour = date->hour(), .minute = date->minute() },
        .value = value,
        .magic = STRUCT_TEMPERATURE_MAGIC, 
    };
}
