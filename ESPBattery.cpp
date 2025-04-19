/////////////////////////////////////////////////////////////////
/*
  Arduino Library to calculate the ESP8266 (Feather Huzzah) LiPo battery level.
  Created by Lennart Hennigs.
*/
/////////////////////////////////////////////////////////////////

#include "ESPBattery.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

//

adc_oneshot_unit_handle_t adc1_handle = nullptr;
adc_cali_handle_t adc_cali_channel_handle = nullptr;

void InitADC() {
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT, // default width is max supported width
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &config));

    // Initialize 
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_channel_handle));

}

/////////////////////////////////////////////////////////////////

ESPBattery::ESPBattery() {
  pin = UNDEFINED_PIN;
}

/////////////////////////////////////////////////////////////////

ESPBattery::ESPBattery(byte analog_pin, int polling_interval_ms, int min_lvl, int max_lvl) {
  begin(analog_pin, polling_interval_ms, min_lvl, max_lvl);
}

/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////

void ESPBattery::begin(byte analog_pin, int polling_interval_ms, int min_lvl, int max_lvl) {
  pin = analog_pin;
  interval = polling_interval_ms;
  min_level = min_lvl;  
  max_level = max_lvl;

  InitADC();
  
  _readData();
}

/////////////////////////////////////////////////////////////////

int ESPBattery::getRaw() const {
  int raw = 0;
  ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &raw));
  return raw;
}

/////////////////////////////////////////////////////////////////

void ESPBattery::_readData() {
  if (pin != UNDEFINED_PIN) {
    last_read = millis();
    level = getRaw();

      // Apply calibration to value
    int voltage = 0;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_channel_handle, level, &voltage));
    voltage=voltage<<1;
    
    percentage = map(level, min_level, max_level, 0, 100);

    if (percentage < 0) {
      percentage = 0;  
    } else if (percentage > 100) {
      percentage = 100;
    }
    getState();
  }
}

/////////////////////////////////////////////////////////////////

void ESPBattery::loop() {
  if (millis() > last_read + interval) {
    _readData();
  }
}

/////////////////////////////////////////////////////////////////

float ESPBattery::getVoltage() const {
  return v;
}

/////////////////////////////////////////////////////////////////

int ESPBattery::getPercentage() const {
  return percentage;
}

/////////////////////////////////////////////////////////////////

int ESPBattery::getLevel() const {
  return level;
}

/////////////////////////////////////////////////////////////////

int ESPBattery::getState() {
  last_state = state;
  if (level >= ESPBATTERY_CHARGING) {
    state = ESPBATTERY_CHARGING;
  } else if (level >= ESPBATTERY_FULL) {
    state = ESPBATTERY_FULL;        
  } else if (level <= ESPBATTERY_CRITICAL) {
    state = ESPBATTERY_CRITICAL;        
  } else if (level <= ESPBATTERY_LOW) {
    state = ESPBATTERY_LOW;        
  } else {
    state = ESPBATTERY_OK;
  }
      
  if (state != last_state) {
    if (changed_cb != NULL) changed_cb (*this);    
      switch (state) {
        case ESPBATTERY_CHARGING:
          if (charging_cb != NULL) charging_cb (*this);    
          break;
        case ESPBATTERY_CRITICAL:
          if (critical_cb != NULL) critical_cb (*this);    
          break;
        case ESPBATTERY_LOW:
          if (low_cb != NULL) low_cb (*this);    
          break;
    }   
  }
  return state;
}

/////////////////////////////////////////////////////////////////

String ESPBattery::stateToString(int state) {
	switch (state) {
		case ESPBATTERY_OK:
			return "OK";
		case ESPBATTERY_FULL:
			return "FULL";
		case ESPBATTERY_CHARGING:
			return "CHARGING";
		case ESPBATTERY_CRITICAL:
			return "CRITICAL";
		case ESPBATTERY_LOW:
			return "LOW";
	}
  return "UNK";
}

/////////////////////////////////////////////////////////////////

int ESPBattery::getPreviousState() const {
    return last_state;
}

/////////////////////////////////////////////////////////////////

void ESPBattery::setLevelChangedHandler(CallbackFunction f) {
    changed_cb = f;
}

/////////////////////////////////////////////////////////////////

void ESPBattery::setLevelLowHandler(CallbackFunction f) {
    low_cb = f;
}

/////////////////////////////////////////////////////////////////

void ESPBattery::setLevelCriticalHandler(CallbackFunction f) {
    critical_cb = f;
}

/////////////////////////////////////////////////////////////////

void ESPBattery::setLevelChargingHandler(CallbackFunction f) {
    charging_cb = f;    
}

/////////////////////////////////////////////////////////////////
