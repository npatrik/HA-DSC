#ifndef HA_MQTT_BIN_SENSOR_DEVICECLASS_H
#define HA_MQTT_BIN_SENSOR_DEVICECLASS_H

#include "ha/utils.h"

enum class HABinarySensor_DeviceClass {
  None,
  battery, //on means low, off means normal
  battery_charging, //on means charging, off means not charging
  cold, //on means cold, off means normal
  connectivity, //on means connected, off means disconnected
  door, //on means open, off means closed
  garage_door, //on means open, off means closed
  gas, //on means gas detected, off means no gas (clear)
  heat, //on means hot, off means normal
  light, //on means light detected, off means no light
  lock, //on means open (unlocked), off means closed (locked)
  moisture, //on means moisture detected (wet), off means no moisture (dry)
  motion, //on means motion detected, off means no motion (clear)
  moving, //on means moving, off means not moving (stopped)
  occupancy, //on means occupied, off means not occupied (clear)
  opening, //on means open, off means closed
  plug, //on means device is plugged in, off means device is unplugged
  power, //on means power detected, off means no power
  presence, //on means home, off means away
  problem, //on means problem detected, off means no problem (OK)
  safety, //on means unsafe, off means safe
  smoke, //on means smoke detected, off means no smoke (clear)
  sound, //on means sound detected, off means no sound (clear)
  vibration, //on means vibration detected, off means no vibration (clear)
  window, //on means open, off means closed
};

constexpr const char* HABinarySensor_DCName[] {
	toSTR(None),
	toSTR(battery),
	toSTR(battery_charging),
	toSTR(cold),
	toSTR(connectivity),
	toSTR(door),
	toSTR(garage_door),
	toSTR(gas),
	toSTR(heat),
	toSTR(light),
	toSTR(lock),
	toSTR(moisture),
	toSTR(motion),
	toSTR(moving),
	toSTR(occupancy),
	toSTR(opening),
	toSTR(plug),
	toSTR(power),
	toSTR(presence),
	toSTR(problem),
	toSTR(safety),
	toSTR(smoke),
	toSTR(sound),
	toSTR(vibration),
	toSTR(window)
};

#endif //HA_MQTT_BIN_SENSOR_DEVICECLASS_H
