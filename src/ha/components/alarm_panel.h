#ifndef HA_MQTT_ALARM_PANEL_H
#define HA_MQTT_ALARM_PANEL_H

#include "ha/component.h"
#include "alarm_state.h"

//https://www.home-assistant.io/integrations/alarm_control_panel.mqtt/

#define HA_DEFAULT_DISARM "DISARM"
#define HA_DEFAULT_ARM_AWAY "ARM_AWAY"
#define HA_DEFAULT_ARM_HOME "ARM_HOME"
#define HA_DEFAULT_ARM_NIGHT "ARM_NIGHT"
#define HA_DEFAULT_ARM_CUSTOM_BYPASS "ARM_CUSTOM_BYPASS"

struct HAAlarmPanelConfig {
  const HAComponentConfig base;
  const char* stateTopic = "~/state";
  const char* commandTopic = "~/set";
  const std::optional<const char*> payload_disarm;
  const std::optional<const char*> payload_arm_home;
  const std::optional<const char*> payload_arm_away;
  const std::optional<const char*> payload_arm_night;
  const std::optional<const char*> payload_arm_custom_bypass;
  const std::optional<bool> code_arm_required;
  const std::optional<bool> code_disarm_required;
  const std::optional<const char*> code;

  void serialize(const JsonObject& json) const;
};

enum HAAlarm_Command {
  disarm,
  arm_home,
  arm_away,
  arm_night,
  arm_custom_bypass
};

typedef std::function<void(HAAlarm_Command command)> AlarmCommandListener;

class HAAlarmPanel : public HAComponent {
	HAAlarmPanelConfig config;
	HAAlarm_State state = disarmed;
	AlarmCommandListener command_listener;
public:
	HAAlarmPanel(const HAAlarmPanelConfig& config, AlarmCommandListener listener)
		:HAComponent(config.base), config(config), command_listener(listener) { }
	void handleMessage(const Message& message) override;
	[[nodiscard]] const char* getType() const override { return "alarm_control_panel"; };
	void serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const override;
	[[nodiscard]] HAAlarm_State getState() const { return state; }
	void setState(HAAlarm_State newState);
	const char* getSubscribeTopic() override;
};

#endif //HA_MQTT_ALARM_PANEL_H
