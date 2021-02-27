#include <uart.h>
#include <util/delay.h>
#include <stdlib.h>

#include "messages.h"


bool receive_n(uint8_t *buffer, uint16_t length) {
    for (uint64_t i = 0; i < length; i++) {
        uint8_t t = 0;
        while (!uart_available()) {
            _delay_us(80);
            t++;
            if (t == 255)
                return false;
        }
        // TODO: check for UART_NO_DATA
        buffer[i] = uart_getc();
    }

    return true;
}

uint8_t receive_byte() {
    uint8_t buffer;
    receive_n(&buffer, 1);
    return buffer;
}

uint16_t receive_word() {
    uint16_t buffer;
    receive_n((uint8_t*)&buffer, 2);
    return buffer;
}

uint32_t receive_dword() {
    uint32_t buffer;
    receive_n((uint8_t*)&buffer, 4);
    return buffer;
}


void send_msg(Message * msg) {
    // Header
    uart_putc((uint8_t) msg->error);
    uart_putc((uint8_t) msg->command);
    uart_putc(msg->length);
    
    // Content
    for (uint64_t i = 0; i < msg->length; i++)
    {
        uart_putc(msg->message[i]);
    }
}


bool receive_msg(Message *msg) {
    // Header
    msg->error = (Error) receive_byte();
    msg->command = (Command) receive_byte();
    msg->length = receive_byte();

    // Content
    if (msg->length == 0) {
        return true;
    }

    if (msg->length != command_length[msg->command]) {
        return false;
    }

    return receive_n(msg->message, msg->length);
}
