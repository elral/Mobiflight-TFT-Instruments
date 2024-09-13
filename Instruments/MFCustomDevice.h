#pragma once

#include <Arduino.h>
#ifdef USE_STANDBY_ATTITUDE_MODULE
#include "StandbyAttitudeModule/StandbyAttitudeModule.h"
#endif
#ifdef USE_AIRSPEED_INDICATOR
#include "AirspeedIndicator/Airspeed_Indicator.h"
#endif
#ifdef USE_ATTITUDE_INDICATOR
#include "AttitudeIndicator/Attitude_Indicator.h"
#endif
#ifdef USE_TURNCOORDINATOR
#include "TurnCoordinator/Turn_Coordinator.h"
#endif
#ifdef USE_ALTIMETER
#include "Altimeter/Altimeter.h"
#endif
#ifdef USE_VERTICAL_SPEED_INDICATOR
#include "VerticalSpeedIndicator/Vertical_Speed_Indicator.h"
#endif
#ifdef USE_HEADING_INDICATOR
#include "HeadingIndicator/Heading_Indicator.h"
#endif

#define GFXFF 1
#define GLCD  0
#define FONT2 2
#define FONT4 4
#define FONT6 6
#define FONT7 7
#define FONT8 8

// only one entry required if you have only one custom device
enum {
    STANDBY_ATTITUDE_MONITOR = 1,
    AIRSPEED_INDICATOR,
    ATTITUDE_INDICATOR,
    TURN_COORDINATOR,
    ALTIMETER,
    VERTICAL_SPEED_INDICATOR,
    HEADING_INDICATOR,
    LAST_INSTRUMENT // MUST be always at last position!!
};

class MFCustomDevice
{
public:
    MFCustomDevice();
    void attach(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig, bool configFromFlash = false);
    void detach();
    void update();
    void set(int16_t messageID, char *setPoint);

private:
    bool     getStringFromMem(uint16_t addreeprom, char *buffer, bool configFromFlash);
    bool     _initialized = false;
    uint8_t  _customType  = 0;
    uint32_t demoMillis   = millis();
    uint16_t loopCounter  = 0;
    uint32_t startMillis  = millis();
    uint16_t interval     = 10;
    String   fps          = "xx.xx fps";
    int16_t  roll = 0, pitch = 0; // only for Demo required, should be deleted for production
    bool     roll_up_down = true; // only for Demo required, should be deleted for production
    void     demo();              // only for Demo required, should be deleted for production

    void calcFPS();
};
