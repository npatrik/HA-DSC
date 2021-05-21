#ifndef HA_MQTT_COMPONENT_H
#define HA_MQTT_COMPONENT_H

#include "availability.h"

typedef std::function<void(const char* topic, const char* payload)> PublishCallback;

struct HAComponentConfig {
  const char* name;
  String unique_id;
  const HAAvailability availability = {
	  .topic = "~/status"
  };
  const std::optional<bool> retain; //for msgs sent by HA
  const std::optional<QoS> qos; //for msgs sent by HA
  const std::optional<HAAvailabilityMode> availability_mode;
  PublishCallback publish_callback = [](const char* topic, const char* payload){};

  void serialize(const JsonObject& json, const HAAvailability& deviceAvailability) const;
};

class HAComponent {
protected:
	bool available = true;
public:
	HAComponentConfig componentConfig;

	explicit HAComponent(HAComponentConfig config)
		:componentConfig(std::move(config)) { };
	virtual void setAvailable(bool is_available);
	virtual ~HAComponent() = default;
	[[nodiscard]] virtual const char* getType() const = 0;
	virtual void serializeConfig(const JsonObject& json, const HAAvailability& deviceAvailability) const;
	virtual void handleMessage(const Message& message){};
	virtual const char* getSubscribeTopic(){ return nullptr; };
};

#endif //HA_MQTT_COMPONENT_H
