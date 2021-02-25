#include <avr/interrupt.h>
#include <avr/io.h>

#include "core.h"
#include "wordclock.h"


int main(void)
{
    Wordclock wordclock;

    setup(&wordclock);
    while (1) {
        loop(&wordclock);
    }

    return 0;
}

