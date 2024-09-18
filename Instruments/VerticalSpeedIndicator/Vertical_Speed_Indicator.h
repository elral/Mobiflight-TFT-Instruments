#pragma once

#include "Arduino.h"
#ifdef USE_VERTICAL_SPEED_INDICATOR
#include <TFT_eSPI.h>
#endif
namespace VerticalSpeedIndicator
{
    void init(TFT_eSPI *_tft, TFT_eSprite *sprites, uint8_t pin_backlight);
    void stop();
    void set(int16_t messageID, char *setPoint);
    void update();

};