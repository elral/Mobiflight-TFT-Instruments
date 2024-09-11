#include "Heading_Indicator.h"

namespace HeadingIndicator
{
#include "./include/logo.h"
#include "./include/main_gauge.h"
#include "./include/compass_rose.h"
#include "./include/hdg_bug.h"

#define PANEL_COLOR 0x7BEE

    TFT_eSPI    *tft;
    TFT_eSprite *mainGaugeSpr;
    TFT_eSprite *compassRoseSpr;
    TFT_eSprite *hdgBugSpr;
    // Pointers to start of Sprites in RAM (these are then "image" pointers)
    uint16_t *mainGaugeSprPtr;

    // Function declarations
    void  setHeading(float value);
    void  setHeadingBug(float value);
    void  setInstrumentBrightnessRatio(float ratio);
    void  setScreenRotation(int rotation);
    void  setPowerSaveMode(bool enabled);
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  drawInstrument();

    // Variables
    int      data;
    float    heading                   = 0; // Heading value from sim
    float    hdgBug                    = 0; // heading bug value from sim
    float    instrumentBrightnessRatio = 1;
    int      instrumentBrightness      = 255;
    int      prevScreenRotation        = 3;
    bool     powerSaveFlag             = false;
    uint32_t startLogoMillis           = 0;
    uint8_t  backlight_pin             = 0;
    uint16_t instrumentX0              = 80;
    uint16_t instrumentY0              = 0;
    bool     showLogo                  = true;

    void init(TFT_eSPI *_tft, TFT_eSprite *sprites, uint8_t pin_backlight)
    {
        backlight_pin = pin_backlight;
        pinMode(backlight_pin, OUTPUT);
        digitalWrite(backlight_pin, HIGH);

        tft = _tft;
        tft->setRotation(3);
        tft->setPivot(320, 160);
        tft->setSwapBytes(true);
        tft->fillScreen(TFT_BLACK);
        tft->startWrite(); // TFT chip select held low permanently

        mainGaugeSpr   = &sprites[0];
        compassRoseSpr = &sprites[1];
        hdgBugSpr      = &sprites[2];

        mainGaugeSprPtr = (uint16_t *)mainGaugeSpr->createSprite(320, 320);
        mainGaugeSpr->setSwapBytes(true);
        mainGaugeSpr->fillSprite(TFT_BLACK);
        mainGaugeSpr->pushImage(0, 0, 320, 320, main_gauge);
        mainGaugeSpr->setPivot(160, 160);

        compassRoseSpr->createSprite(320, 320);
        compassRoseSpr->setSwapBytes(true);
        compassRoseSpr->fillSprite(TFT_BLACK);
        compassRoseSpr->pushImage(0, 0, 320, 320, compass_rose);
        compassRoseSpr->setPivot(160, 160);

        hdgBugSpr->createSprite(hdg_bug_width, hdg_bug_height);
        hdgBugSpr->setSwapBytes(true);
        hdgBugSpr->fillSprite(TFT_BLACK);
        hdgBugSpr->pushImage(0, 0, hdg_bug_width, hdg_bug_height, hdg_bug);
        hdgBugSpr->setPivot(hdg_bug_width / 2.0, 153);

        tft->pushImage(160, 80, 160, 160, logo);
        startLogoMillis = millis();
        tft->setSwapBytes(false);
    }

    void stop()
    {
        tft->endWrite();
        mainGaugeSpr->deleteSprite();
        compassRoseSpr->deleteSprite();
        hdgBugSpr->deleteSprite();
    }

    void set(int16_t messageID, char *setPoint)
    {
        /* **********************************************************************************
            Each messageID has it's own value
            check for the messageID and define what to do.
            Important Remark!
            MessageID == -2 will be send from the board when PowerSavingMode is set
                Message will be "0" for leaving and "1" for entering PowerSavingMode
            MessageID == -1 will be send from the connector when Connector stops running
            Put in your code to enter this mode (e.g. clear a display)

        ********************************************************************************** */

        // do something according your messageID
        switch (messageID) {
        case -1:
            setPowerSaveMode(true);
            break;
        case -2:
            setPowerSaveMode((bool)atoi(setPoint));
            break;
        case 0:
            setHeading(atof(setPoint));
            break;
        case 1:
            setHeadingBug(atof(setPoint));
            break;
        case 2:
            setInstrumentBrightnessRatio(atof(setPoint));
            break;
        case 100:
            setScreenRotation(atoi(setPoint));
            break;
        default:
            break;
        }
    }

    void update()
    {
        drawInstrument();
    }

    void drawInstrument()
    {
        // show start up logo for 3 seconds
        if (millis() - startLogoMillis < 3000)
            return;

        mainGaugeSpr->pushImage(0, 0, 320, 320, main_gauge);
        compassRoseSpr->pushImage(0, 0, 320, 320, compass_rose);
        hdgBugSpr->pushRotated(compassRoseSpr, hdgBug, TFT_BLACK);
        compassRoseSpr->pushRotated(mainGaugeSpr, heading, TFT_BLACK);

        tft->pushImageDMA(instrumentX0, instrumentY0, 320, 320, mainGaugeSprPtr);
        compassRoseSpr->fillSprite(TFT_BLACK);
        mainGaugeSpr->fillSprite(TFT_BLACK);
    }

    void setHeading(float value)
    {
        heading = value * -1.0; // Direction is reversed compared to the sim
        drawInstrument();
    }

    void setHeadingBug(float value)
    {
        hdgBug = value;
        drawInstrument();
    }

    void setInstrumentBrightnessRatio(float ratio)
    {
        instrumentBrightnessRatio = ratio;
        instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
        analogWrite(backlight_pin, instrumentBrightness);
    }

    void setScreenRotation(int rotation)
    {
        if (rotation >= 0 && rotation <= 3) {
            prevScreenRotation = rotation;
            tft->dmaWait();
            tft->setRotation(rotation);
        }

        if (rotation == 1 || rotation == 3) {
            instrumentX0 = 80;
            instrumentY0 = 0;
        } else {
            instrumentX0 = 0;
            instrumentY0 = 80;
        }
        drawInstrument();
    }

    void setPowerSaveMode(bool enabled)
    {
        if (enabled) {
            digitalWrite(backlight_pin, LOW);
            tft->fillScreen(TFT_BLACK);
            powerSaveFlag = true;
        } else {
            analogWrite(backlight_pin, instrumentBrightness);
            powerSaveFlag = false;
        }
    }

    // Scale function
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

}