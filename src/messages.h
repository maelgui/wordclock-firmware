#ifndef FIRMWARE_MESSAGES_H
#define FIRMWARE_MESSAGES_H

#include <stdint.h>

#define MAX_MESSAGE_LENGTH 16


enum Command  {
    Command_VERSION,
    // RTC
    Command_TIME,
    Command_TEMPERATURE,
    Command_TEMPERATURES,
    // Function related
    Command_TIMER,
    // Settings
    Command_MODE,
    Command_FUNCTION,
    Command_BRIGHTNESS,
    Command_THRESHOLD,
    Command_ROTATION,

    Command_NONE,
};


const uint16_t command_length [] = {
    [Command_VERSION] = 0,
    // RTC
    [Command_TIME] = 6,
    [Command_TEMPERATURE] = 8,
    [Command_TEMPERATURES] = 8,
    // Function related
    [Command_TIMER] = 4,
    // Settings
    [Command_MODE] = 1,
    [Command_FUNCTION] = 1,
    [Command_BRIGHTNESS] = 1,
    [Command_THRESHOLD] = 1,
    [Command_ROTATION] = 1,
};


enum Error {
    Error_NONE,
    Error_BAD_PACKET,
    Error_UNKNOWN_COMMAND,
};

struct Message {
    Error error;
    Command command;
    uint8_t length;
    uint8_t message[MAX_MESSAGE_LENGTH];
};

void send_msg(Message * msg);
bool receive_msg(Message *msg);

#endif //FIRMWARE_MESSAGES_H
