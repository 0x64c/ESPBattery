/////////////////////////////////////////////////////////////////
/*
  Arduino Library to calculate the ESP8266 (Feather Huzzah) LiPo battery level.
	Created by Lennart Hennigs
*/
/////////////////////////////////////////////////////////////////

#pragma once

#ifndef ESPBattery_h
#define ESPBattery_h

/////////////////////////////////////////////////////////////////

#include "Arduino.h"

/////////////////////////////////////////////////////////////////

#define ESPBATTERY_CHARGING 840		// lower boundary
#define ESPBATTERY_FULL     645		// lower boundary
#define ESPBATTERY_OK       620		// picked value
#define ESPBATTERY_LOW      561		// upper boundary
#define ESPBATTERY_CRITICAL 526		// upper boundary

#define UNDEFINED_PIN 255

/////////////////////////////////////////////////////////////////

class ESPBattery {
  private:
    byte pin;
    unsigned int level;
    float v;
    int percentage;
    int min_level, max_level;
    int interval, last_read;
    int state, last_state;

    void _readData();

    typedef void (*CallbackFunction) (ESPBattery&);
    
    CallbackFunction changed_cb = NULL;
    CallbackFunction low_cb = NULL;
    CallbackFunction critical_cb = NULL;
    CallbackFunction charging_cb = NULL;
    
  public:
    ESPBattery();
    ESPBattery(byte analog_pin, int polling_interval_ms = 500, int min_lvl = ESPBATTERY_CRITICAL, int max_lvl = ESPBATTERY_FULL);
 
    void begin(byte analog_pin, int polling_interval_ms = 500, int min_lvl = ESPBATTERY_CRITICAL, int max_lvl = ESPBATTERY_FULL);

    float getVoltage() const;
    int getRaw() const;
    int getPercentage() const;
    int getLevel() const;
    int getState();
    int getPreviousState() const;
    String stateToString(int state);
      
    void setLevelChangedHandler(CallbackFunction f);
    void setLevelLowHandler(CallbackFunction f);
    void setLevelCriticalHandler(CallbackFunction f);    
    void setLevelChargingHandler(CallbackFunction f);    

	void loop();
};

/////////////////////////////////////////////////////////////////
#endif
/////////////////////////////////////////////////////////////////
