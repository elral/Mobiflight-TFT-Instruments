#include "Airspeed_Indicator.h"

namespace AirspeedIndicator
{

#include "./include/ssFont.h"
#include "./include/NotoSansMonoSCB20.h"
#include "./include/asi_main_gauge.h"
#include "./include/asi_needle.h"
#include "./include/logo.h"
#include "./include/instrument_bezel.h"

#define digits      NotoSansMonoSCB20
#define PANEL_COLOR 0x7BEE

    TFT_eSPI    *tft;
    TFT_eSprite *mainSpr;
    TFT_eSprite *ASIneedleSpr;
    TFT_eSprite *baroInfoSpr;
    // Pointers to start of Sprites in RAM (these are then "image" pointers)
    uint16_t *mainSprPtr;

    // Function declarations
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  setInstrumentBrightnessRatio(float ratio);
    void  setAirspeed(float value);
    void  setPowerSave(bool enabled);
    void  setScreenRotation(int rotation);

    // Variables
    uint16_t altitude                  = 0;
    float    instrumentBrightness      = 1;
    float    instrumentBrightnessRatio = 0;
    float    ASIneedleRotation         = 0; // angle of rotation of needle based on the Indicated AirSpeed
    float    airSpeed                  = 0;
    bool     powerSaveFlag             = false;
    int      screenRotation            = 3;
    int      prevScreenRotation        = 3;
    uint32_t startLogoMillis           = 0;

    /* **********************************************************************************
        This is just the basic code to set up your custom device.
        Change/add your code as needed.
    ********************************************************************************** */
    void init(TFT_eSPI *_tft, TFT_eSprite *sprites)
    {
        pinMode(TFT_BL, OUTPUT);

        tft = _tft;
        tft->setRotation(3);
        tft->fillScreen(PANEL_COLOR);
        tft->setPivot(320, 160);
        tft->setSwapBytes(true);
        tft->fillScreen(TFT_BLACK);
        tft->startWrite(); // TFT chip select held low permanently

        mainSpr       = &sprites[0];
        ASIneedleSpr = &sprites[1];
        baroInfoSpr  = &sprites[2];

        mainSprPtr = (uint16_t *)mainSpr->createSprite(320, 320);
        mainSpr->setSwapBytes(true);
        mainSpr->setPivot(160, 160);

        ASIneedleSpr->createSprite(asi_needle_width, asi_needle_height);
        ASIneedleSpr->setSwapBytes(false);
        ASIneedleSpr->setPivot(asi_needle_width / 2, 133);
        ASIneedleSpr->setSwapBytes(true);

        tft->pushImage(160, 80, 160, 160, logo);
        startLogoMillis = millis();
        tft->setSwapBytes(false);
    }

    void stop()
    {
        tft->endWrite();
        mainSpr->deleteSprite();
        ASIneedleSpr->deleteSprite();
        baroInfoSpr->deleteSprite();
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

        switch (messageID) {
        case -1:
            setPowerSave(true);
        case -2:
            setPowerSave((bool)atoi(setPoint));
            break;
        case 0:
            setAirspeed(atof(setPoint));
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
       
        mainSpr->fillSprite(TFT_BLACK);
        mainSpr->pushImage(0, 0, 320, 320, asi_main_gauge);
        
        ASIneedleSpr->fillSprite(TFT_BLACK);
        ASIneedleSpr->pushImage(0, 0, asi_needle_width, asi_needle_height, asi_needle);
        ASIneedleSpr->pushRotated(mainSpr, ASIneedleRotation, TFT_BLACK);

        tft->pushImageDMA(80, 0, 320, 320, mainSprPtr);

    }

    void setAirspeed(float value)
    {
        airSpeed = value;

        if (airSpeed <= 40)
            ASIneedleRotation = scaleValue(airSpeed, 0, 40, 0, 20);
        else if (airSpeed > 40 && airSpeed <= 200)
            ASIneedleRotation = scaleValue(airSpeed, 41, 200, 21, 321);
        else if (airSpeed > 200)
            airSpeed = 200;
    }

    void setInstrumentBrightnessRatio(float ratio)
    {
        instrumentBrightnessRatio = ratio;
        instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));

//        analogWrite(TFT_BL, instrumentBrightness);
    }

    void setPowerSave(bool enabled)
    {
        if (enabled) {
            analogWrite(TFT_BL, 0);
            powerSaveFlag = true;
        } else {
            analogWrite(TFT_BL, instrumentBrightness);
            powerSaveFlag = false;
        }
    }

    void setScreenRotation(int rotation)
    {
        if (rotation == 1 || rotation == 3)
            screenRotation = rotation;
    
        if (prevScreenRotation != screenRotation) {
            tft->fillScreen(TFT_BLACK);
            prevScreenRotation = screenRotation;
            tft->setRotation(screenRotation);
        }
    }

    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

}