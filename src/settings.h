#ifndef FIRMWARE_SETTINGS_H
#define FIRMWARE_SETTINGS_H


enum class Function : uint8_t {
    HOUR, TEMPERATURE, TIMER, ALTERNATE
};

enum class Mode : uint8_t {
    OFF, ON, TIME, AMBIENT
};

enum class Rotation : uint8_t{
    ROT_0,
    ROT_90,
    ROT_180,
    ROT_270,
};


static const uint32_t STRUCT_SETTINGS_MAGIC = 0x362f2cfe;
static const uint8_t STRUCT_SETTINGS_VERSION = 2;

struct Settings {
    // EEPROM related
    uint32_t magic;
    uint8_t version;
    // Settings
    Mode mode;
    Function function;
    Rotation rotation;
    uint8_t lightThreshold;
    uint8_t brightness;
};

#endif // FIRMWARE_SETTINGS_H