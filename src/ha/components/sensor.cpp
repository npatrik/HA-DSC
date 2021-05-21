#include "sensor.h"

void HASensorConfig::serialize(const JsonObject& json) const {
	json["stat_t"] = stateTopic;
	jsonFromOptional(json, "unit_of_meas", unit_of_measurement);
	jsonFromOptional(json, "frc_upd", false, forceUpdate);
	jsonFromOptional(json, "exp_aft", 0, expireAfter);
	if (deviceClass != HASensor_DeviceClass::None)
		json["dev_cla"] = HASensor_DCName[static_cast<int>(deviceClass)];
}
