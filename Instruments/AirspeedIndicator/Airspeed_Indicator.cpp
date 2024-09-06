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

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite mainSpr         = TFT_eSprite(&tft); // Main Sprite
    TFT_eSprite ASIneedleSpr    = TFT_eSprite(&tft); // Airspeed Indicator Gauge Sprite
    TFT_eSprite asiInfoSpr      = TFT_eSprite(&tft); // Sprite to hold Air Speed Value
    TFT_eSprite tasInfoSpr      = TFT_eSprite(&tft); // Sprite to hold True Air Speed Value
    TFT_eSprite altitudeInfoSpr = TFT_eSprite(&tft); // Sprite to hold Altitude
    TFT_eSprite vsiInfoSpr      = TFT_eSprite(&tft); // Sprite to hold Vertical Speed Info
    TFT_eSprite baroInfoSpr     = TFT_eSprite(&tft); // Sprite to hold Barometric Pressure Info

    // Function declarations
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  drawASIGauge();
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
    bool     powerSaveFlag;
    int      screenRotation     = 3;
    int      prevScreenRotation = 3;
    int      data;

    /* **********************************************************************************
        This is just the basic code to set up your custom device.
        Change/add your code as needed.
    ********************************************************************************** */
    void init(TFT_eSPI *_tft, TFT_eSprite *sprites)
    {
        tft.init();
        tft.setRotation(3);
        tft.fillScreen(PANEL_COLOR);
        tft.setPivot(320, 160);
        tft.setSwapBytes(true);
        tft.pushImage(160, 80, 160, 160, logo);
        delay(3000);
        tft.fillScreen(TFT_BLACK);
    }

    void stop()
    {
        mainSpr.deleteSprite();
        ASIneedleSpr.deleteSprite();
        asiInfoSpr.deleteSprite();
        tasInfoSpr.deleteSprite();
        altitudeInfoSpr.deleteSprite();
        vsiInfoSpr.deleteSprite();
        baroInfoSpr.deleteSprite();
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
            // tbd., get's called when Mobiflight shuts down
            setPowerSave(true);
        case -2:
            // tbd., get's called when PowerSavingMode is entered
            data = atoi(setPoint);
            setPowerSave((bool)atoi(setPoint));
            break;
        case 0:
            setAirspeed(atof(setPoint));
            break;
        case 1:
            /* code */
            setInstrumentBrightnessRatio(atof(setPoint));
            break;
        case 100:
            /* code */
            setScreenRotation(atoi(setPoint));
            break;
        default:
            break;
        }
    }

    void update()
    {

        // Do something which is required regulary

        // if (!powerSaveFlag)
        // {
        //     if(prevScreenRotation != screenRotation)
        //     {
        //         tft.fillScreen(TFT_BLACK);
        //         prevScreenRotation = screenRotation;
        //         tft.setRotation(screenRotation);
        //     }

        //     drawASIGauge();
        //     analogWrite(TFT_BL, instrumentBrightness);
        // }
        // else
        //     digitalWrite(TFT_BL, LOW);
        analogWrite(TFT_BL, instrumentBrightness);
        if (prevScreenRotation != screenRotation) {
            tft.fillScreen(TFT_BLACK);
            prevScreenRotation = screenRotation;
            tft.setRotation(screenRotation);
        }

        drawASIGauge();
    }

    void drawASIGauge() // Draw the Airspeed Indicator Gauge and other information
    {

        if (airSpeed <= 40)
            ASIneedleRotation = scaleValue(airSpeed, 0, 40, 0, 20);
        else if (airSpeed > 40 && airSpeed <= 200)
            ASIneedleRotation = scaleValue(airSpeed, 41, 200, 21, 321);
        else if (airSpeed > 200)
            airSpeed = 200;

        mainSpr.createSprite(320, 320);
        mainSpr.setSwapBytes(true);
        mainSpr.fillSprite(TFT_BLACK);
        mainSpr.pushImage(0, 0, 320, 320, asi_main_gauge);
        mainSpr.setPivot(160, 160);

        ASIneedleSpr.createSprite(asi_needle_width, asi_needle_height);
        ASIneedleSpr.fillSprite(TFT_BLACK);
        ASIneedleSpr.pushImage(0, 0, asi_needle_width, asi_needle_height, asi_needle);
        ASIneedleSpr.setSwapBytes(false);
        ASIneedleSpr.setPivot(asi_needle_width / 2, 133);
        ASIneedleSpr.pushRotated(&mainSpr, ASIneedleRotation, TFT_BLACK);
        ASIneedleSpr.setSwapBytes(true);
        ASIneedleSpr.deleteSprite();

        mainSpr.pushSprite(80, 0, TFT_BLACK);
        mainSpr.deleteSprite();
        analogWrite(TFT_BL, instrumentBrightness);

    }

    void setAirspeed(float value)
    {
        airSpeed = value;
    }

    void setInstrumentBrightnessRatio(float ratio)
    {
        instrumentBrightnessRatio = ratio;
        instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
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
    }

    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

}