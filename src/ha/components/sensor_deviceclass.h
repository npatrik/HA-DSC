#ifndef HA_MQTT_SENSOR_DEVICECLASS_H
#define HA_MQTT_SENSOR_DEVICECLASS_H

#include "ha/utils.h"

enum class HASensor_DeviceClass {
  None,
  battery,
  current,
  energy,
  humidity,
  illuminance,
  signal_strength,
  temperature,
  power,
  power_factor,
  pressure,
  timestamp,
  voltage,
  carbon_monoxide,
  carbon_dioxide
};

constexpr const char* HASensor_DCName[] {
	toSTR(None),
	toSTR(battery),
	toSTR(current),
	toSTR(energy),
	toSTR(humidity),
	toSTR(illuminance),
	toSTR(signal_strength),
	toSTR(temperature),
	toSTR(power),
	toSTR(power_factor),
	toSTR(pressure),
	toSTR(timestamp),
	toSTR(voltage),
	toSTR(carbon_monoxide),
	toSTR(carbon_dioxide)
};

#endif //HA_MQTT_SENSOR_DEVICECLASS_H
