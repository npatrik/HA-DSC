#ifndef HA_MQTT_SENSOR_H
#define HA_MQTT_SENSOR_H

#include "ha/component.h"
#include "sensor_deviceclass.h"

//https://www.home-assistant.io/integrations/sensor.mqtt/

struct HASensorConfig {
  const HAComponentConfig base;
  const char* stateTopic = "~/state";
  const std::optional<const char*> unit_of_measurement;
  const std::optional<bool> forceUpdate;
  const std::optional<int> expireAfter;
  const HASensor_DeviceClass deviceClass;

  void serialize(const JsonObject& json) const;
};

template<class T>
class HASensor : public HAComponent {
	T state;
	HASensorConfig config;
public:
	explicit HASensor(const HASensorConfig& config, T defaultState)
		:HAComponent(config.base), state(defaultState), config(config) { };
	T getState() const { return state; }
	void setState(T newState);
	[[nodiscard]] const char* getType() const override;
	void serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const override;
};

template<class T>
const char* HASensor<T>::getType() const {
	return "sensor";
}

template<class T>
void HASensor<T>::serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const {
	HAComponent::serializeConfig(json, deviceAvailability);
	config.serialize(json);
}

template<class T>
void HASensor<T>::setState(T newState) {
	state = newState;
	componentConfig.publish_callback(config.stateTopic, newState);
}

#endif //HA_MQTT_SENSOR_H
