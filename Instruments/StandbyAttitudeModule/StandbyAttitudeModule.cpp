#include "StandbyAttitudeModule.h"

namespace StandbyAttitudeMonitor
{
#include "./include/pitch_line_long.h"
#include "./include/pitch_line_short.h"
#include "./include/ball.h"
#include "./include/hdg_box.h"
#include "./include/plane_indicator.h"
#include "./include/roll_indicator.h"
#include "./include/roll_scale.h"
#include "./include/slip_indicator.h"
#include "./include/pitch_scale.h"
#include "./include/chevron_down.h"
#include "./include/chevron_up.h"
#include "./include/altitude_indicator_box_2.h"
#include "./include/speed_indicator_box_2.h"
#include "./include/baro_box.h"
#include "./include/heading_box.h"
#include "./include/B612Font18.h"
#include "./include/B612Font20.h"
#include "./include/B612Font28.h"
#include "./include/logo.h"

#define BROWN       0x80C3 // 0x5960
#define SKY_BLUE    0x255C // 0x0318 //0x039B //0x34BF
#define GRAY        0x18C3
#define PANEL_COLOR 0x7BEE
#define digitsS     B612Font18
#define digitsM     B612Font20
#define digitsL     B612Font28

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite AttitudeIndSpr      = TFT_eSprite(&tft); // Sprite to hold attitude Indicator
    TFT_eSprite AttitudeIndBackSpr  = TFT_eSprite(&tft); // Sprite to hold attitude Indicator to fit only 240x320
    TFT_eSprite AttitudeIndBackSpr2 = TFT_eSprite(&tft); // Sprite to hold attitude Indicator to fit only 240x320
    TFT_eSprite planeSpr            = TFT_eSprite(&tft); // Plane Indicator Sprite
    TFT_eSprite rollScaleSpr        = TFT_eSprite(&tft); // Roll Scale Sprite
    TFT_eSprite pitchScaleSpr       = TFT_eSprite(&tft); // Pitch Scale Sprite
    TFT_eSprite rollIndicatorSpr    = TFT_eSprite(&tft); // Roll Indicator Sprite
    TFT_eSprite slipIndicatorSpr    = TFT_eSprite(&tft); // Slip Indicator Sprite
    TFT_eSprite ballSpr             = TFT_eSprite(&tft); // Ball Sprite
    TFT_eSprite chevronUpSpr        = TFT_eSprite(&tft); // Chevron pointing up
    TFT_eSprite chevronDownSpr      = TFT_eSprite(&tft); // Chevron pointing down
    TFT_eSprite SpeedIndicatorSpr   = TFT_eSprite(&tft); // Sprite to hold the speed indicator
    TFT_eSprite AltitudeIndSpr      = TFT_eSprite(&tft); // Sprite to hold the altitude indicator
    TFT_eSprite headingBoxSpr       = TFT_eSprite(&tft); // Sprite to hold heading Box
    TFT_eSprite speedIndBoxSpr      = TFT_eSprite(&tft); // Sprite to hold speed indicator box
    TFT_eSprite altIndBoxSpr        = TFT_eSprite(&tft); // Sprite to hold altitude indicator box
    TFT_eSprite baroBoxSpr          = TFT_eSprite(&tft); // Sprite to holdt the Baro Box

