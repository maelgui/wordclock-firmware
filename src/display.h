#ifndef FIRMWARE_DISPLAY_H
#define FIRMWARE_DISPLAY_H

#include <stdint.h>

#include "settings.h"


#define DISPLAY_WORD_IL     (1UL << 0)
#define DISPLAY_WORD_EST    (1UL << 1)
#define DISPLAY_WORD_BABO   (1UL << 2)
#define DISPLAY_WORD_UNE    (1UL << 3)
#define DISPLAY_WORD_DIX    (1UL << 4)
#define DISPLAY_WORD_SEPT   (1UL << 5)
#define DISPLAY_WORD_HUIT   (1UL << 6)
#define DISPLAY_WORD_TROIS  (1UL << 7)
#define DISPLAY_WORD_SIX    (1UL << 8)
#define DISPLAY_WORD_CINQ   (1UL << 9)
#define DISPLAY_WORD_QUATRE (1UL << 10)
#define DISPLAY_WORD_NEUF   (1UL << 11)
#define DISPLAY_WORD_DEUX   (1UL << 12)
#define DISPLAY_WORD_ONZE   (1UL << 13)
#define DISPLAY_WORD_MINUIT (1UL << 14)
#define DISPLAY_WORD_HEURES (1UL << 15)
#define DISPLAY_WORD_MIDI   (1UL << 16)
#define DISPLAY_WORD_MOINS  (1UL << 17)
#define DISPLAY_WORD_ET     (1UL << 18)
#define DISPLAY_WORD_DIX2   (1UL << 19)
#define DISPLAY_WORD_VINGT  (1UL << 21)
#define DISPLAY_WORD_DEMI   (1UL << 22)
#define DISPLAY_WORD_LE     (1UL << 23)
#define DISPLAY_WORD_CINQ2  (1UL << 24)
#define DISPLAY_WORD_QUART  (1UL << 25)


typedef struct {
    uint16_t line[10];
} display_t;


// One bit = one word on the matrix (25 word in total)
typedef uint32_t display_time_t;


void display_apply_brightness(uint8_t brightness);

void display_clear(display_t *display);

void display_write_time(display_t *display, display_time_t time);
void display_write_number(display_t *display, uint8_t number);

void display_copy(display_t *display, volatile uint16_t *matrix, settings_rotation_t rotation);


#endif // FIRMWARE_DISPLAY_H