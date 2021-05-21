#include "bin_sensor.h"

void HABinarySensorConfig::serialize(const JsonObject& json) const {
	json["stat_t"] = stateTopic;
	jsonFromOptional(json, "stat_on", HA_DEFAULT_ON, stateOn);
	jsonFromOptional(json, "stat_off", HA_DEFAULT_OFF, stateOff);
	if (deviceClass != HABinarySensor_DeviceClass::None)
		json["dev_cla"] = HABinarySensor_DCName[static_cast<int>(deviceClass)];
}

void HABinarySensor::setState(bool newState) {
	state = newState;
	componentConfig.publish_callback(config.stateTopic, newState ? config.stateOn.value_or(HA_DEFAULT_ON) : config.stateOff.value_or(HA_DEFAULT_OFF));
}

void HABinarySensor::serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const {
	HAComponent::serializeConfig(json, deviceAvailability);
	config.serialize(json);
}
