#ifndef HA_MQTT_ALARM_STATE_H
#define HA_MQTT_ALARM_STATE_H

#include "ha/utils.h"

enum HAAlarm_State {
  disarmed,
  armed_home,
  armed_away,
  armed_night,
  armed_custom_bypass,
  pending,
  triggered,
  arming,
  disarming,
};

constexpr const char* HAAlarm_StateName[] {
	toSTR(disarmed),
	toSTR(armed_home),
	toSTR(armed_away),
	toSTR(armed_night),
	toSTR(armed_custom_bypass),
	toSTR(pending),
	toSTR(triggered),
	toSTR(arming),
	toSTR(disarming)
};

#endif //HA_MQTT_ALARM_STATE_H
