#pragma once

// Define your input custom devices and uncomment -DHAS_CONFIG_IN_FLASH
// in your MFCustomDevice_platformio.ini
const char CustomDeviceConfig[] PROGMEM =
    {
        //"17.Altimeter.34..Altimeter:1.1.Button:"
        //"17.Airspeed_Indicator.34..AirSpeed:"
        "17.StandbyAttitudeModule.34..Stdby Att Mod:"
    };
