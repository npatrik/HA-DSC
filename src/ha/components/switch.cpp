#include "switch.h"
#include "ha/utils.h"

void HASwitchConfig::serialize(const JsonObject& json) const {
	jsonFromOptional(json, "cmd_t", commandTopic, "~/set");
	jsonFromOptional(json, "pl_on", HA_DEFAULT_ON, payloadOn);
	jsonFromOptional(json, "pl_off", HA_DEFAULT_OFF, payloadOff);
	jsonFromOptional(json, "stat_t", stateTopic, "~/state");
	jsonFromOptional(json, "stat_on", payloadOn.value_or(HA_DEFAULT_ON), stateOn);
	jsonFromOptional(json, "stat_off", payloadOff.value_or(HA_DEFAULT_OFF), stateOff);
	jsonFromOptional(json, "opt", !stateTopic.has_value(), optimistic);
}

void HASwitch::serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const {
	HAComponent::serializeConfig(json, deviceAvailability);
	config.serialize(json);
}

void HASwitch::setState(bool newState) {
	state = newState;
	componentConfig.publish_callback(config.stateTopic.value_or("~/state"), newState ? config.stateOn.value_or(HA_DEFAULT_ON) : config.stateOff.value_or(HA_DEFAULT_OFF));
}

void HASwitch::handleMessage(const Message& message) {
	HAComponent::handleMessage(message);

	if (message.topic.indexOf(config.base.unique_id) < 0) return;
	if (message.payload == config.payloadOn.value_or(HA_DEFAULT_ON)){
		command_listener(true);
		setState(true);
	}else if (message.payload == config.payloadOff.value_or(HA_DEFAULT_OFF)){
		command_listener(false);
		setState(false);
	}
}

const char* HASwitch::getSubscribeTopic() {
	return config.commandTopic.value_or(HAComponent::getSubscribeTopic());
}
