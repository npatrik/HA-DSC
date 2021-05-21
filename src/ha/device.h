#ifndef HA_MQTT_DEVICE_H
#define HA_MQTT_DEVICE_H

#include <forward_list>
#include "availability.h"

struct HADevice {
  const char* name; //should be unique. also used as mqtt client id
  const std::optional<const char*> manufacturer;
  const std::optional<const char*> model;
  std::forward_list<String> identifiers;
  const std::optional<const char*> sw_version;
  const std::optional<const char*> via_device;
  const HAAvailability availability = {
	  .topic = "/status"
  };
  std::forward_list<Connection> connections;

  void serialize(const JsonObject& json) const;
};

#endif //HA_MQTT_DEVICE_H
