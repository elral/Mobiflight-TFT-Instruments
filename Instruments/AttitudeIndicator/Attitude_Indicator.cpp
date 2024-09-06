#include "Attitude_Indicator.h"

namespace AttitudeIndicator
{
#include "./include/background.h"
#include "./include/bezel.h"
#include "./include/roll_scale.h"
#include "./include/pitch_scale.h"
#include "./include/logo.h"

#define PANEL_COLOR 0x7BEE

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite mainSpr       = TFT_eSprite(&tft); // Sprite to hold everything
    TFT_eSprite pitchScaleSpr = TFT_eSprite(&tft); // Sprite to hold everything
    TFT_eSprite rollScaleSpr  = TFT_eSprite(&tft); // Sprite to hold everything
    TFT_eSprite bezelSpr      = TFT_eSprite(&tft); // Sprite to hold everything

    // Function declarations
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  drawAll();
    void  setPitch(float value);
    void  setRoll(float value);
    void  setScreenRotation(int rotation);
    void  setPowerSaveMode(bool enabled);
    void  setInstrumentBrightnessRatio(float ratio);

    // Variables
    float pitch                     = 0; // pitch value from sim
    float roll                      = 0; // roll value from sim
    float pitchPosition             = 0; // pitch Postion on the screen based on the pitch angle
    bool  powerSaveFlag             = false;
    float instrumentBrightnessRatio = 1;
    int   instrumentBrightness      = 255;
    int   screenRotation            = 3;
    int   prevScreenRotation        = 3;

    /* **********************************************************************************
        This is just the basic code to set up your custom device.
        Change/add your code as needed.
    ********************************************************************************** */

    void init(TFT_eSPI *_tft, TFT_eSprite *sprites)
    {
        tft.init();
        tft.setRotation(3);
        tft.fillScreen(PANEL_COLOR);
        tft.setSwapBytes(true);
        tft.pushImage(160, 80, 160, 160, logo);
        delay(3000);
        tft.fillScreen(TFT_BLACK);
        tft.setSwapBytes(false);
        tft.setPivot(240, 160);

        mainSpr.createSprite(320, 320);
        mainSpr.setSwapBytes(true);
        mainSpr.fillSprite(TFT_BLACK);
        mainSpr.setPivot(160, 160);

        rollScaleSpr.createSprite(roll_scale_width, roll_scale_height);
        rollScaleSpr.setSwapBytes(false);
        rollScaleSpr.fillSprite(TFT_BLACK);

        pitchScaleSpr.createSprite(pitch_scale_width, pitch_scale_height);
        pitchScaleSpr.setSwapBytes(false);
        pitchScaleSpr.fillSprite(TFT_BLACK);
        pitchScaleSpr.pushImage(0, 0, pitch_scale_width, pitch_scale_height, pitch_scale);

        bezelSpr.createSprite(320, 320);
        bezelSpr.setSwapBytes(true);
        bezelSpr.fillSprite(TFT_BLACK);
        bezelSpr.pushImage(0, 0, bezel_width, bezel_height, bezel);
        bezelSpr.setPivot(160, 160);
    }

    void stop()
    {
        mainSpr.deleteSprite();
        pitchScaleSpr.deleteSprite();
        rollScaleSpr.deleteSprite();
        bezelSpr.deleteSprite();
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
        // uint16_t output;
        // do something according your messageID
        switch (messageID) {
        case -1:
            // // tbd., get's called when Mobiflight shuts down
            setPowerSaveMode(true);
            break;
        case -2:
            // // tbd., get's called when PowerSavingMode is entered
            if (data == 1)
                setPowerSaveMode(true);
            else if (data == 0)
                setPowerSaveMode(false);
            break;
        case 0:
            // output = (uint16_t)data;
            // data   = output;
            setPitch(atof(setPoint));
            break;
        case 1:
            /* code */
            setRoll(atof(setPoint));
            break;
        case 2:
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

        //   if(powerSaveFlag == false)
        //   {
        //     analogWrite(TFT_BL, instrumentBrightness);

        //     if(prevScreenRotation != screenRotation)
        //     {
        //         tft.setRotation(screenRotation);
        //         prevScreenRotation = screenRotation;
        //     }
        //     drawAll();

        //    }

        //    else digitalWrite(TFT_BL, LOW);

        analogWrite(TFT_BL, instrumentBrightness);

        if (prevScreenRotation != screenRotation) {
            tft.setRotation(screenRotation);
            prevScreenRotation = screenRotation;
        }
        drawAll();
    }

    void drawAll()
    {

        pitchPosition = round(scaleValue(pitch, -40, 40, -80, 80));
        mainSpr.pushImage(20, 20, 280, 280, background);
        pitchScaleSpr.pushImage(0, 0, pitch_scale_width, pitch_scale_height, pitch_scale);

        mainSpr.setViewport(30, 30, 260, 260);
        pitchScaleSpr.pushToSprite(&mainSpr, 28, pitchPosition + 58, TFT_BLACK);

        mainSpr.setViewport(0, 0, 320, 320);
        rollScaleSpr.pushImage(0, 0, roll_scale_width, roll_scale_height, roll_scale);
        rollScaleSpr.pushToSprite(&mainSpr, 20, 20, TFT_BLACK);

        mainSpr.setPivot(160, 160);
        mainSpr.setSwapBytes(true);

        bezelSpr.pushRotated(&mainSpr, roll, TFT_BLACK);
        mainSpr.pushRotated(-roll, TFT_BLACK);
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
        if (rotation == 1 || rotation == 3)
            screenRotation = rotation;
    }

    void setPowerSaveMode(bool enabled)
    {
        if (enabled)
            powerSaveFlag = true;
        else
            powerSaveFlag = false;
    }

    void setInstrumentBrightnessRatio(float ratio)
    {
        instrumentBrightnessRatio = ratio;
        instrumentBrightness      = round(scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255));
    }

    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

}
