#ifndef HA_MQTT_SWITCH_H
#define HA_MQTT_SWITCH_H

#include "ha/component.h"

//https://www.home-assistant.io/integrations/switch.mqtt/

struct HASwitchConfig {
  const HAComponentConfig base;
  const std::optional<const char*> commandTopic;
  const std::optional<const char*> payloadOn;
  const std::optional<const char*> payloadOff;
  const std::optional<const char*> stateTopic;
  const std::optional<const char*> stateOn;
  const std::optional<const char*> stateOff;
  const std::optional<bool> optimistic;

  void serialize(const JsonObject& json) const;
};

typedef std::function<void(bool command)> SwitchCommandListener;

class HASwitch : public HAComponent {
	const HASwitchConfig config;
	SwitchCommandListener command_listener;
	bool state;
public:
	HASwitch(const HASwitchConfig& config, SwitchCommandListener listener, bool defaultState = false)
		:HAComponent(config.base), config(config), command_listener(listener), state(defaultState) { };
	[[nodiscard]] bool getState() const { return state; };
	void setState(bool newState);
	[[nodiscard]] const char* getType() const override { return "switch"; };
	void serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const override;
	void handleMessage(const Message& message) override;
	const char* getSubscribeTopic() override;
};

#endif//HA_MQTT_SWITCH_H
