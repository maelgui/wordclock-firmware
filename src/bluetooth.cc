#include <pb.h>
#include <pb_encode.h>
#include <uart.h>

#include "bluetooth.h"

bool send_msg(Message * msg) {
    uint8_t buffer[256];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    pb_encode_ex(&stream, &Message_msg, msg, PB_ENCODE_DELIMITED);

    for(uint16_t i = 0; i<stream.bytes_written; i++){
        uart_putc(buffer[i]);
    }

    return true;
}