#ifndef DSC_H
#define DSC_H

#include <dscKeybusInterface.h>
#include <ha/homeassistant.h>
#include <ha/components/bin_sensor_deviceclass.h>
#include <ha/components/bin_sensor.h>
#include <ha/components/sensor.h>
#include <ha/components/alarm_panel.h>

struct DSCZoneConfig {
  const char* name;
  const HABinarySensor_DeviceClass type;
};

struct DSCPartitionConfig {
  const char* name;
  DSCZoneConfig zones[dscZones] = {};
};

struct DSCWiring {
  byte clockPin;
  byte readPin;
  byte writePin = 255; //default: disabled
};

struct DSCPartition {
  HAAlarmPanel* panel;
  HASensor<const char*>* status_sensor;
  HABinarySensor* zones[dscZones] = {};
  DSCPartitionConfig config = {};
};

struct DSCConfig {
  const DSCWiring wiring;
  const DSCPartitionConfig partitions[dscPartitions]= {};
  const char* accessCode;
};

class DSC {
	HomeAssistant ha;
	dscKeybusInterface dsc;
	DSCConfig config;

	DSCPartition* partitions[dscPartitions] = {};
	HABinarySensor trouble_status;
	HABinarySensor dsc_status;

public:
	DSC(const DSCConfig& config, Client& netClient, const MQTTConfig& mqttConfig, const HAConfig& haConfig, const HADevice& haDevice);
	void setup(const String& mac, const String& serial);
	void loop();
	virtual ~DSC();
	void setPartitionStatus(byte partition);
	void onAlarmCommand(byte partition, HAAlarm_Command command);
};

#endif //DSC_H
