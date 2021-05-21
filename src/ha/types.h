#ifndef HA_MQTT_TYPES_H
#define HA_MQTT_TYPES_H

#include <WString.h>
#include <optional>

#define nullstr String((char*)0)

//HA defaults
#define HA_DEFAULT_ON "ON"
#define HA_DEFAULT_OFF "OFF"
#define HA_DEFAULT_ONLINE "online"
#define HA_DEFAULT_OFFLINE "offline"
#define HA_DEFAULT_DISCOVERY_POSTFIX "/config" //appended to each node's topic

struct MQTTConfig {
  const char* ip;
  const int port = 1883;
  const char* username;
  const char* password;
};

struct Connection {
  const char* type;
  String identifier;
};

enum QoS : byte {
  QoS0 = 0,
  QoS1 = 1,
  QoS2 = 2
};

struct Message {
  String topic;
  String payload;
};

#endif //HA_MQTT_TYPES_H
