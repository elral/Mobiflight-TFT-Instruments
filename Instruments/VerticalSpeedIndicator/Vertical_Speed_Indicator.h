#pragma once

#include "Arduino.h"
#include <TFT_eSPI.h>

namespace VerticalSpeedIndicator
{
    void init(TFT_eSPI *_tft, TFT_eSprite *sprites, uint8_t pin_backlight);
    void stop();
    void set(int16_t messageID, char *setPoint);
    void update();

};