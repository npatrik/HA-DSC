#include "component.h"
#include "utils.h"

void HAComponentConfig::serialize(const JsonObject& json, const HAAvailability& deviceAvailability) const {
	json["name"] = name;
	json["uniq_id"] = unique_id.c_str();

	JsonArray avtyarr = json.createNestedArray("avty");

	//Component availability
	JsonObject avty = avtyarr.createNestedObject();
	availability.serialize(avty);

	//Device availability
	JsonObject dev_avty = avtyarr.createNestedObject();
	deviceAvailability.serialize(dev_avty);

	//Availability mode
	if (availability_mode.has_value())
		serializeAvailabilityMode(json, availability_mode.value());

	jsonFromOptional(json, "qos", QoS0, qos);
	jsonFromOptional(json, "ret", false, retain);
}

void HAComponent::setAvailable(bool is_available) {
	available = is_available;
	if (is_available) {
		componentConfig.publish_callback(componentConfig.availability.topic, componentConfig.availability.payload_available.value_or(HA_DEFAULT_ONLINE));
	} else {
		componentConfig.publish_callback(componentConfig.availability.topic, componentConfig.availability.payload_not_available.value_or(HA_DEFAULT_OFFLINE));
	}
}

void HAComponent::serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const {
	componentConfig.serialize(json, deviceAvailability);
}
