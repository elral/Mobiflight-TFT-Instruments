#include "Vertical_Speed_Indicator.h"

namespace VerticalSpeedIndicator
{
#include "./include/vsi_main_gauge.h"
#include "./include/vsi_needle.h"
#include "./include/logo.h"

#define PANEL_COLOR 0x7BEE

    TFT_eSPI    *tft;
    TFT_eSprite *VSImainSpr;
    TFT_eSprite *VSINeedleSpr;
    // Pointer to start of Sprite in RAM (these are then "image" pointers)
    uint16_t *mainSprPtr;

    // Function declaration
    void  setVerticalSpeed(float value);
    void  setPowerSave(bool enabled);
    void  setInstrumentBrightnessRatio(float ratio);
    void  setScreenRotation(int rotation);
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  drawInstrument();

    // Variables
    float    VSIValue                  = 0; // Vertical Speed Value returned from the simlulator
    float    VSIAngle                  = 0; // Angle of needle based on the VSI Value
    float    instrumentBrightnessRatio = 1;
    int      instrumentBrightness      = 255;
    int      prevScreenRotation        = 3;
    bool     powerSaveFlag             = false;
    uint32_t startLogoMillis           = 0;
    uint8_t  backlight_pin             = 0;
    uint16_t instrumentX0              = 80;
    uint16_t instrumentY0              = 0;
    bool     showLogo                  = true;

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
        tft->setPivot(240, 160);
        tft->setSwapBytes(true);
        tft->setViewport(0, 0, 480, 320, false);
        tft->fillScreen(TFT_BLACK);
        tft->startWrite(); // TFT chip select held low permanently

        VSImainSpr   = &sprites[0];
        VSINeedleSpr = &sprites[1];

        mainSprPtr = (uint16_t *)VSImainSpr->createSprite(320, 320);
        VSImainSpr->setSwapBytes(false);
        VSImainSpr->fillSprite(TFT_BLACK);
        VSImainSpr->pushImage(0, 0, 320, 320, vsi_main_gauge);
        VSImainSpr->setPivot(160, 160);

        VSINeedleSpr->createSprite(vsi_needle_width, vsi_needle_height);
        VSINeedleSpr->setSwapBytes(false);
        VSINeedleSpr->fillScreen(TFT_BLACK);
        VSINeedleSpr->pushImage(0, 0, vsi_needle_width, vsi_needle_height, vsi_needle);
        VSINeedleSpr->setPivot(vsi_needle_width / 2, 134);

        // show start up logo
        tft->pushImage(160, 80, 160, 160, logo);
        startLogoMillis = millis();
    }

    void stop()
    {
        tft->endWrite();
        VSImainSpr->deleteSprite();
        VSINeedleSpr->deleteSprite();
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
            setPowerSave(true);
            break;
        case -2:
            setPowerSave((bool)atoi(setPoint));
            break;
        case 0:
            setVerticalSpeed(atof(setPoint));
            break;
        case 1:
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
        if (showLogo) {
            drawInstrument();
            showLogo = false;
        }
    }

    void drawInstrument()
    {
        // Limit to -2000 to 2000 ft/sec
        if (VSIValue > 2000)
            VSIValue = 2000;
        else if (VSIValue < -2000)
            VSIValue = -2000;

        VSImainSpr->fillSprite(TFT_BLACK);
        VSIAngle = scaleValue(VSIValue, -2000, 2000, 102, 438); // The needle starts at -90 degrees
        VSImainSpr->pushImage(0, 0, 320, 320, vsi_main_gauge);
        VSINeedleSpr->pushRotated(VSImainSpr, VSIAngle, TFT_BLACK);
        tft->pushImageDMA(instrumentX0, instrumentY0, 320, 320, mainSprPtr);
    }

    void setVerticalSpeed(float value)
    {
        VSIValue = value;
        drawInstrument();
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

    // Scale function
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
}