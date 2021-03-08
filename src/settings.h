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


#define STRUCT_SETTINGS_MAGIC 0x371f2cfa

// Number of second DHT values are valid
#define DHT_VALIDITY_LIMIT 120


typedef enum {
    SETTINGS_MODE_ERROR,
    SETTINGS_MODE_OFF,
    SETTINGS_MODE_ON,
    SETTINGS_MODE_TIME,
    SETTINGS_MODE_AMBIENT,
} settings_mode_t;


typedef enum {
    SETTINGS_FUNCTION_ERROR,
    SETTINGS_FUNCTION_HOUR,
    SETTINGS_FUNCTION_TEMPERATURE,
    SETTINGS_FUNCTION_TIMER,
    SETTINGS_FUNCTION_ALTERNATE,
} settings_function_t;


typedef enum {
    SETTINGS_ROTATION_0,
    SETTINGS_ROTATION_90,
    SETTINGS_ROTATION_180,
    SETTINGS_ROTATION_270,
} settings_rotation_t;


typedef struct {
    // EEPROM related
    uint32_t magic;
    // Settings
    settings_mode_t mode;
    settings_function_t function;
    settings_rotation_t rotation;
    uint8_t lightThreshold;
    uint8_t brightness;
} settings_t;


#endif // FIRMWARE_SETTINGS_H