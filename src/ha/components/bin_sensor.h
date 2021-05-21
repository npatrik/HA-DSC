#ifndef HA_MQTT_BIN_SENSOR_H
#define HA_MQTT_BIN_SENSOR_H

#include "ha/component.h"
#include "bin_sensor_deviceclass.h"

//https://www.home-assistant.io/integrations/binary_sensor.mqtt/

struct HABinarySensorConfig {
  const HAComponentConfig base;
  const char* stateTopic = "~/state";
  const std::optional<const char*> stateOn;
  const std::optional<const char*> stateOff;
  const HABinarySensor_DeviceClass deviceClass = HABinarySensor_DeviceClass::None;

  void serialize(const JsonObject& json) const;
};

class HABinarySensor : public HAComponent {
	HABinarySensorConfig config;
	bool state;
public:
	explicit HABinarySensor(const HABinarySensorConfig& config, bool defaultState = false)
		:HAComponent(config.base), config(config), state(defaultState) { };
	[[nodiscard]] bool getState() const { return state; }
	void setState(bool newState);
	[[nodiscard]] const char* getType() const override { return "binary_sensor"; }
	void serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const override;
};

#endif //HA_MQTT_BIN_SENSOR_H
