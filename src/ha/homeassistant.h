#ifndef HA_MQTT_HOMEASSISTANT_H
#define HA_MQTT_HOMEASSISTANT_H

#include <Client.h>
#include <MQTTClient.h>
#include <queue>
#include "component.h"
#include "device.h"

#define SW_VERSION "HA_MQTT-1.0.0"
#define HA_COMPONENT_CONFIG_JSON_BUFFER_SIZE 500

struct HAConfig {
  const char* discoveryPrefix = "homeassistant";
  const HAAvailability availability = {
	  .topic = "/status",
  };
};

class HomeAssistant {
	Client& netClient;
	MQTTClient mqtt = MQTTClient(1000);
	const MQTTConfig mqttConfig;
	const HAConfig config;
	HADevice device;
	std::forward_list<HAComponent*> components = {};

	String availabilityTopic = nullstr;
	String deviceAvailabilityTopic = nullstr;

	std::queue<Message> messageQueue = {};

	void announceComponents();
	bool mqttConnect();
	unsigned long mqttPreviousTime = 0;
public:
	HomeAssistant(Client& netClient, const MQTTConfig& mqttConfig, HAConfig config, HADevice device);
	void setup(std::forward_list<HAComponent*>& components_list, String mac, String serial);
	void loop();
};

#endif//HA_MQTT_HOMEASSISTANT_H
