#include <avr/io.h>

#include "display.h"


Display::Display() { }

Display::Display(Settings *settings) : settings(settings) {}


void Display::writeTime(uint8_t hour, uint8_t minute) {
    if(minute >= 35)
        hour = (hour + 1) % 24;

    matrix[0] = 59; // il est
    matrix[6] = 63; // heures

    switch ((minute / 5) * 5)
    {
    case 0:
        break;
    case 5:
        matrix[9] = 60;
        break;
    case 10:
        matrix[7] = 896;
        break;
    case 15:
        matrix[7] = 96;
        matrix[9] = 992;
        break;
    case 20:
        matrix[8] = 31;
        break;
    case 25:
        matrix[8] = 31;
        matrix[9] = 60;
        break;
    case 30:
        matrix[7] = 96;
        matrix[8] = 480;
        break;
    case 35:
        matrix[7] = 31;
        matrix[8] = 31;
        matrix[9] = 60;
        break;
    case 40:
        matrix[7] = 31;
        matrix[8] = 31;
        break;
    case 45:
        matrix[7] = 31;
        matrix[9] = 3 + 992;
        break;
    case 50:
        matrix[7] = 31 + 896;
        break;
    case 55:
        matrix[7] = 31;
        matrix[9] = 60;
        break;

    default:
        break;
    }

    switch (hour % 12) {
    case 0:
        if (hour == 12) {
            matrix[6] = 960;
        }
        else {
            matrix[5] = 1008;
            matrix[6] = 0;
        }
        break;
    case 1:
        matrix[1] = 7;
        break;
    case 2:
        matrix[4] = 240;
        break;
    case 3:
        matrix[2] = 248;
        break;
    case 4:
        matrix[3] = 504;
        break;
    case 5:
        matrix[3] = 15;
        break;
    case 6:
        matrix[2] = 896;
        break;
    case 7:
        matrix[1] = 960;
        break;
    case 8:
        matrix[2] = 15;
        break;
    case 9:
        matrix[4] = 15;
        break;
    case 10:
        matrix[1] = 56;
        break;
    case 11:
        matrix[5] = 15;
        break;
    }
}

int numbers[10][10] = {
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

void Display::writeNumber(uint8_t n) {
    uint8_t u = n%10;
    uint8_t d = n/10;
    for(uint8_t i = 0; i<10; i++) {
        matrix[i] = 32*numbers[u][i] + numbers[d][i];
    }
}

void Display::clear() {
    for (int i = 0; i < 10; ++i) {
        matrix[i] = 0;
    }
}

void Display::copy(volatile uint16_t *dst) {
    switch (settings->rotation)
    {
    case Rotation::ROT_0:
        for (int k = 0; k < 10; ++k) {
            dst[k] = matrix[k];
        }
        break;
    
    case Rotation::ROT_90:
        for (uint8_t k = 0; k < 10; k++)
        {
            dst[k] = 0;
            for (uint8_t t = 0; t < 10; t++)
            {
                //cli();
                dst[k] |= ((matrix[t] >> k) & 1) << (10 - t - 1) ;
                //sei();
            }
        }
        break;

    case Rotation::ROT_180:
        for (uint8_t k = 0; k < 10; k++)
        {
            dst[10 - k - 1] = 0;
            for (uint8_t t = 0; t < 10; t++)
            {
                dst[10 - k - 1] |= ((matrix[k] >> t) & 1) << (10 - t -1);
            }
        }
        break;
    
    default:
        break;
    }
}

void Display::applyBrightness() {
    // Set duty cycle
    OCR2B = settings->brightness;
}

uint16_t& Display::operator[] (size_t i) {
    return matrix[i];
}