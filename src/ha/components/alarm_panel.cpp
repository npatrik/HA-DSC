#include "alarm_panel.h"

void HAAlarmPanelConfig::serialize(const JsonObject& json) const {
	json["stat_t"] = stateTopic;
	json["cmd_t"] = commandTopic;
	jsonFromOptional(json, "pl_disarm", HA_DEFAULT_DISARM, payload_disarm);
	jsonFromOptional(json, "pl_arm_away", HA_DEFAULT_ARM_AWAY, payload_arm_away);
	jsonFromOptional(json, "pl_arm_home", HA_DEFAULT_ARM_HOME, payload_arm_home);
	jsonFromOptional(json, "pl_arm_nite", HA_DEFAULT_ARM_NIGHT, payload_arm_night);
	jsonFromOptional(json, "pl_arm_custom_b", HA_DEFAULT_ARM_CUSTOM_BYPASS, payload_arm_custom_bypass);
	jsonFromOptional(json, "cod_arm_req", true, code_arm_required);
	jsonFromOptional(json, "cod_dis_req", true, code_disarm_required);
	jsonFromOptional(json, "code", code);
}

void HAAlarmPanel::serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const {
	HAComponent::serializeConfig(json, deviceAvailability);
	config.serialize(json);
}

void HAAlarmPanel::setState(HAAlarm_State newState) {
	state = newState;
	componentConfig.publish_callback(config.stateTopic, HAAlarm_StateName[newState]);
}

void HAAlarmPanel::handleMessage(const Message& message) {
	HAComponent::handleMessage(message);

	if (message.topic.indexOf(config.base.unique_id) < 0) return;
	if (message.payload == config.payload_disarm.value_or(HA_DEFAULT_DISARM))
		command_listener(disarm);
	else if (message.payload == config.payload_arm_home.value_or(HA_DEFAULT_ARM_HOME))
		command_listener(arm_home);
	else if (message.payload == config.payload_arm_away.value_or(HA_DEFAULT_ARM_AWAY))
		command_listener(arm_away);
	else if (message.payload == config.payload_arm_night.value_or(HA_DEFAULT_ARM_NIGHT))
		command_listener(arm_night);
	else if (message.payload == config.payload_arm_custom_bypass.value_or(HA_DEFAULT_ARM_CUSTOM_BYPASS))
		command_listener(arm_custom_bypass);
}
const char* HAAlarmPanel::getSubscribeTopic() {
	return config.commandTopic;
}
