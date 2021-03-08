#include "temperature.h"


temperature_t temperature(DateTime *date, uint16_t value) {
    return temperature_t {
        .value = { .month = date->month(), .day = date->day(), .hour = date->hour(), .minute = date->minute(), .value = value },
        .magic = STRUCT_TEMPERATURE_MAGIC, 
    };
}