    // Function declarations
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max);
    void  drawAll();
    void  drawPitchScale(float pitch);
    void  drawBall();
    void  drawAttitudeIndicator();
    void  drawSpeedIndicator();
    void  drawAltitudeIndicator();
    void  drawSpeedIndicatorLines();
    void  drawSpeedIndicatorValues();
    void  drawAltitudeIndicatorLines();
    void  drawAltitudeIndicatorValues();
    void  setAirSpeed(float value);
    void  setPitch(float value);
    void  setRoll(float value);
    void  setSlipAngle(float value);
    void  setAltitude(float value);
    void  setHeading(float value);
    void  setBaro(float value);
    void  setInstrumentBrightness(float value);
    void  setScreenRotation(int rotation);

    // Variables
    float  pitch                     = 0;
    float  roll                      = 0;
    float  newRoll                   = 0;
    float  pitchPosition             = 0;
    float  newPitch                  = 0;
    float  slipAngle                 = 0;    // slip angle value from sim
    float  airSpeed                  = 0;    // Air speed value from the sim
    double altitude                  = 0;    // Altitude Value from the simulator
    float  heading                   = 0;    // heading value from sim
    float  baro                      = 0;    // baro value from sim
    float  instrumentBrightnessRatio = 0.75; // baro value from sim
    float  instrumentBrightness      = 192;
    int    screenRotation            = 3;
    int    prevScreenRotation        = 3;

    /* **********************************************************************************
        This is just the basic code to set up your custom device.
        Change/add your code as needed.
    ********************************************************************************** */

    void init(TFT_eSPI *_tft, TFT_eSprite *sprites)
    {
        pinMode(TFT_BL, OUTPUT);
        
        tft.init();
        tft.setRotation(screenRotation);
        tft.fillScreen(PANEL_COLOR);
        tft.setPivot(320, 160);
        tft.setSwapBytes(true);
        tft.pushImage(160, 80, 160, 160, logo);
        delay(3000);
        tft.fillScreen(PANEL_COLOR);
        tft.fillCircle(240, 160, 160, TFT_BLACK);

        AttitudeIndSpr.createSprite(400, 400);
        AttitudeIndSpr.setSwapBytes(false);
        AttitudeIndSpr.fillSprite(TFT_BLACK);
        AttitudeIndSpr.setPivot(200, 200);

        pitchScaleSpr.createSprite(123, 148);
        pitchScaleSpr.setSwapBytes(false);
        pitchScaleSpr.fillSprite(TFT_BLACK);

        rollScaleSpr.createSprite(rollScaleWidth, rollScaleHeight);
        rollScaleSpr.setSwapBytes(false);
        rollScaleSpr.fillSprite(TFT_BLACK);

        rollIndicatorSpr.createSprite(rollIndicatorWidth, rollIndicatorHeight);
        rollIndicatorSpr.setSwapBytes(false);
        rollIndicatorSpr.fillSprite(TFT_BLACK);

        // Draw the slip indicator sprite
        slipIndicatorSpr.createSprite(slipIndicatorWidth, slipIndicatorHeight);
        slipIndicatorSpr.setSwapBytes(false);
        slipIndicatorSpr.fillSprite(TFT_BLACK);

        planeSpr.createSprite(planeIndicatorWidth, planeIndicatorHeight);
        planeSpr.setSwapBytes(true);
        planeSpr.fillSprite(TFT_BLACK);

        // Create the sprites to hold the red chevron that points up
        chevronUpSpr.createSprite(76, 36);
        chevronUpSpr.setSwapBytes(false);
        chevronUpSpr.fillSprite(TFT_BLACK);
        chevronUpSpr.pushImage(0, 0, 76, 36, chevron_up);

        // Create the sprites to hold the red chevron that points down
        chevronDownSpr.createSprite(76, 36);
        chevronDownSpr.setSwapBytes(false);
        chevronDownSpr.fillSprite(TFT_BLACK);
        chevronDownSpr.pushImage(0, 0, 76, 36, chevron_down);

        SpeedIndicatorSpr.createSprite(120, 320);
        SpeedIndicatorSpr.setSwapBytes(false);
        SpeedIndicatorSpr.fillSprite(TFT_BLACK);

        AltitudeIndSpr.createSprite(120, 320);
        AltitudeIndSpr.setSwapBytes(false);
        AltitudeIndSpr.fillSprite(TFT_BLACK);

        altIndBoxSpr.createSprite(altitude_indicator_box_2_width, altitude_indicator_box_2_height);
        altIndBoxSpr.setSwapBytes(false);
        altIndBoxSpr.fillScreen(TFT_BLACK); // set blue background and use this for transparency later

        speedIndBoxSpr.createSprite(speed_indicator_box_2_width, speed_indicator_box_2_height);
        speedIndBoxSpr.setSwapBytes(false);
        speedIndBoxSpr.fillScreen(TFT_BLACK); // set blue background and use this for transparency later

        headingBoxSpr.createSprite(heading_box_width, heading_box_height);
        headingBoxSpr.setSwapBytes(false);
        headingBoxSpr.fillScreen(TFT_BLACK); // set blue background and use this for transparency later

        baroBoxSpr.createSprite(baro_box_width, baro_box_height);
        baroBoxSpr.setSwapBytes(false);
        baroBoxSpr.fillScreen(TFT_BLACK); // set blue background and use this for transparency later
    }

    void stop()
    {
        AttitudeIndSpr.deleteSprite();
        AttitudeIndBackSpr.deleteSprite();
        AttitudeIndBackSpr2.deleteSprite();
        planeSpr.deleteSprite();
        rollScaleSpr.deleteSprite();
        pitchScaleSpr.deleteSprite();
        rollIndicatorSpr.deleteSprite();
        slipIndicatorSpr.deleteSprite();
        ballSpr.deleteSprite();
        chevronUpSpr.deleteSprite();
        chevronDownSpr.deleteSprite();
        SpeedIndicatorSpr.deleteSprite();
        AltitudeIndSpr.deleteSprite();
        headingBoxSpr.deleteSprite();
        speedIndBoxSpr.deleteSprite();
        altIndBoxSpr.deleteSprite();
        baroBoxSpr.deleteSprite();
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
        // int32_t  data = atoi(setPoint);
        // uint16_t output;

        // do something according your messageID
        switch (messageID) {
        case -1:
            // tbd., get's called when Mobiflight shuts down
            analogWrite(TFT_BL, 0);
            break;
        case -2:
            // tbd., get's called when PowerSavingMode is entered
            if (atoi(setPoint) == 1)
                analogWrite(TFT_BL, 0);
            else if (atoi(setPoint) == 0)
                analogWrite(TFT_BL, instrumentBrightness);
            break;
        case 0:
            setPitch(atof(setPoint));
            break;
        case 1:
            // /* code */
            setRoll(atof(setPoint));
            break;
        case 2:
            setSlipAngle(atof(setPoint));
            /* code */
            break;
        case 3:
            setAirSpeed(atof(setPoint));
            /* code */
            break;
        case 4:
            setAltitude(atof(setPoint));
            /* code */
            break;
        case 5:
            setHeading(atof(setPoint));
            /* code */
            break;
        case 6:
            setBaro(atof(setPoint));
            /* code */
            break;
        case 7:
            setInstrumentBrightness(atof(setPoint));
            /* code */
            break;
        case 100:
            setScreenRotation(atoi(setPoint));
            /* code */
            break;
        default:
            break;
        }
    }

    void setAirSpeed(float value)
    {
        airSpeed = value;
    }

    void setPitch(float value)
    {
        pitch = value;
    }

    void setRoll(float value)
    {
        roll = value * -1.0; // Value seems to be reversed from sim
    }

    void setSlipAngle(float value)
    {
        slipAngle = value * -1.0; // Value seems to be reversed from sim
    }

    void setAltitude(float value)
    {
        altitude = value;
    }

    void setHeading(float value)
    {
        heading = value;
    }

    void setBaro(float value)
    {
        baro = value;
    }

    void setInstrumentBrightness(float value)
    {
        instrumentBrightnessRatio = value;
        instrumentBrightness      = (int)scaleValue(instrumentBrightnessRatio, 0, 1, 0, 255);
    }

    void setScreenRotation(int rotation)
    {
        if (rotation == 1 || rotation == 3)
            screenRotation = rotation;
    }

    void update()
    {
        // Do something which is required regulary
        drawAttitudeIndicator();
        drawSpeedIndicator();
        drawAltitudeIndicator();
        // drawUpdate(1); // Update the sprites
        analogWrite(TFT_BL, instrumentBrightness);

        // set the screen rotation
        if (prevScreenRotation != screenRotation) {
            prevScreenRotation = screenRotation;
            tft.setRotation(screenRotation);
        }
    }

    // void loop2()
    // {
    //   pushUpdate(0); // Transfer top half
    //   pushUpdate(1); // Transfer bottom half
    // }

    /* **********************************************************************************
        Speed Indicator Section
     ***********************************************************************************/

    void drawSpeedIndicator()
    {

        drawSpeedIndicatorLines();
        speedIndBoxSpr.pushImage(0, 0, speed_indicator_box_2_width, speed_indicator_box_2_height, speed_indicator_box_2);
        speedIndBoxSpr.setSwapBytes(true);
        drawSpeedIndicatorValues();
        speedIndBoxSpr.pushToSprite(&SpeedIndicatorSpr, 21, 137, TFT_BLACK);

        headingBoxSpr.pushImage(0, 0, heading_box_width, heading_box_height, heading_box);
        headingBoxSpr.setSwapBytes(true);
        headingBoxSpr.setTextColor(TFT_GREEN);
        headingBoxSpr.setTextDatum(MC_DATUM);
        // headingBoxSpr.setFreeFont(FSSB12);
        headingBoxSpr.loadFont(digitsM);
        headingBoxSpr.drawString(String((int)round(heading)), heading_box_width / 2, heading_box_height / 2 + 4);
        headingBoxSpr.pushToSprite(&SpeedIndicatorSpr, 12, 267, TFT_BLACK);
        tft.setViewport(0, 0, 120, 320);
        SpeedIndicatorSpr.pushSprite(0, 0);
        headingBoxSpr.fillSprite(TFT_BLACK);
        SpeedIndicatorSpr.fillSprite(TFT_BLACK);
    }

    void drawSpeedIndicatorLines()
    {
        int yPosShortLines[8];
        int yPosLongLines[8];
        int speedValues[8];
        int i;

        int minSpeed; // minimum spped in range
        int maxSpeed; // maximum spped in range

        // minSpeed = round(airSpeed - 40.0);
        minSpeed = airSpeed - 40.0;
        // maxSpeed = round(airSpeed + 40.0);
        maxSpeed = airSpeed + 40.0;

        SpeedIndicatorSpr.loadFont(digitsM);
        // SpeedIndicatorSpr.setFreeFont(d);
        SpeedIndicatorSpr.setTextColor(TFT_WHITE);
        SpeedIndicatorSpr.setTextDatum(ML_DATUM);

        // find the first airspeed value that has as "10" to draw long lines
        for (i = round(minSpeed); i <= round(maxSpeed); i++) {
            if ((i % 10) == 0) // found our first long line
            {
                // yPosLongLines[0] = round(scaleValue(i, minSpeed, maxSpeed, 320, 0));
                yPosLongLines[0] = scaleValue(i, minSpeed, maxSpeed, 320, 0);
                speedValues[0]   = i;
                SpeedIndicatorSpr.drawWideLine(15, yPosLongLines[0] + 2, 28, yPosLongLines[0] + 2, 3, TFT_WHITE, TFT_BLACK);
                if (speedValues[0] > 0)
                    SpeedIndicatorSpr.drawString(String(speedValues[0]), 30, yPosLongLines[0] + 2);
                break;
                // tft.setTextColor(TFT_GREEN);
                // tft.drawString(String(i), 50, 20, 4);
            }
        }

        // Now populate the positions of the other long lines
        for (i = 1; i < 8; i++) {
            // yPosLongLines[i] = round(scaleValue(speedValues[0] + (i * 10), minSpeed, maxSpeed, 320, 0));
            yPosLongLines[i] = scaleValue(speedValues[0] + (i * 10), minSpeed, maxSpeed, 320, 0);
            SpeedIndicatorSpr.drawWideLine(15, yPosLongLines[i] + 2, 28, yPosLongLines[i] + 2, 3, TFT_WHITE, TFT_BLACK);
            speedValues[i] = speedValues[0] + (i * 10);
            if (speedValues[i] > 0)
                SpeedIndicatorSpr.drawString(String(speedValues[i]), 30, yPosLongLines[i] + 2);
        }

        // find the first airspeed value that has as "5" to draw short lines
        for (i = minSpeed; i <= maxSpeed; i++) {
            if ((i % 5) == 0 && (i % 10) != 0) // found our first short line
            {
                // yPosShortLines[0] = round(scaleValue(i, minSpeed, maxSpeed, 320, 0));
                yPosShortLines[0] = scaleValue(i, minSpeed, maxSpeed, 320, 0);
                speedValues[0]    = i;
                SpeedIndicatorSpr.drawWideLine(15, yPosShortLines[0] + 2, 23, yPosShortLines[0] + 2, 3, TFT_WHITE, TFT_BLACK);
                // tft.setTextColor(TFT_GREEN);
                // tft.drawString(String(i), 0, 20, 4);
                break;
            }
        }
        // Now populate the positions of the other short lines

        for (i = 1; i < 8; i++) {
            // yPosShortLines[i] = round(scaleValue(speedValues[0] + (i * 10), minSpeed, maxSpeed, 320, 0));
            yPosShortLines[i] = scaleValue(speedValues[0] + (i * 10), minSpeed, maxSpeed, 320, 0);
            speedValues[i]    = speedValues[0] + (i * 10);
            SpeedIndicatorSpr.drawWideLine(15, yPosShortLines[i] + 2, 23, yPosShortLines[i] + 2, 3, TFT_WHITE, TFT_BLACK);
        }
    }

    void drawSpeedIndicatorValues()
    {
        speedIndBoxSpr.setTextColor(TFT_WHITE);
        speedIndBoxSpr.setTextDatum(ML_DATUM);
        // speedIndBoxSpr.setFreeFont(FSSB12);
        speedIndBoxSpr.loadFont(digitsL);
        speedIndBoxSpr.drawString(String((int)round(airSpeed)), 13, speed_indicator_box_2_height / 2 - 6);
    }

    /* **********************************************************************************
        Altitude Indicator Section
     ***********************************************************************************/

    void drawAltitudeIndicator()
    {
        drawAltitudeIndicatorLines();
        // altIndBoxSpr.setSwapBytes(true);
        altIndBoxSpr.pushImage(0, 0, altitude_indicator_box_2_width, altitude_indicator_box_2_height, altitude_indicator_box_2);
        altIndBoxSpr.setSwapBytes(true);
        drawAltitudeIndicatorValues();
        altIndBoxSpr.pushToSprite(&AltitudeIndSpr, 2, 140, TFT_BLACK);

        baroBoxSpr.setSwapBytes(true);
        baroBoxSpr.pushImage(0, 0, baro_box_width, baro_box_height, baro_box);

        baroBoxSpr.setTextColor(TFT_GREEN);
        baroBoxSpr.setTextDatum(MC_DATUM);
        // baroBoxSpr.setFreeFont(FSSB12);
        baroBoxSpr.loadFont(digitsS);
        baroBoxSpr.drawString(String(baro), baro_box_width / 2, baro_box_height / 2 + 10);
        baroBoxSpr.pushToSprite(&AltitudeIndSpr, 34, 268, TFT_BLACK);

        tft.setViewport(360, 0, 480, 320);
        AltitudeIndSpr.pushSprite(0, 0);
        AltitudeIndSpr.fillSprite(TFT_BLACK);
        baroBoxSpr.fillSprite(TFT_BLACK);
    }

    void drawAltitudeIndicatorLines()
    {

        int yPosShortLines[10];
        int yPosMediumLines[5];
        int yPosLongLines[5];
        int altitudeValues[10];
        int i;
        int tmpMinAlt;
        int tmpMaxAlt;

        int minAltitude; // minimum altitude in range
        int maxAltitude; // maximum altitude in range

        // minAltitude = round(altitude - 250.0);
        minAltitude = altitude - 250.0;
        // maxAltitude = round(altitude + 250.0);
        maxAltitude = altitude + 250.0;

        AltitudeIndSpr.loadFont(digitsS);
        // AltitudeIndSpr.setFreeFont(digitsM);
        AltitudeIndSpr.setTextColor(TFT_WHITE);
        AltitudeIndSpr.setTextDatum(MR_DATUM);

        // find the first altitude value that has a "100" to draw long lines
        for (i = round(minAltitude); i <= round(maxAltitude); i++) {
            if ((i % 100) == 0) // found our first long line
            {
                // yPosLongLines[0] = round(scaleValue(i, minAltitude, maxAltitude, 320, 0));
                yPosLongLines[0]  = scaleValue(i, minAltitude, maxAltitude, 320, 0);
                altitudeValues[0] = i;
                AltitudeIndSpr.drawWideLine(91, yPosLongLines[0] + 2, 109, yPosLongLines[0] + 2, 3, TFT_WHITE, TFT_BLACK);
                AltitudeIndSpr.drawString(String(altitudeValues[0]), 89, yPosLongLines[0] + 2);
                break;
                // tft.setTextColor(TFT_GREEN);
                // tft.drawString(String(i), 0, 210, 4);
            }
        }

        // Now populate the positions of the other long lines
        for (i = 1; i < 5; i++) {
            // yPosLongLines[i] = round(scaleValue(altitudeValues[0] + (i * 100), minAltitude, maxAltitude, 320, 0));
            yPosLongLines[i] = scaleValue(altitudeValues[0] + (i * 100), minAltitude, maxAltitude, 320, 0);
            AltitudeIndSpr.drawWideLine(91, yPosLongLines[i] + 2, 109, yPosLongLines[i] + 2, 3, TFT_WHITE, TFT_BLACK);
            altitudeValues[i] = altitudeValues[0] + (i * 100);
            AltitudeIndSpr.drawString(String(altitudeValues[i]), 89, yPosLongLines[i] + 2);
        }

        // find the first altitude value that has a "50" to draw medium lines
        for (i = minAltitude; i <= maxAltitude; i++) {
            if ((i % 100) != 0 && (i % 50) == 0) // found our first medium ine
            {
                // yPosMediumLines[0] = round(scaleValue(i, minAltitude, maxAltitude, 320, 0));
                yPosMediumLines[0] = scaleValue(i, minAltitude, maxAltitude, 320, 0);
                altitudeValues[0]  = i;
                AltitudeIndSpr.drawWideLine(97, yPosMediumLines[0] + 2, 109, yPosMediumLines[0] + 2, 3, TFT_WHITE, TFT_BLACK);
                // AltitudeIndSpr.drawString(String(altitudeValues[0]), 50, yPosMediumLines[0] + 2);
                break;
            }
        }

        // Now populate the positions of the other medium lines

        for (i = 1; i < 5; i++) {
            // yPosMediumLines[i] = round(scaleValue(altitudeValues[0] + (i * 100), minAltitude, maxAltitude, 320, 0));
            yPosMediumLines[i] = scaleValue(altitudeValues[0] + (i * 100), minAltitude, maxAltitude, 320, 0);
            altitudeValues[i]  = altitudeValues[0] + (i * 100);
            AltitudeIndSpr.drawWideLine(97, yPosMediumLines[i] + 2, 109, yPosMediumLines[i] + 2, 3, TFT_WHITE, TFT_BLACK);
        }

        // find the first altitude value that has a "25" to draw short lines
        for (i = minAltitude; i <= maxAltitude; i++) {
            if ((i % 100) != 0 && (i % 50) != 0 && (i % 25) == 0) // found our first short ine
            {
                // yPosShortLines[0] = round(scaleValue(i, minAltitude, maxAltitude, 320, 0));
                yPosShortLines[0] = scaleValue(i, minAltitude, maxAltitude, 320, 0);
                altitudeValues[0] = i;
                AltitudeIndSpr.drawWideLine(102, yPosShortLines[0] + 2, 109, yPosShortLines[0] + 2, 3, TFT_WHITE, TFT_BLACK);
                break;
            }
        }

        // Now populate the positions of the other short lines

        for (i = 1; i < 10; i++) {
            // yPosShortLines[i] = round(scaleValue(altitudeValues[0] + (i * 50), minAltitude, maxAltitude, 320, 0));
            yPosShortLines[i] = scaleValue(altitudeValues[0] + (i * 50), minAltitude, maxAltitude, 320, 0);
            altitudeValues[i] = altitudeValues[0] + (i * 50);
            AltitudeIndSpr.drawWideLine(102, yPosShortLines[i] + 2, 109, yPosShortLines[i] + 2, 3, TFT_WHITE, TFT_BLACK);
        }
    }

    void drawAltitudeIndicatorValues()
    {
        altIndBoxSpr.setTextColor(TFT_WHITE);
        altIndBoxSpr.setTextColor(TFT_WHITE);
        altIndBoxSpr.setTextDatum(MR_DATUM);
        // altIndBoxSpr.setFreeFont(FSSB12);
        altIndBoxSpr.loadFont(digitsL);
        altIndBoxSpr.drawString(String((int)round(altitude)), 88, altitude_indicator_box_2_height / 2 - 9);
    }

    /* **********************************************************************************
        Attitude Indicator Section
     ***********************************************************************************/

    void drawAttitudeIndicator()
    {
        int i = 0;
        // Do something which is required regulary
        // Calcualte the new pitch when the pitch is more than 90 or more than 270 or less that -90 or less -270 degrees
        // because it just shows a "flipped" 80 degrees instead of 110 degrees, for example
        if (pitch >= -90 && pitch <= 90)
            newPitch = pitch;
        else if (pitch > 90 & pitch <= 270)
            newPitch = 90 - (pitch - 90);
        else if (pitch > 270 & pitch <= 360)
            newPitch = pitch - 360;
        else if (pitch < -90 & pitch >= -270)
            newPitch = -90 - (pitch + 90);
        else if (pitch < -270 && pitch >= -360)
            newPitch = pitch + 360;

        pitchPosition = scaleValue(newPitch, -45, 45, 0, 320);
        // Implement the "smooth flip" when the pitch is 90 or -90 or 270 or -270 degrees
        if (round(pitch) > -90 && round(pitch) < 90) {
            newRoll = roll;
            drawAll(); // Draw all
        } else if (round(pitch) == 90) {
            for (i = 0; i <= 180; i += 30) {
                newRoll = roll + i;
                drawAll(); // Draw all
            }
        } else if (round(pitch) > 90 && round(pitch) < 270) {
            newRoll = roll + 180;
            drawAll(); // Draw all
        } else if (round(pitch) == 270) {
            for (i = 0; i <= 180; i += 30) {
                // newRoll = roll + i;
                newRoll = roll + 180 - i;
                drawAll(); // Draw all
            }
        } else if (round(pitch) > 270 && round(pitch) <= 360) {
            newRoll = roll;
            drawAll(); // Draw all
        } else if (round(pitch) == -90) {
            for (i = 0; i <= 180; i += 30) {
                // newRoll = roll + i;
                newRoll = roll - i;
                drawAll(); // Draw all
            }
        } else if (round(pitch) < -90 && round(pitch) > -270) {
            newRoll = roll - 180;
            drawAll(); // Draw all
        } else if (round(pitch) == -270) {
            for (i = 0; i <= 180; i += 30) {
                // newRoll = roll + i;
                newRoll = roll + 180 + i;
                drawAll(); // Draw all
            }
        } else if ((round(pitch) < -270 && round(pitch) >= -360)) {
            newRoll = roll;
            drawAll(); // Draw all
        }
    }

    void drawAll()
    {

        // Draw main sprite that holds the sky and ground
        AttitudeIndSpr.fillRect(0, 0, 400, pitchPosition + 40, SKY_BLUE);
        AttitudeIndSpr.fillRect(0, pitchPosition + 40, 400, 400, BROWN);
        AttitudeIndSpr.fillRect(0, pitchPosition + 40 - 2, 400, 4, TFT_WHITE);
        AttitudeIndSpr.setPivot(200, 200);

        // Draw the pitch scale sprite

        drawPitchScale(pitch);
        pitchScaleSpr.pushToSprite(&AttitudeIndSpr, 59 + 80, 88 + 40, TFT_BLACK);

        // Draw the roll scale sprite
        rollScaleSpr.pushImage(0, 0, rollScaleWidth, rollScaleHeight, roll_scale);
        rollScaleSpr.setSwapBytes(true);
        rollScaleSpr.pushToSprite(&AttitudeIndSpr, 17 + 80, 42 + 40, TFT_BLACK);
        rollScaleSpr.fillSprite(TFT_BLACK);

        // Finally, rotate the Attitude indicator sprite and copy to AttitudeIndBackSpr
        // AttitudeIndSpr.pushRotated(&AttitudeIndBackSpr, newRoll, TFT_BLACK);

        slipIndicatorSpr.pushImage(0, 0, slipIndicatorWidth, slipIndicatorHeight, slip_indicator);

        // Draw the the ball. The degree of is -8 to 8 degrees according to the sim values by trial and error

        drawBall();
        slipIndicatorSpr.setSwapBytes(true);
        // slipIndicatorSpr.pushToSprite(&AttitudeIndBackSpr, 73, 264, TFT_BLACK);
        slipIndicatorSpr.setPivot(slipIndicatorWidth / 2, -135);
        slipIndicatorSpr.pushRotated(&AttitudeIndSpr, newRoll * -1.0, TFT_BLACK);

        // Draw the roll indicator sprite

        rollIndicatorSpr.setSwapBytes(true);
        rollIndicatorSpr.pushImage(0, 0, rollIndicatorWidth, rollIndicatorHeight, roll_indicator);
        rollIndicatorSpr.setPivot(rollIndicatorWidth / 2, 90);
        rollIndicatorSpr.pushRotated(&AttitudeIndSpr, newRoll * -1.0, TFT_RED);
        // rollIndicatorSpr.pushToSprite(&AttitudeIndBackSpr, 114, 66, TFT_RED);

        // Draw the plane indicator
        planeSpr.pushImage(0, 0, planeIndicatorWidth, planeIndicatorHeight, plane_indicator);
        // planeSpr.pushToSprite(&AttitudeIndBackSpr, 13, 154, TFT_BLACK);
        planeSpr.setPivot(planeIndicatorWidth / 2, 5);
        planeSpr.pushRotated(&AttitudeIndSpr, newRoll * -1.0, TFT_BLACK);

        tft.setViewport(120, 0, 240, 320);
        tft.setSwapBytes(false);
        tft.setPivot(240, 160);
        AttitudeIndSpr.pushRotated(newRoll, TFT_BLACK);
        // AttitudeIndBackSpr.pushSprite(120, 0, TFT_BLACK);
        pitchScaleSpr.fillSprite(TFT_BLACK);
        AttitudeIndSpr.fillScreen(TFT_BLACK);
    }

    void drawPitchScale(float pitch)
    {
        // Draw the pitch scale
        float startPitch = 0, endPitch = 0;
        long  pitchAngle, pitchLinePos, angleIncrement;

        startPitch = newPitch - 20;
        endPitch   = newPitch + 20;

        for (angleIncrement = startPitch; angleIncrement <= endPitch; angleIncrement++) {

            pitchLinePos = scaleValue(angleIncrement, endPitch, startPitch, 0, 141); // scale the angles to the number of pixels
            pitchAngle   = round(angleIncrement);

            if ((pitchAngle % 5 == 0) && pitchAngle >= -40 && pitchAngle <= 40 && (pitchAngle % 10) != 0) {
                pitchScaleSpr.setSwapBytes(true);
                // pitchScaleSpr.drawWideLine(45, pitchLinePos, 45 + pitchLineShortWidth, pitchLinePos, 4, TFT_WHITE, TFT_WHITE);
                pitchScaleSpr.fillRect(45, pitchLinePos, pitchLineShortWidth, 4, TFT_WHITE);
            }

            if ((pitchAngle % 10) == 0) // draw long pitch line and numbers
            {
                pitchScaleSpr.setSwapBytes(true);
                // pitchScaleSpr.drawWideLine(23, pitchLinePos, pitchLineLongWidth + 20, pitchLinePos, 4, TFT_WHITE, TFT_WHITE);
                pitchScaleSpr.fillRect(23, pitchLinePos, pitchLineLongWidth, 4, TFT_WHITE);
                pitchScaleSpr.loadFont(digitsS);
                // pitchScaleSpr.setFreeFont(digits);

                pitchScaleSpr.setTextColor(TFT_WHITE, TFT_WHITE);
                if (pitchAngle != 0) {
                    if (abs(pitchAngle) > 90)
                        pitchAngle = 90 - (abs(pitchAngle) - 90);
                    pitchScaleSpr.setSwapBytes(false);
                    pitchScaleSpr.drawString(String(abs(pitchAngle)), 1, pitchLinePos - 5);
                    pitchScaleSpr.drawString(String(abs(pitchAngle)), 101, pitchLinePos - 5);
                }
                // Draw chevron pointing down to horizon if the angle is 50 or 70 or 90
                if (pitchAngle == 50 || pitchAngle == 70 || pitchAngle == 90) {
                    pitchScaleSpr.setSwapBytes(true);
                    // pitchScaleSpr.pushImage(23, pitchLinePos - 20, 76, 36, chevron_down);
                    chevronDownSpr.pushToSprite(&pitchScaleSpr, 23, pitchLinePos - 20, TFT_BLACK);
                }
                // Draw chevron pointing up to horizon if the angle is -50 or -70 or -90
                else if (pitchAngle == -50 || pitchAngle == -70 || pitchAngle == -90) {
                    pitchScaleSpr.setSwapBytes(true);
                    chevronUpSpr.pushToSprite(&pitchScaleSpr, 23, pitchLinePos - 20, TFT_BLACK);
                    // pitchScaleSpr.pushImage(23, pitchLinePos - 20, 76, 36, chevron_up);
                }
            }
        }
    }

    void drawBall()
    {
        float ballPos = 0;

        ballSpr.createSprite(ballWidth, ballHeight);
        ballSpr.fillSprite(TFT_BLACK);
        ballPos = scaleValue(slipAngle, -8, 8, 0, 79); // scale the angles to the number of pixels
        ballSpr.pushImage(0, 0, ballWidth, ballHeight, ball);
        ballSpr.setSwapBytes(false);
        ballSpr.pushToSprite(&slipIndicatorSpr, ballPos, 0, TFT_BLACK);
    }

    // Scale function
    float scaleValue(float x, float in_min, float in_max, float out_min, float out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    // void pushUpdate(bool sel)
    // {
    //   LOCK_SPRITE(sel);
    //   pushToTFT(sel);
    //   UNLOCK_SPRITE(sel);
    // }

    // void drawUpdate(bool sel)
    // {
    //   LOCK_SPRITE(sel);
    //   drawSprites(sel);
    //   UNLOCK_SPRITE(sel);
    // }

    // void drawSprites(uint_fast8_t sel)
    // {
    //   if (sel == 0)
    //   {
    //     drawAll();
    //   }
    //   else
    //   {
    //     drawSpeedIndicator();
    //     drawAltitudeIndicator();
    //   }
    // }

    // void pushToTFT(uint_fast8_t sel)
    // {
    //   if (sel == 0)
    //   {
    //     // AttitudeIndBackSpr.pushSprite(120, 0, TFT_BLACK);
    //     tft.setViewport(120, 0, 240, 320);
    //     AttitudeIndSpr.pushRotated(newRoll, TFT_BLACK);
    //     // AttitudeIndBackSpr.pushSprite(120, 0, TFT_BLACK);
    //     pitchScaleSpr.fillSprite(TFT_BLACK);
    //     AttitudeIndSpr.fillScreen(TFT_BLACK);
    //     // AttitudeIndBackSpr.fillSprite(TFT_BLACK);
    //   }
    //   else
    //   {
    //     // AttitudeIndBackSpr.pushSprite(120, 0, TFT_BLACK);
    //     tft.setViewport(0, 0, 480, 320);
    //     SpeedIndicatorSpr.pushSprite(0, 0);
    //     headingBoxSpr.fillSprite(TFT_BLACK);
    //     SpeedIndicatorSpr.fillSprite(TFT_BLACK);
    //     AltitudeIndSpr.pushSprite(360, 0);
    //     AltitudeIndSpr.fillSprite(TFT_BLACK);
    //     baroBoxSpr.fillSprite(TFT_BLACK);
    //   }
    // }

}