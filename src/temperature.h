#ifndef FIRMWARE_TEMPERATURE_H
#define FIRMWARE_TEMPERATURE_H

static const uint16_t STRUCT_TEMPERATURE_MAGIC = 0x3528;

struct Temperature {
    uint32_t timestamp;
    uint16_t magic;
    uint16_t value;
};

#endif // FIRMWARE_TEMPERATURE_H