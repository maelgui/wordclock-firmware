#include <avr/interrupt.h>
#include <avr/io.h>

#include "core.h"
#include "wordclock.h"


int main(void)
{
    setup();
    while (1) {
        loop();
    }

    return 0;
}

