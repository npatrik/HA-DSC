#ifndef HA_MQTT_AVAILABILITY_H
#define HA_MQTT_AVAILABILITY_H

#include <ArduinoJson.h>
#include "types.h"

enum HAAvailabilityMode : byte {
  any,
  latest,
  all
};

void serializeAvailabilityMode(const JsonObject& json, HAAvailabilityMode mode);

struct HAAvailability {
  const char* topic = "~/status";
  const std::optional<const char*> payload_available;
  const std::optional<const char*> payload_not_available;

  void serialize(const JsonObject& json) const;
};

#endif //HA_MQTT_AVAILABILITY_H
