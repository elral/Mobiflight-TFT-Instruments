#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace TurnCoordinator
{
    void init(TFT_eSPI *_tft, TFT_eSprite *sprites);
    void stop();
    void set(int16_t messageID, char *setPoint);
    void update();
}