#ifndef LEDMATRIX_H
#define LEDMATRIX_H
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define NEUTRAL 0
#define SMILE 1
#define FROWN 2
#define HAND 3
#define FORK 4
#define TOOTHBRUSH 5

void ShowLed(int n, Adafruit_8x8matrix &mat);


static const uint8_t PROGMEM
  empty[] =
        {B00000000,
         B00000000,
         B00000000,
         B00000000,
         B00000000,
         B00000000,
         B00000000,
         B00000000},
    smile_bmp[] =
        {B00111100,
         B01000010,
         B10100101,
         B10000001,
         B10100101,
         B10011001,
         B01000010,
         B00111100},
    neutral_bmp[] =
        {B00111100,
         B01000010,
         B10100101,
         B10000001,
         B10111101,
         B10000001,
         B01000010,
         B00111100},
    frown_bmp[] =
        {B00111100,
         B01000010,
         B10100101,
         B10000001,
         B10011001,
         B10100101,
         B01000010,
         B00111100},
    hand_bmp[] =
        {B00011000,
         B00100100,
         B01000010,
         B10011001,
         B00011000,
         B11111111,
         B01000000,
         B01111110},
    fork_bmp[] =
        {B01010101,
         B01010101,
         B01010101,
         B00111110,
         B00001000,
         B00001000,
         B00001000,
         B00001000},
    toothbrush_bmp[] =
        {B00000000,
         B00000000,
         B00000000,
         B00011111,
         B00010101,
         B11111111,
         B00000000,
         B00000000};

void ShowLed(int n, Adafruit_8x8matrix &mat)
{
    switch (n)
    {
    case -1:
        mat.clear();
        mat.drawBitmap(0, 0, empty, 8, 8, LED_ON);
        mat.writeDisplay();
        break;
    case 0:
        mat.clear();
        mat.drawBitmap(0, 0, neutral_bmp, 8, 8, LED_ON);
        mat.writeDisplay();
        break;
    case 1:
        mat.clear();
        mat.drawBitmap(0, 0, smile_bmp, 8, 8, LED_ON);
        mat.writeDisplay();
        break;
    case 2:
        mat.clear();
        mat.drawBitmap(0, 0, frown_bmp, 8, 8, LED_ON);
        mat.writeDisplay();
        break;
    case 3:
        mat.clear();
        mat.drawBitmap(0, 0, hand_bmp, 8, 8, LED_ON);
        mat.writeDisplay();
        break;
    case 4:
        mat.clear();
        mat.drawBitmap(0, 0, fork_bmp, 8, 8, LED_ON);
        mat.writeDisplay();
        break;
    case 5:
        mat.clear();
        mat.drawBitmap(0, 0, toothbrush_bmp, 8, 8, LED_ON);
        mat.writeDisplay();
        break;
    default:
        break;
    }
}


#endif
