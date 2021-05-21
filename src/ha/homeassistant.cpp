#include <include/uuid.h>
#include "homeassistant.h"
#include "utils.h"

HomeAssistant::HomeAssistant(Client& netClient, const MQTTConfig& mqttConfig, HAConfig config, HADevice device)
	:netClient(netClient), mqttConfig(mqttConfig), config(std::move(config)), device(std::move(device)) {
	buildSTR(availabilityTopic, { config.discoveryPrefix, config.availability.topic });
	buildSTR(deviceAvailabilityTopic, { config.discoveryPrefix, "/device/", device.name, device.availability.topic });
}

void HomeAssistant::setup(std::forward_list<HAComponent*>& components_list, const String mac, const String serial) {
	components = components_list;

	device.identifiers.push_front(String(serial));
	device.connections.push_front({ "mac", String(mac) });

	//UUID Generator
	uuids::uuid_name_generator uuid_gen(uuids::uuid_namespace_oid);

	//MQTT Config
	mqtt.begin(mqttConfig.ip, mqttConfig.port, netClient);
	//mqtt.setCleanSession(true); //default
	mqtt.setWill(deviceAvailabilityTopic.c_str(),
		device.availability.payload_not_available.value_or(HA_DEFAULT_OFFLINE), true, QoS1);

	if (mqttConnect()) mqttPreviousTime = millis();
	else mqttPreviousTime = 0;

	//HA availability
	mqtt.subscribe(availabilityTopic);

	//Configure components
	for (const auto& component : components_list) {
		if (component == nullptr) continue;
		String uid = nullstr;
		buildSTR(uid, { component->componentConfig.name, component->getType() });
		component->componentConfig.unique_id = String(to_string(uuid_gen(uid.c_str())).c_str());

		auto cmdtopic = component->getSubscribeTopic();
		if (cmdtopic != nullptr) {
			auto cmdtopic2 = String(cmdtopic);
			String topic = nullstr;
			buildSTR(topic,
				{ config.discoveryPrefix, "/", component->getType(), "/", component->componentConfig.unique_id.c_str(),
				  "/", device.name, "/", cmdtopic2.substring(2).c_str() });
			mqtt.subscribe(topic);
		}

		component->componentConfig.publish_callback = [this, component](const char* topic, const char* payload) {
		  auto topic2 = String(topic);
		  String ptopic = nullstr;
		  buildSTR(ptopic, { config.discoveryPrefix, "/", component->getType(), "/", component->componentConfig.unique_id.c_str(),
							 "/", device.name, "/", topic2.substring(2).c_str() });
		  mqtt.publish(ptopic, payload, true, QoS1);
		};
	}
}

void HomeAssistant::loop() {
	if (!mqtt.connected()) {
		unsigned long mqttCurrentTime = millis();
		if (mqttCurrentTime - mqttPreviousTime > 5000) {
			mqttPreviousTime = mqttCurrentTime;
			if (mqttConnect()) {
				Serial.println(F("MQTT disconnected, successfully reconnected."));
				mqttPreviousTime = 0;
			} else Serial.println(F("MQTT disconnected, failed to reconnect."));
		}
	} else mqtt.loop();

	//Process message queue
	while (!messageQueue.empty()) {
		Message msg = messageQueue.front();
		messageQueue.pop();

		//HA Online
		if (msg.topic == availabilityTopic) {
			if (msg.payload == config.availability.payload_available.value_or(HA_DEFAULT_ONLINE)) {
				announceComponents();
				return;
			}
		}

		for (const auto& component : components) {
			if (component == nullptr) continue;
			component->handleMessage(msg);
		}
	}
}
void HomeAssistant::announceComponents() {
	HAAvailability realDevAvty = {
		.topic = deviceAvailabilityTopic.c_str(),
		.payload_available = device.availability.payload_available,
		.payload_not_available = device.availability.payload_not_available
	};

	for (const auto& component : components) {
		if (component == nullptr) continue;

		StaticJsonDocument<HA_COMPONENT_CONFIG_JSON_BUFFER_SIZE> json;
		auto obj = json.to<JsonObject>();

		//Get component config
		component->serializeConfig(obj, realDevAvty);
		device.serialize(obj);

		//Create base topic
		String topic = nullstr;
		buildSTR(topic,
			{ config.discoveryPrefix, "/", component->getType(), "/", component->componentConfig.unique_id.c_str(), "/", device.name });
		const char* topicstr = topic.c_str();
		obj["~"] = topicstr;

		//Create payload - serialize component config
		String payload = nullstr;
		serializeJson(json, payload);

		//Create topic
		String cfgtopic = nullstr;
		buildSTR(cfgtopic, { topicstr, HA_DEFAULT_DISCOVERY_POSTFIX });

		if (json.memoryUsage() > HA_COMPONENT_CONFIG_JSON_BUFFER_SIZE) {
			Serial.println(F("JSON OVERFLOW"));
		}

		//Publish
		mqtt.publish(cfgtopic, payload, true, QoS1);
	}
}

bool HomeAssistant::mqttConnect() {
	if (mqtt.connected()) return true;
	Serial.print(F("MQTT connecting to: "));
	Serial.println(mqttConfig.ip);
	mqtt.onMessage([this](const String& topic, const String& payload) {
	  messageQueue.emplace(topic, payload);
	});
	if (mqtt.connect(device.name)) {
		Serial.println(F("MQTT connected."));
		//Birth message
		//bool result =
		mqtt.publish(deviceAvailabilityTopic, device.availability.payload_available.value_or(HA_DEFAULT_ONLINE),
			true, QoS1);
		//if (result) Serial.println(F("Birth message sent.")); //DEBUG
	} else {
		Serial.print(F("MQTT connection error: "));
		Serial.println(mqtt.lastError());
	}
	return mqtt.connected();
}
