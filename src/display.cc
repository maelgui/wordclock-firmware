#include <avr/io.h>

#include "display.h"


static int numbers[10][10] = {
    {0,0,6,9,9,9,9,9,6,0},
    {0,0,4,4,4,4,4,4,4,0},
    {0,0,6,9,8,6,1,1,15,0},
    {0,0,7,8,8,6,8,8,7,0},
    {0,0,2,2,1,1,15,4,4,0},
    {0,0,15,1,1,7,8,8,7,0},
    {0,0,6,1,1,7,9,9,6,0},
    {0,0,15,8,8,4,4,2,2,0},
    {0,0,6,9,9,6,9,9,6,0},
    {0,0,6,9,9,14,8,8,8,0},
};

void display_apply_brightness(uint8_t brightness) {
    OCR2B = brightness;
}

void display_clear(display_t *display) {
    for (int i = 0; i < 10; ++i) {
        display->line[i] = 0;
    }
}

void display_write_time(display_t *display, display_time_t time) {
    display_clear(display);

    if (time & DISPLAY_WORD_IL)     display->line[0] |= 0b11;
    if (time & DISPLAY_WORD_EST)    display->line[0] |= 0b111000;
    if (time & DISPLAY_WORD_BABO)   display->line[0] |= 0b1111000000;
    if (time & DISPLAY_WORD_UNE)    display->line[1] |= 0b111;
    if (time & DISPLAY_WORD_DIX)    display->line[1] |= 0b111000;
    if (time & DISPLAY_WORD_SEPT)   display->line[1] |= 0b1111000000;
    if (time & DISPLAY_WORD_HUIT)   display->line[2] |= 0b1111;
    if (time & DISPLAY_WORD_TROIS)  display->line[2] |= 0b11111000;
    if (time & DISPLAY_WORD_SIX)    display->line[2] |= 0b1110000000;
    if (time & DISPLAY_WORD_CINQ)   display->line[3] |= 0b1111;
    if (time & DISPLAY_WORD_QUATRE) display->line[3] |= 0b111111000;
    if (time & DISPLAY_WORD_NEUF)   display->line[4] |= 0b1111;
    if (time & DISPLAY_WORD_DEUX)   display->line[4] |= 0b11110000;
    if (time & DISPLAY_WORD_ONZE)   display->line[5] |= 0b1111;
    if (time & DISPLAY_WORD_MINUIT) display->line[5] |= 0b1111110000;
    if (time & DISPLAY_WORD_HEURES) display->line[6] |= 0b111111;
    if (time & DISPLAY_WORD_MIDI)   display->line[6] |= 0b1111000000;
    if (time & DISPLAY_WORD_MOINS)  display->line[7] |= 0b11111;
    if (time & DISPLAY_WORD_ET)     display->line[7] |= 0b1100000;
    if (time & DISPLAY_WORD_DIX2)   display->line[7] |= 0b1110000000;
    if (time & DISPLAY_WORD_VINGT)  display->line[8] |= 0b11111;
    if (time & DISPLAY_WORD_DEMI)   display->line[8] |= 0b111100000;
    if (time & DISPLAY_WORD_LE)     display->line[9] |= 0b11;
    if (time & DISPLAY_WORD_CINQ2)  display->line[9] |= 0b111100;
    if (time & DISPLAY_WORD_QUART)  display->line[9] |= 0b1111100000;
}

void display_write_number(display_t *display, uint8_t number) {
    uint8_t u = number % 10;
    uint8_t d = (number / 10) % 10;
    for(uint8_t i = 0; i < 10; i++) {
        display->line[i] = 32*numbers[u][i] + numbers[d][i];
    }
}

void display_copy(display_t *display, volatile uint16_t *dst, settings_rotation_t rotation) {
    switch (rotation)
    {
    case SETTINGS_ROTATION_0:
        for (int k = 0; k < 10; ++k) {
            dst[k] = display->line[k];
        }
        break;
    
    case SETTINGS_ROTATION_90:
        for (uint8_t k = 0; k < 10; k++)
        {
            dst[k] = 0;
            for (uint8_t t = 0; t < 10; t++)
            {
                dst[k] |= ((display->line[t] >> k) & 1) << (10 - t - 1) ;
            }
        }
        break;

    case SETTINGS_ROTATION_180:
        for (uint8_t k = 0; k < 10; k++)
        {
            dst[10 - k - 1] = 0;
            for (uint8_t t = 0; t < 10; t++)
            {
                dst[10 - k - 1] |= ((display->line[k] >> t) & 1) << (10 - t -1);
            }
        }
        break;
    
    default:
        break;
    }

}
