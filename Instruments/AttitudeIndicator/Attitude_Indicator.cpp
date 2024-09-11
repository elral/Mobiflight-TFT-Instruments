#include "Attitude_Indicator.h"

namespace AttitudeIndicator
{
#include "./include/background.h"
#include "./include/bezel.h"
#include "./include/roll_scale.h"
#include "./include/pitch_scale.h"
#include "./include/logo.h"

#define PANEL_COLOR 0x7BEE

    TFT_eSPI    *tft;
    TFT_eSprite *mainSpr;
    TFT_eSprite *pitchScaleSpr;
    TFT_eSprite *rollScaleSpr;
    TFT_eSprite *bezelSpr;
    TFT_eSprite *backgroundSpr;
    // Pointers to start of Sprites in RAM (these are then "image" pointers)
    uint16_t *mainSprPtr;

    // Function declarations
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  setPitch(float value);
    void  setRoll(float value);
    void  setScreenRotation(int rotation);
    void  setPowerSave(bool enabled);
    void  setInstrumentBrightnessRatio(float ratio);

    // Variables
    float    pitch                     = 0; // pitch value from sim
    float    roll                      = 0; // roll value from sim
    float    pitchPosition             = 0; // pitch Postion on the screen based on the pitch angle
    bool     powerSaveFlag             = false;
    float    instrumentBrightnessRatio = 1;
    int      instrumentBrightness      = 255;
    int      prevScreenRotation        = 3;
    uint32_t startLogoMillis           = 0;
    uint8_t  backlight_pin             = 0;
    uint16_t instrumentX0              = 80;
    uint16_t instrumentY0              = 0;

    /* **********************************************************************************
        This is just the basic code to set up your custom device.
        Change/add your code as needed.
    ********************************************************************************** */

    void init(TFT_eSPI *_tft, TFT_eSprite *sprites, uint8_t pin_backlight)
    {
        backlight_pin = pin_backlight;
        pinMode(backlight_pin, OUTPUT);
        digitalWrite(backlight_pin, HIGH);

        tft = _tft;
        tft->setRotation(3);
        tft->setSwapBytes(true);
        tft->setPivot(240, 160);
        tft->fillScreen(TFT_BLACK);
        tft->startWrite(); // TFT chip select held low permanently

        mainSpr       = &sprites[0];
        pitchScaleSpr = &sprites[1];
        rollScaleSpr  = &sprites[2];
        bezelSpr      = &sprites[3];
        backgroundSpr = &sprites[4];

        mainSprPtr = (uint16_t *)mainSpr->createSprite(320, 320);
        mainSpr->setSwapBytes(true);
        mainSpr->fillSprite(TFT_BLACK);
        mainSpr->setPivot(160, 160);

        backgroundSpr->createSprite(320, 320);
        backgroundSpr->setSwapBytes(true);
        backgroundSpr->fillSprite(TFT_BLACK);

        rollScaleSpr->createSprite(roll_scale_width, roll_scale_height);
        rollScaleSpr->setSwapBytes(false);
        rollScaleSpr->fillSprite(TFT_BLACK);

        pitchScaleSpr->createSprite(pitch_scale_width, pitch_scale_height);
        pitchScaleSpr->setSwapBytes(false);
        pitchScaleSpr->fillSprite(TFT_BLACK);
        pitchScaleSpr->pushImage(0, 0, pitch_scale_width, pitch_scale_height, pitch_scale);

        bezelSpr->createSprite(320, 320);
        bezelSpr->setSwapBytes(true);
        bezelSpr->fillSprite(TFT_BLACK);
        bezelSpr->pushImage(0, 0, bezel_width, bezel_height, bezel);
        bezelSpr->setPivot(160, 160);

        tft->pushImage(160, 80, 160, 160, logo);
        startLogoMillis = millis();
        tft->setSwapBytes(false);
    }

    void stop()
    {
        tft->endWrite();
        mainSpr->deleteSprite();
        pitchScaleSpr->deleteSprite();
        rollScaleSpr->deleteSprite();
        bezelSpr->deleteSprite();
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
        int32_t data = strtoul(setPoint, NULL, 10);

        switch (messageID) {
        case -1:
            setPowerSave(true);
            break;
        case -2:
            setPowerSave((bool)atoi(setPoint));
            break;
        case 0:
            setPitch(atof(setPoint));
            break;
        case 1:
            setRoll(atof(setPoint));
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
        // show start up logo for 3 seconds
        if (millis() - startLogoMillis < 3000)
            return;

        mainSpr->fillSprite(TFT_BLACK);
        backgroundSpr->fillSprite(TFT_BLACK);

        pitchPosition = round(scaleValue(pitch, -40, 40, -80, 80));
        backgroundSpr->pushImage(20, 20, 280, 280, background);

        pitchScaleSpr->pushImage(0, 0, pitch_scale_width, pitch_scale_height, pitch_scale);

        backgroundSpr->setViewport(30, 30, 260, 260);
        pitchScaleSpr->pushToSprite(backgroundSpr, 28, pitchPosition + 58, TFT_BLACK);

        backgroundSpr->setViewport(0, 0, 320, 320);
        rollScaleSpr->pushImage(0, 0, roll_scale_width, roll_scale_height, roll_scale);
        rollScaleSpr->pushToSprite(backgroundSpr, 20, 20, TFT_BLACK);

        backgroundSpr->setPivot(160, 160);
        backgroundSpr->setSwapBytes(true);

        bezelSpr->pushRotated(backgroundSpr, roll, TFT_BLACK);

        // mainSpr->pushRotated(-roll, TFT_BLACK);
        // pushRotated is not available for DMA, use instead a helper sprite (background)
        // and copy this one as last step rotated to the main sprite
        backgroundSpr->pushRotated(mainSpr, -roll, TFT_BLACK);
        tft->pushImageDMA(instrumentX0, instrumentY0, 320, 320, mainSprPtr);
    }

    void setPitch(float value)
    {
        if (value >= 40)
            pitch = 40;
        else if (value <= -40)
            pitch = -40;
        else
            pitch = value;
    }

    void setRoll(float value)
    {
        roll = value;
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
    }

    void setPowerSave(bool enabled)
    {
        if (enabled) {
            digitalWrite(backlight_pin, LOW);
            powerSaveFlag = true;
        } else {
            analogWrite(backlight_pin, instrumentBrightness);
            powerSaveFlag = false;
        }
    }

    void setInstrumentBrightnessRatio(float ratio)
    {
        instrumentBrightnessRatio = ratio;
        instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
        analogWrite(backlight_pin, instrumentBrightness);
    }

    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

}
